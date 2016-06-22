
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
#include "package/pspbtcnfupdater2.h"
#include "package/libpsar1.h"
#include "package/libpsar2.h"
#include "package/ireg.h"
#include "package/dreg.h"

PSP_MODULE_INFO("KernelStuff_driver", 0x1006, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

// function's we get imported from userland
//int (*userWriteFile) (char* filename, void* buffer, int size);
// doesn't work in 3.xx+ kernels :-(

u8 *dataPSAR, *dataOut, *dataOut2;
char *dataMisc;
int use371nids;

#include "psardump.c"

u8 sha1_150[20] =
{
	0x1A, 0x4C, 0x91, 0xE5, 0x2F, 0x67, 0x9B, 0x8B, 
	0x8B, 0x29, 0xD1, 0xA2, 0x6A, 0xF8, 0xC5, 0xCA, 
	0xA6, 0x04,	0xD3, 0x30
};

u8 sha1[20];

SceKernelUtilsSha1Context ctx;


// here we go....
int fnow = 0;
void SetInstallFileOffset(int o)
{
	fnow = o;
}
int DoInstall(int full)
{
	// the main installer routine....
	
	
	//SceUID fd;
	//int size; //, read, i;
	int r;
	
	if( (full == 1) || (full == 2) )
  {

    // whoa, this one is the real magic ;-)
    r = extract_psar(dataPSAR, 10149440, dataOut, dataOut2);
    if(r!=0){ return r; }
    	
    fnow++;
    
    if(fnow==1)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_dreg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.dreg", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==2)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_ireg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.ireg", dataOut2, r);
  	    return r;
      }
    }
    
    fnow--;
  }
  
  if( full != 2)
  {
    fnow++;
    if(fnow<3){ fnow=3; }
    if(fnow==3)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_bootmenu, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/bootmenu.elf", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==4)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_unemu, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/unemu.prx", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==5)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_loadmenu, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/loadmenu.prx", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==6)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnf, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/pspbtcnf.txt", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==7)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfgame, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/pspbtcnf_game.txt", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==8)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfupdater, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/pspbtcnf_updater.txt", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==9)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_libpsar1, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/libpsardumper.pv1", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==10)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_libpsar2, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/libpsardumper.pv2", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==11)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfupdater2, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/pspbtcnf_updater.pv2", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==12)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_dreg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.dreg", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==13)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_ireg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.ireg", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==14)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_dreg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.dpv1", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==15)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_ireg, 0);
      if( r < 0)
      {
      	fnow++;
      } else {
  	    WriteFile("ms0:/registry/system.ipv1", dataOut2, r);
  	    return r;
      }
    }
    
    if(fnow==16)
    {
  	  r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfupdater, 0);
      if( r < 0)
      {
    	  fnow++;
      } else {
  	    WriteFile("ms0:/kd/pspbtcnf_updater.pv1", dataOut2, r);
  	    return r;
      }
    }
  
  }
  
  return 0;
}

// write battery EEPROM
u32 sceSyscon_driver_1165C864(u8 addr, u16 data); 
// read battery EEPROM
u32 sceSyscon_driver_68EF0BEF(u8 addr); 
// same for 3.71
u32 sceSyscon_driver_40CBBD46(u8 addr, u16 data);
u32 sceSyscon_driver_B9C1B0AC(u8 addr);

int module_start(SceSize args, void *argp)
{
	int k = pspSdkSetK1(0);
	int kit = sceKernelDevkitVersion();
	if (kit < 0x03070110) // pre 3.71
	{
		use371nids = 0;
	} else if (kit == 0x03070110) { // 3.71!
		use371nids = 1;
	} else {  // newer then 3.71?!?!
		//pspSdkSetK1(k);
    //return -1;
  }
  
  pspSdkSetK1(k);
  return 0;
}

int module_stop()
{
  return 0;
}

int kstuffInitBuffers(u8 *b1, u8 *b2, u8 *b3, char *b4)
{
	dataPSAR = b1;
	dataOut  = b2;
	dataOut2 = b3;
	dataMisc = b4;
	return 0;
}

int kstuffMsInstallFW(void)
{
	return DoInstall(2);
}

int kstuffMsInstallAdditional(void)
{
	return DoInstall(0);
}

int kstuffBatMakeService(void)
{
	int r=0;
	int k;
	
	k = pspSdkSetK1(0);
	if( use371nids == 0)
	{
	  r = sceSyscon_driver_1165C864(0x07, 0xFFFF);
    r = sceSyscon_driver_1165C864(0x09, 0xFFFF);
  } else if ( use371nids == 1 ) {
  	r = sceSyscon_driver_40CBBD46(0x07, 0xFFFF);
    r = sceSyscon_driver_40CBBD46(0x09, 0xFFFF);
  }
  pspSdkSetK1(k);
  return r;
}

int kstuffBatMakeNormal(void)
{
	int r=0;
	int k;
	
	k = pspSdkSetK1(0);
	if( use371nids == 0)
	{
	  r = sceSyscon_driver_1165C864(0x07, 0x90CA);  // 0x12345678 as serial is boring ;-)
    r = sceSyscon_driver_1165C864(0x09, 0x0815);  // 0x90CA0815 looks more like it :-D
  } else if ( use371nids == 1 ) {
  	r = sceSyscon_driver_40CBBD46(0x07, 0x90CA);
    r = sceSyscon_driver_40CBBD46(0x09, 0x0815);
  }
  pspSdkSetK1(k);
  return r;
}

void kstuffSetUserFileWriteFunc(void* func)
{
	/*userWriteFile = func;*/
}

int kstuff__end(void)
{
	return 0;
}
