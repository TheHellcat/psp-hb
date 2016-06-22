/*  ==========================================  *\
 *                                              *
 *  PANDORA INSTALLER FOR 3.XX KERNELS          *
 *  smashed together by ---==> HELLCAT <==---   *
 *                                              *
\*  ==========================================  */

//  uses PSAR extraction based on PSPet's code  //
//  uses battery knowledge enginiered by Nem    //
//  uses a bit of common sense by.... me ;-)    //
//////////////////////////////////////////////////

//  this is a "get it done FFS" project,        //
//  don't expect the cleanest code ;-)))        //
//////////////////////////////////////////////////

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "kstuff.h"
#include "misc.h"
#include "ddcsup.h"
#include "ddc6sup.h"
#include "pspdecrypt.h"

PSP_MODULE_INFO("PandoraForKernel3xx", 0, 4, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(22500);

#define printf pspDebugScreenPrintf  // as usual *lol*

u8 *dataPSAR, *dataOut, *dataOut2;
char *dataMisc;

// char batSerialService[4];
char batSerialNormal[4];
char batSerialCurrent[4];
int UseCustomMSName = 0;
char CustomMSName[12] = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00 };

u8 sha1_150[20] =
{
  0x1A, 0x4C, 0x91, 0xE5, 0x2F, 0x67, 0x9B, 0x8B,
  0x8B, 0x29, 0xD1, 0xA2, 0x6A, 0xF8, 0xC5, 0xCA,
  0xA6, 0x04, 0xD3, 0x30
};

u8 sha1[20];

int InitFWInstall(void)
{
  SceUID fd;
  int size; //, read, i;
  // int r;

  printf("Reading and checking ms0:/150.PBP\n");
  fd = sceIoOpen("ms0:/150.PBP", PSP_O_RDONLY, 0777);
  if (fd < 0)
  {
    return 0x8CA10000 + (fd & 0xFFFF);
  }
  size = sceIoLseek(fd, 0, SEEK_END);
  sceIoLseek(fd, 0, PSP_SEEK_SET);
  sceIoRead(fd, dataOut, 200);
  if (memcmp(dataOut, "\0PBP", 4) != 0)
  {
    sceIoClose(fd);
    return 0x80CA0002;
  }
  size = size - *(u32 *)&dataOut[0x24];
  sceIoLseek(fd, *(u32 *)&dataOut[0x24], PSP_SEEK_SET);
  if (size != 10149440)
  {
    sceIoClose(fd);
    return 0x80CA0003;
  }
  if (sceIoRead(fd, dataPSAR, 10149440) != 10149440)
  {
    sceIoClose(fd);
    return 0x80CA0004;
  }
  sceIoClose(fd);

  sceKernelUtilsSha1Digest(dataPSAR, 10149440, sha1);
  if (memcmp(sha1, sha1_150, 20) != 0)
  {
    return 0x80CA0005;
  }

  return 0;
}

void Perform150Patches()
{
  /*pspSdkInstallNoDeviceCheckPatch();
  pspSdkInstallNoPlainModuleCheckPatch();
  pspSdkInstallKernelLoadModulePatch();*/
}

void ClearScreen(void)
{
  pspDebugScreenSetBackColor(0x00402000);
  pspDebugScreenSetTextColor(0x00FFFFFF);
  pspDebugScreenClear();
  printf("*** Pandora Installer for 3.xx+ Kernels ***   -- Rev4a --\n");
  printf("*******************************************\n\n");

  pspDebugScreenSetTextColor(0x00FFFF80);
  printf(" by ---==> HELLCAT <==---\n\n");
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf("-----------------------------------------------------------------\n");
  printf("\n");
  pspDebugScreenSetTextColor(0x0080FFFF);
}

void InitMainScreen(void)
{
  ClearScreen();
  pspDebugScreenSetTextColor(0x00FFFFFF);
  pspDebugScreenSetTextColor(0x00808080);
  printf("                           // current battery serial: 0x%08x\n\n", ((batSerialCurrent[0]&0xFF)*0x1000000)+((batSerialCurrent[1]&0xFF)*0x10000)+((batSerialCurrent[2]&0xFF)*0x100)+(batSerialCurrent[3]&0xFF) );
  pspDebugScreenSetTextColor(0x0080FFFF);
}

