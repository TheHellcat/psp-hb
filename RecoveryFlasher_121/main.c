
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
#define appVerMi 21


#ifdef PANDORA

PSP_MODULE_INFO("HCRecoveryFlasher_Pandora", 0x1007, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#else

PSP_MODULE_INFO("HCRecoveryFlasher", 0x800, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);
PSP_HEAP_SIZE_KB(23500);

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
int NoCleanups = 0;
int WIP = 0;

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

  l = hcReadFile( "EBOOT.PBP", dataPSAR, 10000000 );
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


void CopyAdditionalM33Files( int psptype )
{
  int i;
  int r;
  u8* p;
  int l;
  int o;
  char s[128];
  char seqfile[128];

  l = hcReadFile( "EBOOT.PBP", dataPSAR, 10000000 );
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
      if( (TargetFW == 401) && (strcmp(m33files2000[i], "vshctrl_02g.prx") == 0) )
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


int BuildIpl( u8* buffer, int psptype )
{
  int r;
  u8* p;
  int l;
  int o;
  char cpl1[32];
  char cpl2[32];
  
  if( TargetFW == 401 )
  {
    sprintf( cpl1, "cpl1401.bin" );
    sprintf( cpl2, "cpl2401.bin" );
  } else {
    sprintf( cpl1, "cpl1000.bin" );
    sprintf( cpl2, "cpl2000.bin" );
  }

  r = 0;

  l = hcReadFile( "EBOOT.PBP", dataPSAR, 10000000 );
  if (l <= 0)
  {
    printf( " Error 0x%08X reading EBOOT.PBP.\n", l );
  }
  o = *(u32 *)&dataPSAR[0x24];
  p = dataPSAR + o;
  l = l-o;

  if( psptype == 1000 )
  {
    r = hcMiscSeqGetFile( p, l, cpl1, dataOut );
    l = hcReadFile( "sceNandIpl_1k.bin", dataOut + r, 512000 );
    l += r;
  }

  if( psptype == 2000 )
  {
    r = hcMiscSeqGetFile( p, l, cpl2, dataOut );
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

  l = hcReadFile( "EBOOT.PBP", dataPSAR, 10000000 );
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
  f = sceIoOpen( "EBOOT.PBP", PSP_O_RDONLY, 0777);
#endif
  if (f<0)
  {
    printf("\nERROR OPENING ./EBOOT.PBP! (0x%08x)\n", f);
    return -1;
  }

#ifdef PANDORA
  dpsp_l = sceIoRead(f, dataPSAR, 3000000);
#else
  sceIoRead(f, dataPSAR, 40);

  dpsp_o = *(u32 *)&dataPSAR[0x20];
  psar_o = *(u32 *)&dataPSAR[0x24];
  dpsp_l = psar_o - dpsp_o;

  sceIoLseek(f, dpsp_o, SEEK_SET);
  sceIoRead(f, dataPSAR, dpsp_l);
  sceIoClose( f );
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

  l = hcReadFile( "EBOOT.PBP", dataPSAR, 10000000 );
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



///////////////////
//               //
//   Main Code   //
//               //
///////////////////

void DoCFWInstall( int PspType, char* sPspType )
{
  int r;
  int abort = 0;
  int donttouchsettings = 0;
  char s[64];
  int btn;
  SceCtrlData pad;
  
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

  ClearScreen();
  PrintNote( " PSAR extraction based on M33 PSARDumper // IPL flashing by DAX\n thanx to JumpR, cory, jas0nuk and all the others at LAN.ST" );
  printf( "    Custom Firmware (re)Installation\n" );
  printf( "  --------------------------------------\n" );
  printf( "\n" );
  printf( "   From here you can install a fresh copy of a M33 CFW.\n" );
  printf( "   Additionally, you can choose whether or not to format (reset)\n" );
  printf( "   the flash partitions containing your settings, so you can\n" );
  printf( "   flash the firmware without loosing your settings.\n" );

  hcMiscMenuClear();
  hcMiscMenuSetName( "**  CFW Installation  **" );
  hcMiscMenuAddEntry( "reset (format) settings as well", 11, "   This will completely wipe all current settings.\n   (system settings, PSN activation, e.t.c.)\n   After reboot you will get the \"initial setup\" dialog.");
  hcMiscMenuAddEntry( "do not reset (format) settings", 12, "   This will keep all your settings.\n   The new install will run with all current settings.");
  hcMiscMenuAddEntry( " ", -1, " ");
  hcMiscMenuAddEntry( "I changed my mind, let me out....", 13, "   --> back to main menu");
  r = hcMiscMenuShowMenu( 7, 12 );
  if( r == 11 ) { donttouchsettings = 0; }
  if( r == 12 ) { donttouchsettings = 1; }
  if( r == 13 ) { return; }

  ClearScreen();
  sprintf( s, "%i.PBP", TargetFW );
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( "Installing firmware - PSP model: %s\nUpdater used to extract FW: %s\n\n", sPspType, s );
  pspDebugScreenSetTextColor(0x00CCCCCC);
  printf( "Extracting special updater .PRX....\n" );
  hcExtractUpdaterSepcialPRX( dataPSAR, s );

#ifndef PANDORA
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

  printf( "Loading updater modules\n" );
  r = LoadUpdaterModules();
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

  if( PspType == 1000 )
  {
    hcRfHelperPatchLFatEncryption( 0 );
  }
  /*if( (PspType == 2000) && (TargetFW == 401) )
  {
    hcRfHelperPatchLFatEncryption( 1 );
  }*/

  printf( "Checking updater .PBP.... " );
  psarInitPsarDumper( dataPSAR, dataOut, PspType );
  psarSetPBPPath( s );
  r = psarCopyFwFiles( 1 );
  if( r == 0)
  {
    printf( "looks good, we can continue.\n" );
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
  
  sceKernelDelayThread( 1500000 );

  printf( "\ngoing hot now: Formatting flashs.... " );
  r = hcRfHelperIoBridgeLFatfmt( 0 );
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
      r = hcRfHelperIoBridgeLFatfmt( 1 );
      if ( r < 0 )
      {
        printf( "\nWARNING: Error formatting flash1!  (0x%08x)\n", r );
      } else {
        printf( ", 1" );
      }
    }
    r = hcRfHelperIoBridgeLFatfmt( 2 );
    if ( r < 0 )
    {
      printf( "\nNote: Error formatting flash2!  (0x%08x)\n", r );
    } else {
      printf( ", 2" );
    }
    r = hcRfHelperIoBridgeLFatfmt( 3 );
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

  printf( "\nFinally.... installing firmware....\n" );
  MakeFlashFolders();
  psarInitPsarDumper( dataPSAR, dataOut, PspType );
  psarCopyFwFiles( 0 );

  PrintNote( "Performing final operations...." );
  CopyAdditionalM33Files( PspType );
  r = BuildIpl( dataOut, PspType );
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

  printf( "All done\n" );
  printf( "Press any button to shutdown...." );
  hcMiscWaitForAnyButton();
  hfRfHelperPoweroff();
  //scePowerRequestStandby();
  sceKernelDelayThread( 60000000 );
}


int main(int argc, char *argv[])
{
  int r;
  int i;
  int bar, tac;
  int PspType;
  char sPspType[32];
  char s[256];
  int SkipDisclaimer;

  pspDebugScreenInit();
  TargetFW = 371;

#ifndef PANDORA
  if (sceKernelDevkitVersion() < 0x02070100)
  {
    printf( "\n  This program needs at least 2.71 to work propperly!\n\nSorry.... exiting....\n\n\n\n\n If running on 3.52 or lower, launch this program from the XMB\n and make sure kernel is set to 3.xx.\n" );
    sceKernelDelayThread( 10000000 );
    sceKernelExitGame();
  }
#endif

  printf( "\n loading....\n\n" );

  dataPSAR = (u8 *)memalign(0x40, 20500000);
  if (!dataPSAR) { printf(" Cannot allocate memory for PSAR data.... (0x%08x)\n", (int)dataPSAR); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }
  dataOut = (u8 *)memalign(0x40, 2000000);
  if (!dataOut) { printf(" Cannot allocate memory for OUT data.... (0x%08x)\n", (int)dataOut); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }

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
  if( argc != 0 )
  {
    r = strlen( argv[0] ) - 1;
    if( *(argv[0]+r) !=  0x32 )
    {
      r = hcRfHelperRebootToUpdater();
    }
  }
#endif

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

  SkipDisclaimer = 0;
  hcReadFile( "ms0:/PSP/SYSTEM/rflash.spl", &SkipDisclaimer, sizeof(int));

  hcRfHelperGetMoboVersions( &bar, &tac );
  if( tac >= 0x00500000 ) { PspType = 2000; } else { PspType = 1000; }
  printf( "\nPSP version: %i\n", PspType );
  if( PspType == 1000 ) { sprintf( sPspType, "Classic \"Fat\" PSP" ); }
  if( PspType == 2000 ) { sprintf( sPspType, "PSP Slim&Lite" ); }

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

  r = 98;
  while( r != 99 )
  {
    if( r == 98 )
    {
      ClearScreen();

      printf( " Welcome to the Recovery Flasher!\n" );
      printf( " PSP Model: %s                        Battery: %i%%\n", sPspType, scePowerGetBatteryLifePercent());

      hcMiscMenuSetColors( 0x0088CCCC, 0x00FFFFFF, 0x0088FF88, 0x00AAAAAA );
      hcMiscMenuClear();
#ifndef PANDORA
      hcMiscMenuSetName( "**  Main-Menu  **" );
#else
      hcMiscMenuSetName( "**  Main-Menu  **    [[ Pandora Mode ]]" );
#endif
      hcMiscMenuAddEntry( "Flash new 3.71-M33-4", 10, "  This option will install a new and fresh 3.71-M33-4 to your PSP.\n  For recovering semi-bricks or up-/downgrading your CFW.");
      hcMiscMenuAddEntry( "Flash new 4.01-M33-2", 11, "  This option will install a new and fresh 4.01-M33-2 to your PSP.\n  For recovering semi-bricks or up-/downgrading your CFW.");
#ifndef PANDORA
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "Backup Flash content", 21, "  This makes a full (file-wise) backup of the firmware flash.\n  Use the restore options to copy specific parts back to the flash." );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "Restore: XMB theme related files", 31, "  Restores the files in /vsh/resource from your backup." );
      hcMiscMenuAddEntry( "Restore: Settings", 32, "  Restores all your PSP system settings (flash1)." );
      hcMiscMenuAddEntry( "Restore: PSN Store activation", 33, "  Restores the PSN Store activation data from your backup." );
#endif
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "> Launch another app from /PSP/GAME....", 40, "  Open a list of available apps in /PSP/GAME." );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "EXIT", 99, "    ....that's not really hard to guess, is it? ;-)" );
    }

    r = hcMiscMenuShowMenu( 4, 7 );

    if( r == 10 ) { DoCFWInstall( PspType, sPspType ); r = 98; }
    if( r == 11 ) { TargetFW = 401; DoCFWInstall( PspType, sPspType ); r = 98; }

    if( r == 21 )
    {
      RemountFlashs();
      r = 0;
      r += hcCopyFolder( "flash0:/data", "ms0:/flash_backup/f0/data", dataPSAR, 19000000, 1 );
      r += hcCopyFolder( "flash0:/dic", "ms0:/flash_backup/f0/dic", dataPSAR, 19000000, 1 );
      r += hcCopyFolder( "flash0:/font", "ms0:/flash_backup/f0/font", dataPSAR, 19000000, 1 );
      r += hcCopyFolder( "flash0:/kd", "ms0:/flash_backup/f0/kd", dataPSAR, 19000000, 1 );
      r += hcCopyFolder( "flash0:/vsh", "ms0:/flash_backup/f0/vsh", dataPSAR, 19000000, 1 );
      PrintNote( "Copying PSP system settings to backup...." );
      r += hcCopyFolder( "flash1:/dic", "ms0:/flash_backup/f1/dic", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "flash1:/gps", "ms0:/flash_backup/f1/gps", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "flash1:/net", "ms0:/flash_backup/f1/net", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "flash1:/registry", "ms0:/flash_backup/f1/registry", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "flash1:/vsh", "ms0:/flash_backup/f1/vsh", dataPSAR, 19000000, 0 );
      i = hcReadFile( "flash1:/config.se", dataPSAR, 19000000 );
      if( i > 0)
      {
        hcWriteFile( "ms0:/flash_backup/f1/config.se", dataPSAR, i );
        r++;
      }
      PrintNote( "Copying PSN activation to backup...." );
      i = hcReadFile( "flash2:/act.dat", dataPSAR, 19000000 );
      if( i > 0)
      {
        hcWriteFile( "ms0:/flash_backup/f2/act.dat", dataPSAR, i );
        r++;
      }
      sprintf( s, "  Finished backing up\n  (%i files copied)", r );
      PrintNote( s );
      sceKernelDelayThread( 3000000 );
      ClearNote();
      r = 98;
    }

    if( r == 31 )
    {
      RemountFlashs();
      PrintNote( "  Restoring VSH/XMB resources...." );
      r = hcCopyFolder( "ms0:/flash_backup/f0/vsh/resource", "flash0:/vsh/resource", dataPSAR, 19000000, 0 );
      sprintf( s, "  Restored %i files in /vsh/resource", r );
      PrintNote( s );
      sceKernelDelayThread( 3000000 );
      ClearNote();
      r = 98;
    }

    if( r == 32 )
    {
      RemountFlashs();
      PrintNote( "  Restoring PSP system settings...." );
      r = 0;
      r += hcCopyFolder( "ms0:/flash_backup/f1/dic", "flash1:/dic", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "ms0:/flash_backup/f1/gps", "flash1:/gps", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "ms0:/flash_backup/f1/net", "flash1:/net", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "ms0:/flash_backup/f1/registry", "flash1:/registry", dataPSAR, 19000000, 0 );
      r += hcCopyFolder( "ms0:/flash_backup/f1/vsh", "flash1:/vsh", dataPSAR, 19000000, 0 );
      i = hcReadFile( "ms0:/flash_backup/f1/config.se", dataPSAR, 19000000 );
      if( i > 0)
      {
        hcWriteFile( "flash1:/config.se", dataPSAR, i );
        r++;
      }
      sprintf( s, "  Restoring PSP system settings.... DONE! :-)" );
      PrintNote( s );
      sceKernelDelayThread( 3000000 );
      ClearNote();
      r = 98;
    }

    if( r == 33 )
    {
      RemountFlashs();
      PrintNote( "  Restoring PSN activation...." );
      i = hcReadFile( "ms0:/flash_backup/f2/act.dat", dataPSAR, 19000000 );
      if( i > 0)
      {
        r = hcRfHelperIoBridgeWriteFile( "flash2:/act.dat", dataPSAR, i );
      }
      sprintf( s, "  Restoring PSN activation.... DONE! :-)" );
      PrintNote( s );
      sceKernelDelayThread( 3000000 );
      ClearNote();
      r = 98;
    }
    
    if( r == 40 )
    {
    	PrintNote( "  Reading list of apps...." );
    	sceKernelDelayThread( 1000000 );
      r = hcReadEBOOTList( "ms0:/PSP/GAME", (char*)dataPSAR );
      hcMiscMenuClear();
      hcMiscMenuSetName( "Available apps for lunch.... erhm, launch....:" );
      for( i = 0; i < r; i++ )
      {
        hcMiscMenuAddEntry( (char*)(dataPSAR+(i*64)), i+1, " ");
      }
      hcMiscMenuAddEntry( "----------------------------------------", -1, " " );
      hcMiscMenuAddEntry( "> Back to mainmenu....", 99, "  Quit back to the mainmenu without starting anything...." );
      ClearScreen();
      printf( "\n" );
      printf( "        *  Additional Applications  *\n" );
      printf( "\n" );
      printf( "   From here you can launch any additional apps you have in\n" );
      printf( "   /PSP/GAME on your memstick.\n" );
      r = hcMiscMenuShowMenu( 5, 9 );
      
      if( r != 99 )
      {
        sprintf( s, "ms0:/PSP/GAME/%s/EBOOT.PBP", (char*)(dataPSAR+((r-1)*64)) );
      
        printf( "\n\n\n\n\n   %s\n   0x%08x\n\n\n", s, hcRfHelperLaunchEBOOT( s ) );
        sceKernelDelayThread( 10000000 );
      }
      
      r = 98;
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
