
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspsyscon.h>
#include <string.h>
#include "flashemu.h"

PSP_MODULE_INFO("unemuprx", 0x1000, 1, 0);


char BLOB[1] __attribute__((aligned(64)));

SceUID (*oKernelLoadModule)(const char *path, int flags, SceKernelLMOption *option);

struct SyscallHeader 
{ 
  void *unk; 
  unsigned int basenum; 
  unsigned int topnum; 
  unsigned int size; 
}; 

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

SceUID wKernelLoadModule(const char *path, int flags, SceKernelLMOption *option)
{
	char newpath[strlen(path)];
	int i;
	
	if(strncasecmp(path, "flash0:/", 8)==0)
	{
		// when a module is requested from flash0:/...., patch
		// the path to ms0:/....
		// so, the app will be able to load the module, even though
		// flash emulation is disabled :-)
		strncpy(newpath, "ms0:/", 5);
		for(i=5;i<strlen(path)-3;i++)
		{
		  newpath[i]=path[i+3];
		}
		newpath[i]=0;
		
		// call the original LoadModule with the new path....
    return oKernelLoadModule(newpath, flags, option);
	} else {
		// ....or with the original path if we didn't have to patch it
    return oKernelLoadModule(path, flags, option);
	}
}

// Based on FindProc by tyranid's psplink code
u32 FindProc(const char* szMod, const char* szLib, u32 nid)
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

// Heureka, found the following two here:
// http://forums.ps2dev.org/viewtopic.php?t=6734
void *GetSysCallAddr(u32 addr) 
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

static void *PatchProcCall(u32 *addr, void *func) 
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

// worker thread, keep the module alive to keep the patched function valid
int worker_thread(SceSize args, void *argp) //int argc, char *argv[])
{ 
  while(1) 
  { 
    sceKernelDelayThread(1000000); 
  } 
  return 0; 
} 




int main(int argc, char *argv[])
//int module_start(SceSize args, void *argp) __attribute__((alias("_start"))); 
//int _start(SceSize args, void *argp)
{
	// give the system some more time to boot everything up
  sceKernelDelayThread(2000000);  // 2 seconds should be good....
		
	int bootstat = hcMemPeek( (void*)0x883FFF00 ) & 0xFF;
	if(  bootstat == 0xFA )
	{
		/* we are supposed to disable flash emulation....
		 * ....lets do so :-)
		 */
		
		// reset the "marker" so, we'll not unintentionally disable it again on another boot
		BLOB[0] = 0x00;
    hcMemPoke( (void*)0x883FFF00, BLOB );
    
    // patch (redirect) sceKernelLoadModule for changing requests from flash0:/ to ms0:/
    void* oProcAddr = (void *)FindProc("sceModuleManager", "ModuleMgrForUser", 0x977DE386);
    PatchProcCall(GetSysCallAddr((u32)oProcAddr), wKernelLoadModule);
    oKernelLoadModule = (void*)oProcAddr;
		
		// now lets disable the emulation :-D
		pspFlashEmuDisableEmulation();
		
		// setup worker, to keep the module allive and go to sleep
		SceUID thid;
	  thid = sceKernelCreateThread("unemu_worker", worker_thread, 100, 0x1000, 0, NULL); 
	  sceKernelSleepThread();
	} else if(  bootstat == 0xC9 ) {
		// menu is to be skipped at next VSH reboot
		BLOB[0] = 0xCA;
    hcMemPoke( (void*)0x883FFF00, BLOB );
    sceKernelExitDeleteThread(0);
	} else {
		sceKernelExitDeleteThread(0);
	}
  
	return 0;
}

/* Exported functions */
void* getModuleInfo(void)
{
	return (void *) &module_info;
}

int hcUnEmuGetVerId(void)
{
	return 0x0100;
}
