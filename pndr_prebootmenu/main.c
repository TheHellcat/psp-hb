/* ===================================================================== *\

 *  ##############
 *  ##############  E x t e n d e d   P a n d o r a   B o o t m e n u
 *        ##        ---------------------------------------------------
 *       ####
 *    ####  ####             ---------
 *    ####  ####    by ---==> HELLCAT <==---
 *       ####                ---------
 *        ##
 *  ##############
 *  ##############
 *  ####  ##  ####
 *    ##  ##  ##
 *      ######
 *       ####
 *        ##

\* ===================================================================== */

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspsyscon.h>
#include <psppower.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <string.h>
#include <stdlib.h>
#include "sysfunc.h"
#include "patchipl.h"
#include "usb.h"
#include "gfxstuff.h"

#include "builddate.h"  // automatically generated on full build

PSP_MODULE_INFO("pndrbtmnu", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

//#define printf pspDebugScreenPrintText
//#define setxy pspDebugScreenSetTextXY

int selection;
char BLOB[1] __attribute__((aligned(64)));
struct SceKernelLoadExecParam param;
char *config;
char launchpath[64];
int defaultoverride = 0;
char gfxMode;
char s[255];



//////////////////////////////////////////////////
//  some wrappers to use the correct functions  //
//  depending on gfx mode                       //
//////////////////////////////////////////////////

void PrintText(char* text)
{
  if(gfxMode==1)
  {
  	gfxPrintText(text);
  } else {
    pspDebugScreenPrintf(text);
  }
}

void SetTextColor(u32 c)
{
  if(gfxMode==1)
  {
  	gfxSetTextColor(c);
  } else {
    pspDebugScreenSetTextColor(c);
  }
}

void SetTextXY(int x, int y)
{
	if(gfxMode==1)
  {
  	gfxSetTextXY(x, y);
  } else {
    pspDebugScreenSetXY(x, y);
  }
}

void ClearScreen(void)
{
	if(gfxMode==1)
  {
  	gfxClearScreenBgImg();
  } else {
    pspDebugScreenClear();
  }
}

void ToggleGfxMode(void)
{
	if(gfxMode==1)
	{
		gfxMode=0;
	} else {
		gfxMode=1;
		gfxInitGfxStuff();
	}
	
	config[4] = gfxMode;
}

//////////////////////
//  misc functions  //
//////////////////////

void ReadConfig(void)
{
	int r;
	r = hcReadFile("ms0:/kd/bootmenu.cfg", config, 1024);
	
	gfxMode = config[4];
}

void WriteConfig(void)
{
	int r = hcWriteFile("ms0:/kd/bootmenu.cfg", config, 1024);
	
	if( r<0 )
	{
		// could not write config
	}
}

void InitConfig(void)
{
	int i;
	
	config=(char*)malloc(1024*sizeof(char));
	
	// init config buffer
	for(i=0; i<(1024*sizeof(char)); i++)
	{
		config[i]=0;
	}
	
	// now put some defaults....
	config[0] = 0;
	config[1023] = 0;
	//WLAN UP action:
	config[1] = 0;  // nothing, disabled
	// show flashboot notice?
	config[2] = 1;
	// restore original name of nandimage.flash?
	config[3] = 0;
	// gfx mode
	config[4] = 1;
	// path of CFW installer
	strcpy(config+128, "ms0:/PSP/GAME/CFWUPDATE/EBOOT.PBP");
	// path of app for flash0 access
	strcpy(config+256, "ms0:/PSP/GAME/MAINT/EBOOT.PBP");
	//original filename of nandimage.flash
	strcpy(config+512, "0");
	
	// try to load config from file
	ReadConfig();
}

void UpdateCursor(void)
{
  int i;
  int o = 0;
  
  if( gfxMode == 1 ) { o = 2; }

  for(i=6;i<=15;i++)
  {
    SetTextXY(3,i+o);
    PrintText("  ");
  }
  SetTextXY(3,5+selection+o);
  SetTextColor(0x00FFFFFF);
  PrintText("->");
  SetTextColor(0x0080FFFF);
}

char hcMemPeek(void *addr)
{
	// adapted from the kernel sample from the PSPSDK :-)
	char memdata[1] __attribute__((aligned(64)));
	
	memcpy(memdata, addr, 1);  // 1 = len
	return (char)memdata[0];
}

void hcMemPoke(void *addr, void *value)
{
	memcpy(addr, value, 1);
}

void CopyFile(char* src, char* dst)
{
  char* buffer[1024];
  int r = 1024;
  
  int f1 = sceIoOpen( src, PSP_O_RDONLY, 0777 );
  int f2 = sceIoOpen( dst, PSP_O_RDONLY | PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777 );
  
  while (r == 1024)
  {
  	r = sceIoRead(f1, buffer, 1024);
  	sceIoWrite(f2, buffer, r);
  }
  
  sceIoClose(f1);
  sceIoClose(f2);
}

void RebootVSH(void)
{
	pspDebugScreenSetBackColor(0x00402000);
  SetTextColor(0x0080FFFF);
  ClearScreen();
  PrintText("\n\n  Rebooting for full firmware (from memorystick)....");
  
	// set a little marker that on the next start we know
	// to just quit out, exit and let the VSH boot up normally
  BLOB[0] = 0xCA;
  hcMemPoke( (void*)0x883FFF00, BLOB );
  
  CopyFile("ms0:/registry/system.dpv1", "ms0:/registry/system.dreg");
  CopyFile("ms0:/registry/system.ipv1", "ms0:/registry/system.ireg");
  
  // launch VSH

  memset(&param, 0, sizeof(param));

  param.size = sizeof(param);
  param.argp = "flash0:/vsh/module/vshmain.prx";
  param.args = 31;
  param.key = "vsh";

  sceKernelDelayThread(2500000);
  sceKernelLoadExec("flash0:/vsh/module/vshmain.prx", &param);
}

void RebootMyself(void)
{
	// re-start ourselfs, but switched to game mode
	// to prevent the VSH/XMB to continue booting up
	
	//int i;

  pspDebugScreenSetBackColor(0x00008000);
  ClearScreen();

  memset(&param, 0, sizeof(param));

  param.size = sizeof(param);
  param.argp = "flash0:/kd/bootmenu.elf stage2";
  param.args = 31;
  param.key = "game";

  sceKernelLoadExec("flash0:/kd/bootmenu.elf", &param);
}

void RebootForMkDirF1(void)
{
	// re-start ourselfs in updater mode
	// for flash1:/ access
	
	BLOB[0] = 0xFA;
  hcMemPoke( (void*)0x883FFF00, BLOB );
	
	memset(&param, 0, sizeof(param));

  param.size = sizeof(param);
  param.argp = "ms0:/kd/bootmenu.elf sta e3", 0;
  param.args = 29;
  param.key = "updater";

  sceKernelLoadExec("ms0:/kd/bootmenu.elf", &param);
}

void RebootForUSB(void)
{
	BLOB[0] = 0xFA;
  hcMemPoke( (void*)0x883FFF00, BLOB );
	
	memset(&param, 0, sizeof(param));

  param.size = sizeof(param);
  param.argp = "ms0:/kd/bootmenu.elf sta e4";
  param.args = 28;
  param.key = "updater";

  sceKernelLoadExec("ms0:/kd/bootmenu.elf", &param);
}

void LaunchPandora(int ForceV1)
{
	memset(&param, 0, sizeof(param));
	
	pspDebugScreenSetBackColor(0x00402000);
  SetTextColor(0x0080FFFF);
  ClearScreen();
  PrintText("\n\n  Launching original Pandora menu....");
	
	int f = sceIoOpen( "ms0:/kd/resurrection.elf", PSP_O_RDONLY, 0 );
	if( (f<0) | (ForceV1==1) )
	{
		sceIoClose(f);
		CopyFile("ms0:/kd/libpsardumper.pv1", "ms0:/kd/libpsardumper.prx");
		CopyFile("ms0:/kd/pspbtcnf_updater.pv1", "ms0:/kd/pspbtcnf_updater.txt");
  	CopyFile("ms0:/registry/system.dpv1", "ms0:/registry/system.dreg");
  	CopyFile("ms0:/registry/system.ipv1", "ms0:/registry/system.ireg");
    param.size = sizeof(param);
    param.argp = "flash0:/kd/extprxs.elf";
    param.args = 23;
    param.key = "game";
    sceKernelDelayThread(2500000);
    sceKernelLoadExec("flash0:/kd/extprxs.elf", &param);
  } else {
  	sceIoClose(f);
  	CopyFile("ms0:/kd/libpsardumper.pv2", "ms0:/kd/libpsardumper.prx");
  	CopyFile("ms0:/kd/pspbtcnf_updater.pv2", "ms0:/kd/pspbtcnf_updater.txt");
  	CopyFile("ms0:/registry/system.dpv2", "ms0:/registry/system.dreg");
  	CopyFile("ms0:/registry/system.ipv2", "ms0:/registry/system.ireg");
    param.size = sizeof(param);
    param.argp = "flash0:/kd/resurrection.elf";
    param.args = 28;
    param.key = "updater";
    sceKernelDelayThread(2500000);
    sceKernelLoadExec("flash0:/kd/resurrection.elf", &param);
  }
}

void LaunchCFWInstaller(void)
{
	int btn = 0;
	SceCtrlData pad;
	
	btn = sceIoOpen(config+128, PSP_O_RDONLY, 0777);
	if( btn<0 )
	{
		sceIoClose(btn);
		SetTextColor(0x000000FF);
		SetTextXY(3,5+selection);
		PrintText("   PATH DOES NOT EXIST - CHOOSE NEW WITH \"O\" or \"[]\"!        ");
		SetTextColor(0x0080FFFF);
		sceKernelDelayThread(4000000);
		return;
	}
	sceIoClose(btn);
	btn = 0;
	
	pspDebugScreenSetBackColor(0x00002040);
  SetTextColor(0x00FFFFFF);
  ClearScreen();
	PrintText("\n\n               ***   CFW Installation Prerequisits   ***\n\n\n   Before we can successfully launch the CFW installer we need to\n   perform a few preparations:\n\n     - format flash\n     - install 1.50/CFW compatible IPL\n\n\n If you do not run the CFW installer after those have been done, or\n the installer fails for other reasons, you will get a bricky brick.\n\n\n   But since we're on Pandora we should be safe though :-)\n\n   However, neither this program, nor it's creator take any\n   responsibility for whatever happens.\n\n\n   TA-082/086 Note: due to the IPL installed now, you DO NOT have\n   to patch your \"idstorage\" for installing this CFW! :-)\n\n\n\n\n                 X = Let's go        O = Draw back");
	
	sceCtrlReadBufferPositive(&pad, 1);
	while((pad.Buttons & 0xFFFF) != 0) { sceCtrlReadBufferPositive(&pad, 1); };
  
  while(btn==0)
  {
  	sceCtrlReadBufferPositive(&pad, 1);
    if((pad.Buttons & 0xFFFF) == PSP_CTRL_CIRCLE) { btn=1; }
    if((pad.Buttons & 0xFFFF) == PSP_CTRL_CROSS)
    {
    	ClearScreen();
    	PrintText("\n\n   Performing preparations....");
    	SetTextXY(0,6);
    	hcSysfuncFormatF0();
    	PrintText("\n");
      hcSysfuncFormatF1();
      PrintText("\n");
      hcSysfuncFormatF2();
      PrintText("\n\n");
      patchipl_main();
      PrintText("\n\n\nPreparations done, launching installer....");
      sceKernelDelayThread(4000000);
      BLOB[0] = 0xFA;
      hcMemPoke( (void*)0x883FFF00, BLOB );
      memset(&param, 0, sizeof(param));
      param.size = sizeof(param);
      param.argp = config+128;
      param.args = strlen(param.argp)+1;
      param.key = "updater";
      sceKernelLoadExec(param.argp, &param);
    }
  }
}

void hcShowF0BootNote(void)
{
	if(config[2]==1)
	{
	  SceCtrlData pad;
	
	  pspDebugScreenSetBackColor(0x00002040);
    SetTextColor(0x00FFFFFF);
    ClearScreen();
  
    PrintText("\n\n    PLEASE NOTE:\n\n    Rebooting from the internal flash0 currently has\n    a few flaws:\n\n   * UMD drive does not propperly work after reboot!\n     Classic behaviour.... need a better way of rebooting.\n\n   * Only 1.50 and some 2.xx FWs can be booted by this!\n     Due to the IPL on the MS, the system was initialised\n     from, higher FWs will fail to boot.\n\n   * CFWs may cause lockups!\n     If you keep getting mailfunctions when a CFW is starting\n     up (BSODs and/or lockups) set \"Skip Sony Logo\" to ENABLED\n     to be bootable from here.\n\n  See README for more details on the issues.\n");
    PrintText("\n\n\n  Press X to continue....");
  
    sceCtrlReadBufferPositive(&pad, 1);
    while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
    while((pad.Buttons & 0xFFFF) != PSP_CTRL_CROSS){ sceCtrlReadBufferPositive(&pad, 1); }
    
    config[2] = 0;
    WriteConfig();
    ClearScreen();
    sceKernelDelayThread(2500000);
  }
}

void FadeIn(void)
{
	int i;
	int c;
	
	pspDebugScreenSetBackColor(0);
  ClearScreen();
	
	for(i=0;i<=78;i=i+2)
	{
		c=(i*0x20000)+(i*0x100);
		pspDebugScreenSetBackColor(c);
		//sceKernelDelayThread(50000);
		sceDisplayWaitVblankStart();
    ClearScreen();
	}
	for(i=78;i>=20;i=i-2)
	{
		c=(i*0x20000)+(i*0x100);
		pspDebugScreenSetBackColor(c);
		//sceKernelDelayThread(50000);
		sceDisplayWaitVblankStart();
    ClearScreen();
	}
}

void DrawMainScreen(void)
{
	pspDebugScreenSetBackColor(0x00402000);
  SetTextColor(0x0080FFFF);
  ClearScreen();

  // fill the screen a bit :-D
  if( gfxMode==0 )
  {
    PrintText("Extended PANDORA (pre)boot menu\n----------------------------------\n\n");
    PrintText("Please choose action:\n\n\n");
  } else {
  	SetTextXY(0,8);
  }
  PrintText("      Boot to full firmware (from memorystick)\n      Go to Pandora mainmenu\n      Boot PSP from internal flash\n      Fix flash1:/ (format and create directories)\n      Launch app with flash0:/ access  (O or [] to choose path)\n      Launch CFW installer  (O or [] to choose path)\n      USB options\n      Restore NAND dump");
  SetTextColor(0x0080FFFF);
    
  selection = 1;
  UpdateCursor();

  SetTextColor(0x00808080);
  if( gfxMode==0 )
  {
  	SetTextXY(40,2);
  } else {
  	SetTextXY(60,3);
  }
  sprintf(s, "      Build: %s", builddate);
  PrintText(s);

  SetTextColor(0x00FFFFFF);
  if( gfxMode==0 )
  {
  	SetTextXY(40,1);
  } else {
  	SetTextXY(60,2);
  }
  PrintText("                      v0.86");
  if( gfxMode==0 )
  {
  	SetTextXY(40,27);
  } else {
  	SetTextXY(72,21);
  }
  sprintf(s, "              Battery: %3i%%", scePowerGetBatteryLifePercent());
  PrintText(s);
  if( scePowerIsPowerOnline() != 0)
  {
  	if( gfxMode==0 )
    {
  	  SetTextXY(40,26);
    } else {
  	  SetTextXY(72,20);
    }
    PrintText("               AC connected");
  }
  SetTextColor(0x0080FF00);
  SetTextXY(0,23);
  if( gfxMode==1 )
  {
    PrintText(" \n\n ");
  }
  PrintText("   Set WLAN switch to ON to directly boot to default action.\n   /\\ to set currently selected as default.");
  SetTextColor(0x00FF8000);
  SetTextXY(0,29);
  if( gfxMode==1 )
  {
    PrintText("   ");
  }
  PrintText("HUGE thanks and respect go to: The makers of the PANDORA\n");
  if( gfxMode==1 )
  {
    PrintText("   ");
  }
  PrintText("The PSPSDK samples, helpful ppl on PS2DEV and QJ forums");
  if( gfxMode==0 )
  {
    PrintText("\n\n(pre)bootmenu done by ---==> HELLCAT <==---");
  }
  SetTextColor(0x0080FFFF);
}

void DrawUsbStateBase(void)
{
	int x = 38;
	int y = 12;
	
	SetTextColor(0x00A0A0A0);
	
	SetTextXY(x,y);
	PrintText("                 __");
	SetTextXY(x,y+1);
  PrintText("                |  | _____");
  SetTextXY(x,y+2);
  PrintText("                |--|| P C |");
  SetTextXY(x,y+3);
  PrintText("(+[__]+)        |  ||_____|");
  SetTextXY(x,y+4);
  PrintText("                |__| _|_|_");
  SetTextXY(x,y+6);
  PrintText("       kBytes read:");
  SetTextXY(x,y+7);
  PrintText("    kBytes written:");
  
  SetTextColor(0x0080FFFF);
}
void DrawUsbState(int state)
{
	int x = 38;
	int y = 12;
	
	SetTextColor(0x00FFFFFF);
	
	SetTextXY(x+8,y+3);
	if(state & 0x222)
	{
		PrintText("========");
	} else {
		PrintText("        ");
	}
	
	SetTextXY(x+10,y+2);
	if(state & 0x00010000)
	{
		PrintText("<<<<");
	} else {
		PrintText("    ");
	}
	SetTextXY(x+10,y+4);
	if(state & 0x00020000)
	{
		PrintText(">>>>");
	} else {
		PrintText("    ");
	}
	
	SetTextXY(x+18,y+6);
	sprintf(s, "%9i", usbGetBytesRead()/1024);
	PrintText(s);
	SetTextXY(x+18,y+7);
	sprintf(s, "%9i", usbGetBytesWritten()/1024);
	PrintText(s);
	
	SetTextColor(0x0080FFFF);
	if( gfxMode == 1) { sceKernelDelayThread(250000); }
}

void USBMode(void)
{
	SceCtrlData pad;
  int btn;
  int i;
	
	    pspDebugScreenSetBackColor(0x00402000);
	    SetTextColor(0x0080FFFF);
      ClearScreen();
      PrintText("Extended PANDORA (pre)boot menu\n----------------------------------\n\n");
      SetTextColor(0x00FFFFFF);
      PrintText("\n                        ---------------------\n                     ***   USB access mode   ***\n                        ---------------------\n\n");
      SetTextColor(0x0080FF00);
      PrintText("                 with special thanks (and credits) to\nTeam WildCard (for the public released sources) and DAX (of course)");
      SetTextColor(0x00FFFFFF);
      PrintText("\n\n\n\n       Choose device to access:\n\n\n");
      SetTextColor(0x0080FFFF);
      PrintText("          Access Memory Stick\n          Access Flash 0\n          Access Flash 1\n          Disable USB Access\n          EXIT (Back to menu)");
      SetTextXY(0,26);
      SetTextColor(0x0080FF00);
      PrintText("   When switching access to a different device, your PC might\n   take up to a full minute, or two, to regognize the re-activated\n   USB. So be patient and give it some time ;-)");
      SetTextColor(0x0080FFFF);
      
      DrawUsbStateBase();
      
      selection=16;
      while(1)
      {
      	for(i=16;i<=20;i++)
        {
          SetTextXY(6,i);
          PrintText("  ");
        }
        SetTextXY(6,selection);
        SetTextColor(0x00FFFFFF);
        PrintText("->");
        SetTextColor(0x0080FFFF);
      
        sceCtrlReadBufferPositive(&pad, 1);
        while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
      
        btn=0;
        while(btn==0)
        {
        	DrawUsbState(usbGetUsbState());
          sceCtrlReadBufferPositive(&pad, 1);
          btn = pad.Buttons & 0xFFFF;
        }
        if(btn != 0)
        {
          if(btn & PSP_CTRL_DOWN) { selection++; }
          if(btn & PSP_CTRL_UP) { selection--; }
          if(btn & PSP_CTRL_CROSS)
          {
        	  if(selection==16) { usbEnableUsbMS0();
        		                  SetTextXY(4,23);
        		                  SetTextColor(0x00FFFFFF);
                              PrintText("USB access to memory stick enabled");
                              SetTextColor(0x0080FFFF); }
            if(selection==17) { usbEnableUsbF0();
        		                  SetTextXY(4,23);
        		                  SetTextColor(0x00FFFFFF);
                              PrintText("USB access to flash0:/ enabled    ");
                              SetTextColor(0x0080FFFF); }
            if(selection==18) { usbEnableUsbF1();
        		                  SetTextXY(4,23);
        		                  SetTextColor(0x00FFFFFF);
                              PrintText("USB access to flash1:/ enabled    ");
                              SetTextColor(0x0080FFFF); }
            if(selection==19) { usbDisableUsb();
        		                  SetTextXY(4,23);
        		                  SetTextColor(0x00FFFFFF);
                              PrintText("USB access disabled               ");
                              SetTextColor(0x0080FFFF); }
            if(selection==20) { usbDisableUsb();
          	                  SetTextXY(4,23);
        		                  SetTextColor(0x00FFFFFF);
                              PrintText("going back to the menu....        ");
                              SetTextColor(0x0080FFFF);
                              sceKernelDelayThread(4000000);
                              RebootMyself(); }
          }
          if( selection < 16 ) { selection = 20; }
          if( selection > 20 ) { selection = 16; }
        }
      }
}

//int (* rebootbin)(void *a0, void *a1, void *a2, void *a3) = (void *)0x88C00000;
char rebootfunc[100000] __attribute__((aligned(64)));;
void (* rebootbin)(void) = (void *)&rebootfunc;
int scePowerRebootStart(int uk1, int uk2);
void reboottest(void)
{
	//int r;
	//SceUID fd = sceIoOpen("ms0:/reboot.bin", PSP_O_RDONLY, 0777);
	//r = sceIoRead(fd, (void*)0x88c00000, 100000); // 93504
	//r = sceIoRead(fd, rebootfunc+0x100, 100000); // 93504
	//sprintf("     %i, %i  ", fd, r);
	//PrintText(s);
	//rebootbin();
	//PrintText("     %i, %i  ", fd, r);
	sceSysconResetDevice(1, 1);
	//r = rebootbin(NULL, NULL, NULL, NULL);
	//scePowerRebootStart(1, 1);
	//sceSysconResetDevice(1, 1);
}

// =====================================================================|==
// ===== MAIN ==========================================================|==
// =====================================================================|==

int main(int argc, char *argv[])
{	
	SceCtrlData pad;
  int quit;
  int btn;
  //int skipme = 0;
  //int i;
  char* p;
  char s[256];
  int r;
  
  pspDebugScreenInit();
  pspDebugScreenSetBackColor(0x00000000);
  pspDebugScreenSetTextColor(0x00A0A0A0);
  pspDebugScreenClear();
  
  r=0;
  if(argc != 0)
  {
    if(argv[0][strlen(argv[0])-1]==0x32)
    {
    	r=1;
    }
  }
  if( argc == 0) { r=1; }
  // all this printf'ing is not really important, or even necessary.... but I'm a fan of verbose startups :-p
  if( r==1 ) { pspDebugScreenPrintf("initializing configuration....\n"); }
  InitConfig();
  if( config[3] == 1 )
  {
  	sceIoRename( "ms0:/nandimage.flash", config+512 );
  	config[3] = 0;
  	strcpy(config+512, "0");
  	WriteConfig();
  }
  CopyFile("ms0:/kd/pspbtcnf_updater.pv1", "ms0:/kd/pspbtcnf_updater.txt");

  if(gfxMode==1)
  {
  	if( r==1 ) { pspDebugScreenPrintf("initializing gfx core....\n"); }
  	gfxInitGfxStuff();
  }
  sceKernelDelayThread(500000);
  pspDebugScreenSetBackColor(0x00333333);
  ClearScreen();
  
  sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
	sceCtrlReadBufferPositive(&pad, 1);

  quit=0;
	/*if(argc == 0)
	{
		skipme = hcMemPeek( (void*)0x883FFF00 ) & 0xFF;
		if( ( skipme == 0xCA ) || ( pad.Buttons & PSP_CTRL_WLAN_UP ) )
	  {
	  	// if we are supposed to keep quite and just run the default action....
	  	defaultoverride = config[1];
	  	if((skipme==0xCA) || (defaultoverride==1)) { quit=1; }
	  } else {
	  	// ....else restart ourselfs in gamemode to prevent the VSH to boot
	  	//sceDisplayWaitVblankStart();
		  RebootMyself();
		}
	}*/
	if( pad.Buttons & PSP_CTRL_WLAN_UP )
	{
		defaultoverride = config[1];
  }
  
  if( quit==0 )   // <== relict of early versions (0.80 and below), I'm too lazy to put it out ;-)
  {
  	if(argc != 0)
  	{
  		if(argv[0][26]==0x33)
	    {
	      pspDebugScreenSetBackColor(0x00402000);
	      SetTextColor(0x0080FFFF);
        ClearScreen();
        PrintText("Extended PANDORA (pre)boot menu\n----------------------------------\n\n");
        SetTextColor(0x00FFFFFF);
        SetTextXY(2,19);
        PrintText("About to create flash1:/ directories....");
        SetTextXY(2,20);
        sceKernelDelayThread(4000000);
        hcSysfuncMkF1Dirs();
        SetTextXY(2,22);
        SetTextColor(0x0080FFFF);
        PrintText("Going back to the menu....");
        sceKernelDelayThread(3000000);
        RebootMyself();
      }
    
  	  if(argv[0][26]==0x34)
	    {
	  	  USBMode();
      }
    }
    
    if(defaultoverride==0)
  	{
  		FadeIn();
      DrawMainScreen();
    } else {
    	selection = defaultoverride;
    }
    
    while (quit==0)
    {
      sceCtrlReadBufferPositive(&pad, 1);
      btn = pad.Buttons & 0xFFFF;
      if((btn != 0) || (defaultoverride>0))
      {
        if(btn & PSP_CTRL_DOWN) { selection++; }
        if(btn & PSP_CTRL_UP) { selection--; }
        if(btn & PSP_CTRL_SELECT) { ToggleGfxMode(); DrawMainScreen(); WriteConfig(); }
        if(btn & PSP_CTRL_TRIANGLE) { config[1] = selection;
        	                            WriteConfig();
        	                            SetTextColor(0x0080FF00);
                                      SetTextXY(0,23);
                                      PrintText("   Default saved....                                         \n                                             ");
                                      SetTextColor(0x0080FFFF); }
        if(btn & PSP_CTRL_CIRCLE) { if((selection==5) || (selection==6))
        	                          { p = hcFileSelector( "ms0:/PSP/GAME/", NULL );
        	                            strcpy(s, "ms0:/PSP/GAME/");
        	                            strcpy(s+14, p);
        	                            strcpy(s+14+strlen(p), "/EBOOT.PBP");
        	                            if(selection==5)
        	                            {
        	                            	strcpy(config+256, s);
        	                            }
        	                            if(selection==6)
        	                            {
        	                            	strcpy(config+128, s);
        	                            }
        	                            DrawMainScreen();
        	                            WriteConfig();
        	                            SetTextColor(0x0080FF00);
                                      SetTextXY(0,23);
                                      PrintText("   New path saved....                                        \n                                             ");
                                      SetTextColor(0x0080FFFF);
                                    }
                                    if(selection==2)
                                    {
                                    	LaunchPandora(1);
                                    } }
        if(btn & PSP_CTRL_SQUARE) { if((selection==5) || (selection==6))
        	                          { p = hcFileSelector( "ms0:/PSP/GAME150/", NULL );
        	                            strcpy(s, "ms0:/PSP/GAME150/");
        	                            strcpy(s+17, p);
        	                            strcpy(s+17+strlen(p), "/EBOOT.PBP");
        	                            if(selection==5)
        	                            {
        	                            	strcpy(config+256, s);
        	                            }
        	                            if(selection==6)
        	                            {
        	                            	strcpy(config+128, s);
        	                            }
        	                            DrawMainScreen();
        	                            WriteConfig();
        	                            SetTextColor(0x0080FF00);
                                      SetTextXY(0,23);
                                      PrintText("   New path saved....                                        \n                                             ");
                                      SetTextColor(0x0080FFFF);
                                    } }
        if((btn & PSP_CTRL_CROSS) || (defaultoverride>0))
        {
          if( selection == 1 ) { RebootVSH(); }
          if( selection == 2 ) { LaunchPandora(0); }
          if( selection == 3 ) { hcShowF0BootNote(); //reboottest();
          	                     //scePower_0442D852(50000);   // not available on 1.50 kernel :-(
          	                     //sceSysconResetDevice(2, 1); // supposingly "Reset UMD", but PSP hangs for 3-5 minutes, still UMD erroring after reboot
          	                     //sceSysconResetDevice(8, 1);sceSysconResetDevice(4, 1);sceSysconResetDevice(2, 1);
          	                     sceSysconResetDevice(1, 1); } // well, well.... a reboot at least.... but gives an UMD error after the reboot
          if( selection == 4 ) { SetTextXY(2,20);
          	                     SetTextColor(0x00FFFFFF);
        		                     hcSysfuncFormatF1();
        		                     SetTextXY(2,21);
        		                     //hcSysfuncMkF1Dirs();
        		                     PrintText("Please wait, preparing to create flash1:/ directories....");
        		                     sceKernelDelayThread(3000000);
        		                     RebootForMkDirF1(); }
          if( selection == 5 ) { r = sceIoOpen(config+256, PSP_O_RDONLY, 0777);
	                               if( r<0 )
                                 {
                                   sceIoClose(btn);
                                   SetTextColor(0x000000FF);
                                   SetTextXY(6,5+selection);
                                   PrintText("PATH DOES NOT EXIST - CHOOSE NEW WITH \"O\" or \"[]\"!        ");
                                   SetTextColor(0x0080FFFF);
                                 } else {
                                   sceIoClose(btn);
                                   BLOB[0] = 0xFA;
                                   hcMemPoke( (void*)0x883FFF00, BLOB );
                                   memset(&param, 0, sizeof(param));
                                   param.size = sizeof(param);
                                   param.argp = config+256;
                                   param.args = strlen(param.argp)+1;
                                   param.key = "updater";
                                   sceKernelLoadExec(param.argp, &param);
                                 } }
          if( selection == 6 ) { LaunchCFWInstaller();
          	                     DrawMainScreen(); }
          if( selection == 7 ) { SetTextColor(0x00FFFFFF);
        		                     SetTextXY(2,20);
        		                     PrintText("Going to USB mode....");
        		                     sceKernelDelayThread(3000000);
        		                     RebootForUSB(); }
          if( selection == 8 ) { p = hcFileSelector( "ms0:/", ".flash" );
          	                     strcpy(s, "ms0:/");
        	                       strcpy(s+5, p);
        	                       strcpy(config+512, s);
        	                       config[3] = 1;
        	                       WriteConfig();
        	                       sceIoRename(s, "nandimage.flash");
        	                       ClearScreen();
        	                       PrintText("\n\n  ");
        	                       if( gfxMode == 1 ) { SetTextXY(5,8); }
        	                       PrintText("Renaming NAND image, updating config and launching flasher....");
        	                       sceKernelDelayThread(2500000);
        	                       sceKernelLoadExec("flash0:/kd/restore.elf", NULL); }
        }

        if( selection < 1 ) { selection = 8; }
        if( selection > 8 ) { selection = 1; }
        UpdateCursor();

        while((pad.Buttons & 0xFFFF) != 0){ sceCtrlReadBufferPositive(&pad, 1); }
      }
    }
  }


  // in theory we are never supposed to get here....
  sceKernelExitDeleteThread(0);
  return 0;
}
