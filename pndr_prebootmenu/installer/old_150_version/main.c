#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "pspcrypt.h"

#include "package/bootmenu.h"
#include "package/unemu.h"
#include "package/loadmenu.h"
#include "package/pspbtcnf.h"
#include "package/pspbtcnfgame.h"
#include "package/pspbtcnfupdater.h"
#include "package/ireg.h"
#include "package/dreg.h"

PSP_MODULE_INFO("EPBM_Installer", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define printf pspDebugScreenPrintf  // as usual *lol*

#include "psardump.c"

u8 *dataPSAR, *dataOut, *dataOut2;

u8 sha1_150[20] =
{
	0x1A, 0x4C, 0x91, 0xE5, 0x2F, 0x67, 0x9B, 0x8B, 
	0x8B, 0x29, 0xD1, 0xA2, 0x6A, 0xF8, 0xC5, 0xCA, 
	0xA6, 0x04,	0xD3, 0x30
};

u8 sha1[20];

SceKernelUtilsSha1Context ctx;


// here we go....

int DoInstall(int full)
{
	// the main installer routine....
	
	pspDebugScreenSetBackColor(0x00402000);
  pspDebugScreenSetTextColor(0x0080FFFF);
  pspDebugScreenClear();
  printf("Extended PANDORA (pre)boot menu INSTALLATION\n----------------------------------------------\nInstaller uses code for PSAR extraction from (guess who ;-) PSPPet\n\n\n\n");
  pspDebugScreenSetTextColor(0x00FFFFFF);
	
	SceUID fd;
	int size; //, read, i;
	int r;
	
	printf("\n\n");
	
  dataPSAR = (u8 *)memalign(0x40, 16703136+256);
  if (!dataPSAR) { printf("Cannot allocate memory for PSAR data....\n"); return -1; }

  dataOut = (u8 *)memalign(0x40,  2000000);
  if (!dataOut) { printf("Cannot allocate memory for buffer 1....\n"); return -1; }

  dataOut2 = (u8 *)memalign(0x40, 2000000);
  if (!dataOut2) { printf("Cannot allocate memory for buffer 2....\n"); return -1; }
	
	if( full == 1)
  {	
    printf("Checking ms0:/UPDATE.PBP....\n");

  	fd = sceIoOpen("ms0:/UPDATE.PBP", PSP_O_RDONLY, 0777);
	  if (fd < 0)
	  {
		  printf("Where is it? I can't seem to find it....\n");
		  return -1;
	  }
	  size = sceIoLseek(fd, 0, SEEK_END);
	  sceIoLseek(fd, 0, PSP_SEEK_SET);
	  sceIoRead(fd, dataOut, 200);
	  if (memcmp(dataOut, "\0PBP", 4) != 0)
	  {
		  sceIoClose(fd);
		  printf("Invalid PBP file.\n");
		  return -1;
	  }
	  size = size - *(u32 *)&dataOut[0x24];
	  sceIoLseek(fd, *(u32 *)&dataOut[0x24], PSP_SEEK_SET);
	  if (size != 10149440)
	  {
		  printf("Double check ms0:/UPDATE.PBP is of 1.50 FW - that's what we need!\n");
		  sceIoClose(fd);
		  return -1;
	  }
	  if (sceIoRead(fd, dataPSAR, 10149440) != 10149440)
	  {
		  printf("Double check ms0:/UPDATE.PBP is of 1.50 FW - that's what we need!\n");
		  sceIoClose(fd);
		  return -1;
  	}
	  sceIoClose(fd);

	  sceKernelUtilsSha1Digest(dataPSAR, 10149440, sha1);
	  if (memcmp(sha1, sha1_150, 20) != 0)
	  {
		  printf("Double check ms0:/UPDATE.PBP is of 1.50 FW - that's what we need!\n");
		  return -1;
	  }

    printf("\nOK, looking good so far, installing firmware to memorystick.\nThis can take a while....\n");
    // whoa, this one is the real magic ;-)
    r = extract_psar(dataPSAR, 10149440, dataOut, dataOut2);
  
    printf("Writing some default user settings (what's usually in flash1)....\n");
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_dreg, 0);
    if( r < 0)
    {
  	  printf("Error decompressing SYSTEM.DREG (0x%8x)\n",r);
    } else {
  	  WriteFile("ms0:/registry/system.dreg", dataOut2, r);
    }
  
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_ireg, 0);
    if( r < 0)
    {
  	  printf("Error decompressing SYSTEM.IREG (0x%8x)\n",r);
    } else {
  	  WriteFile("ms0:/registry/system.ireg", dataOut2, r);
    }
  }
  
  printf("\nInstalling additional files and updating system configuration....\n");
  //printf(" %s ", f_bootmenu+0x0A);
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_bootmenu, 0);
  if( r < 0)
  {
  	printf("Error decompressing BOOTMENU.ELF (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/bootmenu.elf", dataOut2, r);
  }
  
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_unemu, 0);
  if( r < 0)
  {
  	printf("Error decompressing UNEMU.PRX (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/unemu.prx", dataOut2, r);
  }
  
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_loadmenu, 0);
  if( r < 0)
  {
  	printf("Error decompressing LOADMENU.PRX (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/loadmenu.prx", dataOut2, r);
  }
  
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnf, 0);
  if( r < 0)
  {
  	printf("Error decompressing PSPBTCNF.TXT (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/pspbtcnf.txt", dataOut2, r);
  }
  
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfgame, 0);
  if( r < 0)
  {
  	printf("Error decompressing PSPBTCNF_GAME.TXT (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/pspbtcnf_game.txt", dataOut2, r);
  }
  
  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfupdater, 0);
  if( r < 0)
  {
  	printf("Error decompressing PSPBTCNF_UPDATER.TXT (0x%8x)\n",r);
  } else {
  	WriteFile("ms0:/kd/pspbtcnf_updater.txt", dataOut2, r);
  }
  
  return 0;
}

void InitMainScreen(void)
{
	pspDebugScreenInit();
	pspDebugScreenSetBackColor(0x00402000);
  pspDebugScreenSetTextColor(0x0080FFFF);
  pspDebugScreenClear();
  printf("Extended PANDORA (pre)boot menu INSTALLATION\n----------------------------------------------\nInstaller uses code for PSAR extraction from: (guess who ;-) PSPPet\n\n\n");
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf("You are about to install the extended Pandora-Boot-Menu.\n\n\nWhat do we need for installation?\n");
  printf("A plain Pandora magic memory stick is enough.\nNo need for an already installed firmware that boots from stick.\n\nThe installer takes care of everything.\n");
  printf("\n\n\nPress X to install with firmware setup\n");
  printf("Press [] if you already have a full 1.50 on your stick\n");
  printf("Press O to cancel and quit without doing anyhing\n");
  printf("\n\nYour choice?");
}

int main(int argc, char *argv[])
{
	int btn;
	SceCtrlData pad;
	int quit = 0;
	
	InitMainScreen();
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
	
	while(quit==0)
	{
    sceCtrlReadBufferPositive(&pad, 1);
    while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
      
    btn=0;
    while(btn==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
    }
    if(btn & PSP_CTRL_CIRCLE) { quit=1; }
    if(btn & PSP_CTRL_SQUARE) { DoInstall(0); quit=1; }
    if(btn & PSP_CTRL_CROSS) { DoInstall(1); quit=1; }
  }

  printf("\n\nFINISHED! Autoexit in 5 seconds....");
  sceKernelDelayThread(5000000);
	sceKernelExitGame();
	
	return 0;
}