int mainReadFile(char *file, void *buf, int size)
{
	int seek = 0;
	
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

static int WriteFile(char* file, void* buffer, int size)
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

void ShowSplash( void )
{
  int skip;
  SceUID f;

  skip = 0;

  f = sceIoOpen( "ms0:/kd/pan3xx.spl", PSP_O_RDONLY, 0777);
  sceIoRead( f, &skip, sizeof( skip ) );
  sceIoClose( f );

  if( skip == 1 ) { return; }

  ClearScreen();
  pspDebugScreenSetTextColor(0x0080FFFF);
  printf( "    *** Welcome to the Pandora Installer for 3.xx+ Kernels ***\n" );
  printf( "                   ***   ADDITIONAL CREDITS   ***\n\n" );
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( " while enjoying this small tool and it hopefully helping you a lot,\n" );
  printf( " please note theese additional credits, without which this program\n" );
  printf( " would have never been possible the way you got to know it:\n\n\n" );
  pspDebugScreenSetTextColor(0x00FFFF80);
  printf( "  - all credit for the original Pandora and DDC to their creators\n" );
  printf( "  - PSAR extraction based on PSARDumper from PSPPet and M33\n" );
  printf( "  - knownledge for battery patching engeniered by Nem\n" );
  printf( "  - direct battery HW access code by Silverspring\n" );
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( "\n So, don't forget about all those folks laying the base for this,\n and have fun! :-)" );
  hcMiscMenuSetColors( 0x0080FFFF, 0x00FFFFFF, 0x0080FF00 );
  hcMiscMenuClear();
  hcMiscMenuSetName( " " );
  hcMiscMenuAddEntry( "OK", 1, "  Press X to go to the main menu\n  or choose \"Do not show this again\" to not show this again :-p" );
  hcMiscMenuAddEntry( "Do not show this again", 2, " " );
  skip = hcMiscMenuShowMenu( 39, 22 );

  if( skip == 2 )
  {
    skip = 1;
    WriteFile( "ms0:/kd/pan3xx.spl", &skip, sizeof( skip ) );
  }
}

void RepartitionMemstick(void)
{
  char* MsMbr;
  char* MsBRec;
  int PartSizeOld;
  int PartSizeNew;
  int PartPreceedOld;
  int PartPreceedNew;
  int PartClusterSize;
  SceUID f;
  int r;
  int o;
  int PartStartSize;
  int btn;
  SceCtrlData pad;

  ClearScreen();
  printf("\n--> MemStick partitioning and formatting functions by *HELLCAT* <--\n\n\n");
  printf("  ***  Repartitioning and formatting of the memorystick  ***\n\n");
  printf("WARNING!!!\nThis procedure WILL DELETE ALL DATA currently on the stick!\n");
  printf("Make sure you have backed up everything you like to keep!!!\n\n\n");
  printf("To continue with the repartitioning, and so completely erasing\n");
  printf("all contents of the memorystick, press L + R + X....\n....or O to quit.\n");

  sceCtrlReadBufferPositive(&pad, 1);
  r=0;
  while(r==0)
  {
    btn=0;
    while(btn==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
    }
    if(btn & PSP_CTRL_CIRCLE) { printf("saved at last second ;-)"); sceKernelDelayThread(2000000); return; }
    if(btn == (PSP_CTRL_CROSS | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)) { r=1; }
  }

  ClearScreen();
  printf("Repartitioning and formatting the memorystick....\n(say good bye to whatever was on it)\n\n");

  // allocate some buffers
  MsMbr = (char *)malloc(512);
  if (!MsMbr) { printf("Cannot allocate memory for MBR data.... (0x%08x)\n", (int)MsMbr); return; }
  MsBRec = (char *)malloc(512);
  if (!MsMbr) { printf("Cannot allocate memory for Bootrecord data.... (0x%08x)\n", (int)MsBRec); return; }

  // read everything we need to know about current layout
  r = kstuffGetMsPartitionStart();
  if (r<0)
  {
    printf("ERROR FETCHING PARTITION START BLOCKS!\n");
    return;
  }
  PartStartSize = r;

  if( UseCustomMSName == 1 )
  {
    memcpy( dataOut + 0x2B, CustomMSName, 11 );
    memcpy( dataOut + 0x40000, CustomMSName, 11 );
  }

  f = sceIoOpen( "msstor0:", PSP_O_RDONLY | PSP_O_WRONLY, 0);
  if (f<0)
  {
    printf("ERROR OPENING MSSTOR BLOCKDEVICE! (0x%08x)\n", f);
    return;
  }

  r = sceIoRead(f, MsMbr, 512);
  if(r!=512)
  {
    printf("ERROR READING MBR!\n");
    return;
  }
  PartPreceedOld = (MsMbr[454] & 0xFF) + ((MsMbr[455] & 0xFF)*0x100) + ((MsMbr[456] & 0xFF)*0x10000) + ((MsMbr[457] & 0xFF)*0x1000000);

  o = PartPreceedOld * 512;
  r = sceIoLseek(f, o, SEEK_SET);
  if(r!=o)
  {
    printf("ERROR SEEKING TO BOOTRECORD!\nShould be 0x%08x => is 0x%08x\n", o, r);
    return;
  }
  r = sceIoRead(f, MsBRec, 512);
  if(r!=512)
  {
    printf("ERROR READING BOOTRECORD!\n");
    return;
  }
  PartSizeOld = (MsBRec[32] & 0xFF) + ((MsBRec[33] & 0xFF)*0x100) + ((MsBRec[34] & 0xFF)*0x10000) + ((MsBRec[35] & 0xFF)*0x1000000);

  /*  OK, now we have all the information we need to actually  *
   *  patch the partitioning information on the stick          */

  // move start of data partition to ~1MB from the physical start
  PartPreceedNew = 2064;
  // change size of partition accordingly + some spare space, just in case....
  PartSizeNew = PartSizeOld + PartPreceedOld - PartPreceedNew - 64;

  // try to get a decent cluster size
  r = (PartSizeOld * 512) / 1024 / 1024;
  PartClusterSize = 40;
  if( r > 55 ) { PartClusterSize = 0x10; }
  if( r > 110 ) { PartClusterSize = 0x20; }
  if( r > 900 ) { PartClusterSize = 0x40; }
  if( r > 3500 ) { PartClusterSize = 0x80; }

  // now patch the new information into MBR and bootrecord
  MsMbr[454] = PartPreceedNew & 0xFF;
  MsMbr[455] = (PartPreceedNew & 0xFF00) / 0x100;
  MsMbr[456] = (PartPreceedNew & 0xFF0000) / 0x10000;
  MsMbr[457] = (PartPreceedNew & 0xFF000000) / 0x1000000;

  MsMbr[458] = PartSizeNew & 0xFF;
  MsMbr[459] = (PartSizeNew & 0xFF00) / 0x100;
  MsMbr[460] = (PartSizeNew & 0xFF0000) / 0x10000;
  MsMbr[461] = (PartSizeNew & 0xFF000000) / 0x1000000;

  // dummying out CHS stuff (=> use LBA only, I'm lazy ;-))
  MsMbr[447] = 255;
  MsMbr[448] = 255;
  MsMbr[449] = 255;
  MsMbr[450] = 14;
  MsMbr[451] = 255;
  MsMbr[452] = 255;
  MsMbr[453] = 255;

  dataOut[13] = PartClusterSize;
  dataOut[32] = PartSizeNew & 0xFF;
  dataOut[33] = (PartSizeNew & 0xFF00) / 0x100;
  dataOut[34] = (PartSizeNew & 0xFF0000) / 0x10000;
  dataOut[35] = (PartSizeNew & 0xFF000000) / 0x1000000;

  // time to write everything back to the stick....
  r = sceIoLseek(f, 0, SEEK_SET);
  r = sceIoWrite(f, MsMbr, 512);
  if(r!=512)
  {
    printf("Warning: possible failure at writing new MBR\n");
  }

  o = PartPreceedNew * 512;
  r = sceIoLseek(f, o, SEEK_SET);
  if(r!=o)
  {
    printf("Warning: possible failure at seeking to new partition start.\n\nShould be 0x%08x => is 0x%08x\n", o, r);
  }
  r = sceIoWrite(f, dataOut, PartStartSize);
  if(r!=PartStartSize)
  {
    printf("Warning: possible failure at writing new partition start blocks\n");
  }

  sceKernelDelayThread(1500000);

  printf("\n\nDONE! :-)\n\nReboot PSP for recognizing the new memstick layout!\n\n");
  printf("After rebooting, recopy 150.PBP and this installer\n");
  printf("back on the stick to install the Pandora files to it.\n\n\n");
  printf("Press X to turn off the PSP now....\n\n");
  printf("\n\n");
  sceIoClose(f);

  r=0;
  while(r==0)
  {
    sceCtrlReadBufferPositive(&pad, 1);
    while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }

    btn=0;
    while(btn==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
      if( ( kstuffGetKCtrl() & 0x2000000 ) == 0 ) { btn = 0x2000000; }
    }
    if(btn & PSP_CTRL_CROSS) { r=1; }
    if(btn == 0x2000000) { r=2; }
  }

  if( r == 1 )
  {
    scePowerRequestStandby();
    while(1==1) { sceKernelDelayThread(500000); }
  }
  if( r == 2 )
  {
    printf("Re-Insert stick now....\n");

    r=0;
    while(r==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      if( kstuffGetKCtrl() & 0x2000000 ) { r = 1; }
    }

    printf("Stick reinserted, (not yet) copying back files.... (this may take a while)");
    sceKernelDelayThread(10000000);
  }
}

