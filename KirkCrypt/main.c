
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "intercom/ictypes.h"
#include "intercom.h"
#include "misc.h"


#define KIRK_BUFFER_SIZE (1024*1024)
#define FILE_BUFFER_SIZE (1024*1024*2)


PSP_MODULE_INFO("KirkCrypt", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(10240);



int main(int argc, char *argv[])
{
	intercomDataHeader TestDataHeader;
	//unsigned char __attribute__((aligned(64))) TestData[16];
  unsigned char* kirkbuffer;
  unsigned char* filebuffer;
	SceUID mod;
	int r;
	int fz;
	
	pspDebugScreenInit();
	pspDebugScreenPrintf( "loading intercom.prx.... " );
	sceKernelDelayThread( 1000000 );
	mod = pspSdkLoadStartModule("intercom.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    pspDebugScreenPrintf( " FAILED: 0x%08x\n", mod);
    sceKernelDelayThread( 10000000 );
    sceKernelExitGame();
  }
  pspDebugScreenPrintf( "OK\n" );
  
  sceKernelDelayThread( 1000000 );
  pspDebugScreenPrintf( "allocating KIRK workbuffer (%i bytes).... ", KIRK_BUFFER_SIZE );
  sceKernelDelayThread( 500000 );
  kirkbuffer = (unsigned char*)memalign( 64, KIRK_BUFFER_SIZE );
  if( (int)kirkbuffer == 0 )
  {
  	pspDebugScreenPrintf( " FAILED:\n" );
    sceKernelDelayThread( 10000000 );
    sceKernelExitGame();
  }
  pspDebugScreenPrintf( "OK\n" );
  
  sceKernelDelayThread( 1000000 );
  pspDebugScreenPrintf( "allocating filebuffer (%i bytes).... ", FILE_BUFFER_SIZE );
  sceKernelDelayThread( 500000 );
  filebuffer = (unsigned char*)memalign( 64, FILE_BUFFER_SIZE );
  if( (int)filebuffer == 0 )
  {
  	pspDebugScreenPrintf( " FAILED:\n" );
    sceKernelDelayThread( 10000000 );
    sceKernelExitGame();
  }
  pspDebugScreenPrintf( "OK\n\n\n" );
  
  sceKernelDelayThread( 1000000 );
  
  intercomSetKirkWorkbuffer( (void*)kirkbuffer );
	
	//TestDataHeader.dataSize = 16;
	TestDataHeader.dataSeed = 0x234;
	TestDataHeader.mseedcount = 0;
	TestDataHeader.mseeds[0] = 0x134;
	TestDataHeader.mseeds[1] = 0x11C;
	TestDataHeader.mseeds[2] = 0x12A;
	TestDataHeader.mchunkSize = 16;
	
	TestDataHeader.usecustomkey = 1;
	TestDataHeader.customkey[0] = 0xCA;
	TestDataHeader.customkey[1] = 0x80;
	TestDataHeader.customkey[2] = 0x12;
	TestDataHeader.customkey[3] = 0x23;
	TestDataHeader.customkey[4] = 0x34;
	TestDataHeader.customkey[5] = 0x45;
	TestDataHeader.customkey[6] = 0x56;
	TestDataHeader.customkey[7] = 0x67;
	TestDataHeader.customkey[8] = 0x78;
	TestDataHeader.customkey[9] = 0x89;
	TestDataHeader.customkey[10] = 0x90;
	TestDataHeader.customkey[11] = 0xAA;
	TestDataHeader.customkey[12] = 0x77;
	TestDataHeader.customkey[13] = 0xED;
	TestDataHeader.customkey[14] = 0x09;
	TestDataHeader.customkey[15] = 0x42;
	
	fz = hcReadFile( "ms0:/in.bin", (void*)filebuffer, FILE_BUFFER_SIZE );
	TestDataHeader.dataSize = fz;
	pspDebugScreenPrintf( "infile size: %i bytes\n", fz );
	r = intercomEncryptBuffer( &TestDataHeader, (void*)filebuffer );
	pspDebugScreenPrintf( "enc. = 0x%08x\n", r );
	
	TestDataHeader.mseedcount = 3;
	r = intercomEncryptBuffer( &TestDataHeader, (void*)filebuffer );
	pspDebugScreenPrintf( "enc. = 0x%08x\n", r );
	
	hcWriteFile( "ms0:/enc.bin", (void*)filebuffer, fz );
	
	//filebuffer[fz/2] = 0;
	
	r = intercomDecryptBuffer( &TestDataHeader, (void*)filebuffer );
	pspDebugScreenPrintf( "dec. = 0x%08x\n", r );
  
  TestDataHeader.mseedcount = 0;
	r = intercomDecryptBuffer( &TestDataHeader, (void*)filebuffer );
	pspDebugScreenPrintf( "dec. = 0x%08x\n", r );
	
	hcWriteFile( "ms0:/dec.bin", (void*)filebuffer, fz );
  
  sceKernelExitGame();
  
  return 0;
}
