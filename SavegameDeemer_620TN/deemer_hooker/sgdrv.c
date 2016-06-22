/*  ================================================================  *\
 *                                                                    *
 *  >> Savegame Deemer <<                                             *
 *  a tool to read and write back savegames                           *
 *  with the ability to gather the unique game key for FW2.00+ games  *
 *                                                                    *
 *  by ---==> HELLCAT <==---                                          *
 *                                                                    *
\*  ================================================================  */

//                                                                    //
//  Driver Module                                                     //
//  holding all kernel mode functions                                 //
////////////////////////////////////////////////////////////////////////


//   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   //
//      THIS SOURCE IS PUBLISHED UNDER THE GPL      //
//      everything released based on this MUST      //
//      be released containing it's source and      //
//      be    under   the    GPL    as    well      //
//   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!   //


#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsyscon.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <stdlib.h>


//////////////////////////////////////
//                                  //
//   Declarations and definitions   //
//                                  //
//////////////////////////////////////


struct SyscallHeader 
{ 
  void *unk; 
  unsigned int basenum; 
  unsigned int topnum; 
  unsigned int size; 
};

int hcDeemerDriverCapturedSDParamsCallback;
int hcDeemerSDGetStatusCallback;

// prototype hcWriteFile() from main.c for filewrite actions
int hcWriteFile(char* file, void* buffer, int size);


////////////////////////////
//                        //
//   Internal functions   //
//                        //
////////////////////////////


// I guess everyone knowing to use this snippet knows where it comes from ;-)))
u32 pspFindProc(const char* szMod, const char* szLib, u32 nid)
{
  struct SceLibraryEntryTable *entry;
	SceModule *pMod;
	void *entTab;
	int entLen;

	pMod = sceKernelFindModuleByName(szMod);

	if (!pMod)
	{
		// NOP //
		return 0;
	}
	
	int i = 0;

	entTab = pMod->ent_top;
	entLen = pMod->ent_size;
	
	while(i < entLen)
  {
		int count;
		int total;
		unsigned int *vars;

		entry = (struct SceLibraryEntryTable *) (entTab + i);

    if(entry->libname && !strcmp(entry->libname, szLib))
		{
			total = entry->stubcount + entry->vstubcount;
			vars = entry->entrytable;

			if(entry->stubcount > 0)
			{
				for(count = 0; count < entry->stubcount; count++)
				{
					if (vars[count] == nid)
					{
				    return vars[count+total];
				  }					
				}
			}
		}

		i += (entry->len * 4);
	}

	return 0;
}

// ////////////////////////////////////////////////////////////////////
// Thanx to Davee for pointing me to this version of PatchSyscall() :-)
void cernLhcPatchSyscallAndCreateSomeBlackHoles(u32 addr, void *newaddr)
{  // don't mind the weirdo func' name, was just in the mood for fun like this ;-)
	u32 *vectors, i;
	
	/* get the vectors address from the co-processor */
	__asm__ volatile ("cfc0 %0, $12\n" "nop\n" : "=r" (vectors));
	
	/* loop through them */
	for (i = 0; i < 0x1000; i++)
	{
		/* if this is the address */
		if (vectors[i + 4] == addr)
		{
			/* then replace it :D */
			vectors[i + 4] = (u32)newaddr;
		}
	}
}
// ////////////////////////////////////////////////////////////////////

// the old crap, not working anymore....
///////////////////////////////////////////
/* void* pspGetSysCallAddr(u32 addr) 
{ 
  struct SyscallHeader *head; 
  u32 *syscalls; 
  void **ptr; 
  int size; 
  int i; 

  asm( 
    "cfc0 %0, $12\n" 
    : "=r"(ptr) 
  ); 

  if(!ptr) 
  { 
    return NULL; 
  } 

  head = (struct SyscallHeader *) *ptr; 
  syscalls = (u32*) (*ptr + 0x10); 
  size = (head->size - 0x10);

  for(i = 0; i < size; i++) 
  { 
    if(syscalls[i] == addr) 
    { 
      return &syscalls[i]; 
    } 
  } 

  return NULL; 
}

void* pspPatchProcCall(u32 *addr, void *func) 
{
  if(!addr) 
  { 
    return NULL; 
  } 
  *addr = (u32) func; 
  
  sceKernelDcacheWritebackInvalidateRange(addr, sizeof(addr)); 
  sceKernelIcacheInvalidateRange(addr, sizeof(addr)); 
  
  return addr; 
} */

// the hook-a-boo
int (*oUtilitySavedataInitStart)(SceUtilitySavedataParam* params);
int patchedUtilitySavedataInitStart(SceUtilitySavedataParam* params)
{
	int r;
	int k;
	
	k = pspSdkSetK1(0);
	r = sceKernelNotifyCallback(hcDeemerDriverCapturedSDParamsCallback, (int)params);
	pspSdkSetK1(k);
	
	// give everything some time to become idle....
	sceKernelDelayThread(2500000);
	
  return oUtilitySavedataInitStart(params);
}

int (*oUtilitySavedataGetStatus)(void);
int patchedUtilitySavedataGetStatus(void)
{
  int r, cbr;
  int k;
	
  r = oUtilitySavedataGetStatus();
  
  if( r == 3 )
  {
	  k = pspSdkSetK1(0);
    cbr = sceKernelNotifyCallback(hcDeemerSDGetStatusCallback, 0);
	  pspSdkSetK1(k);
  
    // give everything some time to become idle....
	  sceKernelDelayThread(2500000);
  }
  
  return r;
}


////////////////////////////
//                        //
//   Exported functions   //
//                        //
////////////////////////////


int hcDeemerDriverPatchSavedataInitStart(void)
{
	void* oProcAddr;
	
  oProcAddr = (void*)pspFindProc("sceUtility_Driver", "sceUtility", 0x8874DBE0);
  cernLhcPatchSyscallAndCreateSomeBlackHoles( (u32)oProcAddr, &patchedUtilitySavedataGetStatus );
  oUtilitySavedataGetStatus = (void*)oProcAddr;
  /*  nawt longer wurkin' stuffs, kept for.... uhm.... dunno why I kept this, I just did.... o_o
  pspPatchProcCall(pspGetSysCallAddr((u32)oProcAddr), &patchedUtilitySavedataGetStatus);
  */
  
  oProcAddr = (void*)pspFindProc("sceUtility_Driver", "sceUtility", 0x50C4CD57);
  cernLhcPatchSyscallAndCreateSomeBlackHoles( (u32)oProcAddr, &patchedUtilitySavedataInitStart );
  oUtilitySavedataInitStart = (void*)oProcAddr;
  /*  // see comment above // (yay, a comment within a comment xD)
  pspPatchProcCall(pspGetSysCallAddr((u32)oProcAddr), &patchedUtilitySavedataInitStart);
  */
  
  return (int)oProcAddr;
}


void hcDeemerDriverSetupCallbackCapturedSDParams(int CallbackID1, int CallbackID2)
{
  hcDeemerDriverCapturedSDParamsCallback = CallbackID1;
  hcDeemerSDGetStatusCallback = CallbackID2;
}