void InstallMsIpl(void)
{
  SceUID f1;
  SceUID f2;
  int r;
  int l;

  printf("\nInstalling ms0:/MSIPL.BIN to reserved area on memstick....\n");

  f1 = sceIoOpen( "msstor0:", PSP_O_RDONLY | PSP_O_WRONLY, 0);
  if (f1<0)
  {
    printf("ERROR OPENING MSSTOR BLOCKDEVICE! (0x%08x)\n", f1);
    return;
  }

  f2 = sceIoOpen( "ms0:/msipl.bin", PSP_O_RDONLY, 0777);
  if (f2<0)
  {
    printf("ERROR OPENING MSIPL.BIN! (0x%08x)\n", f2);
    return;
  }

  l = sceIoRead(f2, dataOut, 1000000); // + 512;
  printf("read %i bytes, ", l);
  r = sceIoLseek(f1, 0x2000, SEEK_SET);
  printf("seek = 0x%x, ", r);
  l = ( l + 512 ) & 0xFFFFFE00;   // bring "l" to a round multiple of 512
  r = sceIoWrite(f1, dataOut, l);
  printf("written %i bytes.... ", r);

  sceIoClose(f1);
  sceIoClose(f2);

  sceKernelDelayThread(1500000);
  printf("\nDONE! :-)\n");
  sceKernelDelayThread(3000000);
}

void InstallTmIpl(void)
{
  SceUID f1;
  int r;
  int ln;

  printf("\nInstalling TimeMachine IPL to reserved area on memstick....\n");

  f1 = sceIoOpen("files2.seq", PSP_O_RDONLY, 0777);
  if (f1 < 0)
  {
    printf( "ERROR while loading files2.seq!\n" );
  }
  r = sceIoRead(f1, dataPSAR, 5000000);
  sceIoClose( f1 );
  ln = hcMiscSeqGetFile( dataPSAR, r, "tmmsipl.bin", dataOut );

  f1 = sceIoOpen( "msstor0:", PSP_O_RDONLY | PSP_O_WRONLY, 0);
  if (f1<0)
  {
    printf("ERROR OPENING MSSTOR BLOCKDEVICE! (0x%08x)\n", f1);
    return;
  }

  r = sceIoLseek(f1, 0x2000, SEEK_SET);
  printf("seek = 0x%x, ", r);
  r = sceIoWrite(f1, dataOut, ln);
  printf("written %i bytes.... ", r);

  sceIoClose(f1);

  sceKernelDelayThread(1500000);
  printf("\nDONE! :-)\n");
  sceKernelDelayThread(3000000);
}

void CheckPSPMainboard(void)
{
  int baryon, tachyon;
  int btn;
  SceCtrlData pad;

  kstuffGetMoboVersions(&baryon, &tachyon);

  if( tachyon >= 0x00500000 )
  {
    // it's a slim
    if( baryon >= 0x00234000 )
    {
      // it's a TA-085-v2 or even newer - BAD for making a battery
      ClearScreen();

      pspDebugScreenSetBackColor(0x00000088);
      pspDebugScreenSetTextColor(0x0000FFFF);

      pspDebugScreenSetXY(19, 13);
      printf("*****************************");
      pspDebugScreenSetXY(19, 14);
      printf("**                         **");
      pspDebugScreenSetXY(19, 15);
      printf("**   ");
      pspDebugScreenSetTextColor(0x00FFFFFF);
      printf("/");
      pspDebugScreenSetTextColor(0x0000FFFF);
      printf("!");
      pspDebugScreenSetTextColor(0x00FFFFFF);
      printf("\\");
      pspDebugScreenSetTextColor(0x000088FF);
      printf("  ATTENTION  ");
      pspDebugScreenSetTextColor(0x00FFFFFF);
      printf("/");
      pspDebugScreenSetTextColor(0x0000FFFF);
      printf("!");
      pspDebugScreenSetTextColor(0x00FFFFFF);
      printf("\\");
      pspDebugScreenSetTextColor(0x0000FFFF);
      printf("   **");
      pspDebugScreenSetXY(19, 16);
      printf("**                         **");
      pspDebugScreenSetXY(19, 17);
      printf("*****************************");
      pspDebugScreenSetBackColor(0x00402000);
      pspDebugScreenSetTextColor(0x0000FFFF);

      printf("\n\n\n\n");
      printf("   This program has detected a PSP Slim&Lite ");
      pspDebugScreenSetTextColor(0x00FFFFFF);
      printf("TA-085-v2");
      pspDebugScreenSetTextColor(0x0000FFFF);
      printf(" or newer\n");
      printf("                            mainboard!\n\n");
      printf(" On theese mainboards it is NOT possible to patch a battery into\n");
      printf("                  service mode a.k.a. Pandora.\n\n");
      printf("You can still use any other functions of this program as well as\n");
      printf("  an existing Pandora to boot into service mode, you just can't\n");
      printf("              patch the battery yourself on this PSP.\n");
      pspDebugScreenSetTextColor(0x00AAFFAA);
      printf("\n\npress any button to continue....");

      sceCtrlReadBufferPositive(&pad, 1);
      while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
      btn=0;
      while(btn==0)
      {
        sceCtrlReadBufferPositive(&pad, 1);
        btn = pad.Buttons & 0xFFFF;
      }
      sceCtrlReadBufferPositive(&pad, 1);
      while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }

      InitMainScreen();
    }
  }
}

void CheckPBPFiles( void )
{
  SceUID f;
  int pbp150, pbp340, pbp401, pbpcfw, fail;

  fail = 0;

  f = sceIoOpen( "ms0:/150.PBP", PSP_O_RDONLY, 0777);
  sceIoClose( f );
  if( f<0 ) { pbp150 = 0; fail = 1; } else { pbp150 = 1; }

  f = sceIoOpen( "ms0:/340.PBP", PSP_O_RDONLY, 0777);
  sceIoClose( f );
  if( f<0 ) { pbp340 = 0; fail = 1; } else { pbp340 = 1; }

  f = sceIoOpen( "ms0:/380.PBP", PSP_O_RDONLY, 0777);
  sceIoClose( f );
  if( f<0 ) { pbpcfw = 0; fail = 1; } else { pbpcfw = 1; }
  	
  f = sceIoOpen( "ms0:/401.PBP", PSP_O_RDONLY, 0777);
  sceIoClose( f );
  if( f<0 ) { pbp401 = 0; fail = 1; } else { pbp401 = 1; }

  if( fail == 1 )
  {
    pspDebugScreenSetTextColor(0x000000FF);
    printf( " ! WARNING: " );
    pspDebugScreenSetTextColor(0x0080FFFF);
    printf( "missing file(s): " );
    pspDebugScreenSetTextColor(0x00FFFFFF);
    if( pbp150 == 0 ) { printf( "150.PBP  " ); }
    if( pbp340 == 0 ) { printf( "340.PBP  " ); }
    if( pbpcfw == 0 ) { printf( "380.PBP  " ); }
    if( pbp401 == 0 ) { printf( "401.PBP  " ); }
    printf( "\n" );
  }
}

