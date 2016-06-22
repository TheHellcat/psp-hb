
/*
    #########################################################
    ##                                                     ##
    ##   R e c o v e r y   F l a s h e r                   ##
    ##   Downgrade, or recover from semi-brick, from the   ##
    ##   CFW recovery menu                                 ##
    ##   --  w/o the need for a Pandora ;-)  --            ##
    ##                                                     ##
    ##   by ---==> HELLCAT <==---                          ##
    ##                                                     ##
    #########################################################
*/

/*
         ** ----------------------------------------- **
     additional credits need to go to:
     - DAX/M33 for the PSARDumper modules/functions
     - DAX     for the IPL update function
     - JumpR   for some valuable help, w/o it this would have
               taken a lot longer to be actually done
     - cory, jas0nuk and all the other folks at LAN.ST for
       many help and patient answers to my questions as well
         ** ----------------------------------------- **
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspsyscon.h>
#include <psprtc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "helper.h"
#include "func.h"
#include "pspdecrypt.h"
#include "libpsardumper.h"
#include "psardumper.h"
#include "iplupdate.h"

#define appVerMa 1
#define appVerMi 60


#ifdef PANDORA

PSP_MODULE_INFO("HCRecoveryFlasher_Pandora", 0x1007, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#else

#ifdef LEGACY

PSP_MODULE_INFO("HCRecoveryFlasher_Legacy", 0x1000, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(0);

#else

PSP_MODULE_INFO("HCRecoveryFlasher", 0x800, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);
PSP_HEAP_SIZE_KB(23500);

#endif

#endif



//////////////////////////////
//                          //
//   Declares and Defines   //
//                          //
//////////////////////////////

#define printf pspDebugScreenPrintf

u8* dataPSAR;
u8* dataOut;
u8* dataOut2;
int TargetFW;
int TargetOFW = 0;
int NoCleanups = 0;
int WIP = 0;
int OnTM;
int OnOE;
int Has150AddOn;
int OnHEN;
int RunningAsDDC7;
char MyEBOOTPath[128];
int CachedUpdaterModulesLoaded = 0;
int gPspType;
int processingFwPack = 0;
char FwPackPath[128];
int fwflagOverride = -1;
int fwNoCustomIpl = 0;

char* m33files1000[] = { // filename          flash0: targetpath
                            "pspbtjnf.bin",   "/kd/",
                            "pspbtknf.bin",   "/kd/",
                            "pspbtlnf.bin",   "/kd/",
                            "systemctrl.prx", "/kd/",
                            "vshctrl.prx",    "/kd/",
                            "usbdevice.prx",  "/kd/",
                            "satelite.prx",   "/vsh/module/",
                            "recovery.prx",   "/vsh/module/",
                            "popcorn.prx",    "/kd/",
                            "march33.prx",    "/kd/",
                            "idcanager.prx",  "/kd/",
                            "galaxy.prx",     "/kd/",
                            "0"
                       };

char* m33files2000[] = { // filename              flash0: targetpath
                            "pspbtjnf_02g.bin",   "/kd/",
                            "pspbtknf_02g.bin",   "/kd/",
                            "pspbtlnf_02g.bin",   "/kd/",
                            "systemctrl_02g.prx", "/kd/",
                            "vshctrl_02g.prx",    "/kd/",
                            "usbdevice.prx",      "/kd/",
                            "satelite.prx",       "/vsh/module/",
                            "recovery.prx",       "/vsh/module/",
                            "popcorn.prx",        "/kd/",
                            "march33.prx",        "/kd/",
                            "idcanager.prx",      "/kd/",
                            "galaxy.prx",         "/kd/",
                            "0"
                       };



///////////////////
//               //
//   Functions   //
//               //
///////////////////

void ExtractAdditionalModules( void )
{
  int r;
  u8* p;
  int l;
  int o;

  l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;

  r = hcMiscSeqGetFile( p, l, "00000001.bin", dataOut );
  r = hcWriteFile("libpsardumper.prx", dataOut, r);

  r = hcMiscSeqGetFile( p, l, "00000002.bin", dataOut );
  r = hcWriteFile("pspdecrypt.prx", dataOut, r);

  r = hcMiscSeqGetFile( p, l, "00000003.bin", dataOut );
  r = hcWriteFile("iplupdate.prx", dataOut, r);

  r = hcMiscSeqGetFile( p, l, "khelper.prx", dataOut );
  r = hcWriteFile("khelper.prx", dataOut, r);
}


void CleanupAdditionalModules( void )
{
  if( NoCleanups == 1 ) { return; }
  sceIoRemove( "libpsardumper.prx" );
  sceIoRemove( "pspdecrypt.prx" );
  sceIoRemove( "khelper.prx" );
  sceIoRemove( "iplupdate.prx" );
}


void CleanupUpdaterModules( void )
{
  if( NoCleanups == 1 ) { return; }
  sceIoRemove( "nand_updater.prx" );
  sceIoRemove( "lfatfs_updater.prx" );
  sceIoRemove( "lflash_fatfmt_updater.prx" );
}


int LoadAdditionalModules( void )
{
  SceUID mod;

  mod = pspSdkLoadStartModule("khelper.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error 0x%08X loading/starting khelper.prx.\n", mod);
  }

  mod = pspSdkLoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
  }

  mod = pspSdkLoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
  }

  mod = pspSdkLoadStartModule("iplupdate.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error 0x%08X loading/starting iplupdate.prx.\n", mod);
  }

  return 0;
}


int LoadUpdaterModules( void )
{
  SceUID mod;
  int r;

  if( CachedUpdaterModulesLoaded == 1 ) { return 0; }

  r = 0;

  mod = pspSdkLoadStartModule("nand_updater.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error %08X loading/starting nand_updater.prx\n", mod );
    r = -1;
  }

  mod = pspSdkLoadStartModule("lfatfs_updater.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error %08X loading/starting lfatfs_updater.prx\n", mod );
    r = -1;
  }

  mod = pspSdkLoadStartModule("lflash_fatfmt_updater.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    printf( " Error %08X loading/starting lflash_fatfmt_updater.prx\n", mod );
    r = -1;
  }

  return r;
}


int LoadCachedUpdaterModules( int fw, char* pbp )
{
  SceUID mod;
  int r;
  char s[128];
  char t[16];
  int AllLoaded = 1;

  if( fw == 0 )
  {
    sprintf( t, "%s", pbp );
  } else {
    sprintf( t, "%03i.PBP", fw );
  }

  r = 0;

  sprintf( s, "ms0:/PSP/SYSTEM/UPDPRX/%s/nand_updater.prx", t );
  mod = pspSdkLoadStartModule(s, PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    AllLoaded = 0;
  }

  sprintf( s, "ms0:/PSP/SYSTEM/UPDPRX/%s/lfatfs_updater.prx", t );
  mod = pspSdkLoadStartModule(s, PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    AllLoaded = 0;
  }

  sprintf( s, "ms0:/PSP/SYSTEM/UPDPRX/%s/lflash_fatfmt_updater.prx", t );
  mod = pspSdkLoadStartModule(s, PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    AllLoaded = 0;
  }

  if( AllLoaded == 1 )
  {
    printf( "Updater modules loaded from cache!\n" );
    CachedUpdaterModulesLoaded = 1;
  }

  return r;
}


void MakeFlashFolders( void )
{
  int r;
  SceIoStat fstat;

  r = sceIoMkdir("flash0:/data", 0777);
  r = sceIoMkdir("flash0:/dic", 0777);
  r = sceIoMkdir("flash0:/font", 0777);
  r = sceIoMkdir("flash0:/./kd", 0777);
  r = sceIoMkdir("flash0:/vsh", 0777);
  r = sceIoMkdir("flash0:/data/cert", 0777);
  r = sceIoMkdir("flash0:/./kd/resource", 0777);
  r = sceIoMkdir("flash0:/vsh/etc", 0777);
  r = sceIoMkdir("flash0:/./vsh/module", 0777);
  r = sceIoMkdir("flash0:/vsh/resource", 0777);
  r = sceIoMkdir("flash1:/dic", 0777);
  r = sceIoMkdir("flash1:/gps", 0777);
  r = sceIoMkdir("flash1:/net", 0777);
  r = sceIoMkdir("flash1:/net/http", 0777);
  r = sceIoMkdir("flash1:/registry", 0777);
  r = sceIoMkdir("flash1:/vsh", 0777);
  r = sceIoMkdir("flash1:/vsh/theme", 0777);

  sceIoGetstat("flash0:/./kd", &fstat);
  fstat.st_mode = 0x116d;
  fstat.st_attr = 0x17;
  sceIoChstat("flash0:/./kd", &fstat, 0x00FFFFFF);

  sceIoGetstat("flash0:/./kd/resource", &fstat);
  fstat.st_mode = 0x116d;
  fstat.st_attr = 0x17;
  sceIoChstat("flash0:/./kd/resource", &fstat, 0x00FFFFFF);

  sceIoGetstat("flash0:/./vsh/module", &fstat);
  fstat.st_mode = 0x116d;
  fstat.st_attr = 0x17;
  sceIoChstat("flash0:/./vsh/module", &fstat, 0x00FFFFFF);

  sceIoGetstat("flash0:/data", &fstat);
  fstat.st_mode = 0x116d;
  fstat.st_attr = 0x17;
  sceIoChstat("flash0:/data", &fstat, 0x00FFFFFF);

  sceIoGetstat("flash0:/data/cert", &fstat);
  fstat.st_mode = 0x116d;
  fstat.st_attr = 0x17;
  sceIoChstat("flash0:/data/cert", &fstat, 0x00FFFFFF);
}


void CopyAdditionalFwPackFiles( void )
{
  // int i;
  int r;
  int l;
  char* srcfile;
  char* dstfile;
  u8* fbuf;
  
  r = hcReadFile( FwPackPath, dataPSAR, 10000000 );
  if (r <= 0)
  {
    printf( " Error 0x%08X reading %s.\n", r, FwPackPath );
    return;
  }
  l = r; 
  
  r = hcMiscSeqGetFile( dataPSAR, l-4, "idx_flist", dataOut );
  if (r <= 0)
  {
    printf( " NOTICE: No file index found for additional files.\n" );
    return;
  }
  fbuf = dataOut + r + 1;
  
  srcfile = (char*)dataOut;
  r = 1;
  while( r != 0 )
  {
    dstfile = srcfile + strlen(srcfile) + 1;
    r = hcMiscSeqGetFile( dataPSAR, l, srcfile, fbuf );
    hcWriteFile( dstfile, fbuf, r );
    srcfile = dstfile + strlen(dstfile) + 1;
    r = strlen(srcfile);
  }
}


void CopyAdditionalM33Files( int psptype )
{
  int i;
  int r;
  u8* p;
  int l;
  int o;
  char s[128];
  char seqfile[128];
  
  if( processingFwPack == 1 )
  {
    CopyAdditionalFwPackFiles();
    return;
  }

  l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;

  if( psptype == 1000 )
  {
    i = 0;
    while( 1 )
    {
      if( TargetFW == 371 )
      {
        sprintf( seqfile, "%s", m33files1000[i] );
      } else {
        sprintf( seqfile, "%i_%s", TargetFW, m33files1000[i] );
      }
      r = hcMiscSeqGetFile( p, l, seqfile, dataOut );
      sprintf( s, "flash0:/.%s%s", m33files1000[i+1], m33files1000[i] );
      r = hcWriteFile( s, dataOut, r );
      i += 2;
      if( strcmp(m33files1000[i], "0") == 0 ) { break; }
    }
  }

  if( psptype == 2000 )
  {
    i = 0;
    while( 1 )
    {
      if( TargetFW == 371 )
      {
        sprintf( seqfile, "%s", m33files2000[i] );
      } else {
        sprintf( seqfile, "%i_%s", TargetFW, m33files2000[i] );
      }
      r = hcMiscSeqGetFile( p, l, seqfile, dataOut );
      if( ((TargetFW == 401) || (TargetFW == 500)) && (strcmp(m33files2000[i], "vshctrl_02g.prx") == 0) )
      {
        sprintf( s, "flash0:%svshctrl.prx", m33files2000[i+1] );
      } else {
        sprintf( s, "flash0:%s%s", m33files2000[i+1], m33files2000[i] );
      }
      r = hcWriteFile( s, dataOut, r );
      i += 2;
      if( strcmp(m33files2000[i], "0") == 0 ) { break; }
    }
  }
}


int BuildIpl( u8* buffer, int psptype, char* BackupPath )
{
  int r;
  u8* p;
  int l;
  int o;
  char cpl1[32];
  char cpl2[32];
  char s[128];

  if( TargetFW == 0 )
  {
    sprintf( s, "%s/nandipl.bin", BackupPath );
    r = hcReadFile( s, buffer, 2000000 );
    return r;
  }
  
  if( TargetFW == 371 )
  {
    sprintf( cpl1, "cpl1000.bin" );
    sprintf( cpl2, "cpl2000.bin" );
  } else {
    sprintf( cpl1, "cpl1%03i.bin", TargetFW );
    sprintf( cpl2, "cpl2%03i.bin", TargetFW );
  }

  /*if( TargetFW == 401 )
  {
    sprintf( cpl1, "cpl1401.bin" );
    sprintf( cpl2, "cpl2401.bin" );
  } else if( TargetFW == 500 ) {
    sprintf( cpl1, "cpl1500.bin" );
    sprintf( cpl2, "cpl2500.bin" );
  } else if( TargetFW == 380 ) {
    sprintf( cpl1, "cpl1380.bin" );
    sprintf( cpl2, "cpl2380.bin" );
  } else if( TargetFW == 390 ) {
    sprintf( cpl1, "cpl1390.bin" );
    sprintf( cpl2, "cpl2390.bin" );
  } else {
    sprintf( cpl1, "cpl1000.bin" );
    sprintf( cpl2, "cpl2000.bin" );
  }*/

  r = 0;

  if( processingFwPack == 0)
  {
    l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
    if (l <= 0)
    {
      printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
    }
    o = *(u32 *)&dataPSAR[0x24];
    p = dataPSAR + o;
    l = l-o;
  } else {
    l = hcReadFile( FwPackPath, dataPSAR, 10000000 );
    if (l <= 0)
    {
      printf( " Error 0x%08X reading %s.\n", l, FwPackPath );
    }
    o = 0;
    p = dataPSAR;
  }
  
  if( psptype == 1000 )
  {
    if( fwNoCustomIpl == 0 )
    {
      r = hcMiscSeqGetFile( p, l, cpl1, dataOut );
    } else {
      r = 0;
    }
    l = hcReadFile( "sceNandIpl_1k.bin", dataOut + r, 512000 );
    l += r;
  }

  if( psptype == 2000 )
  {
    if( fwNoCustomIpl == 0 )
    {
      r = hcMiscSeqGetFile( p, l, cpl2, dataOut );
    } else {
      r = 0;
    }
    l = hcReadFile( "sceNandIpl_2k.bin", dataOut + r, 512000 );
    l += r;
  }

  if( NoCleanups == 0 )
  {
    sceIoRemove( "sceNandIpl_1k.bin" );
    sceIoRemove( "sceNandIpl_2k.bin" );
  }

  return l;
}

