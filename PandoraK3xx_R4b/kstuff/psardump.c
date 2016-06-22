// =================================================================== //
// ==== This code is 90% PspPet's PSAR dumper,               ========= //
// ====                     5% DAX mods and 5% mod of me ;-) ========= //
// =================================================================== //

#include "kutil2.h"

#define OVERHEAD    0x150 /* size of encryption block overhead */
#define SIZE_A      0x110 /* size of uncompressed file entry = 272 bytes */

void hcChangePathF0toMS0(char *path)
{
	char newpath[strlen(path)-3];
	int i;
	
  strncpy(newpath, "ms0:/", 5);
	for(i=5;i<strlen(path)-3;i++)
	{
	  newpath[i]=path[i+3];
	}
	newpath[i]=0;
	
  strcpy(path, newpath);
}

// for 1.50 and later, they mangled the plaintext parts of the header
static void Demangle(const u8* pIn, u8* pOut)
{
	int k = pspSdkSetK1(0);
    u8 buffer[20+0x130];
    memcpy(buffer+20, pIn, 0x130);
    u32* pl = (u32*)buffer; // first 20 bytes
    pl[0] = 5;
    pl[1] = pl[2] = 0;
    pl[3] = 0x55;
    pl[4] = 0x130;

    // semaphore_4C537C72   sceUtilsBufferCopyWithRange
    semaphore_4C537C72(buffer, 20+0x130, buffer, 20+0x130, PSP_KIRK_SCRAMBLE);
    memcpy(pOut, buffer, 0x130);
    pspSdkSetK1(k);
}

int DecodeBlock(const u8* pIn, int cbIn, u8* pOut)
{
	  int k = pspSdkSetK1(0);
    // pOut also used as temporary buffer for mangled input
    // assert((((u32)pOut) & 0x3F) == 0); // must be aligned

    memcpy(pOut, pIn, cbIn + 0x10); // copy a little more for $10 page alignment

    int ret;
    int cbOut;
    
    // new style - scrambled PSAR (works with 1.5x and 2.00)
    Demangle(pIn+0x20, pOut+0x20); // demangle the inside $130 bytes
    if (*(u32*)&pOut[0xd0] != 0x0E000000)
    {
    	pspSdkSetK1(k);
        return 0x80CA0DE1;
	}
    
	ret = sceMesgd_driver_102DC8AF(pOut, cbIn, &cbOut);
    if (ret != 0)
    
    if (ret != 0)
    	{
    		pspSdkSetK1(k);
        return 0x80CA0DE2; // error
      }
    
    pspSdkSetK1(k);
    return cbOut;
}

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

    // 2.0 PRXs
    { 0x4467415d, (u8*)g_key44, 0x59, 0x59 },
    { 0x207bbf2f, (u8*)g_key20, 0x5A, 0x5A },
    { 0x3ace4dce, (u8*)g_key3A, 0x5B, 0x5B },
};

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

    int ret = semaphore_4C537C72(buffer2, 20+0xA0, buffer2, 20+0xA0, 7);
    if (ret != 0)
        {}
    // copy result back
    memcpy(buffer1, buffer2, 0xA0);
}

int DecryptPRX1(const u8* pbIn, u8* pbOut, int cbTotal, u32 tag)
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

    ret = semaphore_4C537C72(pbOut+0x2C, 20+0x70, pbOut+0x2C, 20+0x70, 7);
    if (ret != 0)
    {
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
    ret = semaphore_4C537C72(pbOut, cbTotal, pbOut+0x40, cbTotal-0x40, 0x1);
    if (ret != 0)
    {
        return -1;
    }

    // return cbTotal - 0x150; // rounded up size
	return *(u32*)&pbIn[0xB0]; // size of actual data (fix thanks to Vampire)
}

typedef struct
{
    u32 tag; // 4 byte value at offset 0xD0 in the PRX file
    u8  *key; // 16 bytes keys
    u8 code; // code for scramble 
} TAG_INFO2;

int Scramble(u32 *buf, u32 size, u32 code)
{
	buf[0] = 5;
	buf[1] = buf[2] = 0;
	buf[3] = code;
	buf[4] = size;

	if (semaphore_4C537C72(buf, size+0x14, buf, size+0x14, 7) < 0)
	{
		return -1;
	}

	return 0;
}

u32 (* UtilsForKernel_7DD07271) (void* destP, u32 cb, const void* scrP, u32* retP);

int DecryptDecompress(u8 *dataIn, int cbFile, u8 *dataOut, int decompress)
{
	int size = DecryptPRX1(dataIn, dataOut, cbFile, *(u32*)&dataIn[0xD0]);

	if (size <= 0)
	{
	}

	if (size <= 0)
		return size;

	if (!decompress)
		return size;

	if (dataOut[0] == 0x1F && dataOut[1] == 0x8B)
	{
		memcpy(dataIn, dataOut, size);
		size =  sceKernelGzipDecompress(dataOut, 2000000, dataIn, 0);
	}

	else if (memcmp(dataOut, "2RLZ", 4) == 0)
	{
		memcpy(dataIn, dataOut, size);
		size = UtilsForKernel_7DD07271(dataOut, 2000000, dataIn+4, NULL);
	}

	return size;
}

