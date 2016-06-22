/*
 *  Custom IPL builder and installer/flasher
 *  original code by "moonlight" @ ps2dev forums
 *
 *  slightly mofidied for better implementation into
 *  the Extended Pandora Bootmenu
 *
 */


#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <pspnand_driver.h>
#include "ipl.h"
#include "sysfunc.h"

#define printf    pspDebugScreenPrintf

int sceNandEraseBlockWithRetry(u32 ppl);
int sceNandWriteBlockWithVerify(u32 ppn, void *user, void *spare);

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);
	
	//sceKernelDelayThread(milisecs*1000);
	//sceKernelExitGame();
}


////////////////////////////////////////////////////////////////////
// Direct system calls used for PSAR decoding and PRX decryption

typedef int (*PROC_DECODE)(void* r4in, u32 r5in, int* r6Ret); // decodes in place
PROC_DECODE g_decodeProc6;
PROC_DECODE g_decodeProcE;

typedef int (*PROC_MANGLE)(void* r4, u32 r5, void* r6, u32 r7, u32 r8);
PROC_MANGLE g_mangleProc;
    // secret access to hardware decryption AKA "semaphore_2"
    // r8=opcode: 7=>block cypher/scramble (many uses),
        // 11=>SHA1, 1=>magic decode of real PRX code

typedef u32 (*PROC_DEFLATE)(void* destP, u32 cb, const void* scrP, u32* retP);
PROC_DEFLATE g_sceDeflateDecompress;
PROC_DEFLATE g_sceGzipDecompress;

int (* DecryptUpdaterModule)(void *buf, int size, int *retSize);

/* New FindProc based on tyranid's psplink code. PspPet one doesn't work
   well with 2.7X+ sysmem.prx */
u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		printf("Cannot find module %s\n", szMod);
		return 0;
	}
	
	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	//***printf("entTab %p - entLen %d\n", entTab, entLen);
	while(i < entLen)
    {
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

        if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
						return vars[count+total];					
				}
			}
		}

		i += (entry->len * 4);
	}

	printf("Funtion not found.\n");
	return 0;
}

int InitSysEntries()
{
    // dynamic/explicit lookup
    g_decodeProc6 = (PROC_DECODE)FindProc("sceMesgLed", "sceNwman_driver", 0x9555d68d);
    g_decodeProcE = (PROC_DECODE)FindProc("sceMesgLed", "sceMesgd_driver", 0x102dc8af);
    g_mangleProc = (PROC_MANGLE)FindProc("sceMemlmd", "semaphore", 0x4c537c72);
    g_sceDeflateDecompress = (PROC_DEFLATE)FindProc("sceKernelUtils", "UtilsForKernel", 0xe8db3ce6);
    g_sceGzipDecompress = (PROC_DEFLATE)FindProc("sceKernelUtils", "UtilsForKernel", 0x78934841);
	DecryptUpdaterModule = (void *)FindProc("sceMesgLed", "sceMesgLed_driver", 0x67A5ECDF);

    return (g_decodeProc6 != NULL &&
      g_decodeProcE != NULL &&
      g_mangleProc != NULL &&
      g_sceDeflateDecompress != NULL &&
      g_sceGzipDecompress != NULL && DecryptUpdaterModule != NULL);
}

////////////////////////////////////////////////////////////////////
// Hardware direct Decoding of ~PSP/PRX files (side-step the PSP firmware)

#ifdef SPECIAL_KEYS
#include "keys.c_"
#else
// use pre-calculated keys (step1 results)

