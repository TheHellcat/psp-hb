
/*
    #########################################################
    ##                                                     ##
    ##   R e c o v e r y   F l a s h e r                   ##
    ##   by ---==> HELLCAT <==---                          ##
    ##                                                     ##
    #########################################################

    ====  Function Library to keep main part clean  ====
    ----------------------------------------------------
*/

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "pspdecrypt.h"
#include "libpsardumper.h"
#include "fastlz.h"
#include "func.h"
#include "psardumper.h"

#define printf pspDebugScreenPrintf


char* MenuEntries[32];
char* MenuDesc[32];
char* MenuName;
int MenuEntryIds[32];
int MenuEntryCount = 0;
int MenuColorNormal;
int MenuColorSelected;
int MenuColorDesc;
int MenuColorDisabled;
char PBPVersion[32];


void CachePBPVersion( char* version )
{
  sprintf( PBPVersion, "%s", version);
}
int VerifyPBPVersion( int fw )
{
  int r;
  
  r = 0;
  
  printf( "Updater version is %s: ", PBPVersion );
  
  if( (fw==371) && (strcmp(PBPVersion, "3.71") == 0) )
  {
    r = 1;
  }
  if( (fw==401) && (strcmp(PBPVersion, "4.01") == 0) )
  {
    r = 1;
  }
  
  if( r == 1 )
  {
    printf( "OK!\n" );
  } else {
    printf("ERROR! Wrong version!\n\n  Please check the updater .PBP!\n\n" );
  }
  
  return r;
}


u32 pspFindProc(const char* szMod, const char* szLib, u32 nid)
{
  return hcRfHelperPspFindProc( szMod, szLib, nid );
}


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


void hcExtractUpdaterSepcialPRX( u8* b_dataPSAR, char* pbppath )
{
	/*if (magic == 0x5053507E) return true; // standard module
	else if (magic == (0x5053507E^0x75757575)) return true; // 3.7x and 3.5x chkdegen
	else if (magic == (0x5053507E^0xF1F1F1F1)) return true; // 3.7x nand/lfatfs
	else if (magic == (0x5053507E^0x25252525)) return true; // 3.5x nand/lfatfs
	else if (magic == (0x5053507E^0x43434343)) return true; // 3.40 chkdegen
	else if (magic == (0x5053507E^0x34343434)) return true; // 3.40 nand/lfatfs
	else if (magic == (0x5053507E^0x93939393)) return true; // 3.30 chkdegen
	else if (magic == (0x5053507E^0xDEDEDEDE)) return true; // 3.30 nand/lfatfs
	else if (magic == (0x5053507E^0x55555555)) return true; // 3.1x nand/lfatfs
	else if (magic == (0x5053507E^0x77777777)) return true; // 3.0x nand/lfatfs*/
  int dpsp_o;
  int dpsp_l;
  int psar_o;
  SceUID f;
  u32 xorkey = 0xF1F1F1F1;
  u32 morekeys[] = { 0x25252525, 0x34343434, 0xDEDEDEDE, 0x55555555, 0x77777777, 0 };
  int key;
  int foundprx;
  u32 tag;
  int i, j, r;
  u32 m;
  u8* modptr[2];
  int modlen[2];
  u8* buff_off;
  char s[64];

  f = sceIoOpen( pbppath, PSP_O_RDONLY, 0777);
  if (f<0)
  {
  	sprintf( s, "ms0:/%s", pbppath );
  	f = sceIoOpen( s, PSP_O_RDONLY, 0777);
  	if (f<0)
    {
      printf("\nERROR OPENING %s! (0x%08x)\n", pbppath, f);
      return;
    }
  }

  sceIoRead(f, b_dataPSAR, 40);

  dpsp_o = *(u32 *)&b_dataPSAR[0x20];
  psar_o = *(u32 *)&b_dataPSAR[0x24];
  dpsp_l = psar_o - dpsp_o;

  sceIoLseek(f, dpsp_o, SEEK_SET);
  sceIoRead(f, b_dataPSAR, dpsp_l);
  sceIoClose( f );
  buff_off = (u8*)(((int)b_dataPSAR + dpsp_l + 1024) & 0xFFFFFF00);
  r = pspDecryptPRX(b_dataPSAR, buff_off, dpsp_l);

  // locate and save the "hidden/scrambled" updater modules
  key = -1;
  foundprx = 0;
  while( 1 )
  {
  	tag = 0x5053507E ^ xorkey;
    for( i=0; i<dpsp_l; i+=4 )
    {
      m = *(u32*)(buff_off+i);
      if( m == tag )
      {
        modptr[0] = &buff_off[i];
        modlen[0] = (*(u32*)(buff_off+i+0x2C)) ^ xorkey;
        for ( j=0; j<modlen[0]; j+=4)
        {
          *(u32*)(modptr[0]+j) ^= xorkey;
        }
        if( strcmp( (char*)(modptr[0]+0x0A), "sceLFatFs_Updater_Driver" ) == 0 )
        {
          hcWriteFile( "lfatfs_updater.prx", modptr[0], modlen[0] );
          foundprx = 1;
        }
        if( strcmp( (char*)(modptr[0]+0x0A), "sceNAND_Updater_Driver" ) == 0 )
        {
          hcWriteFile( "nand_updater.prx", modptr[0], modlen[0] );
          foundprx = 1;
        }
      }
    }
    if( foundprx == 1 ) { break; }
    key++;
    xorkey = morekeys[key];
    if( xorkey == 0 ) { break; }
  }
  
  // locate and save some more unscrambled updater modules
  tag = 0x5053507E;
  for( i=0; i<dpsp_l; i+=4 )
  {
    m = *(u32*)(buff_off+i);
    if( m == tag )
    {
      modptr[0] = &buff_off[i];
      modlen[0] = (*(u32*)(buff_off+i+0x2C));
      if( strcmp( (char*)(modptr[0]+0x0A), "sceLflashFatfmtUpdater" ) == 0 )
      {
        hcWriteFile( "lflash_fatfmt_updater.prx", modptr[0], modlen[0] );
      }
    }
  }
}