int GetMySig( void )
{
  int r;
  u8* p;
  int l;
  int o;
  int d;

  r = 0;

  l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;

  r = hcMiscSeqGetFile( p, l, "00000004.bin", dataOut );
  r = *(u32*)dataOut;
  d = *(u32*)(dataOut+4);
#ifdef PANDORA
  r = *(u32*)(dataOut+8);
#endif

  if( d == 0xCA004843)
  {
    return -2;
  } else {
    return r;
  }
}

int ModCheck( void )
{
  SceUID f;
  int dpsp_o, psar_o, dpsp_l;

#ifdef PANDORA
  f = sceIoOpen( "rflash.elf", PSP_O_RDONLY, 0777);
#else
  if( RunningAsDDC7 == 1 )
  {
    f = sceIoOpen( "flash0:/kd/resurrection.prx", PSP_O_RDONLY, 0777);
  } else {
    f = sceIoOpen( "EBOOT.PBP", PSP_O_RDONLY, 0777);
  }
#endif
  if (f<0)
  {
    printf("\nERROR OPENING ./EBOOT.PBP! (0x%08x)\n", f);
    return -1;
  }

#ifdef PANDORA
  dpsp_l = sceIoRead(f, dataPSAR, 3000000);
#else
  if( RunningAsDDC7 == 1 )
  {
    dpsp_l = sceIoRead(f, dataPSAR, 3000000);
  } else {
    sceIoRead(f, dataPSAR, 40);

    dpsp_o = *(u32 *)&dataPSAR[0x20];
    psar_o = *(u32 *)&dataPSAR[0x24];
    dpsp_l = psar_o - dpsp_o;

    sceIoLseek(f, dpsp_o, SEEK_SET);
    sceIoRead(f, dataPSAR, dpsp_l);
    sceIoClose( f );
  }
#endif

  return _moduleCheck( (void*)dataPSAR, (void*)(dataPSAR+dpsp_l) );
}


void RemountFlashs( void )
{
  int r;

  PrintNote( "  * Reassinging flashs...." );

  r = hcRfHelperIoBridgeUnassign( "flash0:" );
  r = hcRfHelperIoBridgeUnassign( "flash1:" );
  r = hcRfHelperIoBridgeUnassign( "flash2:" );
  r = hcRfHelperIoBridgeUnassign( "flash3:" );

  r = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
  r = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
  r = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);
  r = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, NULL, 0);

  ClearNote();
}


int ReloadDecrypter( void )
{
  int r;
  u8* p;
  int l;
  int o;
  SceUID mod2;

  r = hcRfHelperIoBridgeUnloadModule( "pspDecrypt_Module" );
  if( r<0 )
  {
    printf( "ERROR reloading decrypter module! (0x%08x)\n", r);
    return -1;
  }

  l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;

  r = hcMiscSeqGetFile( p, l, "000000B2.bin", dataOut );
  r = hcWriteFile("pspdecrypt.prx", dataOut, r);

  mod2 = pspSdkLoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod2 < 0)
  {
    printf( "ERROR reloading decrypter module! (0x%08x)\n", mod2);
    sceIoRemove( "pspdecrypt.prx" );
    return -1;
  }

  if( NoCleanups == 0 )
  {
    sceIoRemove( "pspdecrypt.prx" );
  }

  return 0;
}


int GetBackupSlot( char* returnpath, int IsSave )
{
  int r;
  int i;
  pspTime tm;
  char s[32];

  r = hcReadEBOOTList( "ms0:/flash_backup", (char*)dataPSAR );

  if( (r == 0) && (IsSave == 0) )
  {
    PrintNote( "  There are no backups to restore from!" );
    sceKernelDelayThread( 5000000 );
    return -1;
  }

  hcMiscMenuClear();
  hcMiscMenuSetName( "Backup Slots:" );
  for( i = 0; i < r; i++ )
  {
    hcMiscMenuAddEntry( (char*)(dataPSAR+(i*64)), i+1, " ");
  }
  if( r > 0) { hcMiscMenuAddEntry( "----------------------------------------", -1, " " ); }
  if( IsSave == 1 )
  {
    hcMiscMenuAddEntry( "create new backup set", 99, "  Create a new backup w/o overwriting an existing one" );
  }
  hcMiscMenuAddEntry( "CANCEL", 98, "  Do nothing and go back to the mainmenu...." );
  ClearScreen();
  printf( "\n" );
  printf( "        *  Your Backups  *\n" );
  printf( "\n" );
  printf( "   You can choose an existing backup set/slot to be overwritten\n" );
  printf( "   or to create a new one.\n" );
  r = hcMiscMenuShowMenu( 5, 9 );

  if( r == 98 )
  {
    return 98;
  }

  if( r == 99 )
  {
  	sceRtcGetCurrentClockLocalTime( &tm );
  	if( OnHEN == 1 )
  	{
  	  sprintf( s, "PSP%04i_FW%03i_%02i-%02i_[OFW!HEN]", gPspType, hcGetFwVerAsInt(), tm.day, tm.month );
    } else {
    	sprintf( s, "PSP%04i_FW%03i_%02i-%02i", gPspType, hcGetFwVerAsInt(), tm.day, tm.month );
    }
    i = 0;
    while(1)
    {
      i++;
      sprintf( returnpath, "ms0:/flash_backup/%s_(%02i)/bakinfo.dat", s, i );
      r = sceIoOpen( returnpath, PSP_O_RDONLY, 0777 );
      sceIoClose( r );
      if( r < 0 )
      {
        sprintf( returnpath, "ms0:/flash_backup/%s_(%02i)", s, i );
        break;
      }
    }
  } else {
    sprintf( returnpath, "ms0:/flash_backup/%s", (char*)(dataPSAR+((r-1)*64)) );
  }

  return 0;
}


void DoCacheUpdaterPrx( void )
{
  ClearScreen();
  printf( "\n\n\n" );
  printf( " Please wait, caching sepecial modules from all Updater .PBPs,\n found in memstick root, for use with firmware restores.\n\n" );
  hcCacheUpdaterPRX( "ms0:/", dataPSAR );
  printf( "\n\n You can delete the .PBPs now from your memstick to save space,\n if you don't need them otherwise.\n\n   Press any button...." );
  hcMiscWaitForAnyButton();
}



///////////////////
//               //
//   Main Code   //
//               //
///////////////////