static unsigned long const g_key0[] =
{
  0x7b21f3be, 0x299c5e1d, 0x1c9c5e71, 0x96cb4645, 0x3c9b1be0, 0xeb85de3d,
  0x4a7f2022, 0xc2206eaa, 0xd50b3265, 0x55770567, 0x3c080840, 0x981d55f2,
  0x5fd8f6f3, 0xee8eb0c5, 0x944d8152, 0xf8278651, 0x2705bafa, 0x8420e533,
  0x27154ae9, 0x4819aa32, 0x59a3aa40, 0x2cb3cf65, 0xf274466d, 0x3a655605,
  0x21b0f88f, 0xc5b18d26, 0x64c19051, 0xd669c94e, 0xe87035f2, 0x9d3a5909,
  0x6f4e7102, 0xdca946ce, 0x8416881b, 0xbab097a5, 0x249125c6, 0xb34c0872,
};
static unsigned long const g_key2[] =
{
  0xccfda932, 0x51c06f76, 0x046dcccf, 0x49e1821e, 0x7d3b024c, 0x9dda5865,
  0xcc8c9825, 0xd1e97db5, 0x6874d8cb, 0x3471c987, 0x72edb3fc, 0x81c8365d,
  0xe161e33a, 0xfc92db59, 0x2009b1ec, 0xb1a94ce4, 0x2f03696b, 0x87e236d8,
  0x3b2b8ce9, 0x0305e784, 0xf9710883, 0xb039db39, 0x893bea37, 0xe74d6805,
  0x2a5c38bd, 0xb08dc813, 0x15b32375, 0x46be4525, 0x0103fd90, 0xa90e87a2,
  0x52aba66a, 0x85bf7b80, 0x45e8ce63, 0x4dd716d3, 0xf5e30d2d, 0xaf3ae456,
};
static unsigned long const g_key3[] =
{
  0xa6c8f5ca, 0x6d67c080, 0x924f4d3a, 0x047ca06a, 0x08640297, 0x4fd4a758,
  0xbd685a87, 0x9b2701c2, 0x83b62a35, 0x726b533c, 0xe522fa0c, 0xc24b06b4,
  0x459d1cac, 0xa8c5417b, 0x4fea62a2, 0x0615d742, 0x30628d09, 0xc44fab14,
  0x69ff715e, 0xd2d8837d, 0xbeed0b8b, 0x1e6e57ae, 0x61e8c402, 0xbe367a06,
  0x543f2b5e, 0xdb3ec058, 0xbe852075, 0x1e7e4dcc, 0x1564ea55, 0xec7825b4,
  0xc0538cad, 0x70f72c7f, 0x49e8c3d0, 0xeda97ec5, 0xf492b0a4, 0xe05eb02a,
};
static unsigned long const g_key44[] =
{
  0xef80e005, 0x3a54689f, 0x43c99ccd, 0x1b7727be, 0x5cb80038, 0xdd2efe62,
  0xf369f92c, 0x160f94c5, 0x29560019, 0xbf3c10c5, 0xf2ce5566, 0xcea2c626,
  0xb601816f, 0x64e7481e, 0x0c34debd, 0x98f29cb0, 0x3fc504d7, 0xc8fb39f0,
  0x0221b3d8, 0x63f936a2, 0x9a3a4800, 0x6ecc32e3, 0x8e120cfd, 0xb0361623,
  0xaee1e689, 0x745502eb, 0xe4a6c61c, 0x74f23eb4, 0xd7fa5813, 0xb01916eb,
  0x12328457, 0xd2bc97d2, 0x646425d8, 0x328380a5, 0x43da8ab1, 0x4b122ac9,
};
static unsigned long const g_key20[] =
{
  0x33b50800, 0xf32f5fcd, 0x3c14881f, 0x6e8a2a95, 0x29feefd5, 0x1394eae3,
  0xbd6bd443, 0x0821c083, 0xfab379d3, 0xe613e165, 0xf5a754d3, 0x108b2952,
  0x0a4b1e15, 0x61eadeba, 0x557565df, 0x3b465301, 0xae54ecc3, 0x61423309,
  0x70c9ff19, 0x5b0ae5ec, 0x989df126, 0x9d987a5f, 0x55bc750e, 0xc66eba27,
  0x2de988e8, 0xf76600da, 0x0382dccb, 0x5569f5f2, 0x8e431262, 0x288fe3d3,
  0x656f2187, 0x37d12e9c, 0x2f539eb4, 0xa492998e, 0xed3958f7, 0x39e96523,
};
static unsigned long const g_key3A[] =
{
  0x67877069, 0x3abd5617, 0xc23ab1dc, 0xab57507d, 0x066a7f40, 0x24def9b9,
  0x06f759e4, 0xdcf524b1, 0x13793e5e, 0x0359022d, 0xaae7e1a2, 0x76b9b2fa,
  0x9a160340, 0x87822fba, 0x19e28fbb, 0x9e338a02, 0xd8007e9a, 0xea317af1,
  0x630671de, 0x0b67ca7c, 0x865192af, 0xea3c3526, 0x2b448c8e, 0x8b599254,
  0x4602e9cb, 0x4de16cda, 0xe164d5bb, 0x07ecd88e, 0x99ffe5f8, 0x768800c1,
  0x53b091ed, 0x84047434, 0xb426dbbc, 0x36f948bb, 0x46142158, 0x749bb492,
};
#endif

typedef struct
{
    u32 tag; // 4 byte value at offset 0xD0 in the PRX file
    u8* key; // "step1_result" use for XOR step
    u8 code;
    u8 codeExtra;
} TAG_INFO;

static const TAG_INFO g_tagInfo[] =
{
    // 1.x PRXs
    { 0x00000000, (u8*)g_key0, 0x42 },
    { 0x02000000, (u8*)g_key2, 0x45 },
    { 0x03000000, (u8*)g_key3, 0x46 },
#ifdef LATER
    // add keys 6 and 14 - for PSAR data
        // then we can replace "g_decodeProc6/ProcE" with our own version
#endif
    // 2.0 PRXs
    { 0x4467415d, (u8*)g_key44, 0x59, 0x59 },
    { 0x207bbf2f, (u8*)g_key20, 0x5A, 0x5A },
    { 0x3ace4dce, (u8*)g_key3A, 0x5B, 0x5B },
};

#ifdef SPECIAL_KEYS
#include "initkeys.c_"
#endif


static TAG_INFO const* GetTagInfo(u32 tagFind)
{
    int iTag;
    for (iTag = 0; iTag < sizeof(g_tagInfo)/sizeof(TAG_INFO); iTag++)
        if (g_tagInfo[iTag].tag == tagFind)
            return &g_tagInfo[iTag];
    return NULL; // not found
}


static void ExtraV2Mangle(u8* buffer1, u8 codeExtra)
{
    static u8 g_dataTmp[20+0xA0] __attribute__((aligned(0x40)));
    u8* buffer2 = g_dataTmp; // aligned

    memcpy(buffer2+20, buffer1, 0xA0);
    u32* pl2 = (u32*)buffer2;
    pl2[0] = 5;
    pl2[1] = pl2[2] = 0;
    pl2[3] = codeExtra;
    pl2[4] = 0xA0;

    int ret = (*g_mangleProc)(buffer2, 20+0xA0, buffer2, 20+0xA0, 7);
    if (ret != 0)
        printf("extra de-mangle returns %d\n", ret);
    // copy result back
    memcpy(buffer1, buffer2, 0xA0);
}