void hcMiscMenuClear( void )
{
  int i;

  for( i=0; i<32; i++ )
  {
    MenuEntries[i] = 0;
    MenuEntryIds[i] = 0;
  }
  MenuEntryCount = 0;
}


void hcMiscMenuAddEntry( char* caption, int id , char* desc)
{
  MenuEntries[MenuEntryCount] = caption;
  MenuEntryIds[MenuEntryCount] = id;
  MenuDesc[MenuEntryCount] = desc;
  MenuEntryCount++;
}


void hcMiscMenuSetColors( int norm, int sel, int desc, int disabled )
{
  MenuColorNormal = norm;
  MenuColorSelected = sel;
  MenuColorDesc = desc;
  MenuColorDisabled = disabled;
}


void hcMiscMenuSetName( char* NewMenuName )
{
  MenuName = NewMenuName;
}


int hcMiscMenuShowMenu( int xpos, int ypos )
{
  int i;
  int r;
  int btn;
  char sdesc[512];
  SceCtrlData pad;

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

  pspDebugScreenSetTextColor( MenuColorSelected );
  pspDebugScreenSetXY( xpos, ypos );
  printf( MenuName );

  pspDebugScreenSetTextColor( MenuColorNormal );
  for( i=1; i<=MenuEntryCount; i++ )
  {
    pspDebugScreenSetXY( xpos, ypos+i+1 );
    if( MenuEntryIds[i-1] == -1 ) { pspDebugScreenSetTextColor( MenuColorDisabled ); }
    printf( "   %s", MenuEntries[i-1] );
    if( MenuEntryIds[i-1] == -1 ) { pspDebugScreenSetTextColor( MenuColorNormal ); }
  }

  r = 0;
  i = 0;
  while( r == 0 )
  {
    pspDebugScreenSetXY( xpos, ypos+i+2 );
    pspDebugScreenSetTextColor( MenuColorSelected );
    printf( "-> %s", MenuEntries[i] );
    pspDebugScreenSetXY( 0, ypos+5+MenuEntryCount );
    printf( "                                                                   \n" );
    printf( "                                                                   \n" );
    printf( "                                                                   \n" );
    printf( "                                                                   \n" );
    printf( "                                                                   " );
    pspDebugScreenSetXY( 0, ypos+5+MenuEntryCount );
    sprintf( sdesc, MenuDesc[i] );
    pspDebugScreenSetTextColor( MenuColorDesc );
    printf( sdesc );
    pspDebugScreenSetTextColor( MenuColorNormal );

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
    pspDebugScreenSetXY( xpos, ypos+i+2 );
    pspDebugScreenSetTextColor( MenuColorNormal );
    printf( "   %s", MenuEntries[i] );
    if( btn == PSP_CTRL_UP ) { i--; }
    if( btn == PSP_CTRL_DOWN ) { i++; }
    if( (MenuEntryIds[i] == -1) & (btn == PSP_CTRL_UP) ) { i--; }
    if( (MenuEntryIds[i] == -1) & (btn == PSP_CTRL_DOWN) ) { i++; }
    if( i > MenuEntryCount-1 ) { i = 0; }
    if( i < 0 ) { i = MenuEntryCount-1; }
    if( btn == PSP_CTRL_CROSS ) { r = MenuEntryIds[i]; }
  }

  return r;
}


