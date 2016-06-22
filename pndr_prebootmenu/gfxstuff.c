/*
 *  Collection of GFX functions and wrappers to PSP GFX stuff
 *  to make life more easy....
 *
 */

/* ================================================================== *\
    needs zlib, libpng and freetype in the SDK installed to work:
    
    svn checkout svn://svn.pspdev.org/psp/trunk/zlib
    cd zlib; make; make install
    
    svn checkout svn://svn.pspdev.org/psp/trunk/libpng
    cd libpng; make; make install
    
    svn checkout svn://svn.pspdev.org/psp/trunk/freetype
    cd freetype
    sh autogen.sh
    LDFLAGS="-L$(psp-config --pspsdk-path)/lib -lc -lpspuser" \
    ./configure --host psp --prefix=$(psp-config --psp-prefix)
    make
    make install
    Note: The current version of freetype for the PSP seems to install the include files to the wrong directory. Move the \usr\local\pspdev\psp\include\freetype2\freetype folder to \usr\local\pspdev\psp\include and delete \usr\local\pspdev\psp\include\freetype2.
\* =================================================================== */

#include <psptypes.h>
#include <stdlib.h>
#include <malloc.h>
#include <pspdisplay.h>
#include <psputils.h>
#include <png.h>
#include <pspgu.h>
#include <pspdebug.h>
#include <malloc.h>
#include "gfxstuff.h"

#define IS_ALPHA(color) (((color)&0xff000000)==0xff000000?0:1)
#define FRAMEBUFFER_SIZE (PSP_LINE_SIZE*SCREEN_HEIGHT*4)
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define	PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))

typedef u32 Color;

typedef struct
{
	int textureWidth;  // the real width of data, 2^n with n>=0
	int textureHeight;  // the real height of data, 2^n with n>=0
	int imageWidth;  // the image width
	int imageHeight;
	Color* data;
} Image;

// u32* g_vram_base;
Color* g_vram_base = (Color*) (0x40000000 | 0x04000000);
unsigned int __attribute__((aligned(16))) list[262144];
static int dispBufferNumber;
static int initialized = 0;

typedef struct
{
	unsigned short u, v;
	short x, y, z;
} Vertex;


// --- functions ---

//////////////////////////////////////////////////////////////////////
//
//    Common Graphics Handling
//
//////////////////////////////////////////////////////////////////////

static int getNextPower2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

Color* getVramDrawBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 0) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

Color* getVramDisplayBuffer()
{
	Color* vram = (Color*) g_vram_base;
	if (dispBufferNumber == 1) vram += FRAMEBUFFER_SIZE / sizeof(Color);
	return vram;
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
}

