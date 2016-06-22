/**************************************************\
 **                                              **
 ** ---==> HELLCAT <==---                        **
 **       presents                               **
 **                                              **
 **   Wallpaper Changer                          **
 **                                              **
\**************************************************/

//  --------------------------------------------  //
//       !!! released under the GPL !!!           //
//  --------------------------------------------  //
//            open source for win!                //
//  --------------------------------------------  //


// [voice=tv announcer] Following up next: a f00kin' long list of includz [/voice]
// (gotta check which one of them I can scrap....)
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspiofilemgr.h>
#include <psprtc.h>
#include <pspctrl.h>
#include <pspsysmem.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>


PSP_MODULE_INFO("hcWallpaperChanger", 0x1006, 1, 20);
PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(64);

//#define printf pspDebugScreenPrintf


////////////////////////////////////////////
//                                        //
//   D&D - Definitions and Declarations   //
//                                        //
////////////////////////////////////////////


struct SyscallHeader
{
  void *unk;
  unsigned int basenum;
  unsigned int topnum;
  unsigned int size;
};

struct MallocEntry
{
  int hcMallocID;
  SceUID SceMemID;
  void* addr;
} MallocList[32];


char* dirbuffer;
int direntrycount = 0;
SceKernelUtilsMt19937Context RndCtx;
void* oProcAddr;
void* oProcAddrWO;
char rndwppath[128];
int mainthreadfinished;
int MallocCount = 0;
int apply12BMP = 0;
int applyGB = 0;
int applyWave = 0;
int applyFont = 0;


/////////////////////////
//                     //
//   Misc. Functions   //
//                     //
/////////////////////////


void* hcMalloc(int size)
{
  // yah.... since we don't have malloc() in a pure kernel module (WHY?), this is a wrapper
  // to replace malloc() with a similar easy to use function that uses some SCE syscalls
  // to grab a chunk of memory and some more voodoo to manage it

  SceUID memID;
  char memName[64];
  void* memAddr;

  MallocCount++;
  sprintf( memName, "BigzChunkzOfRAMz%i", MallocCount );

  memID = sceKernelAllocPartitionMemory( 1, memName, PSP_SMEM_Low, size, 0 );
  if( memID < 0 ){ MallocCount--; return NULL; }
  memAddr = sceKernelGetBlockHeadAddr( memID );

  MallocList[MallocCount].hcMallocID = MallocCount;
  MallocList[MallocCount].SceMemID = memID;
  MallocList[MallocCount].addr = memAddr;

  return memAddr;
}

void hcFree(void* memAddr)
{
  // well.... if one has his own malloc() version, he also needs a way to free() it ;)

  int i;

  for(i=1; i<=MallocCount; i++)
  {
    // look for the entry to free
    if( MallocList[i].addr == memAddr )
    {
      // check valadity
      if( MallocList[i].hcMallocID == i )
      {
        // set willy free
        sceKernelFreePartitionMemory( MallocList[i].SceMemID );
        MallocList[i].addr = 0;
        MallocList[i].hcMallocID = 0;
      }
    }
  }
}


time_t hcTime( time_t* t )
{
  // same as for malloc() and free() - this is a standard C compatible
  // wrapper for time() - another goody that's not available in kernel mode
  // but luckiely we have psprtc_driver - which we are using here anyway :D

  pspTime tnow;
  unsigned int tr;

  sceRtcGetCurrentClockLocalTime( &tnow );

  // calculate no. of seconds since UN*X epoch
  // not 100% acurate, but good enough for now....
  tr = (tnow.year - 1970) * 365 * 24 * 60 * 60;
  tr += tnow.month * 30 * 24 * 60 * 60;
  tr += tnow.day * 24 * 60 * 60;
  tr += tnow.hour * 60 * 60;
  tr += tnow.minutes * 60;
  tr += tnow.seconds;

  return (time_t)tr;
}


void strMakeUCase(char* text)
{
  int i;
  u8 b;

  for(i=0; i<strlen( text ); i++)
  {
    b = text[i];
    if( ( b >= 0x61 ) && ( b <= 0x7A ) )
    {
      b -= 0x20;
      text[i] = b;
    }
  }
}