void Run150PandoraInstall( void )
{
  int r;
  SceUID f;

  ClearScreen();
  r=InitFWInstall();
  if(r!=0) { printf("\nERROR WHILE INITIALIZING: 0x%08x\n", r); }
  kstuffInitBuffers(dataPSAR, dataOut, dataOut2, dataMisc);
  printf("Installing firmware to memorystick.\nHang on, this will take a while, time to get a coffee.... ;-)");
  r=1;
  while(r!=0)
  {
    r=kstuffMsInstallFW();
    if(r>0){ WriteFile(dataMisc, dataOut2, r); }
  }
  printf("\nInstalling additional files....");
  r=1;
  while(r!=0)
  {
    r=kstuffMsInstallAdditional();
    if(r>0){ WriteFile(dataMisc, dataOut2, r); }
  }

  printf("\n\nCopying 150.PBP to UPDATE.PBP for Pandora flasher....");
  f = sceIoOpen( "ms0:/150.PBP", PSP_O_RDONLY, 0777);
  if (f<0)
  {
    printf("ERROR OPENING ms0:/150.PBP! (0x%08x)\n", f);
    sceIoClose( f );
    return;
  } else {
    r = sceIoRead(f, dataPSAR, 17000000);
    sceIoClose( f );
    WriteFile( "ms0:/UPDATE.PBP", dataPSAR, r );
    printf( "DONE!\n" );
  }

  printf("\n\nOK, if all went well we now have Pandora on the stick.\nDon't forget to run msipl installer if needed!\n");
  printf("\nPress any button to return to mainmenu....\n\n");

  hcMiscWaitForAnyButton();
}

void ExtractUpdaterSepcialPRX( void )
{
  int dpsp_o;
  int dpsp_l;
  int psar_o;
  SceUID f;
  u32 xorkey = 0xF1F1F1F1;
  u32 tag;
  int i, j, r;
  u32 m;
  u8* modptr[16];
  int modlen[16];
  u8* buff_off;

  printf( "Extracting special updater modules....\n" );

  f = sceIoOpen( "ms0:/380.PBP", PSP_O_RDONLY, 0777);
  if (f<0)
  {
    printf("ERROR OPENING ms0:/380.PBP! (0x%08x)\n", f);
    return;
  }

  printf("loading updater");

  sceIoRead(f, dataPSAR, 40);

  dpsp_o = *(u32 *)&dataPSAR[0x20];
  psar_o = *(u32 *)&dataPSAR[0x24];
  dpsp_l = psar_o - dpsp_o;

  sceIoLseek(f, dpsp_o, SEEK_SET);
  sceIoRead(f, dataPSAR, dpsp_l);
  sceIoClose( f );
  buff_off = (u8*)(((int)dataPSAR + dpsp_l + 1024) & 0xFFFFFF00);
  r = pspDecryptPRX(dataPSAR, buff_off, dpsp_l);
  tag = 0x5053507E ^ xorkey;

  // locate and save the "hidden/scrambled" updater modules
  printf(", saving modules");
  for( i=0; i<dpsp_l; i+=4 )
  {
    m = *(u32*)(buff_off+i);
    if( m == tag )
    {
      modptr[0] = &buff_off[i];
      modlen[0] = (*(u32*)(buff_off+i+0x2C)) ^ xorkey;
      //j++;
      for ( j=0; j<modlen[0]; j+=4)
      {
        *(u32*)(modptr[0]+j) ^= xorkey;
      }
      if( strcmp( (char*)(modptr[0]+0x0A), "sceLFatFs_Updater_Driver" ) == 0 )
      {
        WriteFile( "ms0:/kd/lfatfs_updater.prx", modptr[0], modlen[0] );
      }
      if( strcmp( (char*)(modptr[0]+0x0A), "sceNAND_Updater_Driver" ) == 0 )
      {
        WriteFile( "ms0:/kd/nand_updater.prx", modptr[0], modlen[0] );
      }
    }
  }

  // locate and save some more unscrambled updater modules
  tag = 0x5053507E;
  for( i=0; i<dpsp_l; i+=4 )
  {
    m = *(u32*)(buff_off+i);
    if( m == tag )
    {
      modptr[0] = &buff_off[i];
      modlen[0] = (*(u32*)(buff_off+i+0x2C));
      if( strcmp( (char*)(modptr[0]+0x0A), "sceLflashFatfmtUpdater" ) == 0 )
      {
        WriteFile( "ms0:/kd/lflash_fatfmt_updater.prx", modptr[0], modlen[0] );
      }
    }
  }

  printf( ". DONE\n" );
}