Image* gfxLoadPNG(const char* filename)
{
	// taken from the famos graphics.c and modded a bit....
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	u32* line;
	FILE *fp;
	Image* image = (Image*) malloc(sizeof(Image));
	if (!image) return NULL;

	if ((fp = fopen(filename, "rb")) == NULL) return NULL;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		free(image);
		fclose(fp);
		return NULL;;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	if (width > 512 || height > 512) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	image->imageWidth = width;
	image->imageHeight = height;
	image->textureWidth = getNextPower2(width);
	image->textureHeight = getNextPower2(height);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	image->data = (Color*) memalign(16, image->textureWidth * image->textureHeight * sizeof(Color));
	if (!image->data) {
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	line = (u32*) malloc(width * 4);
	if (!line) {
		free(image->data);
		free(image);
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color = line[x];
			image->data[x + y * image->textureWidth] =  color;
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	return image;
}

void guStart()
{
	sceGuStart(GU_DIRECT, list);
}

void gfxBlitImageToScreen(int sx, int sy, int width, int height, Image* source, int dx, int dy)
{
	if (!initialized) return;
	Color* vram = getVramDisplayBuffer(); //getVramDrawBuffer();
	sceKernelDcacheWritebackInvalidateAll();
	guStart();
	sceGuCopyImage(GU_PSM_8888, sx, sy, width, height, source->textureWidth, source->data, dx, dy, PSP_LINE_SIZE, vram);
	sceGuFinish();
	sceGuSync(0,0);
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

void gfxInitGraphics(void)
{
	dispBufferNumber = 0;

	sceGuInit();

	guStart();
	sceGuDrawBuffer(GU_PSM_8888, (void*)FRAMEBUFFER_SIZE, PSP_LINE_SIZE);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
	sceGuDepthBuffer((void*) (FRAMEBUFFER_SIZE*2), PSP_LINE_SIZE);
	sceGuOffset(2048 - (SCREEN_WIDTH / 2), 2048 - (SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuAmbientColor(0xffffffff);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
	initialized = 1;
}

void disableGraphics(void)
{
	initialized = 0;
}

Image* MenuImg;
Font* MenuFnt;
int MenuFntSize = 8;
int MenuFntWidth = 5;
int OffsetX = 0;
int OffsetY = 0;
void gfxClearScreenBgImg(void)
{
	gfxBlitImageToScreen(0, 0, 480, 272, MenuImg, 0, 0);
	gfxSetTextXY(0,0);
}
void gfxInitGfxStuff(void)
{
	gfxInitGraphics();
	
	MenuImg = gfxLoadPNG("ms0:/kd/bootmenu.png");
	MenuFnt = Font_Load("ms0:/kd/bootmenu.ttf");
	Font_SetSize(MenuFnt, MenuFntSize);
	gfxSetTextXY(0,0);
}

int txtX, txtY;
void gfxSetTextXY(int x, int y)
{
	txtX = (x*MenuFntWidth)+OffsetX;
	txtY = ((y+1)*MenuFntSize)+OffsetY;
}

int txtC;
void gfxSetTextColor(u32 c)
{
	txtC = c;
}

void gfxPrintText(char* text)
{
	int i;
	int o, on, l;
	int r;
	char n[1];
	char s[1024];
	
	n[0] = 10;   // newline character - un*x style ;-)
	strcpy(s, text);  // copy requested text to buffer
	
	// look for linebreaks!
	o = 0;
	on = 0;
	l = strlen(s);
	for(i=0; i<l; i++)
	{
		r = strncmp(s+i, n, 1);
		if( r==0 )
		{
			on = i + 1;
			s[i] = 0;
		}
		if( (on==i+1) | (i==l-1) )
		{
	    // draw black outline
	    Font_PrintScreen(MenuFnt, txtX-1, txtY-1, s+o, 0, getVramDisplayBuffer());
	    Font_PrintScreen(MenuFnt, txtX+1, txtY+1, s+o, 0, getVramDisplayBuffer());
	
	    // print text
	    Font_PrintScreen(MenuFnt, txtX, txtY, s+o, txtC, getVramDisplayBuffer());
	  }
	  if( i==l-1 ) { txtX += strlen(s+o)*MenuFntWidth; }
	  if( on==i+1 ) { txtX = 0; txtY += MenuFntSize; o = on; }
	}
}


//////////////////////////////////////////////////////////////////////
//
//    Font Stuff
//
//////////////////////////////////////////////////////////////////////

typedef struct
{
	unsigned short u, v;
	short x, y, z;
} FontVertex;

static int getPow2(int width)
{
	int b = width;
	int n;
	for (n = 0; b != 0; n++) b >>= 1;
	b = 1 << n;
	if (b == 2 * width) b >>= 1;
	return b;
}

Font* Font_Load(const char* filename) {

        FT_Error initError;
        initError = FT_Init_FreeType( &ft_library );
  	if(initError)
          return 0;

	Font* font = (Font*) malloc(sizeof(Font));

	// Cache font
	FILE* fontFile = fopen(filename, "rb");

	if (!fontFile)
        pspDebugScreenPrintf("Can't open font file.\n");

	fseek(fontFile, 0, SEEK_END);
	int filesize = ftell(fontFile);
	u8* fontData = (u8*) malloc(filesize);
	if (!fontData) {
		fclose(fontFile);
		pspDebugScreenPrintf("Not enough memory to cache font file.\n");
	}
	rewind(fontFile);
	fread(fontData, filesize, 1, fontFile);
	fclose(fontFile);
	int error = FT_New_Memory_Face(ft_library, fontData, filesize, 0, &font->face);
	if (error) {
		free(font);
		free(fontData);
		pspDebugScreenPrintf("Error loading font.\n");
	}
	font->data = fontData;
	font->name = strdup(filename);
	return font;
}

int Font_Unload(Font* font) {
	FT_Done_Face(font->face);
	free(font->name);
	if (font->data)	free(font->data);
	free(font);
	return 0;
}

int Font_SetSize(Font* font, int height) {
	FT_Set_Pixel_Sizes(font->face, 0, height);
	return 1;
}


static void fontPrintTextImpl(FT_Bitmap* bitmap, int xofs, int yofs, u32 color, u32* framebuffer, int width, int height, int lineSize)
{
	u8 rf = color & 0xff;
	u8 gf = (color >> 8) & 0xff;
	u8 bf = (color >> 16) & 0xff;
	u8 af = (color >> 24) & 0xff;
	int x, y;
	
	u8* line = bitmap->buffer;
	u32* fbLine = framebuffer + xofs + yofs * lineSize;
	for (y = 0; y < bitmap->rows; y++) {
		u8* column = line;
		u32* fbColumn = fbLine;
		for (x = 0; x < bitmap->width; x++) {
			if (x + xofs < width && x + xofs >= 0 && y + yofs < height && y + yofs >= 0) {
				u8 val = *column;
			  //if( val<128-32 ) { val=0; } else { val=255; }
		    color = *fbColumn;
				u8 r = color & 0xff;
				u8 g = (color >> 8) & 0xff;
				u8 b = (color >> 16) & 0xff;
				u8 a = (color >> 24) & 0xff;
				r = rf * val / 255 + (255 - val) * r / 255;
				g = gf * val / 255 + (255 - val) * g / 255;
				b = bf * val / 255 + (255 - val) * b / 255;
				a = af * val / 255 + (255 - val) * a / 255;
				*fbColumn = r | (g << 8) | (b << 16) | (a << 24);
			}
			column++;
			fbColumn++;
		}
		line += bitmap->pitch;
		fbLine += lineSize;
	}
}

static void fontPrintTextScreen(FT_Bitmap* bitmap, int x, int y, u32 color, u32* framebuffer)
{
	fontPrintTextImpl(bitmap, x, y, color, framebuffer, 480, 272, 512);
}
//GRAPHICS LIB FRAMEBUFFER IS getVramDrawBuffer()

static void fontPrintTextTexture(FT_Bitmap* bitmap, int x, int y, u32 color, FontTexture* fonttexture)
{
	fontPrintTextImpl(bitmap, x, y, color, fonttexture->data, fonttexture->width, fonttexture->height, fonttexture->textureWidth);
}

int Font_PrintScreen(Font* font, int x, int y, const char* text, u32 color, u32* framebuffer) {

	int num_chars = strlen(text);
	FT_GlyphSlot slot = font->face->glyph;
	int n;
	for (n = 0; n < num_chars; n++) {
		FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
		int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
		if (error) continue;
		error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
		if (error) continue;

			fontPrintTextScreen(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, framebuffer);

		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}

	return 0;
}

int Font_RenderTexture(Font* font, int x, int y, const char* text, u32 color, FontTexture* fonttexture) {

	int num_chars = strlen(text);
	FT_GlyphSlot slot = font->face->glyph;
	int n;
	for (n = 0; n < num_chars; n++) {
		FT_UInt glyph_index = FT_Get_Char_Index(font->face, text[n]);
		int error = FT_Load_Glyph(font->face, glyph_index, FT_LOAD_DEFAULT);
		if (error) continue;
		error = FT_Render_Glyph(font->face->glyph, ft_render_mode_normal);
		if (error) continue;

			fontPrintTextTexture(&slot->bitmap, x + slot->bitmap_left, y - slot->bitmap_top, color, fonttexture);

		x += slot->advance.x >> 6;
		y += slot->advance.y >> 6;
	}

	return 0;
}

FontTexture* Font_CreateTexture(int width, int height)
{
	FontTexture* fonttexture = (FontTexture*) malloc(sizeof(FontTexture));
	if (!fonttexture) return NULL;
	fonttexture->width = width;
	fonttexture->height = height;
	fonttexture->textureWidth = getPow2(width);
	fonttexture->textureHeight = getPow2(height);
	fonttexture->data = (u32*) memalign(16, fonttexture->textureWidth * fonttexture->textureHeight * sizeof(u32));
	if (!fonttexture->data) return NULL;
	memset(fonttexture->data, 0, fonttexture->textureWidth * fonttexture->textureHeight * sizeof(u32));
	return fonttexture;
}

void Font_ClearTexture(FontTexture* fonttexture)
{
	int i;
	int size = fonttexture->textureWidth * fonttexture->textureHeight;
	u32* data = fonttexture->data;
	for (i = 0; i < size; i++, data++) *data = 0x00FFFFFF;
}

void Font_DisplayTexture(FontTexture* source, int dx, int dy)
{

	sceKernelDcacheWritebackInvalidateAll();

	sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, (void*) source->data);
	float u = 1.0f / ((float)source->textureWidth);
	float v = 1.0f / ((float)source->textureHeight);
	sceGuTexScale(u, v);

	int sx = 0;
	int sy = 0;
	int width = source->width;
	int height = source->height;

	int j = 0;
	while (j < width) {
		FontVertex* vertices = (FontVertex*) sceGuGetMemory(2 * sizeof(FontVertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 0;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 0;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
	
	sceGuFinish();
	sceGuSync(0, 0);
}

void Font_UnloadTexture(FontTexture* fonttexture)
{
     free(fonttexture->data);
     free(fonttexture);
}