void GetImageList(char* path, char* filter)
{
  int i, j, k, r, k1;
  int fd = -1;
  SceIoDirent* dentry;
  char s[128];

  direntrycount = 0;
  k = 0;

  dentry = (SceIoDirent*)hcMalloc(sizeof(SceIoDirent));
  if (!dentry) { return; }
  memset(dentry, 0, sizeof(SceIoDirent));

  k1 = pspSdkSetK1(0);

  i = 0;
  r = 1;
  dirbuffer = hcMalloc(12800);
  if (!dirbuffer) { hcFree(dentry); pspSdkSetK1(k1); return; }
  fd = sceIoDopen(path);
  if( fd<0 ){ hcFree(dentry); hcFree(dirbuffer); pspSdkSetK1(k1); return; };
  while(r>0)
  {
    i++;
    r = sceIoDread( fd, dentry );
    if( r>0 )
    {
      memset(s, 0, 128);
      strcpy(s, dentry->d_name);
      strMakeUCase( s );
      if(((int)memchr(dentry->d_name, 0x25, strlen(dentry->d_name))==0) && ((int)memchr(dentry->d_name, 0x2E, 1)==0))
      {
        if( (strlen(dentry->d_name) > strlen(filter)) && ((strlen(filter) > 1)) )
        {
          k = 0;
          for(j=0; j<=(strlen(dentry->d_name)-strlen(filter)); j++)
          {
            if( strncmp(s+j, filter, strlen(filter)) == 0 )
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
  }
  sceIoDclose( fd );
  direntrycount = i - 1;

  hcFree(dentry);
  hcFree(dirbuffer);
  pspSdkSetK1(k1);
}


void InitRND(void)
{
  sceKernelUtilsMt19937Init(&RndCtx, hcTime(NULL));
}


int GetRND(int min, int max)
{
  if( max <= min )
  {
    return min;
  }

  u32 rndval = sceKernelUtilsMt19937UInt(&RndCtx);

  int n = ( rndval % ( (max+1) - min ) ) + min;

  return n;
}


void CopyFile(char* src, char* dst)
{
  u8* buffer;
  int bsize = 256000;
  int r = bsize;
  int k;

  k = pspSdkSetK1(0);

  buffer = (u8 *)hcMalloc( bsize );

  int f1 = sceIoOpen( src, PSP_O_RDONLY, 0777 );
  if( f1<0 ) { sceIoClose( f1 ); hcFree( buffer ); pspSdkSetK1(k); return; }

  int f2 = sceIoOpen( dst, PSP_O_RDONLY | PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777 );

  while (r == bsize)
  {
    r = sceIoRead(f1, buffer, bsize);
    sceIoWrite(f2, buffer, r);
  }

  sceIoClose(f1);
  sceIoClose(f2);

  pspSdkSetK1(k);
  hcFree( buffer );
}


/*
    --------------------------------------------------------------
      the usual three functions I always use for syscall hooking

      pspFindProc() is 99.9% based on Tyranid's PSPLINK ones,
      pspGetSysCallAddr() and pspPatchProcCall() are stitched
      together from various code snippets found on ps2dev
    --------------------------------------------------------------
*/

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

static void* pspPatchProcCall(u32 *addr, void *func)
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


/*
    the functions hooked into the syscalls....
*/

SceUID (*oIoOpen)(const char *file, int flags, SceMode mode);
static SceUID patchedIoOpen(char *file, int flags, SceMode mode)
{
  int r;

  if( ( strcmp(file, "flash0:/vsh/resource/01-12.bmp") == 0 ) || ( strcmp(file, "flash0:/vsh/resource/13-27.bmp") == 0 ) )
  {
    if( apply12BMP == 1  ){ strcpy(file, rndwppath); }
    r = oIoOpen(file, flags, mode);
    mainthreadfinished = 2;
    return r;
  }

  if( ( strcmp(file, "flash0:/vsh/resource/gameboot.pmf") == 0 ) && ( applyGB == 1 ) )
  {
    strcpy(file, "ms0:/PSP/gameboot.pmf");
    return oIoOpen( file, flags, mode );
  }

  if( ( strcmp(file, "flash0:/vsh/resource/system_plugin_bg.rco") == 0 ) && ( applyWave == 1 ) )
  {
    strcpy(file, "ms0:/PSP/xmbwave.rco");
    return oIoOpen( file, flags, mode );
  }

  if( ( strcmp(file, "flash0:/font/ltn0.pgf") == 0 ) && ( applyFont == 1 ) )
  {
    strcpy(file, "ms0:/PSP/font.pgf");
    return oIoOpen( file, flags, mode );
  }

  r = oIoOpen(file, flags, mode);
  return r;
}


// == main ================================================================= //

int mymain(SceSize args, void *argp)
{
  int pic;
  pspTime tnow;
  char datepaper[128];
  char wavepaper[128];
  int btn;
  SceCtrlData pad;
  int i;
  SceUID fd;
  int docopy = 0;
  void* SysCallAddr = 0;
  void* SysCallAddrWO = 0;
  void* tmpptr;
  int skipPTF;

  // beeing lazy r00lz :-D so we don't allocate mem and all that shyte, we just fill up a looooong string and overwrite the spaces with our filename later 8-)
  char* picpath  = "ms0:/PICTURE/WALLPAPER/                                                                                                                                  ";
  char* ptfpath  = "ms0:/PSP/THEME/                                                                                                                                          ";
  char* gameboot = "ms0:/PSP/GAMEBOOT/                                                                                                                                       ";
  char* wavepath = "ms0:/PSP/WAVE/                                                                                                                                           ";
  char* fontpath = "ms0:/PSP/FONT/                                                                                                                                           ";

  sceKernelDelayThread(500000);

  strcpy(rndwppath, "ms0:/picture/wavepaper.bmp");

  sceRtcGetCurrentClockLocalTime( &tnow );
  sceCtrlReadBufferPositive(&pad, 1);
  btn = pad.Buttons & 0xFFFF;

  sprintf( datepaper, "ms0:/PICTURE/DATEPAPER/%02i-%02i.bmp", tnow.month, tnow.day );

  InitRND();

  // get random wallpaper
  GetImageList("ms0:/PICTURE/WALLPAPER", ".BMP");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    strcpy(picpath+23, dirbuffer+((pic)*128));
  }

  // get random PTF theme
  GetImageList("ms0:/PSP/THEME", ".PTF");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    strcpy(ptfpath+15, dirbuffer+((pic)*128));
  }

  // get random wavepaper (wallpaper behind XMB waves)
  GetImageList("ms0:/PICTURE/WAVEPAPER", ".BMP");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    sprintf(wavepaper, "ms0:/PICTURE/WAVEPAPER/%s", dirbuffer+((pic)*128));

    // let the IoOpen hook know to apply the wavepaper patch
    apply12BMP = 1;
  }

  // get random gameboot
  GetImageList("ms0:/PSP/GAMEBOOT", ".PMF");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    strcpy(gameboot+18, dirbuffer+((pic)*128));

    // let the IoOpen hook know to apply the gameboot patch
    applyGB = 1;
  }

  // get random XMB wave
  GetImageList("ms0:/PSP/WAVE", ".RCO");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    strcpy(wavepath+14, dirbuffer+((pic)*128));

    // let the IoOpen hook know to apply the XMB wave patch
    applyWave = 1;
  }

  // get random font
  GetImageList("ms0:/PSP/FONT", ".PGF");
  if( direntrycount > 0 )
  {
    pic = GetRND(1, direntrycount);
    strcpy(fontpath+14, dirbuffer+((pic)*128));

    // let the IoOpen hook know to apply the font patch
    applyFont = 1;
  }

  // check for config stuff
  if(btn & PSP_CTRL_TRIANGLE)
  {
    i = 0;
    fd = sceIoOpen("ms0:/seplugins/wpclast.dat", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    sceIoWrite(fd, &i, sizeof(i));
    sceIoClose(fd);
  }
  if(btn & PSP_CTRL_CIRCLE)
  {
    sceIoRemove("ms0:/seplugins/wpclast.dat");
  }

  skipPTF = 0;
  fd = sceIoOpen( "ms0:/seplugins/wpnoptf.dat", PSP_O_RDONLY | PSP_O_WRONLY, 0);
  if (fd>0)
  {
    sceIoRead(fd, &i, sizeof(i));
    if( i == 0xC0DECA01 ){ skipPTF = 1; }
  }
  sceIoClose( fd );
  if(btn & PSP_CTRL_SQUARE)
  {
    if( skipPTF == 0 )
    {
      i = 0xC0DECA01;
      fd = sceIoOpen("ms0:/seplugins/wpnoptf.dat", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
      sceIoWrite(fd, &i, sizeof(i));
      sceIoClose(fd);
      skipPTF = 1;
    } else {
      sceIoRemove("ms0:/seplugins/wpnoptf.dat");
      skipPTF = 0;
    }
  }

  fd = sceIoOpen( "ms0:/seplugins/wpclast.dat", PSP_O_RDONLY | PSP_O_WRONLY, 0);
  if (fd<0)
  {
    docopy = 1;
  } else {
    sceIoRead(fd, &i, sizeof(i));
    if( i == tnow.day )
    {
      docopy = 0;
    } else {
      docopy = 1;
      i = tnow.day;
      sceIoLseek(fd, 0, SEEK_SET);
      sceIoWrite(fd, &i, sizeof(i));
    }
  }
  sceIoClose( fd );

  // if the config told us to perfom our magic, lets do it
  if( docopy == 1 )
  {
    CopyFile(picpath, "flash1:/vsh/theme/wallpaper.bmp");
    CopyFile(datepaper, "flash1:/vsh/theme/wallpaper.bmp");
    CopyFile(gameboot, "ms0:/PSP/gameboot.pmf");
    CopyFile(wavepath, "ms0:/PSP/xmbwave.rco");
    CopyFile(fontpath, "ms0:/PSP/font.pgf");
    if( skipPTF == 0 ){ CopyFile(ptfpath, "flash1:/vsh/theme/custom_theme.dat"); }
    CopyFile(wavepaper, rndwppath);
  }

  // patch sceIoOpen()
  oProcAddr = (void*)pspFindProc("sceIOFileManager", "IoFileMgrForUser", 0x109F50BC);
  SysCallAddr = pspGetSysCallAddr((u32)oProcAddr);
  pspPatchProcCall(SysCallAddr, &patchedIoOpen);
  oIoOpen = (void*)oProcAddr;

  // OK, please skip this part, DON'T LOOK AT THIS! ;)
  // This is an not so shiny hack/workaround to prevent breaking the CFW's own patch to sceIoOpen
  // If we wouldn't handle this, the XMBs GAME folder would not work propperly anymore, 'cause at this point
  // we patch sceIoOpen() before the CFW (i.e. VSHCTRL) does it.
  oProcAddrWO = (void*)pspFindProc("sceIOFileManager", "IoFileMgrForUser", 0xE8BC6571);  // sceIoCancel
  SysCallAddrWO = pspGetSysCallAddr((u32)oProcAddrWO);
  // WHA?!?! You're still looking? OK, we gotta "cache" the original sceIoOpen syscall
  //  somewhere in the sceIo* function table for VSHCTRL to find and patch it.
  //  We're gonna use the patched address later to restore sceIoOpen, and revert this
  //  one to the original.
  pspPatchProcCall(SysCallAddrWO, oProcAddr);
  // I know, it's not very elegant - but it works :p ....hopefully the "missused" syscall isn't used while we're going....

  mainthreadfinished = 1;

  // wait until our patched sceIoOpen has done its job
  while( mainthreadfinished == 1 ){ sceKernelDelayThread( 10000 ); }

  // restore original syscall
  if( oProcAddr != 0 )
  {
    tmpptr = (void*)(*(u32*)SysCallAddrWO);   //  O_O what a construct ;) only to defeat "dereferenceing void* pointer"
    if( tmpptr != oProcAddrWO )
    {
      // CFW has patched sceIoOpen
      oIoOpen = tmpptr;
    }

    // restore our missused syscall
    pspPatchProcCall(SysCallAddrWO, oProcAddrWO);
  }

  sceKernelSleepThread();
  return 0;
}


int module_start(SceSize args, void *argp)
{
  int tid;
  int r;

  mainthreadfinished = 0;

  tid = sceKernelCreateThread("WPCMainThread", mymain, 0x18, 0x10000, 0, NULL);
  r = sceKernelStartThread(tid, 0, NULL);

  while( mainthreadfinished == 0 ) { sceKernelDelayThread( 250000 ); };

  return 0;
}
int module_stop()
{
  return 0;
}
