
/*
    #########################################################
    ##                                                     ##
    ##   R e c o v e r y   F l a s h e r                   ##
    ##   by ---==> HELLCAT <==---                          ##
    ##                                                     ##
    #########################################################

    ====  DDC-Launcher  ====
    -------------------------
    small helper app to load the original DDC menu from RF
    running as ressurection.prx replacement
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <stdlib.h>

PSP_MODULE_INFO("HCRF_DDCLauncherHelper", 0x800, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);
PSP_HEAP_SIZE_KB(32);

int main(int argc, char *argv[])
{
	int r;
	int nc=0;
	
	//pspDebugScreenInit();
	//pspDebugScreenPrintf( "\n  Launching DDC....\n\n" );
	
  // fire up the original DDC
 	sceIoChdir( "flash0:/kd" );
 	r = pspSdkLoadStartModule("flash0:/vsh/module/vlf.prx", PSP_MEMORY_PARTITION_USER);
 	r = pspSdkLoadStartModule("flash0:/vsh/module/intraFont.prx", PSP_MEMORY_PARTITION_USER);
  r = pspSdkLoadStartModule("flash0:/kd/dcman.prx", PSP_MEMORY_PARTITION_KERNEL);
  r = pspSdkLoadStartModule("flash0:/kd/iop.prx", PSP_MEMORY_PARTITION_KERNEL);
  r = pspSdkLoadStartModule("ms0:/TM/ddc_plain.prx", PSP_MEMORY_PARTITION_USER);
      
  // we don't leave ppl behind.... that goes for tempfiles as well :-p
  if( nc != 1 )
  {
    sceIoRemove( "ms0:/TM/ddc_plain.prx" );
    sceIoRemove( "ms0:/PSP/GAME/DDCLaunch/EBOOT.pbp" );
    sceIoRmdir( "ms0:/PSP/GAME/DDCLaunch" );
  }

  sceKernelSleepThread();
  return 0;
}
