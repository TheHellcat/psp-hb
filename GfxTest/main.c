


#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <pspgu.h>
#include <pspgum.h>


PSP_MODULE_INFO("GFXTest", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(1024);


unsigned int __attribute__((aligned(16))) gulist1[262144];

struct Vertex
{
	float u, v;
	unsigned int color;
	float x,y,z;
};

struct Vertex __attribute__((aligned(16))) MyFirst3DObject[] =
{
  { 0.0f,  0.0f, 0xFFFFFFFF,  0.0f,  0.0f,  0.0f},
  { 0.3f,  0.0f, 0xFFFFFFFF,  0.3f,  0.0f,  0.0f},
  { 0.3f,  1.0f, 0xFFFFFFFF,  0.3f, -1.0f,  0.0f},
  
  { 0.0f,  0.0f, 0xFFFFFFFF,  0.0f,  0.0f,  0.0f},
  { 0.3f,  1.0f, 0xFFFFFFFF,  0.3f, -1.0f,  0.0f},
  { 0.0f,  1.0f, 0xFFFFFFFF,  0.0f, -1.0f,  0.0f},
 
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f,  0.0f},
  { 0.7f,  0.3f, 0xFFFFFFFF,  0.7f, -0.3f,  0.0f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f,  0.0f},
  
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f,  0.0f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f,  0.0f},
  { 0.3f,  0.7f, 0xFFFFFFFF,  0.3f, -0.7f,  0.0f},
  
  { 0.7f,  0.0f, 0xFFFFFFFF,  0.7f,  0.0f,  0.0f},
  { 1.0f,  0.0f, 0xFFFFFFFF,  1.0f,  0.0f,  0.0f},
  { 1.0f,  1.0f, 0xFFFFFFFF,  1.0f, -1.0f,  0.0f},
  
  { 0.7f,  0.0f, 0xFFFFFFFF,  0.7f,  0.0f,  0.0f},
  { 1.0f,  1.0f, 0xFFFFFFFF,  1.0f, -1.0f,  0.0f},
  { 0.7f,  1.0f, 0xFFFFFFFF,  0.7f, -1.0f,  0.0f}
};

struct Vertex __attribute__((aligned(16))) MyFirst3DObject2[] =
{
  { 0.0f,  0.0f, 0xFFFFFFFF,  0.0f,  0.0f, -0.5f},
  { 0.3f,  0.0f, 0xFFFFFFFF,  0.3f,  0.0f, -0.5f},
  { 0.3f,  1.0f, 0xFFFFFFFF,  0.3f, -1.0f, -0.5f},
  
  { 0.0f,  0.0f, 0xFFFFFFFF,  0.0f,  0.0f, -0.5f},
  { 0.3f,  1.0f, 0xFFFFFFFF,  0.3f, -1.0f, -0.5f},
  { 0.0f,  1.0f, 0xFFFFFFFF,  0.0f, -1.0f, -0.5f},
  
  { 0.7f,  0.0f, 0xFFFFFFFF,  0.7f,  0.0f, -0.5f},
  { 1.0f,  0.0f, 0xFFFFFFFF,  1.0f,  0.0f, -0.5f},
  { 1.0f,  1.0f, 0xFFFFFFFF,  1.0f, -1.0f, -0.5f},
  
  { 0.7f,  0.0f, 0xFFFFFFFF,  0.7f,  0.0f, -0.5f},
  { 1.0f,  1.0f, 0xFFFFFFFF,  1.0f, -1.0f, -0.5f},
  { 0.7f,  1.0f, 0xFFFFFFFF,  0.7f, -1.0f, -0.5f},
 
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f, 0.5f},
  { 0.7f,  0.3f, 0xFFFFFFFF,  0.7f, -0.3f, 0.5f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f, 0.5f},
  
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f, 0.5f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f, 0.5f},
  { 0.3f,  0.7f, 0xFFFFFFFF,  0.3f, -0.7f, 0.5f},
 
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f, 0.5f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f, 0.5f},
  { 0.7f,  0.3f, 0xFFFFFFFF,  0.7f, -0.3f, 0.5f},
  
  { 0.3f,  0.3f, 0xFFFFFFFF,  0.3f, -0.3f, 0.5f},
  { 0.7f,  0.7f, 0xFFFFFFFF,  0.3f, -0.7f, 0.5f},
  { 0.3f,  0.7f, 0xFFFFFFFF,  0.7f, -0.7f, 0.5f}
};

//extern unsigned char logo_start[];
unsigned char logo_start[8*1024*4];
unsigned char logo_start2[8*1024*4];