void DoCFWInstall( int PspType, char* sPspType )
{
  int r;
  u8* p;
  int l;
  int o;
  int i;
  int abort = 0;
  int donttouchsettings = 0;
  char s[256];
  char t[256];
  char backpath[256];
  int btn;
  SceCtrlData pad;
  int Restore150 = 0;
  int FmtUseUpd = 1;

  if( scePowerGetBatteryLifePercent() < 50 )
  {
    PrintNote( "  For savety reasons (this isn't Pandora ;) ) the battery\n  needs to be at least 50pct charged!  (hold R+L to override)" );
    sceKernelDelayThread( 6000000 );
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
    sceCtrlReadBufferPositive( &pad, 1 );
    btn = pad.Buttons & 0xFFFF;
    if( btn != (PSP_CTRL_LTRIGGER + PSP_CTRL_RTRIGGER) ) { return; }
    PrintNote( "  MANUAL OVERRIDE ENGAGED!\n  Don't say you haven't been warned! ;-)" );
    sceKernelDelayThread( 3000000 );
  }

  if( TargetFW == 0 )
  {
  	r = GetBackupSlot( backpath, 0 );
    if( r == -1 ) { return; }  // no backups found
    if( r == 98 ) { return; }  // user choose "cancel"
  }

  ClearScreen();
  PrintNote( " PSAR extraction based on M33 PSARDumper // IPL flashing by DAX\n thanx to JumpR, cory, jas0nuk and all the others at LAN.ST" );
  if( processingFwPack == 0 )
  {
    printf( "    Custom Firmware (re)Installation\n" );
  } else {
    printf( "    >> Firmware-Pack Installation <<\n" );
  }
  printf( "  --------------------------------------\n" );
  printf( "\n" );
  if( TargetOFW == 0)
  {
    if( TargetFW != 0 )
    {
    	if( processingFwPack == 0 )
    	{
        printf( "   From here you can install a fresh copy of a M33 CFW.\n" );
        printf( "   Additionally, you can choose whether or not to format (reset)\n" );
        printf( "   the flash partitions containing your settings, so you can\n" );
        printf( "   flash the firmware without loosing your settings.\n" );
      } else {
        printf( (char*)dataOut );
      }
    } else {
      printf( "   You are about to restore a complete firmware from a backup!\n" );
      printf( "\n" );
      printf( "   Please not that this is a bit more dangerous that installing a\n" );
      printf( "   fresh one, as we can't know what has been done to the files!\n" );
    }
  } else {
  	ClearScreen();
    printf( "\n  ***  Installation of ORIGINAL firmware  ***\n\n" );
    printf( "  You are about to install an ORIGINAL firmware, just like Sony\n" );
    printf( "  intended it to be!\n\n\n" );
    pspDebugScreenSetTextColor( 0x0000FFFF );
    printf( "  /!\\  WARNING  /!\\\n\n" );
    printf( "  After installing OFW you CAN NOT run ANY homebrew, you CAN NOT\n" );
    printf( "  easiely down- or crossgrade (only using Pandora)!!!\n" );
  }

  hcMiscMenuClear();
  if( TargetOFW == 0 )
  {
    if( TargetFW != 0 )
    {
    	hcMiscMenuSetName( "**  CFW Installation  **" );
      hcMiscMenuAddEntry( "reset (format) settings as well", 11, "   This will completely wipe all current settings.\n   (system settings, PSN activation, e.t.c.)\n   After reboot you will get the \"initial setup\" dialog.");
      hcMiscMenuAddEntry( "do not reset (format) settings", 12, "   This will keep all your settings.\n   The new install will run with all current settings.");
      hcMiscMenuAddEntry( " ", -1, " ");
      hcMiscMenuAddEntry( "I changed my mind, let me out....", 13, "   --> back to main menu");
    } else {
      sprintf( t, "%s/bakinfo.dat", backpath );
      hcReadFile( t, s, 256 );
      r = *(int*)&s[0];
      if( r != PspType )
      {
        printf( "\n\n\n ERROR:\n\n The backup can not be restored to this PSP!\n\n The backup was made on a different model!\n Flashing it to this PSP would result in brickage!\n We don't want that, do we?  Press any button...." );
        hcMiscWaitForAnyButton();
        return;
      }
      hcMiscMenuSetName( "**  CFW Restore  **" );
      hcMiscMenuAddEntry( "Let's go!", 11, "   This will start the restore process....\n   Settings will be purged as well!");
      hcMiscMenuAddEntry( " ", -1, " ");
      hcMiscMenuAddEntry( "Uhm, nah, quit back to mainmenu....", 13, "   --> back to main menu");
    }
    if( processingFwPack == 0 )
    {
      r = hcMiscMenuShowMenu( 7, 12 );
    } else {
      r = hcMiscMenuShowMenu( 7, 16 );
    }
    if( r == 11 ) { donttouchsettings = 0; }
    if( r == 12 ) { donttouchsettings = 1; }
    if( r == 13 ) { return; }
  } else {
    hcMiscMenuSetName( "**  /!\\ ORIGINAL Firmware Installation  **" );
    hcMiscMenuAddEntry( "NO! NOOO! Do NOT install an original FW!", 1, " " );
    hcMiscMenuAddEntry( "YES! I know what I'm doing and why, INSTALL OFW!", 2, "  YOU PERFORM THIS ACTION KNOWING ABOUT THE CONSEQUENSES\n  AND TOTALLY ON YOUR OWN!!!\n\n  DON'T SHOUT AT ME IF IT DOESNT WORK OUT!" );
    r = hcMiscMenuShowMenu( 7, 19 );
    if( r == 1 ) { return; }
    donttouchsettings = 0;
  }

  if( donttouchsettings == 1 )
  {
    ClearScreen();
    pspDebugScreenSetTextColor(0x0000FFFF);
    printf( "\n    ** Warning **\n\n" );
    pspDebugScreenSetTextColor(0x00FFFFFF);
    printf( "  You've chosen not to reset the settings\n\n" );
    pspDebugScreenSetTextColor(0x00CCCCCC);
    printf( "  Please note, that the firmware can get confused and problems\n" );
    printf( "  may arise when keeping settings but flashing a different\n" );
    printf( "  FW version than the one the settings came from\n\n" );
    printf( "  If flashing the same FW version again, all should be fine,\n" );
    printf( "  otherwise expect at least the \"settings corrupted\" screen on\n" );
    printf( "  next boot." );
    hcMiscMenuClear();
    hcMiscMenuSetName( "Really keep settings?" );
    hcMiscMenuAddEntry( "Yes, keep settings, I have faith!", 11, "   Continue and don't reset settings");
    hcMiscMenuAddEntry( "No, RESET settings and play it safe", 12, "   Continue and RESET/FORMAT settings during install");
    r = hcMiscMenuShowMenu( 4, 16 );
    if( r == 11 ) { donttouchsettings = 1; }
    if( r == 12 ) { donttouchsettings = 0; }
  }

  ClearScreen();
  if( TargetFW != 0 )
  {
    sprintf( s, "%i.PBP", TargetFW );
    pspDebugScreenSetTextColor(0x00FFFFFF);
    printf( "Installing firmware - PSP model: %s\nUpdater used to extract FW: %s\n\n", sPspType, s );
    pspDebugScreenSetTextColor(0x00CCCCCC);
  } else {
    sprintf( t, "%s/f0/vsh/etc/version.txt", backpath );
    r = hcReadFile( t, dataPSAR, 1024 );
    s[10] = dataPSAR[8];
    s[11] = dataPSAR[10];
    s[12] = dataPSAR[11];
    s[13] = 0;
    dataPSAR[12] = 0;
    sprintf( s, "%s.PBP", s+10 );
    if( strcmp( s, "360.PBP" ) == 0 )
    {
      sprintf( s, "352.PBP" );
    }
    pspDebugScreenSetTextColor(0x0000FFFF);
    printf( " Please remember that you need to have the official updater of\n" );
    printf( " the FW you are about the restore on the memstick as well!\n" );
    printf( "\n" );
    pspDebugScreenSetTextColor(0x00FFFFFF);
    printf( " FW to be restored: %s    PBP required: %s\n\n", dataPSAR+8, s );
    pspDebugScreenSetTextColor(0x00CCCCCC);
    if( strcmp("150.PBP", s) == 0 ) { Restore150 = 1; }
  }

#ifdef PANDORA
  if( Restore150 == 1 )
  {
    printf("\n\n\n   ERROR!\n\n Can not restore firmware 1.50 while running from\n Pandora!\n\n Sorry.... press any button....\n" );
    PrintNote( "  Press any button to return to mainmenu...." );
    hcMiscWaitForAnyButton();
    return;
  }
#endif
  if( (OnTM == 1) && (Restore150 == 1))
  {
    printf("\n\n\n   ERROR!\n\n Can not restore firmware 1.50 while running from\n TimeMachine!\n\n Sorry.... press any button....\n" );
    PrintNote( "  Press any button to return to mainmenu...." );
    hcMiscWaitForAnyButton();
    return;
  }
  if( (OnHEN == 1) && (Restore150 == 1))
  {
    printf("\n\n\n   ERROR!\n\n Can not restore firmware 1.50 while running from\n HEN mode!\n\n Sorry.... press any button....\n" );
    PrintNote( "  Press any button to return to mainmenu...." );
    hcMiscWaitForAnyButton();
    return;
  }
  /*if( Restore150 != 1 )
  {
    printf( "Extracting special updater .PRX....\n" );
    hcExtractUpdaterSepcialPRX( dataPSAR, s );
  }*/

#ifndef PANDORA
  if( (TargetFW < 270) && (TargetFW > 0) )
  {
  	printf( "Preloading flash-format module from running firmware....\n" );
    r = pspSdkLoadStartModule("flash0:/kd/lflash_fatfmt.prx", PSP_MEMORY_PARTITION_KERNEL);
    FmtUseUpd = 0;
  }
  printf( "Unassigning flashs....\n" );
  r = hcRfHelperIoBridgeUnassign( "flash0:" );
  if( r<0 )
  {
    printf( "FATAL: Error 0x%08x while unassigning flash0:\n", r );
    abort = 1;
  }
  r = hcRfHelperIoBridgeUnassign( "flash1:" );
  if( r<0 )
  {
    printf( "WARNING: Error 0x%08x while unassigning flash1:\n * Disabling settings format, clearing registry!\n", r );
    donttouchsettings = 2;
    sceIoRemove( "flash1:/registry/system.dreg" );
    sceIoRemove( "flash1:/registry/system.ireg" );
    sceIoRemove( "flash1:/config.se" );
  }
  r = hcRfHelperIoBridgeUnassign( "flash2:" );
  if( r<0 )
  {
    if( r == 0x80020321 )
    {
      printf( "flash2: wasn't assinged - that's OK\n" );
    } else {
      printf( "Weird: Error 0x%08x while unassigning flash2:\n", r );
    }
  }
  r = hcRfHelperIoBridgeUnassign( "flash3:" );
  if( r<0 )
  {
    if( r == 0x80020321 )
    {
      printf( "flash3: wasn't assinged - that's OK\n" );
    } else {
      printf( "Weird: Error 0x%08x while unassigning flash3:\n", r );
    }
  }
#else
  // quick run, w/o errorchecking, good enough for a pandora boot....
  r = hcRfHelperIoBridgeUnassign( "flash0:" );
  r = hcRfHelperIoBridgeUnassign( "flash1:" );
  r = hcRfHelperIoBridgeUnassign( "flash2:" );
  r = hcRfHelperIoBridgeUnassign( "flash3:" );
#endif

  if( Restore150 != 1 )
  {
    LoadCachedUpdaterModules( TargetFW, s );
    if( CachedUpdaterModulesLoaded == 0 )
    {
      printf( "Extracting special updater .PRX....\n" );
      hcExtractUpdaterSepcialPRX( dataPSAR, s );
      printf( "Loading updater modules\n" );
      if( (TargetFW >= 270) || (TargetFW == 0) )
      {
        r = LoadUpdaterModules();
      } else {
        printf( " (using preloaded module....)\n" );
        if( OnTM == 1 )
        {
          printf( "ERROR: Target configuration not compatible with installing\n       from out of TimeMachine/DDC!\n" );
          abort = 1;
        }
        if( OnHEN == 1 )
        {
          printf( "ERROR: Target configuration not compatible with installing\n       from out of HEN!\n" );
          abort = 1;
        }
        r = 0;
      }
    } else {
      r = 0;
    }
  } else {
    r = hcRfHelperIoBridgeUnloadModule( "sceLflashFatfmt" );
    if( r<0 )
    {
      //printf( "No current Fatfmt module to unload! (0x%08x)\n", r);
      //abort = 1;
    }
    sprintf( t, "%s/f0/kd/lflash_fatfmt.prx", backpath );
    r = pspSdkLoadStartModule(t, PSP_MEMORY_PARTITION_KERNEL);
    if (r < 0)
    {
      printf( "\nERROR loading 1.50 Fatfmt module! (0x%08x)\n", r);
      abort = 1;
    } else {
      printf( "\nLoaded 1.50 Fatfmt module....\n" );
      FmtUseUpd = 0;
    }
    r = 0;
  }
  CleanupUpdaterModules();

  if( TargetFW != 371 )
  {
    // for anything higher than 3.71 we need a new(er) decrypt.prx
    // so, scrap the current one and load the other one.
    if( ReloadDecrypter() != 0 ) { abort = 1; }
  }

  if( r<0 ) { abort = 1; }

  if( abort == 1 )
  {
    PrintNote( " There have been critical errors!\n ABORTING!!! No harm has been done! Press any button...." );
    hcMiscWaitForAnyButton();
    return;
  }

  // some special patching for model 1000 (a.k.a. Fat) PSPs
  if( (PspType == 1000) && (Restore150 != 1) && (TargetOFW == 0) && (fwNoCustomIpl == 0) && ((TargetFW >= 270) || (TargetFW == 0)) )
  {
  	if( TargetFW == 0 )
  	{
  	  // check if backup contains OFW
  	  memset( dataOut, 0xFF, 1024 );
  	  sprintf( s, "%s/nandipl.bin", backpath );
      r = hcReadFile( s, dataOut, 1024 );
    
      l = 0;
      for( i=0; i<=1024; i++)
      {
        r = *(u8*)(dataOut+i) & 0xFF;
        if( r == 0 ) { l++; }
      }
      
      sprintf( s, "%s/f0/kn/loadexec.prx", backpath );
      r = sceIoOpen(s, PSP_O_RDONLY, 0777);
      sceIoClose( r );
      if( r<0 )
      {
        sprintf( s, "%s/f0/kn/$loadexec.prx", backpath );
        r = sceIoOpen(s, PSP_O_RDONLY, 0777);
        sceIoClose( r );
      }
  
      if( l > 256 )
      {
    	  // if backup contains CFW, patch the flash driver to disable encryption
        hcRfHelperPatchLFatEncryption( 0 );
      } else {
      	if( r < 0 )
      	{
    	    // if backup contains OFW, leave all as is
    	    printf( "- skipping flash encryption patch, backup contains OFW!\n" );
        } else {
        	// uh, oh, the backup contains an older SE/OE CFW - let's patch!
        	printf( "- SE/OE CFW detected....\n" );
        	hcRfHelperPatchLFatEncryption( 0 );
        }
      }
    } else {
      // installing CFW - just and always apply patch
      hcRfHelperPatchLFatEncryption( 0 );
    }
  }

  if( TargetFW != 0 )
  {
    printf( "Checking updater .PBP.... " );
    
    // Updater .PBP checksum check
    if( processingFwPack == 0)
    {
      l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
      if (l <= 0)
      {
        printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
      }
      o = *(u32 *)&dataPSAR[0x24];
      p = dataPSAR + o;
      l = l-o;
    } else {
      l = hcReadFile( FwPackPath, dataPSAR, 10000000 );
      if (l <= 0)
      {
        printf( " Error 0x%08X reading %s.\n", l, FwPackPath );
      }
      o = 0;
      p = dataPSAR;
    }
    sprintf( s+128, "%03ipbp.chk", TargetFW );
    r = hcMiscSeqGetFile( p, l, s+128, dataOut );
    o = *(int*)dataOut;
    sprintf( s+144, "ms0:/PSP/GAME/RECOVERY/%03i.PBP", TargetFW );
    r = hcGetFileChksum( s+144, dataOut );
    if( r<0 )
    {
      sprintf( s+144, "ms0:/%03i.PBP", TargetFW );
      r = hcGetFileChksum( s+144, dataOut );
    }
    if( o == r )
    {
      printf( "checksum OK.... " );
    } else {
      printf( "CHECKSUM FAILED!\n\n  Expected: 0x%08x\n  Got:      0x%08x\n\nCanceling install!\n", o, r );
      PrintNote( "  press any button...." );
      hcMiscWaitForAnyButton();
      return;
    }
    
    psarInitPsarDumper( dataPSAR, dataOut, PspType );
    psarSetPBPPath( s );
    if( fwflagOverride != -1 ) { psarSetFwFlag( fwflagOverride ); }
    r = psarCopyFwFiles( 1 );
    if( r == 0)
    {
      printf( "PBP looks good.\n" );
    } else {
      printf( "CHECK FAILED!\nThere seems to be some sort of problem with the .PBP (see above)\n\nReturning to main menu...." );
      PrintNote( " There have been critical errors!\n ABORTING!!! No harm has been done! Press any button...." );
      hcMiscWaitForAnyButton();
      return;
    }

    if( VerifyPBPVersion( TargetFW ) == 0 )
    {
      PrintNote( "  Can not continue, no harm has been done!\n  Press any button to go back to the mainmenu...." );
      hcMiscWaitForAnyButton();
      return;
    }
  }

  sceKernelDelayThread( 1500000 );

  printf( "\ngoing hot now: Formatting flashs.... " );
  r = hcRfHelperIoBridgeLFatfmt( 0, FmtUseUpd );
  if ( r < 0 )
  {
    printf( "\nPANIC: Error formatting flash0!  (0x%08x)\n", r );
  } else {
    printf( "0" );
  }
  if( donttouchsettings != 1 )
  {
    if( donttouchsettings != 2 )
    {
      r = hcRfHelperIoBridgeLFatfmt( 1, FmtUseUpd );
      if ( r < 0 )
      {
        printf( "\nWARNING: Error formatting flash1!  (0x%08x)\n", r );
      } else {
        printf( ", 1" );
      }
    }
    r = hcRfHelperIoBridgeLFatfmt( 2, FmtUseUpd );
    if ( r < 0 )
    {
      printf( "\nNote: Error formatting flash2!  (0x%08x)\n", r );
    } else {
      printf( ", 2" );
    }
    r = hcRfHelperIoBridgeLFatfmt( 3, FmtUseUpd );
    if ( r < 0 )
    {
      printf( "\nNote: Error formatting flash3!  (0x%08x)\n", r );
    } else {
      printf( ", 3" );
    }
  }
  printf( "\n" );

  printf( "Reassigning flashs....\n" );
  r = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
  if ( r < 0 )
  {
    printf( "\nPANIC: Error assigning flash0!  (0x%08x)\n", r );
  }
  r = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
  if ( r < 0 )
  {
    printf( "\nWARNING: Error assigning flash1!  (0x%08x)\n", r );
  }
  r = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);
  if ( r < 0 )
  {
    printf( "\nNote: Error assigning flash2!  (0x%08x)\n", r );
  }
  r = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, NULL, 0);
  if ( r < 0 )
  {
    printf( "\nNote: Error assigning flash3!  (0x%08x)\n", r );
  }

  if( TargetFW != 0 )
  {
    printf( "\nFinally.... installing firmware....\n" );
    MakeFlashFolders();
    psarInitPsarDumper( dataPSAR, dataOut, PspType );
    psarCopyFwFiles( 0 );
  } else {
    printf( "\nFinally.... restoring firmware....\n" );
    MakeFlashFolders();
    hcRfHelperInitMangleSyscall();
    sprintf( t, "%s/f0/data", backpath );
    hcCopyFolder( t, "flash0:/data", dataPSAR, 19000000, 1 );
    sprintf( t, "%s/f0/dic", backpath );
    hcCopyFolder( t, "flash0:/dic", dataPSAR, 19000000, 1 );
    sprintf( t, "%s/f0/font", backpath );
    hcCopyFolder( t, "flash0:/font", dataPSAR, 19000000, 1 );
    sprintf( t, "%s/f0/kd", backpath );
    hcCopyFolder( t, "flash0:/./kd", dataPSAR, 19000000, 1 );
    sprintf( t, "%s/f0/vsh", backpath );
    hcCopyFolder( t, "flash0:/./vsh", dataPSAR, 19000000, 1 );
    sprintf( t, "%s/f0/kn/$loadexec.prx", backpath );
    r = sceIoOpen(t, PSP_O_RDONLY, 0777);
    sceIoClose( r );
    if( r > 0 )
    {
      sprintf( t, "%s/f0/kn", backpath );
      hcCopyFolder( t, "flash0:/./kn", dataPSAR, 19000000, 1 );
    }
    sprintf( t, "%s/f0/km/loadexec.prx", backpath );
    r = sceIoOpen(t, PSP_O_RDONLY, 0777);
    sceIoClose( r );
    if( r > 0 )
    {
      sprintf( t, "%s/f0/km", backpath );
      hcCopyFolder( t, "flash0:/./km", dataPSAR, 19000000, 1 );
    }
  }

  if( TargetOFW == 0 )
  {
    PrintNote( "Performing final operations...." );
    if( TargetFW != 0 )
    {
      CopyAdditionalM33Files( PspType );
    }

    r = BuildIpl( dataOut, PspType, backpath );

    if(pspIplUpdate_282EDFEC() < 0)
    {
      printf( " ERROR clearing IPL\n" );
    } else {
      if(pspIplUpdate_F7EF8E3F( dataOut, r ) < 0)
      {
        printf( " ERROR writing IPL\n" );
      }
    }
    ClearNote();
  } else {
    PrintNote( "  Flashing IPL...." );
    
    r = BuildIpl( dataOut, PspType, backpath );

    if(pspIplUpdate_282EDFEC() < 0)
    {
      printf( " ERROR clearing IPL\n" );
    } else {
      if(pspIplUpdate_F7EF8E3F( dataOut+0x4000, r-0x4000 ) < 0)  // since BuildIpl() returns a CFW IPL
      {                                                     // incl. the leading 16k DAX code,
        printf( " ERROR writing IPL\n" );                   // we are "skipping" the first 16k of the
      }                                                     // result for OFW installation
    }
    
    ClearNote();
  }
  printf( "All done\n" );
  printf( "Press any button to shutdown...." );
  hcMiscWaitForAnyButton();
  hfRfHelperPoweroff();
  //scePowerRequestStandby();
  sceKernelDelayThread( 60000000 );
}


