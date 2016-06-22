
#include <pspsdk.h>
#include <pspkernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



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