static int MyDecryptPRX(const u8* pbIn, u8* pbOut, int cbTotal, u32 tag)
{
    TAG_INFO const* pti = GetTagInfo(tag);
    if (pti == NULL)
        return -1;

    // build conversion into pbOut
    memcpy(pbOut, pbIn, cbTotal);
    memset(pbOut, 0, 0x150);
    memset(pbOut, 0x55, 0x40); // first $40 bytes ignored

    // step3 demangle in place
    u32* pl = (u32*)(pbOut+0x2C);
    pl[0] = 5; // number of ulongs in the header
    pl[1] = pl[2] = 0;
    pl[3] = pti->code; // initial seed for PRX
    pl[4] = 0x70;   // size

    // redo part of the SIG check (step2)
    u8 buffer1[0x150];
    memcpy(buffer1+0x00, pbIn+0xD0, 0x80);
    memcpy(buffer1+0x80, pbIn+0x80, 0x50);
    memcpy(buffer1+0xD0, pbIn+0x00, 0x80);
    if (pti->codeExtra != 0)
        ExtraV2Mangle(buffer1+0x10, pti->codeExtra);
    memcpy(pbOut+0x40 /* 0x2C+20 */, buffer1+0x40, 0x40);

    int ret;
    int iXOR;
    for (iXOR = 0; iXOR < 0x70; iXOR++)
        pbOut[0x40+iXOR] = pbOut[0x40+iXOR] ^ pti->key[0x14+iXOR];

    ret = (*g_mangleProc)(pbOut+0x2C, 20+0x70, pbOut+0x2C, 20+0x70, 7);
    if (ret != 0)
    {
        printf("mangle#7 returned $%x\n", ret);
        return -1;
    }

    for (iXOR = 0x6F; iXOR >= 0; iXOR--)
        pbOut[0x40+iXOR] = pbOut[0x2C+iXOR] ^ pti->key[0x20+iXOR];

    memset(pbOut+0x80, 0, 0x30); // $40 bytes kept, clean up
    pbOut[0xA0] = 1;
    // copy unscrambled parts from header
    memcpy(pbOut+0xB0, pbIn+0xB0, 0x20); // file size + lots of zeros
    memcpy(pbOut+0xD0, pbIn+0x00, 0x80); // ~PSP header

    // step4: do the actual decryption of code block
    //  point 0x40 bytes into the buffer to key info
    ret = (*g_mangleProc)(pbOut, cbTotal, pbOut+0x40, cbTotal-0x40, 0x1);
    if (ret != 0)
    {
        printf("mangle#1 returned $%x\n", ret);
        return -1;
    }

    // return cbTotal - 0x150; // rounded up size
	return *(u32*)&pbIn[0xB0]; // size of actual data (fix thanks to Vampire)
}

/***2.60-2.80 PRX Decryption routines and sigcheck generation (Dark_AleX) ***/

/* kernel modules 2.80 */
u8 keys280_0[0x10] =
{
	0xCA, 0xFB, 0xBF, 0xC7, 0x50, 0xEA, 0xB4, 0x40,
	0x8E, 0x44, 0x5C, 0x63, 0x53, 0xCE, 0x80, 0xB1
};

/* user modules 2.80 */
u8 keys280_1[0x10] =
{
	0x40, 0x9B, 0xC6, 0x9B, 0xA9, 0xFB, 0x84, 0x7F,
	0x72, 0x21, 0xD2, 0x36, 0x96, 0x55, 0x09, 0x74
};

/* vshmain executable 2.80 */
u8 keys280_2[0x10] =
{
	0x03, 0xA7, 0xCC, 0x4A, 0x5B, 0x91, 0xC2, 0x07,
	0xFF, 0xFC, 0x26, 0x25, 0x1E, 0x42, 0x4B, 0xB5
};

/* kernel modules 2.60-2.71 */
u8 keys260_0[0x10] =
{
	0xC3, 0x24, 0x89, 0xD3, 0x80, 0x87, 0xB2, 0x4E,
	0x4C, 0xD7, 0x49, 0xE4, 0x9D, 0x1D, 0x34, 0xD1

};

/* user modules 2.60-2.71 */
u8 keys260_1[0x10] =
{
	0xF3, 0xAC, 0x6E, 0x7C, 0x04, 0x0A, 0x23, 0xE7,
	0x0D, 0x33, 0xD8, 0x24, 0x73, 0x39, 0x2B, 0x4A
};

/* vshmain 2.60-2.71 */
u8 keys260_2[0x10] =
{
	0x72, 0xB4, 0x39, 0xFF, 0x34, 0x9B, 0xAE, 0x82,
	0x30, 0x34, 0x4A, 0x1D, 0xA2, 0xD8, 0xB4, 0x3C
};

/* kernel modules 3.00 */
u8 keys300_0[0x10] =
{
	0x9F, 0x67, 0x1A, 0x7A, 0x22, 0xF3, 0x59, 0x0B,
    0xAA, 0x6D, 0xA4, 0xC6, 0x8B, 0xD0, 0x03, 0x77

};

/* user modules 3.00 */
u8 keys300_1[0x10] =
{
	0x15, 0x07, 0x63, 0x26, 0xDB, 0xE2, 0x69, 0x34,
    0x56, 0x08, 0x2A, 0x93, 0x4E, 0x4B, 0x8A, 0xB2

};