int main( void )
{
  int vsize;
  void* fb1;
  void* fb2;
  void* dbuf;
  void* zbuf;
  ScePspFVector3 vec;
  float f;
  PspGeContext gectx;
  
  memset(logo_start, 0xBB, 8*1024*4);
  memset(logo_start2, 0xFF, 8*1024*4);
  
  sceDisplaySetMode( 0, 480, 272 );
  
  // init VRAM pointers
  vsize = 4 * 512 * 272;
  fb1 = (void*)(0);
  fb2 = (void*)(fb1 + vsize);
  dbuf = (void*)(fb2 + vsize);
  zbuf = (void*)(dbuf + vsize);
  
  // fire up the GU
  sceGuInit();
	sceGuStart(GU_DIRECT, gulist1);
	sceGuDrawBuffer(GU_PSM_8888,dbuf,512);
	sceGuDispBuffer(480,272,fb1,512);
	sceGuDepthBuffer(zbuf,512);
	sceGuOffset(2048 - (480/2),2048 - (272/2));
	sceGuViewport(2048,2048,480,272);
	sceGuDepthRange(0xFFFF,0);
	//sceGuDepthOffset(0);
	sceGuScissor(0,0,480,272);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL); //GU_GEQUAL);
	sceGuDepthMask(GU_FALSE);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuAlphaFunc(GU_LESS, 0x7F, 0xFF);
	//sceGuEnable(GU_ALPHA_TEST);
	//sceGuClearStencil(0);
	//sceGuStencilFunc(GU_GEQUAL);
	//sceGuEnable(GU_STENCIL_TEST);
	sceGuEnable(GU_BLEND);
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuEnable(GU_LIGHT1);
	//sceGuEnable(GU_LIGHT2);
	//sceGuEnable(GU_LIGHT3);
	sceGuFinish();
	sceGuSync(0,0);

  // uhm, erh.... let the GU start showing stuff?
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	
	// now lets get the 3D thing on screen....
	
	for( f = -40.0f; f <= -0.9f; f += 0.05f )
  {
	  // here's some info-feeding so GU knows how to render stuff
	  sceGuStart(GU_DIRECT, gulist1); //GU_DIRECT,gulist1);
	
	  sceGuClearColor(0x00008800);
	  sceGuClearDepth(0);
  	//sceGuDepthOffset(0xFFFF);
	  sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	
	  sceGumMatrixMode(GU_PROJECTION);
	  sceGumLoadIdentity();
	  sceGumPerspective(75.0f,16.0f/9.0f,0.1f,1000.0f);
    sceGumMatrixMode(GU_VIEW);
	  sceGumLoadIdentity();
	  sceGumMatrixMode(GU_MODEL);
	  sceGumLoadIdentity();
	  
	  
	  // setting some advanced stuff: blending (tranparency), lights
	  sceGuBlendFunc( GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0 );
	
	  vec.x = 0.0f;
	  vec.y = 2.0f;
	  vec.z = 2.0f;
	  sceGuLight(0,GU_POINTLIGHT,GU_AMBIENT_AND_DIFFUSE,&vec);
		sceGuLightColor(0,GU_DIFFUSE,0xFF80FFFF);
		//sceGuLightColor(0,GU_SPECULAR,0xFFFF0000);
		sceGuLightAtt(0,0.0f,0.3f,0.0f);
	  sceGuSpecular(12.0f);
		sceGuAmbient(0xFF004400);
		
		vec.x = 0.0f;
	  vec.y = 1.0f;
	  vec.z = -1.0f;
	  sceGuLight(1,GU_POINTLIGHT,GU_AMBIENT_AND_DIFFUSE,&vec);
		sceGuLightColor(1,GU_DIFFUSE,0xFF0000FF);
		//sceGuLightColor(1,GU_SPECULAR,0xFFFFFFFF);
		sceGuLightAtt(1,0.0f,0.1f,0.0f);
		
  
    vec.x = -0.5f;
    vec.y = 0.5f;
    vec.z = f; //-2.5f;
    sceGumTranslate(&vec);
    //ScePspFVector3 rot = { val * 0.79f * (GU_PI/180.0f), val * 0.98f * (GU_PI/180.0f), val * 1.32f * (GU_PI/180.0f) };
    vec.x = 0;
    vec.y = 45;
    vec.z = 0;
    //sceGumRotateXYZ(&vec);
  
    // setting a texture
    sceGuTexMode(GU_PSM_8888,0,0,0);
	  sceGuTexImage(0,64,64,64,logo_start2);
	  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	  sceGuTexEnvColor(0xFF222222);
	  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	  sceGuTexScale(1.0f,1.0f);
	  sceGuTexOffset(0.0f,0.0f);
	  sceGuAmbientColor(0xFF222222);
	  
	  // get the damn thing one screen.... finally....

	  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,6*3,0,MyFirst3DObject);
	

	  sceGumLoadIdentity();
	  vec.x = -0.5f;
    vec.y = 0.5f;
    vec.z = f-0.3f; //-2.5f;
    sceGumTranslate(&vec);
    vec.x = 0;
    vec.y = 45;
    vec.z = 0;
    //sceGumRotateXYZ(&vec);
    sceGuTexMode(GU_PSM_8888,0,0,0);
	  sceGuTexImage(0,64,64,64,logo_start);
	  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	  sceGuTexEnvColor(0x00FFFFFF);
	  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	  sceGuTexScale(1.0f,1.0f);
	  sceGuTexOffset(0.0f,0.0f);
	  //sceGuAmbientColor(0xFFFFFFFF);
	  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,8*3,0,MyFirst3DObject2);
	
	  // finish scene
	  //sceGuSendList( GU_TAIL, gulist1, &gectx );
	  sceGuFinish();
	  sceGuSync(0,0);

	  // swap buffers
	  sceDisplayWaitVblankStart();
	  sceGuSwapBuffers();
	}
	
	
	// second loop
	sceKernelDelayThread( 5000000 );
	
	for( f = 0.0f; f <= 2.0f; f += 0.005f )
  {
	  // here's some info-feeding so GU knows how to render stuff
	  sceGuStart(GU_DIRECT, gulist1); //GU_DIRECT,gulist1);
	
	  sceGuClearColor(0x00008800);
	  sceGuClearDepth(0);
//  	sceGuDepthOffset(0xFFFF);
	  sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	
	  sceGumMatrixMode(GU_PROJECTION);
	  sceGumLoadIdentity();
	  sceGumPerspective(75.0f,16.0f/9.0f,0.1f,1000.0f);
    sceGumMatrixMode(GU_VIEW);
	  sceGumLoadIdentity();
	  sceGumMatrixMode(GU_MODEL);
	  sceGumLoadIdentity();
  
    vec.x = -0.5f;
    vec.y = 0.5f;
    vec.z = -0.9f;
    sceGumTranslate(&vec);
    //ScePspFVector3 rot = { val * 0.79f * (GU_PI/180.0f), val * 0.98f * (GU_PI/180.0f), val * 1.32f * (GU_PI/180.0f) };
    vec.x = 0;
    vec.y = f;
    vec.z = 0;
    sceGumRotateXYZ(&vec);
  
    // setting a texture
    sceGuTexMode(GU_PSM_8888,0,0,0);
	  sceGuTexImage(0,64,64,64,logo_start2);
	  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	  sceGuTexEnvColor(0x00FFFFFF);
	  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	  sceGuTexScale(1.0f,1.0f);
	  sceGuTexOffset(0.0f,0.0f);
	  //sceGuAmbientColor(0xFFFFFFFF);
	
	  // get the damn thing one screen.... finally....

	  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,6*3,0,MyFirst3DObject);
	

	  sceGumLoadIdentity();
	  vec.x = -0.5f;
    vec.y = 0.5f;
    vec.z = -1.2f;
    sceGumTranslate(&vec);
    vec.x = 0;
    vec.y = f;
    vec.z = 0;
    sceGumRotateXYZ(&vec);
    sceGuTexMode(GU_PSM_8888,0,0,0);
	  sceGuTexImage(0,64,64,64,logo_start);
	  sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
	  sceGuTexEnvColor(0x00FFFFFF);
	  sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	  sceGuTexScale(1.0f,1.0f);
	  sceGuTexOffset(0.0f,0.0f);
	  //sceGuAmbientColor(0xFFFFFFFF);
	  sceGumDrawArray(GU_TRIANGLES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_3D,8*3,0,MyFirst3DObject2);
	
	  // finish scene
	  //sceGuSendList( GU_TAIL, gulist1, &gectx );
	  sceGuFinish();
	  sceGuSync(0,0);

	  // swap buffers
	  sceDisplayWaitVblankStart();
	  sceGuSwapBuffers();
	}
	
	// end second loop
	
	// end the show
	//sceKernelDelayThread( 10000000 );
	//memcpy((void*)(((int)fb1) + ((int)sceGeEdramGetAddr())), (void*)(((int)zbuf) + ((int)sceGeEdramGetAddr())), 512*272*4); memcpy((void*)(((int)dbuf) + ((int)sceGeEdramGetAddr())), (void*)(((int)zbuf) + ((int)sceGeEdramGetAddr())), 512*272*4);
	sceKernelDelayThread( 10000000 );
	sceGuTerm();
	sceKernelExitGame();
	return 0;
}
