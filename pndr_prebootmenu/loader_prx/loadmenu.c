/*
 * simple loader for the menu
 * to (hopefully) better stability
 */
 
#include <pspkernel.h>
#include <pspdebug.h>
//#include <pspdisplay.h>
//#include <pspctrl.h>
//#include <pspsyscon.h>
//#include <psppower.h>
#include <pspsdk.h>
#include <string.h>
//#include <stdlib.h>

PSP_MODULE_INFO("MenuLoader", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);


char BLOB[1] __attribute__((aligned(64)));
struct SceKernelLoadExecParam param;

 
char hcMemPeek(void *addr)
{
	// adapted from the kernel sample from the PSPSDK :-)
	char memdata[1] __attribute__((aligned(64)));
	
	memcpy(memdata, addr, 1);  // 1 = len
	return (char)memdata[0];
}

void hcMemPoke(void *addr, void *value)
{
	memcpy(addr, value, 1);
}



int main(int argc, char *argv[])
{
	int load = 0;
	
	sceKernelDelayThread(1500000);
	
	load = hcMemPeek( (void*)0x883FFF00 ) & 0xFF;
	
	if( load == 0xCA )
	{
		BLOB[0] = 0xC9;
    hcMemPoke( (void*)0x883FFF00, BLOB );
    pspSdkLoadStartModule("flash0:/vsh/module/vshmain.prx", PSP_MEMORY_PARTITION_USER);
    sceKernelDelayThread(1500000);
    sceKernelExitDeleteThread(0);
	} else {
		pspDebugScreenInit();
		pspDebugScreenSetBackColor(0x00008000);
    pspDebugScreenClear();

    memset(&param, 0, sizeof(param));

    param.size = sizeof(param);
    param.argp = "flash0:/kd/bootmenu.elf stage2";
    param.args = 31;
    param.key = "game";

    sceKernelLoadExec("flash0:/kd/bootmenu.elf", &param);
	}
	
	return 0;
}