/* vshmain 3.00 */
u8 keys300_2[0x10] =
{
	0x56, 0x3B, 0x69, 0xF7, 0x29, 0x88, 0x2F, 0x4C,
    0xDB, 0xD5, 0xDE, 0x80, 0xC6, 0x5C, 0xC8, 0x73

};

/* kernel modules 3.00 */
u8 keys303_0[0x10] =
{
	0x7b, 0xa1, 0xe2, 0x5a, 0x91, 0xb9, 0xd3, 0x13,
	0x77, 0x65, 0x4a, 0xb7, 0xc2, 0x8a, 0x10, 0xaf
};

/* kernel modules 3.10 */
u8 keys310_0[0x10] =
{
	0xa2, 0x41, 0xe8, 0x39, 0x66, 0x5b, 0xfa, 0xbb,
	0x1b, 0x2d, 0x6e, 0x0e, 0x33, 0xe5, 0xd7, 0x3f
};

/* user modules 3.10 */
u8 keys310_1[0x10] =
{
	0xA4, 0x60, 0x8F, 0xAB, 0xAB, 0xDE, 0xA5, 0x65,
	0x5D, 0x43, 0x3A, 0xD1, 0x5E, 0xC3, 0xFF, 0xEA
};

/* vshmain 3.10 */
u8 keys310_2[0x10] =
{
	0xE7, 0x5C, 0x85, 0x7A, 0x59, 0xB4, 0xE3, 0x1D,
	0xD0, 0x9E, 0xCE, 0xC2, 0xD6, 0xD4, 0xBD, 0x2B
};

/* reboot.bin 3.10 */
u8 keys310_3[0x10] =
{
    0x2E, 0x00, 0xF6, 0xF7, 0x52, 0xCF, 0x95, 0x5A,
    0xA1, 0x26, 0xB4, 0x84, 0x9B, 0x58, 0x76, 0x2F
};

/* kernel modules 3.30 */ 
u8 keys330_0[0x10] = 
{ 
	0x3B, 0x9B, 0x1A, 0x56, 0x21, 0x80, 0x14, 0xED,
	0x8E, 0x8B, 0x08, 0x42, 0xFA, 0x2C, 0xDC, 0x3A
};

/* user modules 3.30 */ 
u8 keys330_1[0x10] = 
{ 
    0xE8, 0xBE, 0x2F, 0x06, 0xB1, 0x05, 0x2A, 0xB9, 
    0x18, 0x18, 0x03, 0xE3, 0xEB, 0x64, 0x7D, 0x26 
}; 

/* vshmain 3.30 */ 
u8 keys330_2[0x10] = 
{ 
    0xAB, 0x82, 0x25, 0xD7, 0x43, 0x6F, 0x6C, 0xC1, 
    0x95, 0xC5, 0xF7, 0xF0, 0x63, 0x73, 0x3F, 0xE7 
}; 

/* reboot.bin 3.30 */ 
u8 keys330_3[0x10] = 
{ 
    0xA8, 0xB1, 0x47, 0x77, 0xDC, 0x49, 0x6A, 0x6F, 
    0x38, 0x4C, 0x4D, 0x96, 0xBD, 0x49, 0xEC, 0x9B 
}; 

/* stdio.prx 3.30 */
u8 keys330_4[0x10] =
{
	0xEC, 0x3B, 0xD2, 0xC0, 0xFA, 0xC1, 0xEE, 0xB9,
	0x9A, 0xBC, 0xFF, 0xA3, 0x89, 0xF2, 0x60, 0x1F
};

typedef struct
{
    u32 tag; // 4 byte value at offset 0xD0 in the PRX file
    u8  *key; // 16 bytes keys
    u8 code; // code for scramble
} TAG_INFO2;

static TAG_INFO2 g_tagInfo2[] =
{
	{ 0x4C940BF0, keys330_0, 0x43 }, 
	{ 0x457B0AF0, keys330_1, 0x5B }, 
	{ 0x38020AF0, keys330_2, 0x5A }, 
	{ 0x4C940AF0, keys330_3, 0x62 }, 
	{ 0x4C940CF0, keys330_4, 0x43 }, 

	{ 0xcfef09f0, keys310_0, 0x62 },
	{ 0x457b08f0, keys310_1, 0x5B },
	{ 0x380208F0, keys310_2, 0x5A },
	{ 0xcfef08f0, keys310_3, 0x62 },

	{ 0xCFEF07F0, keys303_0, 0x62 },
	{ 0xCFEF06F0, keys300_0, 0x62 },
	{ 0x457B06F0, keys300_1, 0x5B },
	{ 0x380206F0, keys300_2, 0x5A },
	{ 0xCFEF05F0, keys280_0, 0x62 },
	{ 0x457B05F0, keys280_1, 0x5B },
	{ 0x380205F0, keys280_2, 0x5A },
	{ 0x16D59E03, keys260_0, 0x62 },
	{ 0x76202403, keys260_1, 0x5B },
	{ 0x0F037303, keys260_2, 0x5A }
};


static TAG_INFO2 *GetTagInfo2(u32 tagFind)
{
    int iTag;

    for (iTag = 0; iTag < sizeof(g_tagInfo2) / sizeof(TAG_INFO2); iTag++)
	{
        if (g_tagInfo2[iTag].tag == tagFind)
		{
            return &g_tagInfo2[iTag];
		}
	}

	return NULL; // not found
}

int Scramble(u32 *buf, u32 size, u32 code)
{
	buf[0] = 5;
	buf[1] = buf[2] = 0;
	buf[3] = code;
	buf[4] = size;

	if (g_mangleProc(buf, size+0x14, buf, size+0x14, 7) < 0)
	{
		return -1;
	}

	return 0;
}

