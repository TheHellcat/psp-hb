
/*
    ====  PSAR extraction / FW unpacking and installing  ====
    ---------------------------------------------------------
    based on M33 PSARDumper
    ---------------------------------------------------------
    
    actually, this is the FW installation part of
    "Recovery Flasher" :-D slightly modified to install
    everything to the memstick, and all files (for both PSPs)
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "libpsardumper.h"
#include "pspdecrypt.h"
#include "misc.h"

#define printf    pspDebugScreenPrintf


char* exfiles1000[] = { "flash0:/kd/loadexec.prx",
                        "flash0:/kd/mesg_led.prx",
                        "flash0:/kd/memlmd.prx",
                        "flash0:/kd/pspbtcnf.bin",
                        "flash0:/kd/wlanfirm_magpie.prx",
                        "flash0:/kd/hpremote_01g.prx",
                        "flash0:/kd/resource/meimg.img",
                        "flash0:/kd/resource/me_sdimg.img",
                        "flash0:/kd/resource/me_blimg.img",
                        "0"
                     };

char* exfiles2000[] = { "flash0:/font/arib.pgf",
                        "flash0:/kd/umdcache.prx",
                        "flash0:/kd/dve.prx",
                        "flash0:/kd/usb1seg.prx",
                        "flash0:/kd/hibari.prx",
                        "flash0:/kd/wlanfirm_voyager.prx",
                        "flash0:/kd/loadexec_02g.prx",
                        "flash0:/kd/memlmd_02g.prx",
                        "flash0:/kd/mesg_led_02g.prx",
                        "flash0:/kd/hpremote_02g.prx",
                        "flash0:/kd/pspbtcnf_02g.bin",
                        "flash0:/kd/resource/me_t2img.img",
                        "flash0:/vsh/module/oneseg_plugin.prx",
                        "flash0:/vsh/module/oneseg_launcher_plugin.prx",
                        "flash0:/vsh/module/oneseg_sal.prx",
                        "flash0:/vsh/module/oneseg_hal_toolbox.prx",
                        "flash0:/vsh/module/oneseg_sdkcore.prx",
                        "flash0:/vsh/module/oneseg_core.prx",
                        "flash0:/vsh/module/oneseg_sdk.prx",
                        "flash0:/vsh/module/skype_main_plugin.prx",
                        "flash0:/vsh/module/skype_plugin.prx",
                        "flash0:/vsh/module/skype_ve.prx",
                        "flash0:/vsh/module/skype_skyhost.prx",
                        "flash0:/vsh/resource/custom_theme.dat",
                        "flash0:/vsh/resource/oneseg_plugin.rco",
                        "flash0:/vsh/resource/13-27.bmp",
                        "flash0:/vsh/resource/skype_main_plugin.rco",
                        "0"
                      };

int PSPType;
char PBPPath[64];
int fwflag;

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
/*
typedef int (*PROC_MANGLE)(void* r4, u32 r5, void* r6, u32 r7, u32 r8);
PROC_MANGLE g_mangleProc;

int InitSysEntries()
{
    // dynamic/explicit lookup
    g_mangleProc = (PROC_MANGLE)pspFindProc("sceMemlmd", "semaphore", 0x4c537c72);
printf( "   %x\n", (int)g_mangleProc );
    return (g_mangleProc != NULL);
}

int Encrypt(u32 *buf, int size)
{
	buf[0] = 4;
	buf[1] = buf[2] = 0;
	buf[3] = 0x100;
	buf[4] = size;

	// Note: this encryption returns different data in each psp,
	// But it always returns the same in a specific psp (even if it has two nands)
	if (g_mangleProc(buf, size+0x14, buf, size+0x14, 5) < 0)
		return -1;

	return 0;
}*/

/*int GenerateSigCheck(u8 *buf)
{
	u8 enc[0xD0+0x14];
	int iXOR, res;

	memcpy(enc+0x14, buf+0x110, 0x40);
	memcpy(enc+0x14+0x40, buf+0x80, 0x90);
	
	for (iXOR = 0; iXOR < 0xD0; iXOR++)
	{
		enc[0x14+iXOR] ^= check_keys0[iXOR&0xF];
	}

	//if ((res = Encrypt((u32 *)enc, 0xD0)) < 0)
	if ((res = hcRfHelperPspEncrypt((u32 *)enc, 0xD0)) < 0)
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
}*/


////////////////////////////////////////////////////////////////////
// big buffers for data. Some system calls require 64 byte alignment

// big enough for the full PSAR file
static u8* g_dataPSAR; //[19000000] __attribute__((aligned(64))); 

