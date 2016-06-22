/*                      *\

  **********************
  **                  **
  **  PSP Core Stuff  **
  **                  **
  **********************

\*                       */

//ifdef __cplusplus
//extern "C" {
//#endif

#include <pspkernel.h>
#include "pspcore.h"


// Exit callback
int Exit_Callback(int arg1, int arg2, void *common)
{
	sceKernelExitGame();
	return 0;
}
// Callback thread 
int ExitCallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", Exit_Callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}
// Sets up the callback thread and returns its thread id
int SetupExitCallback(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", ExitCallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


//ifdef __cplusplus
//}
//#endif