int DecryptPRX2(u8 *inbuf, u8 *outbuf, u32 size, u32 tag)
{
	TAG_INFO2 const* pti = GetTagInfo2(tag);

	if (!pti)
	{
		printf("--Unknown-2.80-tag %08X.\n",(int)tag);
		return -1;
	}

	int retsize = *(int *)&inbuf[0xB0];
	u8	tmp1[0x150], tmp2[0x90+0x14], tmp3[0x60+0x14];

	memset(tmp1, 0, 0x150);
	memset(tmp2, 0, 0x90+0x14);
	memset(tmp3, 0, 0x60+0x14);

	memcpy(outbuf, inbuf, size);

	if (*((u32 *)outbuf) != 0x5053507E) // "~PSP"
	{
		printf("Error: unknown signature.\n");
		return -1;
	}

	if (size < 0x160)
	{
		printf("Buffer not big enough.\n");
		return -1;
	}

	if (((u32)outbuf & 0x3F))
	{
		printf("Buffer not aligned to 64 bytes.\n");
		return -1;
	}

	if ((size - 0x150) < retsize)
	{
		printf("No enough data.\n");
		return -1;
	}

	memcpy(tmp1, outbuf, 0x150);

	int i, j;
	u8 *p = tmp2+0x14;

	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 0x10; j++)
		{
			p[(i << 4) + j] = pti->key[j]; 			
		}

		p[(i << 4)] = i;
	}	

	if (Scramble((u32 *)tmp2, 0x90, pti->code) < 0)
	{
		printf("Error in Scramble #1.\n");
		return -1;
	}

	memcpy(outbuf, tmp1+0xD0, 0x5C);
	memcpy(outbuf+0x5C, tmp1+0x140, 0x10);
	memcpy(outbuf+0x6C, tmp1+0x12C, 0x14);
	memcpy(outbuf+0x80, tmp1+0x080, 0x30);
	memcpy(outbuf+0xB0, tmp1+0x0C0, 0x10);
	memcpy(outbuf+0xC0, tmp1+0x0B0, 0x10);
	memcpy(outbuf+0xD0, tmp1+0x000, 0x80);

	memcpy(tmp3+0x14, outbuf+0x5C, 0x60);	

	if (Scramble((u32 *)tmp3, 0x60, pti->code) < 0)
	{
		printf("Error in Scramble #2.\n");
		return -1;
	}

	memcpy(outbuf+0x5C, tmp3, 0x60);
	memcpy(tmp3, outbuf+0x6C, 0x14);
	memcpy(outbuf+0x70, outbuf+0x5C, 0x10);
	memset(outbuf+0x18, 0, 0x58);
	memcpy(outbuf+0x04, outbuf, 0x04);

	*((u32 *)outbuf) = 0x014C;
	memcpy(outbuf+0x08, tmp2, 0x10);	

	/* sha-1 */
	if (g_mangleProc(outbuf, 3000000, outbuf, 3000000, 0x0B) < 0)
	{
		printf("Error in semaphore2  #1.\n");
		return -1;
	}	

	if (memcmp(outbuf, tmp3, 0x14) != 0)
	{
		printf("SHA-1 is incorrect.\n");
        sceKernelExitGame();
//		return -1;
	}

	int iXOR;

	for (iXOR = 0; iXOR < 0x40; iXOR++)
	{
		tmp3[iXOR+0x14] = outbuf[iXOR+0x80] ^ tmp2[iXOR+0x10];
	}

	if (Scramble((u32 *)tmp3, 0x40, pti->code) != 0)
	{
		printf("Error in Scramble #2.\n");
		return -1;
	}
	
	for (iXOR = 0x3F; iXOR >= 0; iXOR--)
	{
		outbuf[iXOR+0x40] = tmp3[iXOR] ^ tmp2[iXOR+0x50]; // uns 8
	}

	memset(outbuf+0x80, 0, 0x30);
	*(u32 *)&outbuf[0xA0] = 1;

	memcpy(outbuf+0xB0, outbuf+0xC0, 0x10);
	memset(outbuf+0xC0, 0, 0x10);

	// The real decryption
	if (g_mangleProc(outbuf, size, outbuf+0x40, size-0x40, 0x1) < 0)
	{
		printf("Error in semaphore2  #2.\n");
		return -1;
	}

	if (retsize < 0x150)
	{
		// Fill with 0
		memset(outbuf+retsize, 0, 0x150-retsize);		
	}

	return retsize;

}

/* Generation of sig check area: it has only be tested with 2.71-2.80 files */
/* Maybe previous firmwares updates use different keys */

u8 check_keys0[0x10] =
{
	0x71, 0xF6, 0xA8, 0x31, 0x1E, 0xE0, 0xFF, 0x1E,
	0x50, 0xBA, 0x6C, 0xD2, 0x98, 0x2D, 0xD6, 0x2D
};

u8 check_keys1[0x10] =
{
	0xAA, 0x85, 0x4D, 0xB0, 0xFF, 0xCA, 0x47, 0xEB,
	0x38, 0x7F, 0xD7, 0xE4, 0x3D, 0x62, 0xB0, 0x10
};