void NANDDumpOptions( int PspType, int DoDumpOnly )
{
  int r;
  int i;
  int l;
  int errc;
  int c;
  SceUID fd;
  char s[256];
  u8* user;
  u8* spare;
  int numblocks;   // blocks per write
  int needshutdown;
  char fname[128];
  int bar, tac, fuh, ful;
  pspTime tm;
  
  needshutdown = 0;
  
  if( DoDumpOnly != 1 )
  {
    ClearScreen();
  
    printf( "\n  ***  NAND dumping and writing  ***\n\n" );
    printf( "  /!\\  Please be very carefull with using theese options!\n" );
    printf( "  From here you can create complete dumps of the PSPs internal\n" );
    printf( "  NAND flash memory and/or write those dumps back into the NAND.\n" );
  }
  
  while( 1 )
  {
  	hcMiscMenuClear();
    hcMiscMenuSetName( "NAND Options" );
    hcMiscMenuAddEntry( "Dump NAND to file", 1, "  Does a physical byte-by-byte copy from the PSPs\n  NAND flash to ms0:/nand-dump.bin" );
    hcMiscMenuAddEntry( " ", -1, " " );
    hcMiscMenuAddEntry( "Restore LFlash (FW partitions) from dump", 3, "  Semi-Dangerous! * May cause BRICK!\n  This will write the part of a dump back to the NAND\n  containing the flashX:/ partitions." );
    hcMiscMenuAddEntry( "Restore IPL from dump", 4, "  Semi-Dangerous! * May cause BRICK!\n  This will write the IPL from the image back into\n  the NAND." );
    hcMiscMenuAddEntry( "Restore complete NAND image from dump", 2, "  /!\\ DANGEROUS!!! /!\\  only use dump of THIS PSP!!!\n  This writes the contents of ms0:/nand-dump.bin\n  directely into the PSPs NAND flash.\n  Including things like idstorage and other sensitive stuff!" );
    hcMiscMenuAddEntry( " ", -1, " " );
    if( needshutdown == 0 )
    {
      hcMiscMenuAddEntry( "> back to mainmenu", 9, " " );
    } else {
      hcMiscMenuAddEntry( "* Shutdown        ", 9, "  Due to writes to the NAND we need to shut down the PSP\n  before doing anthing else." );
    }
    if( DoDumpOnly != 1 ) { r = hcMiscMenuShowMenu( 10, 14 ); } else { r = 1; }
    	
    l = 0;
    if( PspType == 1000 ) { l = 2048; }
    if( PspType == 2000 ) { l = 4096; }
    if( PspType == 2500 ) { l = 4096; }
    if( PspType == 3000 ) { l = 4096; }
    
    if( r == 1 )
    {
    	PrintNote( "  Dumping NAND flash to ms....\n  Please wait...." );
      	
      sprintf( s+200, "          " );
      
      sceRtcGetCurrentClockLocalTime( &tm );
      hcRfHelperGetMoboVersions( &bar, &tac, &fuh, &ful );
      i = bar ^ tac ^ fuh ^ ful;
      c = ((i & 0xFFFF) ^ (i / 0x10000)) & 0xFFFF;
  	  sprintf( fname, "ms0:/flash_nanddumps/NAND%04i_FW%03i_%02i-%02i_%04x.bin", gPspType, hcGetFwVerAsInt(), tm.day, tm.month, c );
  	  
  	  sceIoMkdir("ms0:/flash_nanddumps", 0777);
    
      fd = sceIoOpen(fname, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      for( i=0; i<l; i+=64 )
      {
      	memset( s+200, 0x3D, ((i*100)/l)/10 );
      	sprintf( s, "  Dumping NAND flash to ms....   [%3ipct|%s]\n  Please wait....", (i*100)/l, s+200 );
      	PrintNote( s );
        r = hcRfHelperDumpNandBlockRangeToBuffer( dataOut, i, 64 );
        sceIoWrite(fd, dataOut, r);
      }
      sceIoClose( fd );
      
      sprintf( s+200, "          " );
      errc = 0;
      fd = sceIoOpen(fname, PSP_O_RDONLY, 0777);
      for( i=0; i<l; i+=1 )
      {
      	memset( s+200, 0x3D, ((i*100)/l)/10 );
        if( (i & 0x7) == 0 )
        {
      	  sprintf( s, "  Verifying NAND dump....   [%3ipct|%s]\n  Please wait....", (i*100)/l, s+200 );
      	  PrintNote( s );
      	}
      	memset( dataOut, 0xAA, r );
      	memset( dataOut+(r+1024), 0x55, r );
        r = hcRfHelperDumpNandBlockRangeToBuffer( dataOut, i, 1 );
        sceIoRead(fd, dataOut+(r+1024), r);
        c = memcmp ( (void*)dataOut, (void*)dataOut+(r+1024), r );
        if( c != 0 ) { errc++; }
      }
      sceIoClose( fd );
      
      memset( s+200, 0x3D, 10 );
      sprintf( s, "  Verifying NAND dump....   [%3ipct|%s]\n  Finished....", 100, s+200 );
      PrintNote( s );
      sceKernelDelayThread( 3000000 );
      
      if( errc < 11 )   // we allow an error ration up to 15, cause it's uncertain what happens on bad blocks
      {                // so, basically, we allow a number of bad blocks, more and verify fails
      	PrintNote( "  NAND dump has been successfully written to ms." );
      	sceKernelDelayThread( 3000000 );
      } else {
        PrintNote( "  *** VERIFY FAILED *** the dump may be corrupt or incomplete!\n  Memstick full?  -  Press any button to continue...." );
        hcMiscWaitForAnyButton();
      }
      
      ClearNote();
      if( DoDumpOnly == 1 ) { return; }
    }
  
    if( r == 2 )
    {
    	c = hcGetNandDumpPath( fname, (char*)dataOut );
    	
    	if( c != -1 )
    	{
        PrintNote( "  Writing complete image back to NAND!\n  Please wait...." );
        needshutdown = 1;
      	
        sprintf( s+200, "          " );
      
        numblocks = 64;    // fetch and process/write 64 blocks at each loop
        user = dataPSAR;
        spare = dataPSAR + (528*32*numblocks);
      
        fd = sceIoOpen( fname, PSP_O_RDONLY, 0777);
        for( i=0; i<l; i+=numblocks )
        {
      	  memset( s+200, 0x3D, ((i*100)/l)/10 );
      	  sprintf( s, "  Writing complete image back to NAND!   [%3ipct|%s]\n  Please wait....", (i*100)/l, s+200 );
      	  PrintNote( s );
      	
      	  memset( dataOut, 0xFF, 528*32*numblocks );
          r = sceIoRead( fd, dataOut, 528*32*numblocks );
        
          SplitNANDPages( dataOut, 32*numblocks, user, spare );
  
          // write all prepared blocks to the NAND....
          r = hcRfHelperWriteNandBlockRange( user, spare, i, numblocks );
        }
        sceIoClose( fd );
      
        memset( s+200, 0x3D, 10 );
        sprintf( s, "  Writing complete image back to NAND!   [%3ipct|%s]\n  Finished....", 100, s+200 );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
    }
    
    if( r == 3 )
    {
    	c = hcGetNandDumpPath( fname, (char*)dataOut );
    	
    	if( c != -1 )
    	{
        PrintNote( "  Writing LFlash back to NAND!\n  Please wait...." );
        needshutdown = 1;
      	
        sprintf( s+200, "          " );
      
        numblocks = 64;    // fetch and process/write 64 blocks at each loop
        user = dataPSAR;
        spare = dataPSAR + (528*32*numblocks);
      
        fd = sceIoOpen( fname, PSP_O_RDONLY, 0777);
        sceIoLseek( fd, 0x40*32*528, SEEK_SET );
        for( i=0x40; i<l; i+=numblocks )
        {
      	  memset( s+200, 0x3D, ((i*100)/l)/10 );
      	  sprintf( s, "  Writing LFlash back to NAND!   [%3ipct|%s]\n  Please wait....", (i*100)/l, s+200 );
      	  PrintNote( s );
      	
      	  memset( dataOut, 0xFF, 528*32*numblocks );
          r = sceIoRead( fd, dataOut, 528*32*numblocks );
        
          SplitNANDPages( dataOut, 32*numblocks, user, spare );
  
          // write all prepared blocks to the NAND....
          r = hcRfHelperWriteNandBlockRange( user, spare, i, numblocks );
        }
        sceIoClose( fd );
      
        memset( s+200, 0x3D, 10 );
        sprintf( s, "  Writing LFlash back to NAND!   [%3ipct|%s]\n  Finished....", 100, s+200 );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
    }
    
    if( r == 4 )
    {
    	c = hcGetNandDumpPath( fname, (char*)dataOut );
    	
    	if( c != -1 )
    	{
        PrintNote( "  Writing IPL back to NAND!\n  Please wait...." );
        needshutdown = 1;
      	
        sprintf( s+200, "          " );
      
        numblocks = 1;    // fetch and process/write 1 block at each loop
        user = dataPSAR;
        spare = dataPSAR + (528*32*numblocks);
      
        fd = sceIoOpen( fname, PSP_O_RDONLY, 0777);
        sceIoLseek( fd, 0x10*32*528, SEEK_SET );
        for( i=0x10; i<0x30; i+=numblocks )
        {
      	  //PrintNote( s );
      	
      	  memset( dataOut, 0xFF, 528*32*numblocks );
          r = sceIoRead( fd, dataOut, 528*32*numblocks );
        
          SplitNANDPages( dataOut, 32*numblocks, user, spare );
  
          // write all prepared blocks to the NAND....
          r = hcRfHelperWriteNandBlockRange( user, spare, i, numblocks );
        }
        sceIoClose( fd );
      
        sprintf( s, "  Writing IPL back to NAND!\n  Finished...." );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
    }
  
    if( r == 9 )
    {
      if( needshutdown == 0 )
      {
        return;
      } else {
      	PrintNote( "  Shutting down...." );
        scePowerRequestStandby();
        sceKernelDelayThread( 10000000 );
      }
    }
  }
}


void ExtractOFW150( void )
{
	int r;
	u8* p;
	int l;
	int o;
	
	ClearScreen();
	printf( "\n   **  Install of clean OFW 1.50  **\n\n\n" );
	printf( "  Installing OFW 1.50 is a two-step process here!\n" );
	printf( "  At first, I will create a new \"backup\" slot and extract\n" );
	printf( "  the firmware into it.\n" );
	printf( "  You can then use the \"Restore FW from backup\" function to\n" );
	printf( "  actually install the fresh'n'clean 1.50 to your PSP.\n\n" );
	printf( "  Press any button to start....\n" );
	hcMiscWaitForAnyButton();
	ClearScreen();
	pspDebugScreenSetTextColor( 0x00FFFFFF );
	printf( "\n\n   **  NOW PREPARING 1.50 DUMP  **\n\n\n" );
	pspDebugScreenSetTextColor(0x00CCCCCC);
	printf( "  I will now extract a complete set of OFW 1.50!\n\n" );
	printf( "  Once this step is finished, select\n   \"Restore complete FW from backup\"\n  from the main menu and select the backup named\n   \"OFW_1.50\"\n  for the restore\n\n" );
	printf( "  That will get you a fresh and clean 1.50 on your PSP.\n\n" );

  PrintNote( "  Extracting 1.50 firmware files.\n  Please wait...." );
    	
  TargetFW = 150;
  psarInitPsarDumper( dataPSAR, dataOut, 1000 );
  psarSetPBPPath( "150.PBP" );
  r = psarCopyFwFiles( 1 );
  if( r == 0 )
  {
    if( VerifyPBPVersion( TargetFW ) != 0 )
    {
      r = psarCopyFwFiles( 0 );
    }
  }
      
  PrintNote( "  Building 1.50 IPL...." );
      
  hcExtractUpdaterSepcialPRX( dataPSAR, "150.PBP" );
  r = hcReadFile( "ipl_update.prx", dataPSAR, 2000000 );
  sceIoRemove( "ipl_update.prx" );
  r = pspDecryptPRX( dataPSAR, dataOut, r );
  if (r <= 0) { printf( "ERROR while decrypting ipl_update.prx\n" ); }
  r = pspDecryptIPL1( dataOut+0x980, dataPSAR, 225280 );
  if (r <= 0) { printf( "ERROR while decrypting SCE IPL\n" ); }
      	
  u32 *code = (u32 *)(dataPSAR+0x10);
  code[0xE0/4] = 0x09038000; // j 0x040e0000
  code[0xE4/4] = 0; // nop
  memcpy( dataOut+0x4000, dataPSAR, 225280 );
    	
  l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;
  r = hcMiscSeqGetFile( p, l, "cpl1150.bin", dataOut );
    	
  hcWriteFile( "ms0:/flash_backup/OFW_1.50/nandipl.bin", dataOut, 225280+0x4000 );
      
  memset( dataOut, 0, 127 );
  *(int*)(&dataOut[0x00]) = 1000;
  hcWriteFile( "ms0:/flash_backup/OFW_1.50/bakinfo.dat", dataOut, 127 );
  
  PrintNote( "  Finished! Now use the new backup slot for installation!\n  Press any button...." );
  hcMiscWaitForAnyButton();
  ClearNote();
}


int main(int argc, char *argv[])
{
  int r;
  int i;
  int bar, tac, fuseh, fusel;
  int PspType;
  char sPspType[32];
  char s[256];
  int SkipDisclaimer;
  int SelectedFWH, SelectedFWL;
  int RFAdvancedMode = 0;
  SceCtrlData pad;
  u8* p;
  int l;
  int o;
  
  // init of some default settings
  SelectedFWH = 5;
  SelectedFWL = 00;
  TargetOFW = 0;

#ifdef LEGACY
  pspSdkInstallNoDeviceCheckPatch();
  pspSdkInstallNoPlainModuleCheckPatch();
  pspSdkInstallKernelLoadModulePatch();
#endif

  pspDebugScreenInit();
  TargetFW = 371;

#ifndef PANDORA
#ifndef LEGACY
  if (sceKernelDevkitVersion() < 0x02070100)
  {
    printf( "\n  This program needs at least 2.71 to work propperly!\n\nSorry.... exiting....\n\n\n\n\n If running on 3.52 or lower, launch this program from the XMB\n and make sure kernel is set to 3.xx.\n" );
    sceKernelDelayThread( 10000000 );
    sceKernelExitGame();
  }
#endif
#endif

  printf( "\n loading....\n\n" );

  dataPSAR = (u8 *)memalign(0x40, 19000000);
  if (!dataPSAR) { printf(" Cannot allocate memory for PSAR data.... (0x%08x)\n", (int)dataPSAR); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }
  dataOut = (u8 *)memalign(0x40, 3000000);
  if (!dataOut) { printf(" Cannot allocate memory for OUT data.... (0x%08x)\n", (int)dataOut); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }

  r = sceIoOpen( "EBOOT.PBP", PSP_O_RDONLY, 0777);
  sceIoClose( r );
  if( r > 0 )
  {
    sprintf( MyEBOOTPath, "EBOOT.PBP" );
  } else {
    r = sceIoOpen( "RFLASH.DAT", PSP_O_RDONLY, 0777);
    sceIoClose( r );
    if( r > 0 )
    {
      sprintf( MyEBOOTPath, "RFLASH.DAT" );
    } else {
      r = sceIoOpen( "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP", PSP_O_RDONLY, 0777);
      sceIoClose( r );
      if( r > 0 )
      {
        sprintf( MyEBOOTPath, "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP" );
      } else {
        sprintf( MyEBOOTPath, "nul:/" );
      }
    }
  }
#ifdef LEGACY
  sprintf( MyEBOOTPath, "RFLASH.DAT" );
#endif

  RunningAsDDC7 = 0;
  if( (hcCheckForDDC7() == 2) && (hcCheckForTM() == 1) )
  {
  	// running on DDC as ressurecion.prx replacement
  	RunningAsDDC7 = 1;
  	sceIoChdir( "ms0:/" );
  	sprintf( MyEBOOTPath, "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP" );
  }

  ExtractAdditionalModules();
  LoadAdditionalModules();
  CleanupAdditionalModules();

  //void scePower_0442D852( void );
  //printf( "\n\nTEMP ACTION....\n\n\n" );
  /*r = hcReadFile( "dcman.prx", dataPSAR, 20500000 );
  i = pspDecompress( dataPSAR+0x150, dataOut, 2000000 );
  printf( "0x%08x 0x%08x\n", r, i );
  hcWriteFile( "dcman_decompressed.prx", dataOut, i );
  sceKernelDelayThread( 5000000 );*/
  /*void (*scePower_0442D852)(void) = (void*)hcRfHelperPspFindProc("scePower_Service", "scePower", 0x0442D852);
  if(!scePower_0442D852) { printf( "whooops, function not found...." ); }
  scePower_0442D852();
  sceKernelDelayThread( 5000000 );
  sceKernelExitGame();*/
  
#ifndef PANDORA
#ifndef LEGACY
  if( argc != 0 )
  {
    r = strlen( argv[0] ) - 1;
    if( *(argv[0]+r) !=  0x32 )
    {
      r = hcRfHelperRebootToUpdater();
    }
  } else {
  	r = hcRfHelperRebootToUpdater();
  }
#endif
#endif

#ifndef LEGACY
  r = ModCheck();
  i = GetMySig();
  if( r != i )
  {
    if( i == -2 )
    {
      printf( "0x%08x  0x%08x\n", r, i );
    }
    printf( "\n\n*** ERROR  0x0BADC0DE !\n    can not continue....\n" );
    if( WIP != 1 )
    {
      sceKernelDelayThread( 10000000 );
      sceKernelExitGame();
    } else {
      printf( " \n  OVERRIDING ERROR, commencing app start....\n" );
    }
  }
#endif

  OnTM = hcCheckForTM();
  OnOE = hcCheckForOE();
  Has150AddOn = hcCheckFor150AddOn();
  OnHEN = hcCheckForHEN( dataOut );
  if( OnTM == 1 ) { OnHEN = 0; }  // on TM CheckForHEN could give a false positive, but when on TM, we can't be on HEN, so....
  if( OnOE == 1 ) { OnHEN = 0; }  // if we're on OE we can't be on HEN, can we?
  
  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
  sceCtrlReadBufferPositive( &pad, 1 );
  r = pad.Buttons & 0xFFFF;
  if( r == PSP_CTRL_SELECT + PSP_CTRL_LTRIGGER + PSP_CTRL_RTRIGGER )
  {
    pspDebugScreenSetXY(0,6);
    printf( "   ***   ADVAMCED MODE TRIGGER DETECTED   ***\n" );
    printf( "\n" );
    printf( "  Are you sure you want to enable advanced mode features?\n" );
    printf( "  Those function are meant for experienced users only and\n" );
    printf( "  may damage the system to the need of Pandora!\n" );
    printf( "\n" );
    printf( "  You are completely on your own with this!!!\n" );
    hcMiscMenuSetColors( 0x00CCCCCC, 0x00FFFFFF, 0x00FFFFFF, 0x00CCCCCC );
    hcMiscMenuClear();
    hcMiscMenuSetName( "Enable Advanced Mode?" );
    hcMiscMenuAddEntry( "NO!  Stay in NORMAL mode", 1, " " );
    hcMiscMenuAddEntry( "YES! Enable Advanced Mode, I know what I'm doing!", 2, " " );
    r = hcMiscMenuShowMenu( 10, 15 );
    
    if( r == 2 ) { RFAdvancedMode = 1; }
  }

  SkipDisclaimer = 0;
  hcReadFile( "ms0:/PSP/SYSTEM/rflash.spl", &SkipDisclaimer, sizeof(int));

  hcRfHelperGetMoboVersions( &bar, &tac, &fuseh, &fusel );
  if( tac >= 0x00500000 ) { PspType = 2000; } else { PspType = 1000; }
  if( tac >= 0x00600000 ) { PspType = 3000; }
  if( ( tac == 0x00600000 ) && ( bar == 0x00243000 ) ) { PspType = 2500; }
//PspType = 3000;
  printf( "\nPSP version: %i\n", PspType );
  if( PspType == 1000 ) { sprintf( sPspType, "Classic \"Fat\" PSP" ); }
  if( PspType == 2000 ) { sprintf( sPspType, "PSP Slim&Lite" ); }
  if( PspType == 2500 ) { sprintf( sPspType, "PSP Slim (TA088v3+)" ); }
  if( PspType == 3000 ) { sprintf( sPspType, "PSP Brite" ); }// { printf( "\n\n  FATAL ERROR:\n\n  PSP Model 3000 detected!\n\n  In it's current state this tool CAN NOT run on a PSP 3k!\n  90%% of the functions will definately brick it,\n  the remaining 10%% might most likely do so.\n\n  Unless there's a tested 3k compatible version of Recovery-Flasher\n  we will stop right here.... sorry!\n\n\n  press any button...." ); hcMiscWaitForAnyButton(); sceKernelExitGame(); }

  gPspType = PspType;  // cache PspType in a global copy for functions wanting to know but don't get supllied it in the arguments

  pspDebugScreenSetXY(0,27);
  printf( "Recovery Flasher v%i.%02i\nby ---==> HELLCAT <==---\n\n", appVerMa, appVerMi );
  printf( "PSAR extraction based on M33 PSARDumper, IPL flashing code by DAX\nwith lots of patient answers from the devs at LAN.ST" );

  if( SkipDisclaimer != 0xCA000001 )
  {
    pspDebugScreenSetXY(0,6);
    printf( "          ***   NOTICE   ***\n" );
    printf( "\n" );
    printf( "  This program access the internal FW flash of the PSP!\n" );
    printf( "  While having done extensive tests, accessing the flash is\n" );
    printf( "  a somewhat dangerous process!\n" );
    printf( "\n" );
    printf( " The author of this program takes NO RESPONSIBILITY for any issues!\n" );
    hcMiscMenuSetColors( 0x00CCCCCC, 0x00FFFFFF, 0x00FFFFFF, 0x00CCCCCC );
    hcMiscMenuClear();
    hcMiscMenuSetName( " " );
    hcMiscMenuAddEntry( "* I do NOT agree to this!", 1, " " );
    hcMiscMenuAddEntry( "* I AGREE, now take me to the program!", 2, " " );
    hcMiscMenuAddEntry( " ", -1, " " );
    hcMiscMenuAddEntry( "* I AGREE - never show this again!", 3, " " );
    r = hcMiscMenuShowMenu( 10, 13 );

    if( r == 1 ) { sceKernelExitGame(); }
    if( r == 3 )
    {
      SkipDisclaimer = 0xCA000001;
      hcWriteFile( "ms0:/PSP/SYSTEM/rflash.spl", &SkipDisclaimer, sizeof(int));
    }
  }

  sceKernelDelayThread( 2000000 );
  
  if( ( PspType == 2500 ) || ( PspType == 3000 ) )
  {
    ClearScreen();
    printf("\n\n");
    pspDebugScreenSetTextColor( 0x000000FF );
    printf( "        !!!  ATTENTION   !!!\n\n" );
    pspDebugScreenSetTextColor( 0x0000FFFF );
    printf( "  Recovery Flasher detected a not CFW capable PSP model\n" );
    printf( "  Your model is: " );
    pspDebugScreenSetTextColor( 0x00FFFFFF );
    printf( "%s", sPspType );
    pspDebugScreenSetTextColor( 0x0000FFFF );
    printf( "\n\n  Unfortunately, due to added/fixed security measures by Sony,\n" );
    printf( "  this model of PSP is NOT capable of having a CFW flashed to it.\n" );
    printf( "  Also any other flash related actions are way more risky, as there\n" );
    printf( "  is no way of running a Pandora for fixing.\n\n\n" );
    printf( "  Therefore, all flash writing functions will be DISABLED!\n\n\n\n" );
    pspDebugScreenSetTextColor( 0x00AAAAAA );
    printf( "  Press any button to continue...." );
    hcMiscWaitForAnyButton();
  }

  if( OnTM == 1 )
  {
    ClearScreen();
    PrintNote( "  TimeMachine detected!\n  Backup and Restore functions are DISABLED!" );
    sceKernelDelayThread( 5000000 );
  }
  if( OnHEN == 1 )
  {
    ClearScreen();
    PrintNote( "  HEN mode detected!\n  Some functionality will be limited!" );
    sceKernelDelayThread( 5000000 );
  }

  r = 98;
  while( r != 99 )
  {
    if( (r == 98) || (r == 96) )
    {
      if( r == 96 )
      {
      	pspDebugScreenSetXY( 0, 3 );
        r = 98;
      } else {
        ClearScreen();
      }
      
      printf( " Welcome to the Recovery Flasher!\n" );
      printf( " PSP Model: %s                        Battery: %i%%\n", sPspType, scePowerGetBatteryLifePercent());

      hcMiscMenuSetColors( 0x0088CCCC, 0x00FFFFFF, 0x0088FF88, 0x00AAAAAA );
      hcMiscMenuClear();
#ifndef PANDORA
#ifndef LEGACY
      hcMiscMenuSetName( "**  Main-Menu  **" );
#else
      hcMiscMenuSetName( "**  Main-Menu  **    [[ Legacy 1.50 Mode ]]" );
#endif
#else
      hcMiscMenuSetName( "**  Main-Menu  **    [[ Pandora Mode ]]" );
#endif
      if( (PspType != 2500) && (PspType != 3000) )
      {
        sprintf(s, "Target FW: %i.%02i  (X to cycle/change)    ", SelectedFWH, SelectedFWL);
        if( SelectedFWH == 9 )
        {
          sprintf( s, "Target FW: load from external FW pack    " );
        }
        hcMiscMenuAddEntry(s, 51, "  Use this option to cycle through all available/supported FWs\n  to choose the one you want to install" );
        hcMiscMenuAddEntry( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", -1, " " );
        hcMiscMenuAddEntry( "Install CFW  (of version selected above)", 10, "  This option will install a fresh M33 CFW of the selected verion.\n  For recovering semi-bricks or up-/downgrading your CFW.");
        if (RFAdvancedMode == 1)
        {
          hcMiscMenuAddEntry( "/!\\ Install clean ORIGINAL FW (DANGEROUS!!!)", 52, "  This will install an ORIGINAL SONY FW(!) of the selected version!\n  WARNING: You need a PANDORA to go back to CFW and you will NOT\n  be able to run ANY homebrew with this!" );
        }
        if( PspType == 1000 )
        {
          hcMiscMenuAddEntry( "Install fresh OFW 1.50", 11, "  This will flash a clean, original 1.50 to your PSP" );
        }
      }
      if( (PspType != 2500) && (PspType != 3000) )
      {
        hcMiscMenuAddEntry( "Flash/Restore complete FW from backup", 12, "  This option restores a complete firmware from a backup\n  that has been made earlier." );
      }
#ifndef PANDORA
      if( OnTM == 0 )
      {
      	if( (PspType != 2500) && (PspType != 3000) )
        {
          hcMiscMenuAddEntry( " ", -1, " " );
        }
        hcMiscMenuAddEntry( "Backup Flash content", 21, "  This makes a full (file-wise) backup of the firmware flash.\n  Use the restore options to copy specific parts back to the flash." );
        hcMiscMenuAddEntry( "Cache special updater modules", 22, "  This will extract and save the special updater modules\n  from any updater .PBP you have on your stick.\n  If you don't need them otherwise you can then delete the\n  updater .PBP to save space" );
        hcMiscMenuAddEntry( " ", -1, " " );
        if( (PspType != 2500) && (PspType != 3000) )
        {
        	hcMiscMenuAddEntry( "Restore: XMB theme related files", 31, "  Restores the files in /vsh/resource from your backup." );
        }
        hcMiscMenuAddEntry( "Restore: Settings", 32, "  Restores all your PSP system settings (flash1)." );
#ifndef LEGACY
        hcMiscMenuAddEntry( "Restore: PSN Store activation", 33, "  Restores the PSN Store activation data from your backup." );
#endif
      }
#endif
      hcMiscMenuAddEntry( " ", -1, " " );
#ifdef LEGACY
      hcMiscMenuAddEntry( "> Launch another (kXploited!) app from /PSP/GAME....", 40, "  Open a list of available apps in /PSP/GAME.\n\n  THE EBOOT MUST BE KXPLOITED TO BE ABLE TO BE LAUNCHED\n  ON AN ORIGINAL, VIRGIN 1.50!!!" );
#else
      hcMiscMenuAddEntry( "> Launch another app from /PSP/GAME....", 40, "  Open a list of available apps in /PSP/GAME." );
#endif
      if (RFAdvancedMode == 1)
      {
        hcMiscMenuAddEntry( "> NAND dump options....", 25, "  Additional options for performing a physical, byte-by-byte\n  dump of the PSPs NAND" );
      } else {
        hcMiscMenuAddEntry( "Make NAND dump", 26, "  Makes a byte-by-byte copy of the internal NAND flash memory\n  to ms0:/nand-dump.bin" );
      }
      hcRfHelperBatGetSerial( s+128 );
      r = *(u32*)(s+128);
      if( r != 0xFFFFFFFF )
      {
        hcMiscMenuAddEntry( "Make the battery a PANDORA one", 55, "  This will patch your battery to PANDORA mode\n  (if your hardware is compatible with doing this)" );
      } else {
        hcMiscMenuAddEntry( "Make the battery a NORMAL one again", 56, "  This will revert your battery back to normal" );
      }
      hcMiscMenuAddEntry( " ", -1, " " );
      if( RunningAsDDC7 == 1 )
      {
        hcMiscMenuAddEntry( "> Go to original DDC menu, or boot FW", 97, "  Go to the original DDC menu\n  or bootup DDC's CFW directely from the memstick" );
      } else {
        hcMiscMenuAddEntry( "EXIT", 99, "    ....that's not really hard to guess, is it? ;-)" );
      }
    }

    r = hcMiscMenuShowMenu( 4, 5 );
    
    if( r == 51 )
    {
      r = ( SelectedFWH * 100 ) + SelectedFWL;
      if( r == 371 ) { SelectedFWH = 3; SelectedFWL = 80; }
      if( r == 380 ) { SelectedFWH = 3; SelectedFWL = 90; }
      if( r == 390 ) { SelectedFWH = 4; SelectedFWL = 01; }
      if( r == 401 ) { SelectedFWH = 9; SelectedFWL = 99; }
      if( r == 999 ) { SelectedFWH = 5; SelectedFWL = 00; }
      if( r == 500 ) { SelectedFWH = 3; SelectedFWL = 71; }
      r = 96;
    }

    if( r == 10 )
    {
      TargetFW = ( SelectedFWH * 100 ) + SelectedFWL;
      if( TargetFW == 999 )
      {
      	r = hcGetFwPackPath( FwPackPath, (char*)dataOut );
      	if( r >= 0 )
        {
        	sprintf( s+64, "  Loading Firmware-Pack:\n  %s", FwPackPath );
        	PrintNote( s+64 );
        	sceKernelDelayThread( 1500000 );
          r = hcInitFwPack( FwPackPath, &TargetFW, &fwflagOverride, &fwNoCustomIpl, (char*)dataOut, &l, dataPSAR );  // l = req'ed PSP type; s = description
        }
        if( r >= 0 )
        {
        	if( l == PspType )
        	{
            processingFwPack = 1;
            DoCFWInstall( PspType, sPspType );
          } else {
            ClearScreen();
            printf( "\n\n\n   ERROR!\n\n\n   This firmware pack can not be applied to this PSP!\n   This firmware pack is not for this model of PSP!\n\n   Your PSP is model %i, the FW pack is for model %i.\n\n\n   Please press any button....", PspType, l );
            hcMiscWaitForAnyButton();
          }
        } else {
        	sprintf( s+64, "  ERROR while loading firmware pack (0x%08x)!\n  Press any button....", r );
          PrintNote( s+64 );
          hcMiscWaitForAnyButton();
        }
      } else {
        DoCFWInstall( PspType, sPspType );
      }
      r = 98;
    }
    if( r == 11 )
    {
    	ExtractOFW150();
      r = 98;
    }
    if( r == 52 )
    {
    	if( SelectedFWH == 9 )
    	{
    	  PrintNote( "  This function can not be used with firmware packs!" );
    	  sceKernelDelayThread( 2500000 );
    	} else {
        TargetFW = ( SelectedFWH * 100 ) + SelectedFWL;
        TargetOFW = 1;
        DoCFWInstall( PspType, sPspType );
        TargetOFW = 0;
      }
      r = 98;
    }
    if( r == 12 ) { TargetFW = 000; DoCFWInstall( PspType, sPspType ); r = 98; }

    if( r == 22 ) { DoCacheUpdaterPrx(); r = 98; }

    if( r == 21 )
    {
      if( GetBackupSlot( s, 1 ) != 98 )
      {
        RemountFlashs();
        hcRfHelperInitMangleSyscall();
        r = 0;
        sprintf( s+128, "%s/f0/data", s );
        r += hcCopyFolder( "flash0:/data", s+128, dataPSAR, 19000000, 1 );
        sprintf( s+128, "%s/f0/dic", s );
        r += hcCopyFolder( "flash0:/dic", s+128, dataPSAR, 19000000, 1 );
        sprintf( s+128, "%s/f0/font", s );
        r += hcCopyFolder( "flash0:/font", s+128, dataPSAR, 19000000, 1 );
        sprintf( s+128, "%s/f0/kd", s );
        r += hcCopyFolder( "flash0:/./kd", s+128, dataPSAR, 19000000, 1 );
        sprintf( s+128, "%s/f0/vsh", s );
        r += hcCopyFolder( "flash0:/./vsh", s+128, dataPSAR, 19000000, 1 );
        if( OnOE == 1 )
        {
          sprintf( s+128, "%s/f0/kn", s );
          r += hcCopyFolder( "flash0:/./kn", s+128, dataPSAR, 19000000, 1 );
        }
        if( Has150AddOn == 1 )
        {
          sprintf( s+128, "%s/f0/km", s );
          r += hcCopyFolder( "flash0:/./km", s+128, dataPSAR, 19000000, 1 );
        }
        PrintNote( "Copying PSP system settings to backup...." );
        sprintf( s+128, "%s/f1/dic", s );
        r += hcCopyFolder( "flash1:/dic", s+128, dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/gps", s );
        r += hcCopyFolder( "flash1:/gps", s+128, dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/net", s );
        r += hcCopyFolder( "flash1:/net", s+128, dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/registry", s );
        r += hcCopyFolder( "flash1:/registry", s+128, dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/vsh", s );
        r += hcCopyFolder( "flash1:/vsh", s+128, dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/config.se", s );
        i = hcReadFile( "flash1:/config.se", dataPSAR, 19000000 );
        if( i > 0)
        {
          hcWriteFile( s+128, dataPSAR, i );
          r++;
        }
        PrintNote( "Copying PSN activation to backup...." );
        sprintf( s+128, "%s/f2/act.dat", s );
        i = hcReadFile( "flash2:/act.dat", dataPSAR, 19000000 );
        if( i > 0)
        {
          hcWriteFile( s+128, dataPSAR, i );
          r++;
        }
        PrintNote( " Dumping IPL...." );
        i = hcRfHelperDumpNandIplToBuffer( dataOut );
        sprintf( s+128, "%s/nandipl.bin", s );
        hcWriteFile( s+128, dataOut, i );
        sceKernelDelayThread( 2000000 );
        PrintNote( " Finishing...." );
        sprintf( s+128, "%s/bakinfo.dat", s );
        memset( s, 0, 127 );
        *(int*)(&s[0x00]) = PspType;
        hcWriteFile( s+128, s, 127 );
        sceKernelDelayThread( 2000000 );
        sprintf( s, "  Finished backing up\n  (%i files copied)", r );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
      r = 98;
    }
    
    if( r == 25 )
    {
    	NANDDumpOptions( PspType, 0 );
    	r = 98;
    }
    
    if( r == 26 )
    {
    	NANDDumpOptions( PspType, 1 );
    	r = 98;
    }

    if( r == 31 )
    {
    	r = GetBackupSlot( s, 0 );
      if( (r != -1) && (r != 98) )
      {
        RemountFlashs();
        PrintNote( "  Restoring VSH/XMB resources...." );
        sprintf( s+128, "%s/f0/vsh/resource", s );
        r = hcCopyFolder( s+128, "flash0:/vsh/resource", dataPSAR, 19000000, 0 );
        sprintf( s, "  Restored %i files in /vsh/resource", r );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
      r = 98;
    }

    if( r == 32 )
    {
      r = GetBackupSlot( s, 0 );
      if( (r != -1) && (r != 98) )
      {
        RemountFlashs();
        PrintNote( "  Restoring PSP system settings...." );
        r = 0;
        sprintf( s+128, "%s/f1/dic", s );
        r += hcCopyFolder( s+128, "flash1:/dic", dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/gps", s );
        r += hcCopyFolder( s+128, "flash1:/gps", dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/net", s );
        r += hcCopyFolder( s+128, "flash1:/net", dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/registry", s );
        r += hcCopyFolder( s+128, "flash1:/registry", dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/vsh", s );
        r += hcCopyFolder( s+128, "flash1:/vsh", dataPSAR, 19000000, 0 );
        sprintf( s+128, "%s/f1/config.se", s );
        i = hcReadFile( s+128, dataPSAR, 19000000 );
        if( i > 0)
        {
          hcWriteFile( "flash1:/config.se", dataPSAR, i );
          r++;
        }
        sprintf( s, "  Restoring PSP system settings.... DONE! :-)" );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
      r = 98;
    }

    if( r == 33 )
    {
      r = GetBackupSlot( s, 0 );
      if( (r != -1) && (r != 98) )
      {
        RemountFlashs();
        PrintNote( "  Restoring PSN activation...." );
        sprintf( s+128, "%s/f2/act.dat", s );
        i = hcReadFile( s+128, dataPSAR, 19000000 );
        if( i > 0)
        {
          r = hcRfHelperIoBridgeWriteFile( "flash2:/act.dat", dataPSAR, i );
        }
        sprintf( s, "  Restoring PSN activation.... DONE! :-)" );
        PrintNote( s );
        sceKernelDelayThread( 3000000 );
        ClearNote();
      }
      r = 98;
    }

    if( r == 40 )
    {
    	o = 0;
    	while( r != 99 )
      {
        PrintNote( "  Reading list of apps...." );
        sceKernelDelayThread( 1000000 );
        r = hcReadEBOOTList( "ms0:/PSP/GAME", (char*)dataPSAR );
        hcMiscMenuClear();
        hcMiscMenuSetName( "Available apps for lunch.... erhm, launch....:" );
        if( r > 12 ) { l = 12; } else { l = r; }
        if( o > r ) { o = 0; }
        for( i = o; i < o+l; i++ )
        {
          if( i < r ) { hcMiscMenuAddEntry( (char*)(dataPSAR+(i*64)), i+1, " "); }
        }
        hcMiscMenuAddEntry( "----------------------------------------", -1, " " );
        if( r>l ) { hcMiscMenuAddEntry( "> more....", 98, "  show more apps available for launch...." ); }
        hcMiscMenuAddEntry( "> Back to mainmenu....", 99, "  Quit back to the mainmenu without starting anything...." );
        ClearScreen();
        printf( "\n" );
        printf( "        *  Additional Applications  *\n" );
        printf( "\n" );
        printf( "   From here you can launch any additional apps you have in\n" );
        printf( "   /PSP/GAME on your memstick.\n" );
        r = hcMiscMenuShowMenu( 5, 9 );
        
        if( r == 98 )
        {
          o += l;
        }

        if( (r != 99) && (r != 98 ) )
        {
          sprintf( s, "ms0:/PSP/GAME/%s/EBOOT.PBP", (char*)(dataPSAR+((r-1)*64)) );

          printf( "\n\n\n\n\n   %s\n   0x%08x\n\n\n", s, hcRfHelperLaunchEBOOT( s ) );
          sceKernelDelayThread( 10000000 );
        }
      }
      r = 98;
    }
    
    if( r == 56 )
    {
      char NewSer[4] = { 0x12, 0x34, 0x56, 0x78 };
      
      PrintNote( "  Attempting to patch battery to NORMAL mode...." );
      
      r = hcRfHelperBatSetSerial( NewSer );
      r = 98;
      
      sceKernelDelayThread( 1500000 );
      ClearNote();
    }
    
    if( r == 55 )
    {
      char NewSer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
      
      PrintNote( "  Attempting to patch battery to PANDORA mode...." );
      
      r = hcRfHelperBatSetSerial( NewSer );
      r = 98;
      
      sceKernelDelayThread( 1500000 );
      ClearNote();
    }
    
    if( r == 97 )
    {
    	ClearScreen();
    	hcMiscMenuClear();
    	hcMiscMenuSetName( " " );
    	hcMiscMenuAddEntry( "Go to original DDC menu....", 1, " " );
    	hcMiscMenuAddEntry( "Boot up CFW from stick....", 2, " " );
    	r = hcMiscMenuShowMenu( 4, 10 );
    	
    	if( r == 2 ) { sceKernelExitGame(); }
    	
    	PrintNote( "  >>> jumping to original DDC....\n  (this'll take just a few seconds)" );
  	  // DAX' original .PRX is M33 compressed, need to decompress it
  	  // for LoadStartModule to be able to load'n'start it
  	  r = hcReadFile( "flash0:/kd/ddc.prx", dataPSAR, 20500000 );
      r = pspDecompress( dataPSAR+0x150, dataOut, 2000000 );
      hcWriteFile( "ms0:/TM/ddc_plain.prx", dataOut, r );
      
      // extract the launch helper .PBP (required to get a clean env)
      l = hcReadFile( MyEBOOTPath, dataPSAR, 10000000 );
      if (l <= 0)
      {
        printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
      }
      o = *(u32 *)&dataPSAR[0x24];
      p = dataPSAR + o;
      l = l-o;
      r = hcMiscSeqGetFile( p, l, "ddclaunch.pbp", dataOut );
      r = hcWriteFile("ms0:/PSP/GAME/DDCLaunch/EBOOT.pbp", dataOut, r);
      
      // fire up the DDC launcher
      hcRfHelperLaunchEBOOT( "ms0:/PSP/GAME/DDCLaunch/EBOOT.PBP" );
      sceKernelDelayThread( 10000000 );
    }
  }

  ClearScreen();
  printf( " formatting flash0:/....\n\n" );
  sceKernelDelayThread( 1000000 );
  printf( " nah, just kidding ;-p   see you!" );
  sceKernelDelayThread( 3000000 );
  sceKernelExitGame();

  return 0;
}