void hcMiscWaitForAnyButton( void )
{
  int btn;
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
}


// this function expects the name of the file to find in the SEQ archive
// in "filname", the buffer where the archive is loaded into in "buffer"
// (size of the buffer in "bufsize")
// and returns the start/offset in "start" and the filesize in "size"
void hcMiscSeqFindFile( u8* buffer, int bufsize, char* filename, int* start, int* size )
{
  int i;
  int o;
  int l;

  o = 0;
  i = 0;
  l = 0;
  while( o == 0 )
  {
    if( i >= bufsize )
    {
      o = -1;
    } else {
      l = ( buffer[i] * 0x01000000 ) + ( buffer[i+1] * 0x00010000 ) + ( buffer[i+2] * 0x00000100 ) + buffer[i+3];
      if( strcmp( filename, (char*)buffer+i+32 ) == 0 )
      {
        o = i + 128;
      }
      i += l + 128;
    }
  }

  if( o == -1) { l = 0; }

  *start = o;
  *size  = l;
}


int hcMiscSeqGetFile( u8* buffer, int bufsize, char* filename, u8* destbuffer )
{
  int o;
  int l;
  u8* h;
  int lz;
  int sc;
  char sc1;
  char sc2;
  int r;
  int i, j;

  hcMiscSeqFindFile( buffer, bufsize, filename, &o, &l );
  if( o == -1 ) { return -1; }
  h = (u8*)(buffer+o-128);

  lz = h[4];
  sc = h[5];

  if( sc == 1 )
  {
    j = 0x11;
    for( i=1; i<=l; i++ )
    {
      sc2 = buffer[o+i-1] ^ j;
      sc2 ^= 0xAA;
      if( j == 0xFF ) { j = 0x11; } else { j += 0x11; }
      sc1 = (sc2 & 0x01) * 0x80;
      sc2 = ((sc2 >> 1) & 0x7F) + sc1;
      buffer[o+i-1] = sc2;
    }
  }

  if( lz == 1 )
  {
    r = fastlz_decompress(buffer+o, l, destbuffer, 5000000);
    l = r;
  } else {
    memcpy( destbuffer, buffer+o, l );
  }

  return l;
}


int _(int __, int ___, int ____)   // fill in the missing words *lol*
{
  int r;

  r = __ ^ ( ( ___ << ( ( ____ % 24 ) + 1 ) ) + ___ );
  r = ( r << ( ( ___ % 6 ) + 2 ) ) ^ r;

  return r;
}


int _moduleCheck( void* _moduleStart, void* _moduleEnd )
{
  // internal function
  // checks integrity of the to memory loaded binary itself
  int i;
  int v;
  int b;
  int c;

  v = 0xCA074011;
  for( i=(int)_moduleStart; i<=(int)_moduleEnd; i++ )
  {
    c = i - (int)_moduleStart;
    b = *(unsigned char*)(i);
    v = _(v, b, c);
  }

  return v;
}