int Encrypt(u32 *buf, int size)
{
	buf[0] = 4;
	buf[1] = buf[2] = 0;
	buf[3] = 0x100;
	buf[4] = size;

	/* Note: this encryption returns different data in each psp,
	   But it always returns the same in a specific psp (even if it has two nands) */
	if (g_mangleProc(buf, size+0x14, buf, size+0x14, 5) < 0)
		return -1;

	return 0;
}

int GenerateSigCheck(u8 *buf)
{
	u8 enc[0xD0+0x14];
	int iXOR, res;

	memcpy(enc+0x14, buf+0x110, 0x40);
	memcpy(enc+0x14+0x40, buf+0x80, 0x90);
	
	for (iXOR = 0; iXOR < 0xD0; iXOR++)
	{
		enc[0x14+iXOR] ^= check_keys0[iXOR&0xF];
	}

	if ((res = Encrypt((u32 *)enc, 0xD0)) < 0)
	{
		printf("Encrypt failed.\n");
		return res;
	}

	for (iXOR = 0; iXOR < 0xD0; iXOR++)
	{
		enc[0x14+iXOR] ^= check_keys1[iXOR&0xF];
	}

	memcpy(buf+0x80, enc+0x14, 0xD0);
	return 0;
}

/*** End of 2.60-2.80 PRX Decryption and sigcheck ***/

////////////////////////////////////////////////////////////////////
// IPL decode logic - thanks to 'nem'
//  for more details see "http://forums.ps2dev.org/viewtopic.php?t=3573"
// part1_/part2_ and part3_ prefix different steps of decryption process

static int DecryptIPL1(const u8* pbIn, u8* pbOut, int cbIn)
{
    // 0x1000 pages
    static u8 g_dataTmp[0x1040] __attribute__((aligned(0x40)));
    int cbOut = 0;
    while (cbIn >= 0x1000)
    {
	    memcpy(g_dataTmp+0x40, pbIn, 0x1000);
        pbIn += 0x1000;
        cbIn -= 0x1000;

        int ret = (*g_mangleProc)(g_dataTmp, 0x1040, g_dataTmp+0x40, 0x500, 1);
	    if (ret != 0)
        {
	        printf("mangle#1 returned $%x\n", ret);
            break; // stop, save what we can
        }
        memcpy(pbOut, g_dataTmp, 0x1000);
        pbOut += 0x1000;
        cbOut += 0x1000;
    }
    return cbOut;
}

// commented out, 'cause their are never called....
/* static int LinearizeIPL2(const u8* pbIn, u8* pbOut, int cbIn)
{
    u32 nextAddr = 0;
    int cbOut = 0;
    while (cbIn > 0)
    {
        u32* pl = (u32*)pbIn;
        u32 addr = pl[0];
        if (addr != nextAddr && nextAddr != 0)
            return 0;   // error
        u32 count = pl[1];
        nextAddr = addr + count;
        memcpy(pbOut, pbIn+0x10, count);
        pbOut += count;
        cbOut += count;
        pbIn += 0x1000;
        cbIn -= 0x1000;
    }
    return cbOut;
}

static int DecryptIPL3(const u8* pbIn, u8* pbOut, int cbIn)
{
    // all together now (pbIn/pbOut must be aligned)
    pbIn += 0x10000;
    cbIn -= 0x10000;
	memcpy(pbOut+0x40, pbIn, cbIn);
	int ret = (*g_mangleProc)(pbOut, cbIn+0x40, pbOut+0x40, cbIn, 1);
	if (ret != 0)
    {
		printf("mangle#1 returned $%x\n", ret);
        return 0;
    }
    return *(u32*)&pbIn[0x70]; // true size
} */

////////////////////////////////////////////////////////////////////
// Decode encrypted data blocks, minimal support for PSAR files
// keys #6 (old) or #14 (new)

#define OVERHEAD    0x150 /* size of encryption block overhead */
#define SIZE_A      0x110 /* size of uncompressed file entry = 272 bytes */

static int g_bOldSchool = 0; // true=> 1.00 "Bogus" update

// for 1.50 and later, they mangled the plaintext parts of the header
static void Demangle(const u8* pIn, u8* pOut)
{
    u8 dmbuffer[20+0x130];
    memcpy(dmbuffer+20, pIn, 0x130);
    u32* pl = (u32*)dmbuffer; // first 20 bytes
    pl[0] = 5;
    pl[1] = pl[2] = 0;
    pl[3] = 0x55;
    pl[4] = 0x130;

    (*g_mangleProc)(dmbuffer, 20+0x130, dmbuffer, 20+0x130, 7);
    memcpy(pOut, dmbuffer, 0x130);
}

int DecodeBlock(const u8* pIn, int cbIn, u8* pOut)
{
    // pOut also used as temporary buffer for mangled input
    // assert((((u32)pOut) & 0x3F) == 0); // must be aligned

    memcpy(pOut, pIn, cbIn + 0x10); // copy a little more for $10 page alignment

    int ret;
    int cbOut;
    if (g_bOldSchool)
    {
        // old style - unscrambled PSAR (1.00 "BOGUS" update)
        if (*(u32*)&pOut[0xd0] != 0x06000000)
        {
            printf("Oldschool 6 failed\n");
            return -1;
        }
        ret = (*g_decodeProc6)(pOut, cbIn, &cbOut);
        if (ret != 0)
            printf("Sys_Decode6 returned $%x %d\n", ret, cbOut);
    }
    else
    {
        // new style - scrambled PSAR (works with 1.5x and 2.00)
        Demangle(pIn+0x20, pOut+0x20); // demangle the inside $130 bytes
        if (*(u32*)&pOut[0xd0] != 0x0E000000)
        {
            printf("Demangle failed\n");
            printf(" PSAR format not supported\n");
            // SaveFile("ms0:/err1.bin", pIn, cbIn);
            // SaveFile("ms0:/err2.bin", pOut, cbIn);
            return -1;
        }
        ret = (*g_decodeProcE)(pOut, cbIn, &cbOut);
        if (ret != 0)
            printf("Sys_DecodeE returned $%x %d\n", ret, cbOut);
    }
    if (ret != 0)
        return -1; // error
    return cbOut;
}