void ExtractUpdaterSepcialPRX2( void )
{
  int dpsp_o;
  int dpsp_l;
  int psar_o;
  SceUID f;
  u32 xorkey = 0xF1F1F1F1;
  u32 tag;
  int i, j, r;
  u32 m;
  u8* modptr[16];
  int modlen[16];
  u8* buff_off;

  printf( "Extracting special updater modules....\n" );

  f = sceIoOpen( "ms0:/401.PBP", PSP_O_RDONLY, 0777);
  if (f<0)
  {
    printf("ERROR OPENING ms0:/401.PBP! (0x%08x)\n", f);
    return;
  }

  printf("loading updater");

  sceIoRead(f, dataPSAR, 40);

  dpsp_o = *(u32 *)&dataPSAR[0x20];
  psar_o = *(u32 *)&dataPSAR[0x24];
  dpsp_l = psar_o - dpsp_o;

  sceIoLseek(f, dpsp_o, SEEK_SET);
  sceIoRead(f, dataPSAR, dpsp_l);
  sceIoClose( f );
  buff_off = (u8*)(((int)dataPSAR + dpsp_l + 1024) & 0xFFFFFF00);
  r = pspDecryptPRX(dataPSAR, buff_off, dpsp_l);
  tag = 0x5053507E ^ xorkey;

  // locate and save the "hidden/scrambled" updater modules
  printf(", saving modules");
  for( i=0; i<dpsp_l; i+=4 )
  {
    m = *(u32*)(buff_off+i);
    if( m == tag )
    {
      modptr[0] = &buff_off[i];
      modlen[0] = (*(u32*)(buff_off+i+0x2C)) ^ xorkey;
      //j++;
      for ( j=0; j<modlen[0]; j+=4)
      {
        *(u32*)(modptr[0]+j) ^= xorkey;
      }
      if( strcmp( (char*)(modptr[0]+0x0A), "sceLFatFs_Updater_Driver" ) == 0 )
      {
        WriteFile( "ms0:/TM/DC6/kd/lfatfs_updater.prx", modptr[0], modlen[0] );
      }
      if( strcmp( (char*)(modptr[0]+0x0A), "sceNAND_Updater_Driver" ) == 0 )
      {
        WriteFile( "ms0:/TM/DC6/kd/emc_sm_updater.prx", modptr[0], modlen[0] );
      }
    }
  }

  // locate and save some more unscrambled updater modules
  tag = 0x5053507E;
  for( i=0; i<dpsp_l; i+=4 )
  {
    m = *(u32*)(buff_off+i);
    if( m == tag )
    {
      modptr[0] = &buff_off[i];
      modlen[0] = (*(u32*)(buff_off+i+0x2C));
      if( strcmp( (char*)(modptr[0]+0x0A), "sceLflashFatfmtUpdater" ) == 0 )
      {
        WriteFile( "ms0:/TM/DC6/kd/lflash_fatfmt_updater.prx", modptr[0], modlen[0] );
      }
    }
  }

  printf( ". DONE\n" );
}

void RunDDCPandoraInstall( void )
{
  int r;
  int l;
  SceUID fd;
  int i;
  char s[128];
  char* AdditionalDDCFiles[] = { "flashemu.prx",
                                 "hibari.prx",
                                 "libpsardumper.prx",
                                 "pspbtcnf.txt",
                                 "pspbtcnf_game.txt",
                                 "pspbtcnf_updater.txt",
                                 "pspcnf_tbl.txt",
                                 "pspdecrypt.prx",
                                 "resurrection_ddc.elf",
                                 "system.dreg",
                                 "system.ireg",
                                 "0" };

  ClearScreen();

  if (sceKernelDevkitVersion() < 0x02070110)
  {
    printf( "!! This part requires 2.71 or higher.\n!! If you are in a CFW, please reexecute on the higher kernel.\n" );
    sceKernelDelayThread(7000000);
    return;
  }
  r = pspSdkLoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (r < 0)
  {
    printf( "Error 0x%08X loading/starting libpsardumper.prx.\n", r);
    sceKernelDelayThread(7000000);
    return;
  }
  r = pspSdkLoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (r < 0)
  {
    printf( "Error 0x%08X loading/starting pspdecrypt.prx.\n", r);
    sceKernelDelayThread(7000000);
    return;
  }

  r=InitFWInstall();
  if(r!=0) { printf("\nERROR WHILE INITIALIZING: 0x%08x\n", r); }
  kstuffInitBuffers(dataPSAR, dataOut, dataOut2, dataMisc);
  printf("Installing firmware to memorystick.\nHang on, this will take a while, time to get a coffee.... ;-)");
  r=1;
  while(r!=0)
  {
    r=kstuffMsInstallFW();
    if(r>0){ WriteFile(dataMisc, dataOut2, r); }
  }
  printf("\nInstalling additional files....");
  r=1;
  while(r!=0)
  {
    r=kstuffMsInstallAdditional();
    if(r>0){ WriteFile(dataMisc, dataOut2, r); }
  }

  printf( "\n" );

  //
  // load the archive containing even more additional files into the buffer
  //////
  fd = sceIoOpen("files2.seq", PSP_O_RDONLY, 0777);
  if (fd < 0)
  {
    printf( "ERROR while loading files2.seq!\n" );
  }
  r = sceIoRead(fd, dataPSAR, 5000000);
  sceIoClose( fd );

  //
  // extract all required files
  //////
  i = 0;
  while( 1 )
  {
    l = hcMiscSeqGetFile( dataPSAR, r, AdditionalDDCFiles[i], dataOut );
    if( strcmp( AdditionalDDCFiles[i], "resurrection_ddc.elf" ) == 0 ) { sprintf( s, "ms0:/kd/resurrection.elf" ); }
    else if( strcmp( AdditionalDDCFiles[i], "system.dreg" ) == 0 ) { sprintf( s, "ms0:/registry/system.dreg" ); }
    else if( strcmp( AdditionalDDCFiles[i], "system.ireg" ) == 0 ) { sprintf( s, "ms0:/registry/system.ireg" ); }
    else { sprintf( s, "ms0:/kd/%s", AdditionalDDCFiles[i] ); }
    WriteFile( s, dataOut, l );
    i++;
    if( strcmp( AdditionalDDCFiles[i], "0" ) == 0 ) { break; }
  }

  printf("\n\nOK, stage 1 finished.\nWill now jump to stage 2: installing 3.40 modules....\n");

  sceKernelDelayThread( 3000000 );
  ddcsup( dataPSAR, dataOut, dataOut2 );

  printf("\n\nStage 3:\n");
  ExtractUpdaterSepcialPRX();

  printf("\n\nDDC installation finished....\n");

  sceKernelDelayThread( 6000000 );
}

void MoveMemBlock( u8* source, u8* dest, int len )
{
	// this is slow, but overlap save, thus not using std C functions
  int i;
  
  for( i=0; i<len; i++ )
  {
    dest[i] = source[i];
  }
}

