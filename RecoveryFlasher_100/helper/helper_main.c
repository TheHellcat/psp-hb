
/*
    #########################################################
    ##                                                     ##
    ##   R e c o v e r y   F l a s h e r                   ##
    ##   by ---==> HELLCAT <==---                          ##
    ##                                                     ##
    #########################################################

    ====  Kernel Helper Module  ====
    --------------------------------
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <psploadexec_kernel.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <pspnand_driver.h>
#include <pspsyscon.h>
#include <pspsysreg.h>

#include "consts.h"
#include "systemctrl.h"


PSP_MODULE_INFO("HCRcFlKernelHelper_driver", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);


//////////////////////////////
//                          //
//   Declares and Defines   //
//                          //
//////////////////////////////

int sceSyscon_driver_7EC5A957(u32 *baryon);
u32 sceSysreg_driver_E2A5D1EE(void);
extern int sceNandEraseBlockWithRetry(u32);
extern int sceNandWriteBlockWithVerify(u32, void *, void *);
extern int sceNandEraseIplBlockWithRetry(u32);


//////////////////////////////
//                          //
//   Main Code              //
//   (exported functions)   //
//                          //
//////////////////////////////

int module_start(SceSize args, void *argp)
{
  return 0;
}
int module_stop( void )
{
  return 0;
}


void hcRfHelperGetMoboVersions( int* baryon, int* tachyon )
{
  u32 by, ty;
  int k;

  k = pspSdkSetK1(0);
  ty = sceSysreg_driver_E2A5D1EE();
  sceSyscon_driver_7EC5A957(&by);
  pspSdkSetK1(k);
  (*baryon) = by;
  (*tachyon) = ty;
}


int hcRfHelperLoadStartModule( char *module )
{
  int k1 = pspSdkSetK1(0);
  SceUID mod = sceKernelLoadModule(module, 0, NULL);

  if (mod < 0)
  {
    pspSdkSetK1(k1);
    return mod;
  }

  int res = sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
  pspSdkSetK1(k1);
  return res;
}


u32 hcRfHelperPspFindProc(const char* szMod, const char* szLib, u32 nid)
{
  struct SceLibraryEntryTable *entry;
  SceModule *pMod;
  void *entTab;
  int entLen;

  pMod = sceKernelFindModuleByName(szMod);

  if (!pMod)
  {
    // NOP //
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
          {
            return vars[count+total];
          }
        }
      }
    }

    i += (entry->len * 4);
  }

  return 0;
}


int hcRfHelperIoBridgeUnassign( char* dev )
{
  int r;
  int k;

  k = pspSdkSetK1(0);
  r = sceIoUnassign( dev );
  pspSdkSetK1(k);

  return r;
}


int hcRfHelperIoBridgeLFatfmt( int flash )
{
  char* fmtargv[2];
  int k;
  int r;

  u32 (* sceLflashFatfmtStartFatfmt)(int, char **) = (void *)hcRfHelperPspFindProc("sceLflashFatfmtUpdater", "LflashFatfmt", 0xB7A424A4);

  r = -1;
  k = pspSdkSetK1( 0 );

  fmtargv[0] = "fatfmt";

  if( flash == 0 )
  {
    fmtargv[1] = "lflash0:0,0";
    r = sceLflashFatfmtStartFatfmt(2, fmtargv);
  }
  if( flash == 1 )
  {
    fmtargv[1] = "lflash0:0,1";
    r = sceLflashFatfmtStartFatfmt(2, fmtargv);
  }
  if( flash == 2 )
  {
    fmtargv[1] = "lflash0:0,2";
    r = sceLflashFatfmtStartFatfmt(2, fmtargv);
  }
  if( flash == 3 )
  {
    fmtargv[1] = "lflash0:0,3";
    r = sceLflashFatfmtStartFatfmt(2, fmtargv);
  }

  pspSdkSetK1( k );

  return r;
}


int hcRfHelperRebootToUpdater( void )
{
  int r;
  struct SceKernelLoadExecVSHParam param;
  int k;

  k = pspSdkSetK1( 0 );

  memset(&param, 0, sizeof(param));
  param.size = sizeof(param);
  param.argp = "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP s2";
  param.args = 36;
  param.key = "updater";
  param.vshmain_args_size = 0;
  param.configfile = 0;
  param.unk4 = 0;
  param.unk5 = 0x10000;
  r = sceKernelLoadExecVSHMs1( "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP", &param );

  pspSdkSetK1( k );
  return r;
}


typedef int (*PROC_MANGLE)(void* r4, u32 r5, void* r6, u32 r7, u32 r8);
PROC_MANGLE g_mangleProc;

int hcRfHelperInitMangleSyscall( void )
{
    // dynamic/explicit lookup
    g_mangleProc = (PROC_MANGLE)hcRfHelperPspFindProc("sceMemlmd", "semaphore", 0x4c537c72);

    return (g_mangleProc != NULL);
}

int hcRfHelperPspEncrypt( u32 *buf, int size )
{
  int k;

  buf[0] = 4;
  buf[1] = buf[2] = 0;
  buf[3] = 0x100;
  buf[4] = size;

  // Note: this encryption returns different data in each psp,
  // But it always returns the same in a specific psp (even if it has two nands)
  k = pspSdkSetK1( 0 );
  if (g_mangleProc(buf, size+0x14, buf, size+0x14, 5) < 0)
  {
    pspSdkSetK1( k );
    return -1;
  }

  pspSdkSetK1( k );
  return 0;
}


void hcRfHelperPatchLFatEncryption( void )
{
  u32* value = 0;
  u32 store[2] = { 0, 0 };
  value = (u32*)hcRfHelperPspFindProc("sceNAND_Updater_Driver", "sceNand_updater_driver", 0x0BEE8F36);
  store[0] = value[0];
  store[1] = value[1];
  value[0] = 0x03e00008;  // branch return
  value[1] = 0x00000000;  // NOP
}


int hcRfHelperIoBridgeWriteFile(char* file, void* buffer, int size)
{
  int k;
  int u;

  k = pspSdkSetK1( 0 );
  u = sctrlKernelSetUserLevel( 8 );

  SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if (fd < 0)
  {
    sctrlKernelSetUserLevel( u );
    pspSdkSetK1( k );
    return fd;
  }

  int written = sceIoWrite(fd, buffer, size);

  if (sceIoClose(fd) < 0)
  {
    sctrlKernelSetUserLevel( u );
    pspSdkSetK1( k );
    return fd;
  }

  sctrlKernelSetUserLevel( u );
  pspSdkSetK1( k );
  return written;
}