int ReadFile(const char* file, u32 offset, void *buf, u32 size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	int read;

	if (fd < 0)
		return fd;

	if (offset != 0)
	{
		sceIoLseek32(fd, offset, PSP_SEEK_SET);
	}

	read = sceIoRead(fd, buf, size);
	sceIoClose(fd);

	return read;
}

int WriteFile(const char* file, void *buf, u32 size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	int written;

	if (fd < 0)
		return fd;

	written = sceIoWrite(fd, buf, size);
	sceIoClose(fd);

	return written;
}

#define IPL_SIZE			241664
#define UPDATER_PRX_SIZE	3737904
#define IPL150_SIZE			225280

u8 nandipl[IPL_SIZE]; 

u8 md5_prx[16] = 
{
	0x9F, 0xB7, 0xC7, 0xE9, 0x96, 0x79, 0x7F, 0xF2,
	0xD7, 0xFF, 0x4D, 0x14, 0xDD, 0xF5, 0xD3, 0xF2
};

u8 *buffer; //[10*1024*1024] __attribute__((aligned(64)));
u8 *dataOut; //[3*1024*1024] __attribute__((aligned(64)));
u8 *dataOut2; //3*1024*1024] __attribute__((aligned(64)));

void BuildIpl()
{
	int size, i;
	u8  md5[16];
	
	if (ReadFile("ms0:/UPDATE.PBP", 0x3251, buffer, UPDATER_PRX_SIZE) != UPDATER_PRX_SIZE)
	{
		ErrorExit(5000, "Cannot read updater file.\n");
	}	

	sceKernelUtilsMd5Digest(buffer, UPDATER_PRX_SIZE, md5);

	if (memcmp(md5, md5_prx, 16) != 0)
	{
		ErrorExit(5000, "Incorrect or corrupt updater file.\n");
	}

	if (DecryptUpdaterModule(buffer, UPDATER_PRX_SIZE, &size) < 0)
	{
		ErrorExit(5000, "Cannot decrypt updater main module.\n");
	}

	for (i = 0; i < size-20; i++)
	{
		if (memcmp(buffer+i, "~PSP", 4) == 0)
		{
			if (strcmp((char *)buffer+i+0x0A, "IplUpdater") == 0)
			{
				size = *(int *)&buffer[i+0x2C];
				memcpy(dataOut, buffer+i, size);
				break;
			}
		}
	}

	if (i == (size-20))
	{
		ErrorExit(5000, "Curious error.\n");
	}

	if (MyDecryptPRX(dataOut, dataOut2, size, 0) < 0)
	{
		ErrorExit(5000, "Error decrypting ipl_update.prx.\n");
	}

	if (DecryptIPL1(dataOut2+0x980, dataOut, IPL150_SIZE) != IPL150_SIZE)
	{
		ErrorExit(5000, "Error decrypting ipl.\n");
	}

	u32 *code = (u32 *)(dataOut+0x10);
	code[0xE0/4] = 0x09038000; // j 0x040e0000
	code[0xE4/4] = 0; // nop

	memcpy(nandipl, custom_block, sizeof(custom_block));
	memcpy(nandipl+sizeof(custom_block), dataOut, IPL150_SIZE);
}


#define PSP_NAND_PAGE_USER_SIZE			512
#define	PSP_NAND_PAGE_SPARE_SIZE		16
#define PSP_NAND_PAGE_SPARE_SMALL_SIZE	(PSP_NAND_PAGE_SPARE_SIZE-4)
#define PSP_NAND_PAGE_TOTAL_SIZE		(PSP_NAND_PAGE_USER_SIZE+PSP_NAND_PAGE_SPARE_SIZE)
#define	PSP_NAND_PAGES_PER_BLOCK		32

#define PSP_NAND_BLOCK_USER_SIZE		(PSP_NAND_PAGE_USER_SIZE*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_BLOCK_SPARE_SIZE		(PSP_NAND_PAGE_SPARE_SIZE*PSP_NAND_PAGES_PER_BLOCK)	
#define PSP_NAND_BLOCK_SPARE_SMALL_SIZE	(PSP_NAND_PAGE_SPARE_SMALL_SIZE*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_BLOCK_TOTAL_SIZE		(PSP_NAND_PAGE_TOTAL_SIZE*PSP_NAND_PAGES_PER_BLOCK)

#define PSP_NAND_TOTAL_BLOCKS			2048
#define PSP_NAND_TOTAL_PAGES			(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_PAGES_PER_BLOCK)
#define PSP_NAND_SIZE_USER				(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_USER_SIZE)
#define PSP_NAND_SIZE_SPARE				(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_SPARE_SIZE)
#define PSP_NAND_SIZE					(PSP_NAND_TOTAL_BLOCKS*PSP_NAND_BLOCK_TOTAL_SIZE)

#define PSP_IPL_MAX_DATA_BLOCKS	0x20
#define PSP_IPL_MAX_SIZE		(PSP_NAND_BLOCK_USER_SIZE*PSP_IPL_MAX_DATA_BLOCKS)
#define PSP_IPL_SIGNATURE		0x6DC64A38

