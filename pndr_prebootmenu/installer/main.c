/*
 *  Installer for the Extended Pandora Bootmenu
 */

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

PSP_MODULE_INFO("EPBMInstall", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(21000);

#define printf pspDebugScreenPrintf  // as usual *lol*

u8 *dataPSAR, *dataOut, *dataOut2;
char *dataMisc;

u8 sha1_150[20] =
{
	0x1A, 0x4C, 0x91, 0xE5, 0x2F, 0x67, 0x9B, 0x8B, 
	0x8B, 0x29, 0xD1, 0xA2, 0x6A, 0xF8, 0xC5, 0xCA, 
	0xA6, 0x04,	0xD3, 0x30
};

u8 sha1[20];

int InitFWInstall(void)
{
	SceUID fd;
	int size; //, read, i;
	// int r;
	
	printf("Reading and checking ms0:/UPDATE.PBP\n");
	fd = sceIoOpen("ms0:/UPDATE.PBP", PSP_O_RDONLY, 0777);
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
  printf("Press O to cancel and quit without doing anything\n");
  printf("\n\nYour choice?");
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

int main(int argc, char *argv[])
{
	int btn;
	SceCtrlData pad;
	int quit = 0;
	int r;
	
	InitMainScreen();
	
	//Perform150Patches();
	
	SceUID mod;
	mod = pspSdkLoadStartModule("kstuff.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (mod < 0) { printf(" Error 0x%08X loading/starting kstuff.prx.\n", mod); }
  
  dataPSAR = (u8 *)memalign(0x40, 16703136+256);
  if (!dataPSAR) { printf("Cannot allocate memory for PSAR data.... (0x%08x)\n", (int)dataPSAR); }

  dataOut = (u8 *)memalign(0x40,  2000000);
  if (!dataOut) { printf("Cannot allocate memory for buffer 1.... (0x%08x)\n", (int)dataOut); }

  dataOut2 = (u8 *)memalign(0x40, 2000000);
  if (!dataOut2) { printf("Cannot allocate memory for buffer 2.... (0x%08x)\n", (int)dataOut2); }
  	
  dataMisc = (char *)memalign(0x40, 1024);
  if (!dataOut2) { printf("Cannot allocate memory for buffer 3.... (0x%08x)\n", (int)dataOut2); }
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
	
	while(quit==0)
	{
    sceCtrlReadBufferPositive(&pad, 1);
    //while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
      
    btn=0;
    while(btn==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
    }
    if(btn & PSP_CTRL_CIRCLE) { quit=1; }
    if(btn & PSP_CTRL_SQUARE) { kstuffInitBuffers(dataPSAR, dataOut, dataOut2, dataMisc);
    	                          printf("\nInstalling additional files....");
    	                          sceIoRename("ms0:/registry/system.dreg", "ms0:/registry/system.dpv2");
    	                          sceIoRename("ms0:/registry/system.ireg", "ms0:/registry/system.ipv2");
    	                          r=1;
    	                          while(r!=0)
    	                          {
    	                          	 r=kstuffMsInstallAdditional();
    	                          	 if(r>0){ WriteFile(dataMisc, dataOut2, r); }
    	                          }
    	                          printf("\n\nALL DONE :-)\n");
    	                          quit=1; }
    if(btn & PSP_CTRL_CROSS) { r=InitFWInstall();
    	                         if(r!=0) { printf("\nERROR WHILE INITIALIZING: 0x%08x\n", r); }
    	                         kstuffInitBuffers(dataPSAR, dataOut, dataOut2, dataMisc);
    	                         printf("Installing firmware to memorystick.\nHang on, this will take a while, time to get a coffee.... ;-)");
    	                         sceIoRename("ms0:/registry/system.dreg", "ms0:/registry/system.dpv2");
    	                         sceIoRename("ms0:/registry/system.ireg", "ms0:/registry/system.ipv2");
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
    	                         printf("\n\nALL DONE :-)\n");
    	                         quit=1; }
  }

  printf("\n\nFINISHED! Autoexit in 5 seconds....");
  sceKernelDelayThread(5000000);
	sceKernelExitGame();
	
	return 0;
}