// big enough for the largest (multiple uses)
static u8* g_dataOut; //[3000000] __attribute__((aligned(0x40)));
   
// for deflate output
//u8 g_dataOut2[3000000] __attribute__((aligned(0x40)));
static u8* g_dataOut2;

void DDC6ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);
	
	sceKernelDelayThread(milisecs*1000);
}

////////////////////////////////////////////////////////////////////
// File helpers

int DDC6ReadFile(char *file, int seek, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	if (seek > 0)
	{
		if (sceIoLseek(fd, seek, PSP_SEEK_SET) != seek)
		{
			sceIoClose(fd);
			return -1;
		}
	}

	int read = sceIoRead(fd, buf, size);
	
	sceIoClose(fd);
	return read;
}

static int DDC6hcWriteFile(char* file, void* buffer, int size)
{
  scePowerTick(0);

  int i;
  int pathlen = 0;
  char path[128];

  // because I'm so lazy, I need folders created "on the fly"
  // this does so, create every folder in the path of the
  // file we are to save.
  // A little bruty forcy, yah, but it does the job :-D
  for(i=1; i<(strlen(file)); i++)
  {
    if(strncmp(file+i-1, "/", 1) == 0)
    {
      pathlen=i-1;
      strncpy(path, file, pathlen);
      path[pathlen] = 0;
      sceIoMkdir(path, 0777);
    }
  }

  // now up to the file write....
  SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if (fd < 0)
  {
    return -1;
  }

  scePowerTick(0);

  int written = sceIoWrite(fd, buffer, size);

  scePowerTick(0);

  if (sceIoClose(fd) < 0)
    return -1;

  return written;
}

int LastFilePspType;
int DDC6WriteFile(char *file, void *buf, int size)
{
	int i;
	int skipfile;
	char tfile[256];
	
	//SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	
	//if (fd < 0)
	//{
	//	return fd;
	//}
	
	skipfile = 0;
	i = 0;
	//if( PSPType == 1000 )
  //{
    while( 1 )
    {
    	if( strcmp( exfiles2000[i], file ) == 0 ) { LastFilePspType = 2000; }
      i++;
      if( strcmp( exfiles2000[i], "0" ) == 0 ) { break; }
    }
  //}
  i = 0;
	//if( PSPType == 2000 )
  //{
    while( 1 )
    {
    	if( strcmp( exfiles1000[i], file ) == 0 ) { LastFilePspType = 1000; }
      i++;
      if( strcmp( exfiles1000[i], "0" ) == 0 ) { break; }
    }
  //}
  if( strcmp( file, "flash0:/vsh/etc/index.dat" ) == 0 )
  {
    if( LastFilePspType == 2000 ) { sprintf( file, "flash0:/vsh/etc/yndex.dat" ); }
  }
  
  // we're only interested in the files supposed to go to flash0,
  // so filter those out, and ignore anything else
  if( strncmp(file, "flash0:/", 8) == 0 )
  {
    sprintf( tfile, "ms0:/TM/DC6/%s", file+8 );
  } else {
    return size;
  }
  
	int written = DDC6hcWriteFile(tfile, buf, size);

	//sceIoClose(fd);
	return written;
}

static char com_table[0x4000];
static int comtable_size;

static char _1g_table[0x4000];
static int _1gtable_size;

static char _2g_table[0x4000];
static int _2gtable_size;

enum
{
	MODE_ENCRYPT_SIGCHECK,
	MODE_ENCRYPT,
	MODE_DECRYPT,
};

static int FindTablePath(char *table, int table_size, char *number, char *szOut)
{
	int i, j, k;

	for (i = 0; i < table_size-5; i++)
	{
		if (strncmp(number, table+i, 5) == 0)
		{
			for (j = 0, k = 0; ; j++, k++)
			{
				if (table[i+j+6] < 0x20)
				{
					szOut[k] = 0;
					break;
				}

				if (!strncmp(table+i+6, "flash", 5) &&
					j == 6)
				{
					szOut[6] = ':';
					szOut[7] = '/';
					k++;
				}
				else if (!strncmp(table+i+6, "ipl", 3) &&
					j == 3)
				{
					szOut[3] = ':';
					szOut[4] = '/';
					k++;
				}
				else
				{				
					szOut[k] = table[i+j+6];
				}
			}

			return 1;
		}
	}

	return 0;
}

