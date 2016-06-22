/*                                         *\

   ***************************************
   **                                   **
   **    HELLCAT's Demo #1              **
   **    The crappy side of life ;-)    **
   **                                   **
   ***************************************

               == preflight ==

            |\      _,,,---,,_
     ZZZzz /,`.-'`'    -.  ;-;;,_
          |,4-  ) )-,_. ,\ (  `'-'
         '---''(_/--'  `-'\_)

\*                                         */

#include <pspkernel.h>
#include <pspdebug.h>
#include <string.h>
#include <stdlib.h>
#include "part0.h"
#include "common.h"



MatrixStream MatrixStreams[129];



void hcd1InitMatrixStreams(void)
{
	int i;

	for(i=1;i<=96;i++)
	{
		MatrixStreams[i].text = " ";
		MatrixStreams[i].color = 0x00CC8800;
		MatrixStreams[i].fade = 0x00080400;
		MatrixStreams[i].x = 0;
		MatrixStreams[i].y = 1;
		MatrixStreams[i].speed = 1;
		MatrixStreams[i].visible = false;
	}
}

void hcd1UpdateMatrixStreams(void)
{
	int i;

	for(i=1;i<=96;i++)
	{
		if (MatrixStreams[i].visible == true) { MatrixStreams[i].y = MatrixStreams[i].y + MatrixStreams[i].speed; }
	}
}


void hcd1DrawMatrixStreams(void)
{
	unsigned int i;
	unsigned int j;
	unsigned int c;
	unsigned int v1;
	unsigned int v2;
	unsigned int o;
	unsigned int l;
	char *s;

	for (i=1;i<=96;i++)
	{
		if (MatrixStreams[i].visible == true)
		{
			l = strlen(MatrixStreams[i].text);
			c = MatrixStreams[i].color;
			o = 0;
			if (MatrixStreams[i].y > 270)
			{
				o = ((MatrixStreams[i].y - 271) / 8) + 1;
			}
			for (j=o;j<=l;j++)
			{
				v1 = MatrixStreams[i].y - (o * 8);
				v2 = (j - o) * 8;
				if (v1<271 && v2<MatrixStreams[i].y)
				{
					s = MatrixStreams[i].text;
					pspDebugScreenPutChar(MatrixStreams[i].x*7, MatrixStreams[i].y-(j*8), c, s[j]);
				}
				c = c - MatrixStreams[i].fade;
			}
		}
		if (o == l) { MatrixStreams[i].y = 1; }
	}
}

void hcd1FancyWrite1(int x, int y, char text[])
{
	// pspDebugScreenPutChar(int x, int y, u32 color, u8 ch);
	char curChar = 127;
	unsigned int i;

	if (x == -1) { x = (480/14) - (strlen(text)/2); }

	while (curChar > 31)
	{
		for (i=0;i<=strlen(text);i++)
		{
			if (curChar > text[i])
			{
				pspDebugScreenPutChar((x+i-1)*7, y*8, 0x00ffffff, curChar);
			} else {
				pspDebugScreenPutChar((x+i-1)*7, y*8, 0x00ffffff, text[i]);
			}
		}
		curChar--;
		hcWait(0.05);
	}
}

void FillStringRandom(char *s)
{
	unsigned int i;

	for (i=0; i<=strlen(s)-1; i++)
	{
		s[i] = (char)GetRandomNum(32, 127);
		//pspDebugScreenPrintf("-");
		//pspDebugScreenPrintf((char*)s[i]);
	}
}

void hcd1RunPart0(void)
{
	int i,j;
	char *s;

	pspDebugScreenInit();
	//hcWait(2);
	hcd1FancyWrite1(-1, 10, "---==> HELLCAT <==---");
	hcWait(1);
	hcd1FancyWrite1(-1, 13, "THE MATRIX HAS YOU!!!");
	hcd1FancyWrite1(-1, 19, "here comes the code.... ;-)");
	/*hcd1FancyWrite1(10, 12, "test of hcd1FancyWrite1()....");
	hcd1FancyWrite1(10, 16, "PSP ** PSP ** PSP ** PSP");
	hcd1FancyWrite1(10, 22, "looks OK, doesn't it? :-)");
*/
	hcWait(4); 

	pspDebugScreenSetBackColor(0x00221100);
	pspDebugScreenClear();
	hcd1InitMatrixStreams();
	for (i=1;i<=96;i++)
	{
		MatrixStreams[i].text = (char*)malloc(22);
		strcpy(MatrixStreams[i].text, "*********************");
		FillStringRandom(MatrixStreams[i].text);
		//MatrixStreams[i].text = s;
		MatrixStreams[i].x = GetRandomNum(1, 67);
		MatrixStreams[i].speed = GetRandomNum(2, 4);
	}

	for (i=1; i<=96; i++)
	{
		MatrixStreams[i].visible = true;
		for (j=1; j<=16; j++)
		{
			hcd1UpdateMatrixStreams();
			hcWaitVBlank();
			//hcWait(0.1);
			hcd1DrawMatrixStreams();
			pspDebugScreenSetXY(25,10);
			pspDebugScreenPrintf("---==> HELLCAT <==---");
		}
	}

	while (1)
	{
		hcd1UpdateMatrixStreams();
		hcWaitVBlank();
		//hcWait(0.1);
		hcd1DrawMatrixStreams();
		pspDebugScreenSetXY(25,10);
		pspDebugScreenPrintf("---==> HELLCAT <==---");
	}
}