void ClearScreen( void )
{
  pspDebugScreenSetBackColor(0x00000022);
  pspDebugScreenSetTextColor(0x00FFFFFF);
  pspDebugScreenClear();
  pspDebugScreenSetXY(0, 30);
  printf( "-------------------------------------------------------------------\n" );

  /*pspDebugScreenSetTextColor(0x000000FF);
  pspDebugScreenSetXY(0, 6);
  printf( "     BETA   **   BETA   **   BETA   **   BETA   **   BETA   **\n");
  pspDebugScreenSetXY(0, 32);
  printf( "     BETA   **   BETA   **   BETA   **   BETA   **   BETA   **\n");*/
  
  /*pspDebugScreenSetTextColor(0x0000FFFF);
  pspDebugScreenSetXY(0, 6);
  printf( "     WIP!   **   WIP!   **   WIP!   **   WIP!   **   WIP!   **\n");
  pspDebugScreenSetXY(0, 32);
  printf( "     WIP!   **   WIP!   **   WIP!   **   WIP!   **   WIP!   **\n");*/

  pspDebugScreenSetXY(0, 0);
  pspDebugScreenSetTextColor(0x0080FFFF);
  printf( "  **   HELLCAT's CFW Recovery Flasher   **\n" );
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( "-------------------------------------------------------------------\n" );
  printf( "\n" );
  pspDebugScreenSetTextColor(0x00CCCCCC);
}


void ClearNote( void )
{
  int x,y;

  x = pspDebugScreenGetX();
  y = pspDebugScreenGetY();

  pspDebugScreenSetXY(0, 27);
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( "                                                                   \n" );
  printf( "                                                                   \n" );
  printf( "                                                                   " );

  pspDebugScreenSetTextColor(0x00CCCCCC);
  pspDebugScreenSetXY(x, y);
}


void PrintNote( char* text )
{
  char s[256];
  int x,y;

  x = pspDebugScreenGetX();
  y = pspDebugScreenGetY();

  pspDebugScreenSetXY(0, 27);
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf( "-------------------------------------------------------------------\n" );
  printf( "                                                                   \n" );
  printf( "                                                                   " );
  pspDebugScreenSetXY(0, 28);
  pspDebugScreenSetTextColor(0x0080FFFF);
  sprintf( s, text );
  printf( s );

  pspDebugScreenSetTextColor(0x00CCCCCC);
  pspDebugScreenSetXY(x, y);
}


int CheckSigCheck( u8 *buffer )
{
  u8 nul[0x58];
  int r;
  
  if( strncmp( (char*)buffer, "~PSP", 4 ) != 0 ) { return 0; }
  
  memset( nul, 0, 0x58 );
  
  r = memcmp( (void*)nul, (void*)(buffer+0xD4), 0x58 );
  
  if( r == 0)
  {
    return 0;
  } else {
    return 1;
  }
}


u8 sigcheck_keys0[0x10] = { 
   0x71, 0xF6, 0xA8, 0x31, 0x1E, 0xE0, 0xFF, 0x1E, 
   0x50, 0xBA, 0x6C, 0xD2, 0x98, 0x2D, 0xD6, 0x2D 
}; 
u8 sigcheck_keys1[0x10] = { 
   0xAA, 0x85, 0x4D, 0xB0, 0xFF, 0xCA, 0x47, 0xEB, 
   0x38, 0x7F, 0xD7, 0xE4, 0x3D, 0x62, 0xB0, 0x10 
}; 
int RemoveSigCheck( u8* buffer )
{
  u8 enc[0xD0+0x14]; 
  int iXOR, res;
 
  memcpy(enc+0x14, buffer+0x80, 0xD0); 
 
  for (iXOR = 0; iXOR < 0xD0; iXOR++)
  { 
    enc[iXOR+0x14] ^= sigcheck_keys1[iXOR&0xF]; 
  } 
 
  if ((res = hcRfHelperPspDecryptSC((u32 *)enc, 0xD0)) < 0)
  { 
    return res; 
  } 
 
  for (iXOR = 0; iXOR < 0xD0; iXOR++)
  { 
    enc[iXOR] ^= sigcheck_keys0[iXOR&0xF]; 
  } 
 
  memcpy(buffer+0x80, enc+0x40, 0x90); 
  memcpy(buffer+0x110, enc, 0x40); 
 
  return 0; 
}


