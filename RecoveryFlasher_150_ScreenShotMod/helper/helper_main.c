
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
#include <psppower.h>
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
int sceNandWriteBlockWithVerify(u32 page, void* user, void* spare);
int sceNandEraseBlockWithRetry(u32 ppn);
// int sceNandWritePages( u32 page, void* user, void* spare, u32 count );
int sceNandReadPagesRawAll(u32 ppn, void *buf, void *buf2, u32 count);
int sceNandReadExtraOnly(u32 page, void* buf, u32 count);
int nandLocked = 0;


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


int hcRfHelperIoBridgeLFatfmt( int flash, int UseUpdaterModule )
{
  char* fmtargv[2];
  int k;
  int r;
  
  u32 (*sceLflashFatfmtStartFatfmt)(int, char**);

  if( UseUpdaterModule == 1)
  {
    sceLflashFatfmtStartFatfmt = (void *)hcRfHelperPspFindProc("sceLflashFatfmtUpdater", "LflashFatfmt", 0xB7A424A4);
  } else {
    // u32 (* sceLflashFatfmtStartFatfmt)(int, char **) = (void *)hcRfHelperPspFindProc("sceLflashFatfmt", "LflashFatfmt", 0xB7A424A4);
    sceLflashFatfmtStartFatfmt = (void *)hcRfHelperPspFindProc("sceLflashFatfmt", "LflashFatfmt", 0xB7A424A4);
  }
  
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

int hcRfHelperPspDecryptSC( u32* buf, int size )
{
	int k;
	
	k = pspSdkSetK1( 0 );
	
  buf[0] = 5; 
  buf[1] = buf[2] = 0; 
  buf[3] = 0x100; 
  buf[4] = size; 

  if (g_mangleProc(buf, size+0x14, buf, size+0x14, 8) < 0)    // sceUtilsBufferCopyWithRange_
  { 
  	pspSdkSetK1( k );
    return -1; 
  } 

  pspSdkSetK1( k );
  return 0; 
} 

void sceKernelIcacheInvalidateAll();
void hcRfHelperPatchLFatEncryption( int patch_type )
{
	int k;
  u32* value;
  
  if( patch_type == 0 )  // FAT PSP for any FW
  {
    value = (u32*)hcRfHelperPspFindProc("sceNAND_Updater_Driver", "sceNand_updater_driver", 0x0BEE8F36);
    value[0] = 0x03e00008;  // branch return
    value[1] = 0x00000000;  // NOP
  }
  
  /*if( patch_type == 1 )  // SLIM PSP for FW 4.01
  {
    u32 mod = (u32)sceKernelFindModuleByName("sceNAND_Updater_Driver");
	  _sh (0x0D7E, mod+0xAC64);
  }*/
  
  k = pspSdkSetK1( 0 );
  sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
  sceKernelIcacheInvalidateAll();
  sceKernelDcacheWritebackInvalidateAll();
  pspSdkSetK1( k );
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


int hcRfHelperIoBridgeUnloadModule( char* module )
{
  int r;
  SceModule* mod1;
  int prxstatus;
  int k;
  
  k = pspSdkSetK1( 0 );
  
  mod1 = sceKernelFindModuleByName( module );
  if(!mod1)
	{
		pspSdkSetK1( k );
	  return -1;
	}
	
  prxstatus = -1;
  r = sceKernelStopModule( mod1->modid, 0, NULL, &prxstatus, NULL );
  if( r<0 )
  {
  	pspSdkSetK1( k );
    return r;
  }
  
  r = sceKernelUnloadModule(mod1->modid);
  if( r<0 )
  {
  	pspSdkSetK1( k );
    return r;
  }
  
  pspSdkSetK1( k );
  return 0;
}


void hfRfHelperPoweroff( int unk )
{
	int k;
	
	k = pspSdkSetK1( 0 );
	
	/*void (*scePower_0442D852)(int unk) = (void*)hcRfHelperPspFindProc("scePower_Service", "scePower", 0x0442D852);
  //if(!scePower_0442D852) { printf( "whooops, function not found...." ); }
  scePower_0442D852( 0 );
  sceKernelDelayThread( 5000000 );*/
	
  sceIoDelDrv( "flashfat" );
  sceIoDelDrv( "lflash" );
  //sceKernelDelayThread( 500000 );
  scePowerRequestStandby(); //
  //sceSysconPowerStandby();
  //sceKernelDelayThread( 1000000 );
  //void (*scePower_0442D852)(int unk) = (void*)hcRfHelperPspFindProc("scePower_Service", "scePower", 0x0442D852);
  //scePower_0442D852( 0 );
  //sceKernelDelayThread( 1000000 );
  pspSdkSetK1( k );
}


int hcRfHelperLaunchEBOOT( char* path )
{
	int r;
  struct SceKernelLoadExecVSHParam param;
  int k;

  k = pspSdkSetK1( 0 );

  memset(&param, 0, sizeof(param));
  param.size = sizeof(param);
  param.argp = path;
  param.args = strlen(path)+1;
  param.key = "game";
  param.vshmain_args_size = 0;
  param.configfile = 0;
  param.unk4 = 0;
  param.unk5 = 0x10000;
  
	r = sceKernelLoadExecVSHMs2( path, &param );
	
	pspSdkSetK1( k );
	
  return r;
}


void LockNand( void ) 
{
  if (!nandLocked)
    sceNandLock(0);
        
  nandLocked = 1;
}
void UnlockNand( void ) 
{
  if (nandLocked)
    sceNandUnlock();
 
  nandLocked = 0;
}
int ReadNandBlock(u32 page, u8 *buffer, int readextra)
{
  u32 i;
  u32 k1;
 
  k1 = pspSdkSetK1( 0 );
 
  LockNand();
 
  if (sceNandIsBadBlock(page))
  {
    memset(buffer, 0xFF, 528);
    UnlockNand();
    pspSdkSetK1( k1 );                
    return -1;
  }
 
  for (i = 0; i < 32; i++)
  {
    //for (j = 0; j < 4; j++)
    //{
      sceNandReadPagesRawAll(page, buffer, NULL, 1);
      if( readextra == 1 ) { sceNandReadExtraOnly(page, buffer+512, 1); }
    //}
 
    page++;
    if( readextra != 1 ) { buffer += 512; } else { buffer += 528;}
  }
 
  UnlockNand();
  pspSdkSetK1( k1 );
 
  return 0;
}
int WriteNandBlock(u32 page, u8* user, u8* spare)
{
  u32 k;
  int r;
  
  k = pspSdkSetK1( 0 );
  
  sceNandLock(0);
  sceNandSetWriteProtect(0);
  
  //r = sceNandEraseBlockWithRetry( page );
  //if( r<0 ) { return r + 0x10000000; }
  r = sceNandWriteBlockWithVerify( page, user, spare );
  if( r<0 ) { return r; }
  
  sceNandSetWriteProtect(1);
  sceNandUnlock();
  
  pspSdkSetK1( k );
  
  return 0;
}

int hcRfHelperDumpNandIplToBuffer( u8* buffer )
{
  int i;
  int eoipl;
  u8 endblock[512];
  
  eoipl = 0;
  memset( endblock, 0xFF, 512 );
  
  for( i=0x10; i<0x30; i++ )
  {
    ReadNandBlock( i*32, buffer, 0 );
    if( memcmp(endblock, buffer, 512) == 0)  // first block AFTER the IPL?
    {
      if( eoipl == 0 ) { eoipl = i - 0x10; }    // note the last block number of the IPL
    } else {
      if( eoipl != 0 ) { eoipl = 0; }        // valid data block when we already saved end of IPL?
    }                                        // seems it didn't have ended yet. reset marker!
    buffer += 512*32;
  }
  
  return eoipl*32*512;
}

int hcRfHelperDumpNandBlockRangeToBuffer( u8* buffer, int startblock, int blockcount )
{
  int i;
  unsigned int b, l;
  
  b = (unsigned int)buffer;
  
  for( i=startblock; i<(startblock+blockcount); i++ )
  {
    ReadNandBlock( i*32, buffer, 1 );
    buffer += 528*32;
  }
  
  l = (unsigned int)buffer - b;
  
  return l;
}
int hcRfHelperWriteNandBlockRange( u8* user, u8* spare, int startblock, int blockcount )
{
  int i, r, page;
  
  r = 0;
  page = startblock * 32;
  
  for( i=startblock; i<(startblock+blockcount); i++ )
  {
    r = WriteNandBlock( page, user, spare );
    if( r<0 ) { return r; }
    page += 32;
    user += 512*32;
    spare += 12*32;
  }
  
  return 0;
}


// ===== BEGIN of direct HW access code, credits to Silverspring for this :-) ===== //
int sceSysconCmdExec(u8* param, int unk);  // prototype for Syscon call
int batDirectWriteEeprom(u8 addr, u16 data)
{
  int res;
  u8 param[0x60];

  if (addr > 0x7F)
  return(0x80000102);

  param[0x0C] = 0x73; // write battery eeprom command
  param[0x0D] = 5; // tx packet length

  // tx data
  param[0x0E] = addr;
  param[0x0F] = data;
  param[0x10] = data>>8;

  res = sceSysconCmdExec(param, 0);

  if (res < 0)
  return(res);

  return 0;
}

int batDirectReadEeprom(u8 addr)
{
  int res;
  u8 param[0x60];

  if (addr > 0x7F)
  return(0x80000102);

  param[0x0C] = 0x74; // read battery eeprom command
  param[0x0D] = 3; // tx packet length

  // tx data
  param[0x0E] = addr;

  res = sceSysconCmdExec(param, 0);

  if (res < 0)
  return(res);

  // rx data
  return((param[0x21]<<8) | param[0x20]);
}
// ===== END of direct HW access code ===== //

int hcRfHelperBatSetSerial(char* serial)
{
  int r=0;
  int k;
  int shi;
  int slo;

  shi = ((serial[0]&0xFF)*0x100)+(serial[1]&0xFF);
  slo = ((serial[2]&0xFF)*0x100)+(serial[3]&0xFF);

  k = pspSdkSetK1(0);
  
  r = batDirectWriteEeprom(0x07, shi);
  r = batDirectWriteEeprom(0x09, slo);

  pspSdkSetK1(k);
  return r;
}

void hcRfHelperBatGetSerial(char* serial)
{
  int r=0;
  int k;

  k = pspSdkSetK1(0);
  
  r = batDirectReadEeprom(0x07);
  serial[0] = (r / 0x100) & 0xFF;
  serial[1] = r & 0xFF;
  r = batDirectReadEeprom(0x09);
  serial[2] = (r / 0x100) & 0xFF;
  serial[3] = r & 0xFF;
    
  pspSdkSetK1(k);
}
