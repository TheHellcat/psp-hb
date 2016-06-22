#include "sparta_sdk.h"

void _start()
{
	//unsigned char* vramtop=(unsigned char *)0x04000000;
  //unsigned long drawframe;
	unsinged char* vram = (unsigned char*)0x44000000;
	
	
	
	sceKernelExitGame();
	return;
}
