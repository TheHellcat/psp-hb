/*
 * some system near functions
 *
 */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <string.h>
#include <stdlib.h>
#include "lflashfatfmt.h"


#define printf pspDebugScreenPrintf
#define setxy pspDebugScreenSetXY

void hcSysfuncFormatF0(void)
{
	char *argv[2];
	
	printf("Logical formating flash0:/.... ");

	if(sceIoUnassign("flash0:") < 0)
	{ 
		printf("FAILED: Could not unassign flash0:/");
	} else {
	  argv[0] = "fatfmt";
	  argv[1] = "lflash0:0,0";
	  int r;

	  if((r = sceLflashFatfmtStartFatfmt(2, argv)) < 0)
	  {
		  printf("FAILED: Could not format flash0:/\n  %08X.", r);
	  } else {
	  	if (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0) < 0)
	    {
		    printf("ERROR: Could not reassign flash0:/");
	    } else {
	    	printf("DONE :-)");
	    }
	  }
	}
}

void hcSysfuncFormatF1(void)
{
	char *argv[2];
	
	printf("Logical formating flash1:/.... ");

	if(sceIoUnassign("flash1:") < 0)
	{ 
		printf("FAILED: Could not unassign flash1:/");
	} else {
	  argv[0] = "fatfmt";
	  argv[1] = "lflash0:0,1";
	  int r;

	  if((r = sceLflashFatfmtStartFatfmt(2, argv)) < 0)
	  {
		  printf("FAILED: Could not format flash1:/\n  %08X.", r);
	  } else {
	  	if (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0) < 0)
	    {
		    printf("ERROR: Could not reassign flash1:/");
	    } else {
	    	printf("DONE :-)");
	    }
	  }
	}
}

void hcSysfuncFormatF2(void)
{
	char *argv[2];
	
	printf("Logical formating flash2:/.... ");

	if((sceIoUnassign("flash2:") < 0) & 0)
	{ 
		printf("FAILED: Could not unassign flash2:/");
	} else {
	  argv[0] = "fatfmt";
	  argv[1] = "lflash0:0,2";
	  int r;

	  if((r = sceLflashFatfmtStartFatfmt(2, argv)) < 0)
	  {
		  printf("FAILED: Could not format flash2:/\n  %08X.", r);
	  } else {
	  	if (sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0) < 0)
	    {
		    printf("ERROR: Could not reassign flash2:/");
	    } else {
	    	printf("DONE :-)");
	    }
	  }
	}
}

void hcSysfuncMkF1Dirs(void)
{
	printf("Creating flash1:/ directories.... ");
	sceIoUnassign("flash1:");
	sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
	sceIoMkdir("flash1:/dic", 0777);
	sceIoMkdir("flash1:/registry", 0777);
	sceIoMkdir("flash1:/vsh", 0777);
	sceIoMkdir("flash1:/vsh/theme", 0777);
	sceIoMkdir("flash1:/net", 0777);
	sceIoMkdir("flash1:/net/http", 0777);
	sceIoMkdir("flash1:/gps", 0777);
	printf("DONE :-)");
}

// allocate memory with a specific alignment!
// align_size has to be a power of two !!
void *hcAlignedMalloc(size_t size, size_t align_size)
{
  char *ptr,*ptr2,*aligned_ptr;
  int align_mask = align_size - 1;

  ptr=(char *)malloc(size + align_size + sizeof(int));
  if(ptr==NULL) return(NULL);

  ptr2 = ptr + sizeof(int);
  aligned_ptr = ptr2 + (align_size - ((size_t)ptr2 & align_mask));


  ptr2 = aligned_ptr - sizeof(int);
  *((int *)ptr2)=(int)(aligned_ptr - ptr);

  return(aligned_ptr);
}

void hcAlignedFree(void *ptr)
{

  int *ptr2=(int *)ptr - 1;
  ptr -= *ptr2;
  free(ptr);
}

int hcWriteFile(char *file, void *buf, int size)
{
	// automagic directory creation for those that don't exist already
	// no need to create target dir prior to writing the file
	// creates ALL subdirectories up to targetpath
	//   by ---==> HELLCAT <==---
	int i;
	int pathlen = 0;
	char path[64];
	// first, find last "/"
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
	// done :-) and on to the filewrite....
	
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0)
	{
		return -1;
	}

	int written = sceIoWrite(fd, buf, size);
	
	if (sceIoClose(fd) < 0)
		return -1;

	return written;
}

int hcReadFile(char *file, void *buf, int size)
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

