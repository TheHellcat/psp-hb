
/*

          **************************************************
          **                                              **
          **   HELLCAT's                                  **
          **   A r c a d e - P l a n e                    **
          **               game-engine                    **
          **                                              **
          **************************************************

*/

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pspgu.h>
#include <pspgum.h>

#include "arcpln.h"


#define myVerH 0
#define myVerL 1


PSP_MODULE_INFO("ArcadePlaneGameEngineCore", 0, myVerH, myVerL);
//PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
//PSP_HEAP_SIZE_KB(1024);



//
//  global vars
//////////////////

char gdatName[128];
int  gdatVer;

void* fbuf1;  // framebuffer 1
void* fbuf2;  // framebuffer 2 (draw buffer, switching woth fbuf1)
void* zbuf;   // GU depth buffer
void* fbuf;   // currently visible framebuffer

HcArcplnObjectInfo* ObjectList[256];
int ObjectListMax;
HcArcplnLightInfo ListList[4];

unsigned int __attribute__((aligned(16))) guList[1024 * 1024];



//
//  internal functions
/////////////////////////


// from http://wiki.pspdev.org/psp:ge_faq
void hcArcplnSwizzleTexture(u8* out, const u8* in, unsigned int width, unsigned int height)
{
// To use a swizzled texture pass GU_TRUE into the swizzled argument of sceGuTexMode: 
// sceGuTexMode(GU_PSM_8888,0,0,GU_TRUE);
// sceGuTexImage(0,widge,height,width,p_swizzled_data);

  unsigned int blockx, blocky;
  unsigned int i,j;
 
  unsigned int width_blocks = (width / 16);
  unsigned int height_blocks = (height / 8);
 
  unsigned int src_pitch = (width-16)/4;
  unsigned int src_row = width * 8;
 
  const u8* ysrc = in;
  u32* dst = (u32*)out;
 
  for (blocky = 0; blocky < height_blocks; ++blocky)
  {
    const u8* xsrc = ysrc;
    for (blockx = 0; blockx < width_blocks; ++blockx)
    {
      const u32* src = (u32*)xsrc;
      for (j = 0; j < 8; ++j)
      {
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        src += src_pitch;
      }
      xsrc += 16;
    }
    ysrc += src_row;
  }
}


void hcArcplnCleanupObjectlist( void )
{
	int i;
	int j;
	int o;
  
  j=1;
  for(i=1, i<=ObjectListMax; i++)
  {
    
  }
}



//
//  exported functions
/////////////////////////


int module_start(SceSize args, void *argp)
{
	int i;
	
	gdatVer = (myVerH * 0x10000) + (myVerL * 0x100);
	sprintf( gdatName, "ArcadePlane Game-Engine v%i.%02i - by HELLCAT", myVerH, myVerL );
	ObjectListMax = 0;
	for(i=0; i<=256; i++)
	{
	  ObjectList[i] = 0;
	}
	for(i=0; i<4; i++)
	{
	  LightList[i].posx = 0.0f;
    LightList[i].posy = 0.0f;
    LightList[i].posz = 0.0f;
    LightList[i].color = 0;
    LightList[i].att_const = 0.0f;
    LightList[i].att_linear = 0.0f;
    LightList[i].att_quad = 0.0f;
    LightList[i].specular = 0.0f;
	}
	
  return 0;
}
int module_stop( void )
{
  return 0;
}


void hcArcplnInitGU( void )
{
	int vsize;
	int i;
	
  // init VRAM pointers
  vsize = 4 * 512 * 272;   // 4 bytes/pixel, 512 pixel buffer width, 272 pixel hight = 544kByte
  fbuf1 = (void*)(0);
  fbuf2 = (void*)(fbuf1 + vsize);
  zbuf = (void*)(fbuf2 + vsize);
  
  fbuf = fbuf1;
  
  // fire up the GU
  sceGuInit();
	sceGuStart(GU_DIRECT, guList);
	sceGuDrawBuffer(GU_PSM_8888,fbuf2,512);
	sceGuDispBuffer(480,272,fbuf1,512);
	sceGuDepthBuffer(zbuf,512);
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuDepthRange(0xFFFF,0);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuDepthMask(GU_FALSE);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuEnable(GU_BLEND);
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuEnable(GU_LIGHT1);
	sceGuEnable(GU_LIGHT2);
	sceGuEnable(GU_LIGHT3);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	// clear all buffers
	for( i=1; i<=2; i++ )
  {
    sceGuStart(GU_DIRECT, guList);
    sceGuClearColor(0xFF000000);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
    sceGuFinish();
	  sceGuSync(0,0);
    sceGuSwapBuffers();
  }
}


void* hcArcplnGetFBufAddr( void )
{
  return (void*)(((int)fbuf) + ((int)sceGeEdramGetAddr()))
}


void hcArcplnAddObject( HcArcplnObjectInfo* NewObject )
{
  ObjectListMax++;
  ObjectList[ObjectListMax] = NewObject;
}


void hcArcplnCalcNewCoords( HcArcplnObjectInfo* Object )
{

}


void hcArcplnRenderScene( void )
{
	int i;
	ScePspFVector3 vec;
	
  sceGuStart(GU_DIRECT, guList);
	
  sceGuClearColor(0x00000000);
  sceGuClearDepth(0);
  sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	
  sceGumMatrixMode(GU_PROJECTION);
  sceGumLoadIdentity();
  sceGumPerspective(75.0f,16.0f/9.0f,0.1f,1000.0f);
  sceGumMatrixMode(GU_VIEW);
  sceGumLoadIdentity();
  
  // setup lights
  for( i=0; i<4; i++ )
  {
  	vec.x = LightList[i].posx;
  	vec.y = LightList[i].posy;
  	vec.z = LightList[i].posy;
    sceGuLight(i,GU_POINTLIGHT,GU_AMBIENT_AND_DIFFUSE,&vec);
		sceGuLightColor(i,GU_DIFFUSE,LightList[i].color);
		sceGuLightAtt(i,LightList.att_const,LightList.att_linear,LightList.att_quad);
	  sceGuSpecular(LightList[i].specular);
  }
  sceGuAmbient(0xFF808080);
  
  // setup objects and pass them to the GU
  for( i=0; i<=ObjectListMax; i++ )
  {
    sceGumMatrixMode(GU_MODEL);
	  sceGumLoadIdentity();
	  
	  sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	  
	  // coordinate translations
	  /*vec.x = 0.0f;
    vec.y = 0.0f;
    vec.z = 0.0f;
    sceGumTranslate(&vec);
    vec.x = 0.0f;
    vec.y = 0.0f;
    vec.z = 0.0f;
    sceGumRotateXYZ(&vec);*/
    
    sceGuTexMode(GU_PSM_8888,0,0,ObjectList[i].texSwizzled);
	  sceGuTexImage(0,ObjectList[i].texWidth,ObjectList[i].texHeight,ObjectList[i].texBufWidth,ObjectList[i].guTextData);
	  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	  sceGuTexEnvColor(0xFF222222);
	  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	  sceGuTexScale(1.0f,1.0f);
	  sceGuTexOffset(0.0f,0.0f);
	  sceGuAmbientColor(0xFF222222);
	  
	  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,ObjectList[i].guModelNumTris*3,0,ObjectList[i].guModelData);
  }
  
  // finish scene
  sceGuFinish();
  sceGuSync(0,0);

  // swap buffers
  sceDisplayWaitVblankStart();
  sceGuSwapBuffers();
}