/*static int FindReboot(u8 *input, u8 *output, int size)
{
	int i;

	for (i = 0; i < (size - 0x30); i++)
	{
		if (memcmp(input+i, "~PSP", 4) == 0)
		{
			size = *(u32 *)&input[i+0x2C];

			memcpy(output, input+i, size);
			return size;
		}
	}

	return -1;
}*/

/*static void ExtractReboot(int mode, char *loadexec, char *reboot, char *rebootname)
{
	int s = ReadFile(loadexec, 0, g_dataOut, 3000000);

	if (s <= 0)
		return;
	
	//printf("Extracting %s... ", rebootname);

	if (mode != MODE_DECRYPT)
	{
		if (mode == MODE_ENCRYPT_SIGCHECK)
		{
			memcpy(g_dataOut2, g_dataOut, s);
			pspSignCheck(g_dataOut2);

			if (WriteFile(loadexec, g_dataOut2, s) != s)
			{
				ErrorExit(5000, "Cannot write %s.\n", loadexec);
			}
		}
			
		s = pspDecryptPRX(g_dataOut, g_dataOut2, s);
		if (s <= 0)
		{
			ErrorExit(5000, "Cannot decrypt %s.\n", loadexec);
		}

		s = pspDecompress(g_dataOut2, g_dataOut, 3000000);
		if (s <= 0)
		{
			ErrorExit(5000, "Cannot decompress %s.\n", loadexec);
		}
	}

	s = FindReboot(g_dataOut, g_dataOut2, s);
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot find %s inside loadexec.\n", rebootname);
	}

	s = pspDecryptPRX(g_dataOut2, g_dataOut, s);
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot decrypt %s.\n", rebootname);
	}

	s = pspDecompress(g_dataOut, g_dataOut2, 3000000);
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot decompress %s.\n", rebootname);
	}

	if (WriteFile(reboot, g_dataOut2, s) != s)
	{
		ErrorExit(5000, "Cannot write %s.\n", reboot);
	}

	//printf("done.\n");
}*/

static char *GetVersion(char *buf)
{
	char *p = strrchr(buf, ',');

	if (!p)
		return NULL;

	return p+1;
}

	int cbFile;
