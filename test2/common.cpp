// common functions used by most modules

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <time.h>
#include "common.h"


SceKernelUtilsMt19937Context ctx;


void hcWait(float seconds)
{
	int i;

	for (i=1;i<=30*seconds;i++)
	{
		sceDisplayWaitVblankStart();
	}
}

void hcWaitVBlank(void)
{
	sceDisplayWaitVblankStart();
}

int RandomInit(void)
{
	sceKernelUtilsMt19937Init(&ctx, time(NULL));
	return 0;
}
// generates random number
int GetRandomNum(int lo, int hi)
{
	u32 rand_val = sceKernelUtilsMt19937UInt(&ctx);
	rand_val = lo + rand_val % hi;
	return (int)rand_val;
}