// some quick'n'dirty filebrowser
// not pretty, but does it's job ;-)
//
// not much commenting here, let's just get over with it....
static char FileSelectorSelectedFile[128];
char* hcFileSelector(char* path, char* filter)
{
	int listwidth = 45;
	int listheight = 22;
	int listx = 20;
	int listy = 4;
	char frame1[80];
	char frame2[80];
	char frame3[80];
	int i, j, k, r, o, btn;
	char* dirbuffer;
	int direntrycount = 0;
	SceIoDirent *dentry;
	int listlength;
	int selected;
	char s[256];
	SceCtrlData pad;
	char nulstr[]="";
	
	if( filter == NULL ) { filter = nulstr; }
	
	dentry = malloc(sizeof(SceIoDirent));
	
	// draw "window"
	frame1[0] = 0x2B;              // "+"
	frame1[listwidth-1] = 0x2B;
	frame1[listwidth] = 0;
	frame2[0] = 0x3A;              // ":"
	frame2[listwidth-1] = 0x3A;
	frame2[listwidth] = 0;
	frame3[0] = 0x2B; // "+"
	frame3[listwidth-1] = 0x2B;
	frame3[listwidth] = 0;
  for(i=1; i<listwidth-1; i++)
  {
  	frame1[i] = 0x2D;            // "-"
  	frame2[i] = 0x20;            // " "
  	frame3[i] = 0x2D;
  }
  setxy(listx,listy);
  printf("%s",frame1);
  setxy(listx,listy+listheight-1);
  printf("%s",frame3);
  setxy(listx+2,listy);
  printf("[%s]", path);
  for(i=1; i<listheight-1; i++)
  {
  	setxy(listx,listy+i);
  	printf("%s",frame2);
  }
  i = 0;
  r = 1;
  dirbuffer = malloc(12800);
  int fd = sceIoDopen(path);
  while(r>0)
  {
  	i++;
  	r = sceIoDread( fd, dentry );
  	if(((int)memchr(dentry->d_name, 0x25, strlen(dentry->d_name))==0) & ((int)memchr(dentry->d_name, 0x2E, 1)==0))
  	{
  		if( (strlen(dentry->d_name) > strlen(filter)) & ((strlen(filter) > 1)) )
  		{
  			k = 0;
  			for(j=0; j<=(strlen(dentry->d_name)-strlen(filter)); j++)
  			{
  				if( strncmp(dentry->d_name+j, filter, strlen(filter)) == 0 )
  				{
  					strcpy(dirbuffer+(i*128), dentry->d_name);
  					k = 1;
  					if(r==0){ i--; }
  				}
  			}
  			if( k == 0 ) { i--; }
  		} else {
  			if( (strlen(filter) > 1) )
  	    {
  	    	i--;
  	    } else {
  	    	strcpy(dirbuffer+(i*128), dentry->d_name);
  	    }
  	  }
  	} else {
  		i--;
  	}
  }
  sceIoDclose( fd );
  if( strlen(filter)>1 ) { i++; }
  direntrycount = i - 1;
  listlength = listheight - 1;
  if( listlength > direntrycount ) { listlength = direntrycount + 1; }
  
  selected = 1;
  
  r=0;
  o=0;
  sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
  while(r==0)
  {
  	for(i=1; i<=listlength-1; i++)
    {
    	setxy(listx, listy+i);
    	printf("%s", frame2);
  	  setxy(listx+5, listy+i);
  	  strcpy(s, dirbuffer+((i+o)*128));
  	  s[listwidth-7] = 0;
  	  printf("%s", s);
  	}
  	setxy(listx+2,listy+selected);
  	printf("->");
  	
  	sceCtrlReadBufferPositive(&pad, 1);
    while((pad.Buttons & 0xFFFF) != 0) { sceCtrlReadBufferPositive(&pad, 1); }
      
    btn=0;
    while(btn==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
    }
    if(btn & PSP_CTRL_DOWN) { selected++; }
    if(btn & PSP_CTRL_UP) { selected--; }
    if(btn & PSP_CTRL_CROSS) { r = 1; }
    if(selected>listlength-1) { selected--; o++; }
    if(selected<1) { selected++; o--; }
    if(o>direntrycount-listlength+1) { o = direntrycount-listlength+1; }
    if(o<0) { o = 0; }
  }
  strcpy(FileSelectorSelectedFile, dirbuffer+((selected+o)*128));
  
  return FileSelectorSelectedFile;
}