int main_psar( void )
{
    int mode=0, s;
	u8 pbp_header[0x28];
	
	mode = MODE_ENCRYPT; //_SIGCHECK;

	sceKernelVolatileMemLock(0, (void *)&g_dataOut2, &s);

	//printf("Loading psar...\n");

	if (DDC6ReadFile(PBPPath, 0, pbp_header, sizeof(pbp_header)) != sizeof(pbp_header))
	{
		printf( "Cannot find updater .PBP.  (%s)", PBPPath );
		DDC6ErrorExit(5000, "\n");
		return -1;
	}

	cbFile = DDC6ReadFile(PBPPath, *(u32 *)&pbp_header[0x24], g_dataPSAR, 20500000);
	if (cbFile <= 0)
	{
		printf( "Error Reading updater .PBP.  (%s)", PBPPath );
		DDC6ErrorExit(5000, "\n");
		return -1;
	}
	else if (cbFile == 20500000)
	{
		DDC6ErrorExit(5000, "PSAR too big. Recompile with bigger buffer.\n");
		return -1;
	}

	//printf("PSAR file loaded (%d bytes)\n", cbFile);

    if (memcmp(g_dataPSAR, "PSAR", 4) != 0)
    {
        DDC6ErrorExit(5000, "Not a PSAR file\n");
		    return -1;
    }
   
	if (pspPSARInit(g_dataPSAR, g_dataOut, g_dataOut2) < 0)
	{
		DDC6ErrorExit(5000, "pspPSARInit failed!.\n");
		return -1;
	}

	//printf("Version %s.\n", GetVersion((char *)g_dataOut+0x10));


    while (1)
	{
		char name[128];
		int cbExpanded;
		int pos;
		int signcheck;

		int res = pspPSARGetNextFile(g_dataPSAR, cbFile, g_dataOut, g_dataOut2, name, &cbExpanded, &pos, &signcheck);

		if (res < 0)
		{
			DDC6ErrorExit(5000, "PSAR decode error, pos=0x%08X.\n", pos);
		}
		else if (res == 0) /* no more files */
		{
			break;
		}
		
		if (!strncmp(name, "com:", 4) && comtable_size > 0)
		{
			if (!FindTablePath(com_table, comtable_size, name+4, name))
			{
				DDC6ErrorExit(5000, "Error: cannot find path of %s.\n", name);
			}
		}

		else if (!strncmp(name, "01g:", 4) && _1gtable_size > 0)
		{
			if (!FindTablePath(_1g_table, _1gtable_size, name+4, name))
			{
				DDC6ErrorExit(5000, "Error: cannot find path of %s.\n", name);
			}
		}

		else if (!strncmp(name, "02g:", 4) && _2gtable_size > 0)
		{
			if (!FindTablePath(_2g_table, _2gtable_size, name+4, name))
			{
				DDC6ErrorExit(5000, "Error: cannot find path of %s.\n", name);
			}
		}

        //printf("'%s' ", name);

		char* szFileBase = strrchr(name, '/');
		
		if (szFileBase != NULL)
			szFileBase++;  // after slash
		else
			szFileBase = "err.err";

		if (cbExpanded > 0)
		{
			char szDataPath[128];
			
			if (!strncmp(name, "flash0:/", 8))
			{
				sprintf(szDataPath, "%s", name);
				//sprintf(szDataPath, "ms0:/f0/%s", name+8);
				//sprintf(szDataPath, "ms0:/F0/%s", name+8);
			}

			else if (!strncmp(name, "flash1:/", 8))
			{
				sprintf(szDataPath, "nul:/F1/%s", name+8);
				//sprintf(szDataPath, "ms0:/F1/%s", name+8);
			}

			else if (!strcmp(name, "com:00000"))
			{
				comtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, fwflag);
							
				if (comtable_size <= 0)
				{
					DDC6ErrorExit(5000, "Cannot decrypt common table.  (%i %i 0x%08x)\n", cbExpanded, fwflag, comtable_size);
				}

				if (comtable_size > sizeof(com_table))
				{
					DDC6ErrorExit(5000, "Com table buffer too small. Recompile with bigger buffer.\n");
				}

				memcpy(com_table, g_dataOut2, comtable_size);						
				strcpy(szDataPath, "nul:/F0/PSARDUMPER/common_files_table.bin");
				//strcpy(szDataPath, "ms0:/F0/PSARDUMPER/common_files_table.bin");
			}
					
			else if (!strcmp(name, "01g:00000"))
			{
				_1gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, fwflag);
							
				if (_1gtable_size <= 0)
				{
					DDC6ErrorExit(5000, "Cannot decrypt 1g table.\n");
				}

				if (_1gtable_size > sizeof(_1g_table))
				{
					DDC6ErrorExit(5000, "1g table buffer too small. Recompile with bigger buffer.\n");
				}

				memcpy(_1g_table, g_dataOut2, _1gtable_size);						
				strcpy(szDataPath, "nul:/F0/PSARDUMPER/fat_files_table.bin");
				//strcpy(szDataPath, "ms0:/F0/PSARDUMPER/fat_files_table.bin");
			}
					
			else if (!strcmp(name, "02g:00000"))
			{
				_2gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, fwflag);
							
				if (_2gtable_size <= 0)
				{
					DDC6ErrorExit(5000, "Cannot decrypt 2g table %08X.\n", _2gtable_size);
				}

				if (_2gtable_size > sizeof(_2g_table))
				{
					DDC6ErrorExit(5000, "2g table buffer too small. Recompile with bigger buffer.\n");
				}

				memcpy(_2g_table, g_dataOut2, _2gtable_size);						
				strcpy(szDataPath, "nul:/F0/PSARDUMPER/slim_files_table.bin");
				//strcpy(szDataPath, "ms0:/F0/PSARDUMPER/slim_files_table.bin");
			}

			else
			{
				sprintf(szDataPath, "nul:/F0/PSARDUMPER/%s", strrchr(name, '/') + 1);
				//sprintf(szDataPath, "ms0:/F0/PSARDUMPER/%s", strrchr(name, '/') + 1);
			}

			//printf("expanded"); 

			if (signcheck && (mode == MODE_ENCRYPT_SIGCHECK)
				&& (strcmp(name, "flash0:/kd/_loadexec.prx") != 0)
				&& (strcmp(name, "flash0:/kd/_loadexec_02g.prx") != 0))
			{
				//pspSignCheck(g_dataOut2);
				//GenerateSigCheck(g_dataOut2);
			}

			if ((mode != MODE_DECRYPT) || (memcmp(g_dataOut2, "~PSP", 4) != 0))
			{
				if (strstr(szDataPath, "ipl") && (strstr(szDataPath, "2000") || strstr(szDataPath, "02h")))
				{
					// IPL Pre-decryption
					cbExpanded = pspDecryptPRX(g_dataOut2, g_dataOut, cbExpanded);
					if (cbExpanded <= 0)
					{
						printf("Warning: cannot pre-decrypt 2000 IPL.\n");
					}
					else
					{
						memcpy(g_dataOut2, g_dataOut, cbExpanded);
					}							
				}
						
				if (DDC6WriteFile(szDataPath, g_dataOut2, cbExpanded) != cbExpanded)
	            {
					DDC6ErrorExit(5000, "Cannot write %s.\n", szDataPath);
					break;
				}
	                    
				//printf(",saved");
			}

			if ((memcmp(g_dataOut2, "~PSP", 4) == 0) &&
				(mode == MODE_DECRYPT))
			{
				int cbDecrypted = pspDecryptPRX(g_dataOut2, g_dataOut, cbExpanded);

				// output goes back to main buffer
				// trashed 'g_dataOut2'
				if (cbDecrypted > 0)
				{
					u8* pbToSave = g_dataOut;
					int cbToSave = cbDecrypted;

					//printf(",decrypted");
                            
					if ((g_dataOut[0] == 0x1F && g_dataOut[1] == 0x8B) ||
						memcmp(g_dataOut, "2RLZ", 4) == 0)
					{
						int cbExp = pspDecompress(g_dataOut, g_dataOut2, 3000000);
						
						if (cbExp > 0)
						{
							//printf(",expanded");
							pbToSave = g_dataOut2;
							cbToSave = cbExp;
						}
						else
						{
							printf("Decompress error\n"
								   "File will be written compressed.\n");
						}
					}
        			
					if (DDC6WriteFile(szDataPath, pbToSave, cbToSave) != cbToSave)
					{
						DDC6ErrorExit(5000, "Error writing %s.\n", szDataPath);
					}
                    
					//printf(",saved!");
				}
				else
				{
					DDC6ErrorExit(5000, "Error in decryption.\n");
				}
			}

			else if (strncmp(name, "ipl:", 4) == 0)
			{
				if( strcmp(szFileBase+(strlen(szFileBase)-4), ".bin") == 0 )
				{
				  sprintf(szDataPath, "flash0:/sceNandIpl_1k.bin");
				} else if( strcmp(szFileBase+(strlen(szFileBase)-4), ".ipl") == 0 ) {
				  sprintf(szDataPath, "flash0:/sceNandIpl_2k.bin");
				} else {
				  sprintf(szDataPath, "nul:/0");
				}
				//DDC6WriteFile( szDataPath, g_dataOut2, cbExpanded );
				//sprintf(szDataPath, "flash0:/ipl_part1_%s", szFileBase);
				//sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part1_%s", szFileBase);
                        
				int cb1 = pspDecryptIPL1(g_dataOut2, g_dataOut, cbExpanded);
				if (cb1 > 0 && (DDC6WriteFile(szDataPath, g_dataOut, cb1) == cb1))
				{
					int cb2 = pspLinearizeIPL2(g_dataOut, g_dataOut2, cb1);
					//sprintf(szDataPath, "flash0:/ipl_part2_%s", szFileBase);
					//sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part2_%s", szFileBase);
							
					DDC6WriteFile(szDataPath, g_dataOut2, cb2);
					
					int cb3 = pspDecryptIPL3(g_dataOut2, g_dataOut, cb2);
					//sprintf(szDataPath, "flash0:/ipl_part3_%s", szFileBase);
					//sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part3_%s", szFileBase);
					//DDC6WriteFile(szDataPath, g_dataOut, cb3);
				}
			}
		}
		else if (cbExpanded == 0)
		{
			//printf("empty");
		}

		//printf("\n");
		scePowerTick(0);
	}

	//ExtractReboot(mode, "ms0:/F0/kd/loadexec.prx", "ms0:/F0/reboot.bin", "reboot.bin");
	//ExtractReboot(mode, "ms0:/F0/kd/loadexec_02g.prx", "ms0:/F0/reboot_02g.bin", "reboot_02g.bin");

    scePowerTick(0);	
	//ErrorExit(10000, "Done.\nAuto-exiting in 10 seconds.\n");

    return 0;
}


void DDC6psarInitPsarDumper( u8* buf1, u8* buf2, int g )
{
  g_dataPSAR = buf1;
  g_dataOut = buf2;
  PSPType = g;
  
  sprintf( PBPPath, ".PBP" );
}
int DDC6psarCopyFwFiles( int checkonly )
{
  return main_psar();
}
void DDC6psarSetPBPPath( char* path )
{
  sprintf( PBPPath, "%s", path );
  
  fwflag = 0;
  if( strcmp(path, "ms0:/401.PBP") == 0 ) { fwflag = 2; }
}
