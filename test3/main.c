#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspsyscon.h>
#include <psprtc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#define appVerMa 1
#define appVerMi 65


PSP_MODULE_INFO("test", 0x1000, appVerMa, appVerMi);
PSP_MAIN_THREAD_ATTR(0);
// PSP_HEAP_SIZE_KB(600);


void hcMiscWaitForAnyButton( void )
{
/*  int btn;
  SceCtrlData pad;

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

  btn=1;
  while( btn != 0 )
  {
    sceCtrlReadBufferPositive( &pad, 1 );
    btn = pad.Buttons & 0xFFFF;
  }
  btn=0;
  while( btn == 0 )
  {
    sceCtrlReadBufferPositive( &pad, 1 );
    btn = pad.Buttons & 0xFFFF;
  }
*/ }

int hcReadFile(char* file, void* buf, int size)
{
  SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0777);

  if (fd < 0)
  {
    return -1;
  }

  int read = sceIoRead(fd, buf, size);

  if (sceIoClose(fd) < 0)
    return -1;

  return read;
}

int hcWriteFile(char* file, void* buffer, int size)
{
  int i;
  int pathlen = 0;
  char path[128];

  // because I'm so lazy, I need folders created "on the fly"
  // this does so, create every folder in the path of the
  // file we are to save.
  // A little bruty forcy, yah, but it does the job :-D
  for(i=1; i<(strlen(file)); i++)
  {
    if(strncmp(file+i-1, "/", 1) == 0)
    {
      pathlen=i-1;
      strncpy(path, file, pathlen);
      path[pathlen] = 0;
      sceIoMkdir(path, 0777);
    }
  }

  // now up to the file write....
  SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if (fd < 0)
  {
    return fd;
  }

  int written = sceIoWrite(fd, buffer, size);
  //printf( "-written: %s  (%i)\n", file, written );

  if (sceIoClose(fd) < 0)
    return fd;

  return written;
}

int hcRfHelperIoBridgeUnassign( char* dev )
{
  int r;
  int k;

  k = pspSdkSetK1(0);
  r = sceIoUnassign( dev );
  pspSdkSetK1(k);

  return r;
}


int main(int argc, char *argv[])
{
  int r;
  u8 dataPSAR[512000];
  
  printf( "***\n\n" );
  //dprintf( "***\n\n" );
  
  
  r = hcRfHelperIoBridgeUnassign( "flash0:" );
  printf( "hcRfHelperIoBridgeUnassign = 0x%08x\n", r );
  r = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);
  printf( "sceIoAssign = 0x%08x\n", r );
  
  //dataPSAR = (u8 *)memalign(0x40, 512000);
  printf( "dataPSAR = 0x%08x\n", (int)dataPSAR );
  
  r = hcReadFile( "flash0:/kd/vshmain.prx", dataPSAR, 512000 );
  printf( "hcReadFile = 0x%i\n", (int)dataPSAR );
  r = hcWriteFile( "flash0:/kd/balloon.prx", dataPSAR, r );
  printf( "hcWriteFile = 0x%i\n", (int)dataPSAR );
  
  hcMiscWaitForAnyButton();
  
  sceKernelExitGame();
  
  return 0;
  
  
  
}
