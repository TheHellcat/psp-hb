
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspsdk.h>
#include <string.h>

PSP_MODULE_INFO("EBOOTPandoraLauncher", 0x1000, 1, 1);
PSP_MAIN_THREAD_ATTR(0);

#define printf pspDebugScreenPrintf

void main()
{
  int r;
  int i;

  pspDebugScreenInit();
  printf( "EBOOT Pandora Launcher\n------------------------\nHuge thanks, respect and credits to the creators\nof the PANDORA Battery/MemStick\n" );
  printf( "\n\n   launching Pandora menu....\n\n" );

  for(i=1; i<=180; i++)
  {
    r = sceDisplayWaitVblankStart();
  }

  r = sceKernelLoadExec("flash0:/kd/extprxs.elf", NULL );

  printf( "Error launching Pandora main: 0x%x\n\nMake sure you are running this from a Pandora booted 1.50\nand you followed the guide to prep it to run Pandora!",r );
  
  for(i=1; i<=600; i++)
  {
    r = sceDisplayWaitVblankStart();
  }

  sceKernelSleepThread();  

}