int fsizetowrite;
int WriteFile(char *file, void *buf, int size)
{
	// OK, since we're in 3.xx kernel now, we need to instruct a usermode
	// function to write the file for us....
  
  /*strcpy(dataMisc, file);
  int written = userWriteFile(dataMisc, buf, size);
	return written;*/
	// direct call to the usermode function was too
	// unstable, PSP always crashed at random points
	// lets just save some data for the userthread to
	// retrieve and use....
	strcpy(dataMisc, file);
	fsizetowrite = size;
	return size;
}

int ReadFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);

	if (fd < 0)
	{
		return -1;
	}

	int read = sceIoRead(fd, buf, size);
	
	if (sceIoClose(fd) < 0)
		return -1;

	return read;
}

u32 FindProc(const char* szMod, const char* szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		return 0;
	}
	
	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	
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

	return 0;
}

int cbOut;
int sz;
char name[64];
int iBase;
int cbChunk;
u32 cbDataChunk;
u32 cbExpanded;
int sigcheck;
u32* pl;
const u8* pbIn;
u32 pbEnd;
int ret;
int running=0;
int extract_psar(u8 *dataPSAR, int cbFile, u8 *dataOut, u8 *dataOut2)
{
	if(running==0)
	{
    if (memcmp(dataPSAR, "PSAR", 4) != 0)
    {
      return 0x80CA0021;
    }
  
  ////int i;
  //int cbOut;
  //int sz;
  //char name[64];

    // at the start of the PSAR file,
    //   there are one or two special version data chunks
    cbOut = DecodeBlock(&dataPSAR[0x10], OVERHEAD+SIZE_A, dataOut);
    if (cbOut <= 0)
    {
      return cbOut; //0x80CA0022;
    }
    if (cbOut != SIZE_A)
    {
      return 0x80CA0023;
    }
   
    iBase = 0x10+OVERHEAD+SIZE_A; // after first entry
            // iBase points to the next block to decode (0x10 aligned)

    // second block
    cbOut = DecodeBlock(&dataPSAR[0x10+OVERHEAD+SIZE_A], OVERHEAD+100, dataOut);
    if (cbOut <= 0)
    {
      cbOut = DecodeBlock(&dataPSAR[0x10+OVERHEAD+SIZE_A], OVERHEAD+144, dataOut);
      if (cbOut <= 0)
      {
        return 0x80CA0024;
      }
    }
       
    cbChunk = (cbOut + 15) & 0xFFFFFFF0;
    iBase += OVERHEAD+cbChunk;
    
    running=1;
  }
  
  //while (iBase < cbFile-OVERHEAD)
  //{
  if(iBase < cbFile-OVERHEAD)
  {
    scePowerTick(0);
    
    cbOut = DecodeBlock(&dataPSAR[iBase], OVERHEAD+SIZE_A, dataOut);
    if (cbOut <= 0)
    {
      //break;
    }
    if (cbOut != SIZE_A)
    {
      //break;
    }

    strcpy(name, (const char*)&dataOut[4]);

    pl = (u32*)&dataOut[0x100];
    sigcheck = (dataOut[0x10F] == 2);
        
    // pl[0] is 0
    // pl[1] is the PSAR chunk size (including OVERHEAD)
    // pl[2] is true file size (TypeA=272=SIZE_A, TypeB=size when expanded)
    // pl[3] is flags or version?
    if (pl[0] != 0)
    {
      //break;
    }

    iBase += OVERHEAD + SIZE_A;
    cbDataChunk = pl[1]; // size of next data chunk (including OVERHEAD)
    cbExpanded = pl[2]; // size of file when expanded
    
    if (cbExpanded > 0)
    {
      cbOut = DecodeBlock(&dataPSAR[iBase], cbDataChunk, dataOut);
      if (cbOut > 10 && dataOut[0] == 0x78 && dataOut[1] == 0x9C)
      {
        // standard Deflate header

        pbIn = &dataOut[2]; // after header
        ret = sceKernelDeflateDecompress(dataOut2, cbExpanded, pbIn, &pbEnd);
        hcChangePathF0toMS0(name);
        if (ret == cbExpanded)
        {
        	WriteFile(name, dataOut2, cbExpanded);
      
          if (strcmp(name, "flash0:/kd/loadexec.prx") == 0)
          {
            sz = DecryptDecompress(dataOut2, cbExpanded, dataOut, 1);
                
            if (sz < 0)
            {
            } else {
              //WriteFile("ms0:/loadexec.prx", dataOut, sz);
            }
          }
        } else {
        	
        }
      } else {
      	
      }
    } else if (cbExpanded == 0) {
            
    } else {
    	
    }
    iBase += cbDataChunk;   // skip over data chunk  
  //}
  } else {
  	WriteFile("0", dataOut2, 0);
  }


  return fsizetowrite;
}