void RunDDC6PandoraInstall( void )
{
  int r;
  int l;
  SceUID fd;
  int i;
  char s[128];
  char* AdditionalDDCFiles[] = { "DDC6_bg.bmp",             "/vsh/resource/bg.bmp",
  	                             "DDC6_config.txt",         "/../config.txt",
  	                             "DDC6_dcman.prx",          "/kd/dcman.prx",
  	                             "DDC6_galaxy.prx",         "/kd/galaxy.prx",
  	                             "DDC6_idcanager.prx",      "/kd/idcanager.prx",
  	                             "DDC6_intraFont.prx",      "/vsh/module/intraFont.prx",
  	                             "DDC6_iop.prx",            "/kd/iop.prx",
  	                             "DDC6_ipl.bin",            "/ipl.bin",
  	                             "DDC6_ipl_update.prx",     "/kd/ipl_update.prx",
  	                             "DDC6_libpsardumper.prx",  "/kd/libpsardumper.prx",
  	                             "DDC6_march33.prx",        "/kd/march33.prx",
  	                             "DDC6_popcorn.prx",        "/kd/popcorn.prx",
  	                             "DDC6_pspbtdnf.bin",       "/kd/pspbtdnf.bin",
  	                             "DDC6_pspbtdnf_02g.bin",   "/kd/pspbtdnf_02g.bin",
  	                             "DDC6_pspbtjnf.bin",       "/kd/pspbtjnf.bin",
  	                             "DDC6_pspbtjnf_02g.bin",   "/kd/pspbtjnf_02g.bin",
  	                             "DDC6_pspbtknf.bin",       "/kd/pspbtknf.bin",
  	                             "DDC6_pspbtknf_02g.bin",   "/kd/pspbtknf_02g.bin",
  	                             "DDC6_pspbtlnf.bin",       "/kd/pspbtlnf.bin",
  	                             "DDC6_pspbtlnf_02g.bin",   "/kd/pspbtlnf_02g.bin",
  	                             "DDC6_pspdecrypt.prx",     "/kd/pspdecrypt.prx",
  	                             "DDC6_recovery.prx",       "/vsh/module/recovery.prx",
  	                             "DDC6_resurrection.prx",   "/kd/resurrection.prx",
  	                             "DDC6_satelite.prx",       "/vsh/module/satelite.prx",
  	                             "DDC6_systemctrl.prx",     "/kd/systemctrl.prx",
  	                             "DDC6_systemctrl_02g.prx", "/kd/systemctrl_02g.prx",
  	                             "DDC6_tmctrl401.prx",      "/tmctrl401.prx",
  	                             "DDC6_usbdevice.prx",      "/kd/usbdevice.prx",
  	                             "DDC6_vlf.prx",            "/vsh/module/vlf.prx",
  	                             "DDC6_vshctrl.prx",        "/kd/vshctrl.prx",
  	                             "DDC6_system.ireg",        "/registry/system.ireg",
  	                             "DDC6_system.dreg",        "/registry/system.dreg",
                                 "0" };

  ClearScreen();

  if (sceKernelDevkitVersion() < 0x02070110)
  {
    printf( "!! This part requires 2.71 or higher.\n!! If you are in a CFW, please reexecute on the higher kernel.\n" );
    sceKernelDelayThread(7000000);
    return;
  }
  r = pspSdkLoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (r < 0)
  {
    printf( "Error 0x%08X loading/starting libpsardumper.prx.\n", r);
    sceKernelDelayThread(7000000);
    return;
  }
  r = pspSdkLoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (r < 0)
  {
    printf( "Error 0x%08X loading/starting pspdecrypt.prx.\n", r);
    sceKernelDelayThread(7000000);
    return;
  }

  free(dataPSAR);
  free(dataOut);
  free(dataOut2);
  free(dataMisc);
  dataPSAR = (u8 *)memalign(0x40, 20500000);
  if (!dataPSAR) { printf(" Cannot allocate memory for PSAR data.... (0x%08x)\n", (int)dataPSAR); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }
  dataOut = (u8 *)memalign(0x40, 2000000);
  if (!dataOut) { printf(" Cannot allocate memory for OUT data.... (0x%08x)\n", (int)dataOut); sceKernelDelayThread( 10000000 ); sceKernelExitGame(); }
  	
  printf( "Installing DDCv6....\n" );
  printf( "\n" );
  printf( "What will be done:\n" );
  printf( "- all required files will be copied\n" );
  printf( "- TimeMachine IPL will be installed on the stick\n" );
  printf( "- TimeMachine config will be written\n" );
  printf( "- if present, MSIPL.BIN will be converted for TM\n" );
  printf( "\n" );
  printf( "When booting Pandora, you have the following options:\n" );
  printf( "\n" );
  printf( "no buttons pressed: PSP will boot from flash, if possible.\n" );
  printf( "\"Up\" will boot any MSIPL based MagicStick setup installed\n" );
  printf( "\"Down\" will boot DDCv6\n" );
  printf( "X, [], O and /\\ will boot TM firmwares\n" );
  printf( "\n" );
  printf( "\n" );
  
  printf( "Now copying files.... (this will take some time)\n" );
  DDC6psarInitPsarDumper( dataPSAR, dataOut, 0 );
  DDC6psarSetPBPPath( "ms0:/401.PBP" );
  DDC6psarCopyFwFiles( 0 );

  //
  // load the archive containing even more additional files into the buffer
  //////
  fd = sceIoOpen("files2.seq", PSP_O_RDONLY, 0777);
  if (fd < 0)
  {
    printf( "ERROR while loading files2.seq!\n" );
  }
  r = sceIoRead(fd, dataPSAR, 5000000);
  sceIoClose( fd );

  //
  // extract all required files
  //////
  i = 0;
  while( 1 )
  {
    l = hcMiscSeqGetFile( dataPSAR, r, AdditionalDDCFiles[i], dataOut );
    sprintf( s, "ms0:/TM/DC6%s", AdditionalDDCFiles[i+1] );
    WriteFile( s, dataOut, l );
    i += 2;
    if( strcmp( AdditionalDDCFiles[i], "0" ) == 0 ) { break; }
  }
  
  printf( "Building IPLs.... " );
  l = mainReadFile( "ms0:/TM/DC6/sceNandIpl_1k.bin", dataOut + 0x3000, 2000000 );
  i = hcMiscSeqGetFile( dataPSAR, r, "DDC6_cpl1000.bin", dataOut );
  WriteFile( "ms0:/TM/DC6/ipl_01g.bin", dataOut, l + 0x3000 );
  l = mainReadFile( "ms0:/TM/DC6/sceNandIpl_2k.bin", dataOut + 0x3000, 2000000 );
  i = hcMiscSeqGetFile( dataPSAR, r, "DDC6_cpl2000.bin", dataOut );
  WriteFile( "ms0:/TM/DC6/ipl_02g.bin", dataOut, l + 0x3000 );
  sceIoRemove( "ms0:/TM/DC6/sceNandIpl_1k.bin" );
  sceIoRemove( "ms0:/TM/DC6/sceNandIpl_2k.bin" );
  printf( "DONE\n" );
  
  ExtractUpdaterSepcialPRX2();
  
  printf( "Converting MSIPL for TM IPL loader.... " );
  r = mainReadFile( "ms0:/MSIPL.BIN", dataOut, 2000000 );
  if( r > 0 )
  {
  	l = 0;
  	for( i=0x5010; i<r; i+=0x1000 )
  	{
  	  MoveMemBlock( dataOut+i-(0xB0*l), dataOut+i-0xB0-(0xB0*l), r-i+(0xB0*l) );
  	  l++;
  	}
  	MoveMemBlock( dataOut+0x4010, dataOut+0x4000, r-0x4010 );
    WriteFile( "ms0:/TM/pandora.bin", dataOut + 0x1000, 0x371F0 );
    printf( "DONE\n" );
  } else {
    printf( "no MSIPL present\n" );
  }
  
  printf( "Installing TM IPL loader to memstick....\n " );
  InstallTmIpl();

  printf("\n\nDDC installation finished....\n");

  sceKernelDelayThread( 6000000 );
}