// do not pass trailing slashes in the paths!
int hcCopyFolder( char* path, char* destpath, u8* workbuffer, int buffersize, int verbose )
{
  SceIoDirent dentry;
  char s[128];
  char sNote[256];
  int fd;
  int copycount;
  int r;
  int sz1, sz2;
  char rs1[128];
  char rs2[128];
  SceIoStat fstat;
  int IsSC;
  int DoSC;

  memset(&dentry, 0, sizeof(SceIoDirent));

  fd = sceIoDopen( path );
  if( fd<0 ){ return 0; };
  copycount = 0;
  r = 1;
  while( r>0 )
  {
    r = sceIoDread( fd, &dentry );
    if( r>0 )
    {
      sprintf( s, "%s/%s", path, dentry.d_name );
      sceIoGetstat( s, &fstat );
      if( (fstat.st_mode & 0x1000) == 0x1000 )
      {
        if( (strcmp(".", dentry.d_name) == 0) || (strcmp("..", dentry.d_name) == 0) )
        {
          sz2 = 0;
        } else {
          sprintf( rs1, "%s/%s", path, dentry.d_name );
          sprintf( rs2, "%s/%s", destpath, dentry.d_name );
          sz1 = hcCopyFolder( rs1, rs2, workbuffer, buffersize, verbose );
          sz2 = 0;
          copycount += sz1;
        }
      } else {
        sz1 = hcReadFile( s, workbuffer, buffersize );
        if( sz1>0 )
        {
        	if( (CheckSigCheck( workbuffer ) == 1 ) ) //&& ((strncmp(dentry.d_name+(strlen(dentry.d_name)-4), ".prx", 4) == 0) || (strncmp(dentry.d_name+(strlen(dentry.d_name)-4), ".PRX", 4) == 0)) )
        	{
        	  IsSC = 1;
          } else {
          	IsSC = 0;
          }
        	if( dentry.d_name[0] == 0x24 ) { DoSC = 1; } else { DoSC = 0; }
        	
        	if( IsSC == 1 )
        	{
        	  RemoveSigCheck( workbuffer );
            sprintf( s, "%s/$%s", destpath, dentry.d_name );
        	} else if( DoSC == 1 ) {
        	  GenerateSigCheck( workbuffer );
        	  sprintf( s, "%s/%s", destpath, dentry.d_name+1 );
          } else {
            sprintf( s, "%s/%s", destpath, dentry.d_name );
          }
        	
          //sprintf( sNote, "  Copying file: %s\n  from %s to %s", dentry.d_name, path, destpath );
          sprintf( sNote, "  Copying file: %s\n  from %s", dentry.d_name, path );
          if( verbose == 1 ) { PrintNote( sNote ); }
          sceIoGetstat( s, &fstat );
          fstat.st_mode = 0x21FF;
          fstat.st_attr = 0x20;
          sceIoChstat( s, &fstat, 0x00FFFFFF );
          sz2 = hcWriteFile( s, workbuffer, sz1 );
        } else {
          sz2 = 0;
        }
      }
      if( sz2>0 ) { copycount++; }
    }
  }
  sceIoDclose( fd );

  if( verbose == 1 ) { ClearNote(); }
  return copycount;
}


int hcReadEBOOTList( char* path, char* listbuffer )
{
  SceIoDirent dentry;
  int fd;
  int c;
  int r;

  memset(&dentry, 0, sizeof(SceIoDirent));

  fd = sceIoDopen( path );
  if( fd<0 ){ return 0; };
  c = 0;
  r = 1;
  while( r>0 )
  {
    r = sceIoDread( fd, &dentry );
    if( r>0 )
    {
    	if( (strcmp( dentry.d_name, "." ) != 0) && (strcmp( dentry.d_name, ".." ) != 0) )
    	{
    	  sprintf( listbuffer+(64*c), "%s", dentry.d_name );
    	  c++;
    	}
    }
  }
  sceIoDclose( fd );

  return c;
}


int hcCheckForTM( void )
{
  SceIoDirent dentry;
  int fd;
  int c;
  int r;

  memset(&dentry, 0, sizeof(SceIoDirent));
  c = 0;

  fd = sceIoDopen( "flash0:/" );
  if( fd<0 ){ return -1; };
  c = 0;
  r = 1;
  while( r>0 )
  {
    r = sceIoDread( fd, &dentry );
    if( r>0 )
    {
    	if( strncmp( dentry.d_name, "tmctrl", 6 ) == 0 )
    	{
    	  c = 1;
    	}
    }
  }
  sceIoDclose( fd );

  return c;
}