int sceNandEraseIplBlockWithRetry(u32 ppn)
{
	if (ppn >= 0x600)
	{
		ErrorExit(5000, "FATAL ERROR: ppn=0x%08X outside of IPL area.\n");
	}

	return sceNandEraseBlockWithRetry(ppn);
}

int sceNandWriteIplBlockWithVerify(u32 ppn, void *user, void *spare)
{
	if (ppn >= 0x600)
	{
		ErrorExit(5000, "FATAL ERROR: ppn=0x%08X outside of IPL area.\n");
	}

	return sceNandWriteBlockWithVerify(ppn, user, spare);
}

int pspIplClearIpl()
{
	u32 block, ppn;
	int res;

	for (block = 0; block < 0x30; block++)
	{
		ppn = block*PSP_NAND_PAGES_PER_BLOCK;

		res = sceNandEraseIplBlockWithRetry(ppn);
		if (res < 0)
		{ printf("!");
			//sceNandDoMarkAsBadBlock(ppn);			
		}
	}

	return 0;
}

int pspIplSetIpl(u8 *buf, u32 size)
{
	int i, res, nblocks, written;
	u32 block, ppn;
	u16 blocktable[32];
	u8  user[PSP_NAND_BLOCK_USER_SIZE], spare[PSP_NAND_BLOCK_SPARE_SMALL_SIZE];
	u32 *p;
	
	nblocks = (size + (PSP_NAND_BLOCK_USER_SIZE-1)) / PSP_NAND_BLOCK_USER_SIZE;

	if (nblocks > PSP_IPL_MAX_DATA_BLOCKS)
	{
		printf("IPL too big (%d). Max size is %d.\n", (int)size, PSP_IPL_MAX_SIZE);
		return -1;
	}

	// Init spare data
	for (i = 0, p = (u32 *)spare; i < PSP_NAND_PAGES_PER_BLOCK; i++, p += (PSP_NAND_PAGE_SPARE_SMALL_SIZE/4))
	{
		p[0/4] = p[8/4] = 0xFFFFFFFF;
		p[4/4] = PSP_IPL_SIGNATURE;
	}

	// Write data blocks
	block = 0x10;
	for (i = 0; i < nblocks; i++)
	{
		if (i == (nblocks-1))
		{
			int mod = size % PSP_NAND_BLOCK_USER_SIZE;
			
			if (mod != 0)
			{
				memset(user, 0xFF, PSP_NAND_BLOCK_USER_SIZE);
				memcpy(user, buf, mod);

				buf = user;
			}		
		}
			
		while (1)
		{
			ppn = block*PSP_NAND_PAGES_PER_BLOCK;
			res = sceNandWriteIplBlockWithVerify(ppn, buf, spare);
			if (res < 0)
			{
				res = sceNandEraseIplBlockWithRetry(ppn);
				if (res < 0)
				{	
					//sceNandDoMarkAsBadBlock(ppn);					
				}

				block++;
			}
			else
			{
				blocktable[i] = block;
				block++;
				break;
			}
		}

		buf += PSP_NAND_BLOCK_USER_SIZE;
	}

	// Write block table to all mirrors
	memset(user, 0, PSP_NAND_BLOCK_USER_SIZE);
	memcpy(user, blocktable, nblocks*2);
	memset(spare, 0xFF, PSP_NAND_BLOCK_SPARE_SMALL_SIZE);
	*(u32 *)&spare[4] = PSP_IPL_SIGNATURE;

	written = 0;
	
	for (block = 4; block < 0x0C; block++)
	{
		ppn = block*PSP_NAND_PAGES_PER_BLOCK;
		res = sceNandWriteIplBlockWithVerify(ppn, user, spare);
		if (res < 0)
		{
			res = sceNandEraseIplBlockWithRetry(ppn);
			if (res < 0)
			{
				//sceNandDoMarkAsBadBlock(ppn);				
			}
		}
		else
		{
			written = 1;
		}
	}

	if (!written)
	{
		printf("IPL block table could not be written to any mirror.\n");
		return -1;
	}

	return 0;
}

void FlashIpl()
{
	sceNandLock(1);
	if (pspIplClearIpl() != 0)
	{
		sceNandUnlock();
		ErrorExit(6000, "Error in clear ipl.\n");
	}


	int res = pspIplSetIpl(nandipl, IPL_SIZE);
	sceNandUnlock();

	if (res != 0)
	{
		ErrorExit(6000, "Error writing ipl.\n");
	}
}

int patchipl_main(void)
{
	// I changed the static decrlaration of this buffers (16MB in total)
	// to a dynamic one - better memory management, the constantly
	// reserved (but not often used) 16MB caused heavy crashing....
	buffer   = hcAlignedMalloc(((10*1024*1024)+1)*sizeof(u8), 64);
  dataOut  = hcAlignedMalloc(((3*1024*1024)+1)*sizeof(u8), 64);
  dataOut2 = hcAlignedMalloc(((3*1024*1024)+1)*sizeof(u8), 64);
  
  if (!InitSysEntries())
  {
    printf("ERROR: system doesn't jive\n");
  }

  printf("Engaging custom IPL installer (IPL and flasher by moonlight!)\n\n");
	printf("Building IPL.... ");
	BuildIpl();

	printf("and flashing IPL.... ");
	FlashIpl();
  printf("DONE :-)");
  return 0;
}