void InstallEnhancedTools( void )
{
  int r;
  int l;
  SceUID fd;

  ClearScreen();

  printf( "\n\n  This option installs some very valuable, additional tools\n  to your DDC MagicStick:\n\n" );
  printf( "    * ELF-Menu   - by jas0nuk\n    * NAND-Tool  - by cory1492\n\n" );
  printf( "  Please note that all credits for theese additional apps\n  belong ONLY to the devs mentioned above!\n  I only added them (with kind permission) for more easy\n  installation into this installer! ;-)\n\n" );

  sceKernelDelayThread( 3000000 );
  printf( "Installing the tools now...." );

  //
  // load the archive containing even more additional files into the buffer
  //////
  fd = sceIoOpen("files2.seq", PSP_O_RDONLY, 0777);
  if (fd < 0)
  {
    printf( "ERROR while loading files2.seq!\n" );
  }
  r = sceIoRead(fd, dataPSAR, 5000000);
  sceIoClose( fd );

  //
  // extract all required files
  //////
  l = hcMiscSeqGetFile( dataPSAR, r, "nandtool_03_beta1.elf", dataOut );
  WriteFile( "ms0:/elf/nandtool_03.elf", dataOut, l );

  l = hcMiscSeqGetFile( dataPSAR, r, "nandtool_03_beta1_nousb.elf", dataOut );
  WriteFile( "ms0:/elf/nandtool_03_nousb.elf", dataOut, l );

  l = hcMiscSeqGetFile( dataPSAR, r, "resurrection_ddc.elf", dataOut );
  WriteFile( "ms0:/elf/resurrection.elf", dataOut, l );

  l = hcMiscSeqGetFile( dataPSAR, r, "resurrection_menu.elf", dataOut );
  WriteFile( "ms0:/kd/resurrection.elf", dataOut, l );

  sceKernelDelayThread( 2000000 );
  printf( " DONE! :-)\nGoing back to the menu...." );
  sceKernelDelayThread( 6000000 );
}

