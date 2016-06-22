/*
 *  misc functions to support the app
 *
 *  code
 *
 ***/

#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "fastlz.h"

#define printf pspDebugScreenPrintf



// some vars
//////////////

char* MenuEntries[32];
char* MenuDesc[32];
char* MenuName;
int MenuEntryIds[32];
int MenuEntryCount = 0;
int MenuColorNormal;
int MenuColorSelected;
int MenuColorDesc;



// and some functions :-)
///////////////////////////

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


void hcMiscMenuSetColors( int norm, int sel, int desc )
{
  MenuColorNormal = norm;
  MenuColorSelected = sel;
  MenuColorDesc = desc;
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
    printf( "   %s", MenuEntries[i-1] );
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
  int r;

  hcMiscSeqFindFile( buffer, bufsize, filename, &o, &l );
  h = (u8*)(buffer+o-128);

  lz = h[4];

  if( lz == 1 )
  {
    r = fastlz_decompress(buffer+o, l, destbuffer, 5000000);
    l = r;
  } else {
    memcpy( destbuffer, buffer+o, l );
  }

  return l;
}
