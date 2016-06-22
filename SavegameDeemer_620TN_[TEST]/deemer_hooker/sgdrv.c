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

// ***************************************************************** //
void cernLhcPatchSyscall(u32 addr, void *newaddr)
{
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
// ***************************************************************** //
void* pspGetSysCallAddr(u32 addr) 
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
hcWriteFile("ms0:/deemer9a.bin", &ptr, 4);
hcWriteFile("ms0:/deemer9b.bin", ptr, 4);
hcWriteFile("ms0:/deemer9c.bin", *ptr, 0x20);

  head = (struct SyscallHeader *) *ptr; 
  syscalls = (u32*) (*ptr + 0x10); 
  size = (head->size - 0x10);
hcWriteFile("ms0:/deemerA.bin", &size, 4);
  for(i = 0; i < size; i++) 
  { 
    if(syscalls[i] == addr) 
    { 
      return &syscalls[i]; 
    } 
  } 
hcWriteFile("ms0:/deemerB.bin", syscalls, 1024);

  return NULL; 
}

void* pspPatchProcCall(u32* addr, void* func) 
{
  if(!addr) 
  { 
    return NULL; 
  } 
  *addr = (u32) func; 
  
  sceKernelDcacheWritebackInvalidateRange(addr, sizeof(addr)); 
  sceKernelIcacheInvalidateRange(addr, sizeof(addr)); 
  
  return addr; 
}

// the hook-a-boo
int (*oUtilitySavedataInitStart)(SceUtilitySavedataParam* params);
int patchedUtilitySavedataInitStart(SceUtilitySavedataParam* params)
{
	int r;
	int k;
r=1; int i = hcWriteFile("ms0:/deemer6.bin", &r, 4);
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
r=1; int i = hcWriteFile("ms0:/deemer5.bin", &r, 4);
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
  //pspPatchProcCall(pspGetSysCallAddr((u32)oProcAddr), &patchedUtilitySavedataGetStatus);
  cernLhcPatchSyscall((u32)oProcAddr, &patchedUtilitySavedataGetStatus);
  oUtilitySavedataGetStatus = (void*)oProcAddr;
int r=(int)(pspGetSysCallAddr((u32)oProcAddr)); int i = hcWriteFile("ms0:/deemer7.bin", &r, 4);
  oProcAddr = (void*)pspFindProc("sceUtility_Driver", "sceUtility", 0x50C4CD57);
  //pspPatchProcCall(pspGetSysCallAddr((u32)oProcAddr), &patchedUtilitySavedataInitStart);
  cernLhcPatchSyscall((u32)oProcAddr, &patchedUtilitySavedataInitStart);
  oUtilitySavedataInitStart = (void*)oProcAddr;
r=(int)(pspGetSysCallAddr((u32)oProcAddr));   i = hcWriteFile("ms0:/deemer8.bin", &r, 4);
  
  return (int)oProcAddr;
}


void hcDeemerDriverSetupCallbackCapturedSDParams(int CallbackID1, int CallbackID2)
{
  hcDeemerDriverCapturedSDParamsCallback = CallbackID1;
  hcDeemerSDGetStatusCallback = CallbackID2;
}