int main(int argc, char *argv[])
{
  int btn;
  int r;
  int i;
  SceUID fd;
  SceUID mod;

  pspDebugScreenInit();
  ShowSplash();
  InitMainScreen();
  pspDebugScreenSetXY(0, 28);

  //Perform150Patches();

  mod = pspSdkLoadStartModule("kstuff.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0) { printf(" ! Error 0x%08X loading/starting kstuff.prx.\n", mod); }

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

  CheckPSPMainboard();

  fd = sceIoOpen("ms0:/batser.bin", PSP_O_RDONLY, 0777);
  r = sceIoRead(fd, batSerialNormal, 4);
  sceIoClose( fd );
  if( r == 4 )
  {
    printf( " > ms0:/batser.bin found - serial: 0x%08x\n", ((batSerialNormal[0]&0xFF)*0x1000000)+((batSerialNormal[1]&0xFF)*0x10000)+((batSerialNormal[2]&0xFF)*0x100)+(batSerialNormal[3]&0xFF) );
    // *(u32*)batSerialNormal => that would be nicer'n'better, right? But doesn't work here :-/ gives us "78563412" when it should be "12345678" ;-) I always hated that endian thing....
  } else {
    batSerialNormal[0] = 0x90;
    batSerialNormal[1] = 0xCA;
    batSerialNormal[2] = 0x08;
    batSerialNormal[3] = 0x15;
  }

  fd = sceIoOpen("ms0:/msname.txt", PSP_O_RDONLY, 0777);
  r = sceIoRead(fd, CustomMSName, 11);
  sceIoClose( fd );
  if( r > 1 )
  {
    // validate MS name (remove illegal characters)
    for( i=0; i<=10; i++)
    {
      int isValidChar = ((CustomMSName[i] >= 0x30) & (CustomMSName[i]<= 0x39)) | ((CustomMSName[i] >= 0x41) & (CustomMSName[i]<= 0x5A)) | ((CustomMSName[i] >= 0x61) & (CustomMSName[i]<= 0x7A)) | (CustomMSName[i] == 0x2D) | (CustomMSName[i] == 0x5F) | (CustomMSName[i] == 0x20);
      if( isValidChar == 0 )
      {
        if( (CustomMSName[i] == 0x0D) | (CustomMSName[i] == 0x0A))
        {  // convert any illegal char to "_" and CR/LF to blanks
          CustomMSName[i] = 0x20;
        } else {
          CustomMSName[i] = 0x5F;
        }
      }
    }

    UseCustomMSName = 1;
    printf( " > Custom MS name found: %s\n", CustomMSName );
  }

  CheckPBPFiles();

  //dataPSAR = (u8 *)memalign(0x40, 16703136+256);
  dataPSAR = (u8 *)memalign(0x40, 17000000);
  if (!dataPSAR) { printf("Cannot allocate memory for PSAR data.... (0x%08x)\n", (int)dataPSAR); }

  dataOut = (u8 *)memalign(0x40,  2500000);
  if (!dataOut) { printf("Cannot allocate memory for buffer 1.... (0x%08x)\n", (int)dataOut); }

  dataOut2 = (u8 *)memalign(0x40, 2500000);
  if (!dataOut2) { printf("Cannot allocate memory for buffer 2.... (0x%08x)\n", (int)dataOut2); }

  dataMisc = (char *)memalign(0x40, 1024);
  if (!dataMisc) { printf("Cannot allocate memory for buffer 3.... (0x%08x)\n", (int)dataMisc); }

  batSerialCurrent[1] = 0xAB;
  kstuffBatGetSerial( batSerialCurrent );
  pspDebugScreenSetXY(0, 7);
  pspDebugScreenSetTextColor(0x00808080);
  printf("                           // current battery serial: 0x%08x", ((batSerialCurrent[0]&0xFF)*0x1000000)+((batSerialCurrent[1]&0xFF)*0x10000)+((batSerialCurrent[2]&0xFF)*0x100)+(batSerialCurrent[3]&0xFF) );
  pspDebugScreenSetTextColor(0x0080FFFF);
  pspDebugScreenSetXY(0, 30);

  hcMiscMenuSetColors( 0x0080FFFF, 0x00FFFFFF, 0x0080FF00 );
  hcMiscMenuClear();
  hcMiscMenuSetName( "Main Menu" );
  hcMiscMenuAddEntry( "Install a DDCv4 MagicStick (for Fat + Slim)", 41, "   This option sets up a complete DDCv4 (DAX' universal unbricker)\n   MagicStick that will work for the Fat and Slim PSPs and will\n   flash a M33 CFW to your PSP.");
  hcMiscMenuAddEntry( "Install a DDCv6 MagicStick (for Fat + Slim)", 43, "   This option sets up a complete DDCv6 (DAX' universal unbricker)\n   MagicStick that will work for the Fat and Slim PSPs and will\n   flash a M33 CFW to your PSP.");
  hcMiscMenuAddEntry( "Install ELF-Menu and NAND-Tool to the DDCv4 stick", 42, "   This requires a set up DDCv4 stick and will add jas0nuk's ELF-Menu\n   and cory's NAND-Tool to it!\n\n   DOES NOT WORK ON DDCv6!" );
  hcMiscMenuAddEntry( "Install classic Pandora (Fat PSP and 1.50 FW only)", 11, "   This installs the old, original, classic Pandora\n   Working only on Fat PSPs, installing only 1.50!" );
  hcMiscMenuAddEntry( " ", -1, " " );
  hcMiscMenuAddEntry( "Battery options....", 12, "   ....more options for patching the battery" );
  hcMiscMenuAddEntry( "MemStick and MS-IPL options....", 13, "   ....more options regarding memstick actions" );
  hcMiscMenuAddEntry( " ", -1, " " );
  hcMiscMenuAddEntry( "Quit", 99, "   buybuy! ;-)" );
  while( 1 == 1 )
  {
    btn = hcMiscMenuShowMenu( 3, 9 );

    if( btn == 11 )
    {
      Run150PandoraInstall();
    }

    if( btn == 41 )
    {
      RunDDCPandoraInstall();
    }
    
    if( btn == 43 )
    {
      RunDDC6PandoraInstall();
    }

    if( btn == 12 )
    {
      hcMiscMenuClear();
      hcMiscMenuSetName( "Battery Options:" );
      hcMiscMenuAddEntry( "make battery Pandora", 21, "   make your battery to a service-mode one" );
      hcMiscMenuAddEntry( "make battery normal", 22, "   make your battery to a normal one" );
      hcMiscMenuAddEntry( "dump battery serial to file", 23, "   save current battery serial to ms0:/batser.bin" );
      hcMiscMenuAddEntry( "set battery serial from file", 24, "   set the battery serial to the one stored in ms0:/batser.bin" );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "back to mainmenu....", 98, " " );
    }

    if( btn == 13 )
    {
      hcMiscMenuClear();
      hcMiscMenuSetName( "Memstick Options:" );
      hcMiscMenuAddEntry( "Install ms0:/MSIPL.BIN to the MemStick", 31, "   This will install the contents of the file MSIPL.BIN from the\n   root of the memstick to special sectors on the stick, so the PSP\n   can boot from it when starting in service mode\n   NEEDS PROPPER REPARTITIONING!!!" );
      hcMiscMenuAddEntry( "Install TimeMachine IPL to the MemStick", 32, "   Installs TimeMachine's IPL loader to the special sectors\n   on the memstick.\n\n   Does not require repartitioning - not recommended to apply\n   without though!" );
      hcMiscMenuAddEntry( "------------------------------------------------", -1, " " );
      hcMiscMenuAddEntry( "Repartition and format MemStick (DESTRUCTIVE!)", 33, "   This repartitions the memstick (mooving the active data\n   area to 1MB further from the start) and formats the new partition\n   therefore COMPLETELY WIPING all existing data!" );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "back to mainmenu....", 98, " " );
    }

    if( btn == 21 )
    {
      r=kstuffBatMakeService();
      pspDebugScreenSetXY( 0, 27 );
      printf(" > Battery pandorized....\n");
      sceKernelDelayThread( 2500000 );
      kstuffBatGetSerial( batSerialCurrent );
    }

    if( btn == 22 )
    {
      r=kstuffBatMakeNormal();
      pspDebugScreenSetXY( 0, 27 );
      printf(" > Battery normalized....\n");
      sceKernelDelayThread( 2500000 );
      kstuffBatGetSerial( batSerialCurrent );
    }

    if( btn == 23 )
    {
      WriteFile("ms0:/batser.bin", batSerialCurrent, 4);
      pspDebugScreenSetXY( 0, 27 );
      printf(" > Current battery serial dumped to file....\n");
      sceKernelDelayThread( 2500000 );
    }

    if( btn == 24 )
    {
      kstuffBatSetSerial( batSerialNormal );
      pspDebugScreenSetXY( 0, 27 );
      printf(" > Battery set to serial from file....\n");
      sceKernelDelayThread( 2500000 );
      kstuffBatGetSerial( batSerialCurrent );
    }

    if( btn == 31 )
    {
      pspDebugScreenSetXY( 0, 27 );
      InstallMsIpl();
    }

    if( btn == 32 )
    {
      pspDebugScreenSetXY( 0, 27 );
      InstallTmIpl();
    }

    if( btn == 33 )
    {
      kstuffInitBuffers(dataPSAR, dataOut, dataOut2, dataMisc);
      RepartitionMemstick();
    }

    if( btn == 42 )
    {
      InstallEnhancedTools();
    }

    if( btn == 98 )
    {
      hcMiscMenuClear();
      hcMiscMenuSetName( "Main Menu" );
      hcMiscMenuAddEntry( "Install a DDCv4 MagicStick (for Fat + Slim)", 41, "   This option sets up a complete DDCv4 (DAX' universal unbricker)\n   MagicStick that will work for the Fat and Slim PSPs and will\n   flash a M33 CFW to your PSP.");
      hcMiscMenuAddEntry( "Install a DDCv6 MagicStick (for Fat + Slim)", 43, "   This option sets up a complete DDCv6 (DAX' universal unbricker)\n   MagicStick that will work for the Fat and Slim PSPs and will\n   flash a M33 CFW to your PSP.");
      hcMiscMenuAddEntry( "Install ELF-Menu and NAND-Tool to the DDCv4 stick", 42, "   This requires a set up DDCv4 stick and will add jas0nuk's ELF-Menu\n   and cory's NAND-Tool to it!\n\n   DOES NOT WORK ON DDCv6!" );
      hcMiscMenuAddEntry( "Install classic Pandora (Fat PSP and 1.50 FW only)", 11, "   This installs the old, original, classic Pandora\n   Working only on Fat PSPs, installing only 1.50!" );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "Battery options....", 12, "   ....more options for patching the battery" );
      hcMiscMenuAddEntry( "MemStick and MS-IPL options....", 13, "   ....more options regarding memstick actions" );
      hcMiscMenuAddEntry( " ", -1, " " );
      hcMiscMenuAddEntry( "Quit", 99, "   buybuy! ;-)" );
    }

    if( btn == 99 )
    {
      InitMainScreen();
      printf("bye....\n\n\nso long, and thanks for all the fish :-)" );
      sceKernelDelayThread( 5000000 );
      sceKernelExitGame();
    }

    InitMainScreen();
  }

  return 0;
}
