/*
 *  USB access functions
 *  for accessing f0, f1 and ms via USB
 *
 *  credits @ Team WildCards public released sourcecode
 *            and the one and only original CFW creator DAX
 *
 *  This code has been adapted (and learned from) from Team WildCards sources
 *    without them, this function would probbably have taken me a few years longer to be done.... ;-)
 *
 *  additional IO hooks to gather information about Read/Write access
 *    are indeed by me: ---==> HELLCAT <==---
 *    believe it or not, I really understood what's going here *lol*
 *
 */


#include <pspkernel.h>
#include <pspsdk.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int usbStatus = 0;
int usbModuleStatus = 0;
int usbAccessWrite = 0;
int usbAccessRead = 0;
int iTotalBytesRead = 0;
int iTotalBytesWritten = 0;

PspIoDrv *lflash_driver;
PspIoDrv *msstor_driver;

int (* Orig_IoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (* Orig_IoClose)(PspIoDrvFileArg *arg);
int (* Orig_IoRead)(PspIoDrvFileArg *arg, char *data, int len);
int (* Orig_IoWrite)(PspIoDrvFileArg *arg, const char *data, int len);
SceOff(* Orig_IoLseek)(PspIoDrvFileArg *arg, SceOff ofs, int whence);
int (* Orig_IoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int (* Orig_IoDevctl)(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, 
int outlen);

int unit;

/* 1.50 specific function */
PspIoDrv *FindDriver(char *drvname)
{
  u32 *mod = (u32 *)sceKernelFindModuleByName("sceIOFileManager");

  if (!mod)
  {
    return NULL;
  }

  u32 text_addr = *(mod+27);

  u32 *(* GetDevice)(char *) = (void *)(text_addr+0x16D4);
  u32 *u;

  u = GetDevice(drvname);

  if (!u)
  {
    return NULL;
  }

  return (PspIoDrv *)u[1];
}

static int New_IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
  if (!lflash_driver->funcs->IoOpen)
    return -1;

  if (unit == 0)
    file = "0,0";
  else
    file = "0,1";

  return lflash_driver->funcs->IoOpen(arg, file, flags, mode);
}

static int New_IoClose(PspIoDrvFileArg *arg)
{
  if (!lflash_driver->funcs->IoClose)
    return -1;

  return lflash_driver->funcs->IoClose(arg);
}

static int New_IoRead(PspIoDrvFileArg *arg, char *data, int len)
{
  if (!lflash_driver->funcs->IoRead)
    return -1;
  usbAccessRead = 1;
  iTotalBytesRead = iTotalBytesRead + len;
  return lflash_driver->funcs->IoRead(arg, data, len);
}
static int New_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
  if (!lflash_driver->funcs->IoWrite)
    return -1;
  usbAccessWrite = 1;
  iTotalBytesWritten = iTotalBytesWritten + len;
  return lflash_driver->funcs->IoWrite(arg, data, len);
}

static SceOff New_IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence)
{
  if (!lflash_driver->funcs->IoLseek)
    return -1;

  return lflash_driver->funcs->IoLseek(arg, ofs, whence);
}

u8 data_5803[96] = 
{
  0x02, 0x00, 0x08, 0x00, 0x08, 0x00, 0x07, 0x9F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x21, 0x21, 0x00, 0x00, 0x20, 0x01, 0x08, 0x00, 0x02, 0x00, 0x02, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int New_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
  if (cmd == 0x02125008)
  {
    u32 *x = (u32 *)outdata;
    *x = 1; /* Enable writing */
    return 0;
  }
  else if (cmd == 0x02125803)
  {
    memcpy(outdata, data_5803, 96);
    return 0;
  }

  return -1;
}

static int New_IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void 
*outdata, int outlen)
{
  if (cmd == 0x02125801)
  {
    u8 *data8 = (u8 *)outdata;

    data8[0] = 1;
    data8[1] = 0;
    data8[2] = 0,
    data8[3] = 1;
    data8[4] = 0;
        
    return 0;
  }

  return -1;
}

////////////////////////////////////////
//  some of Hellcat's Hook-A-Boo ;-)  //
////////////////////////////////////////
static int hcHooked_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
  usbAccessWrite = 1;
  iTotalBytesWritten = iTotalBytesWritten + len;
  return Orig_IoWrite(arg, data, len);
}
static int hcHooked_IoRead(PspIoDrvFileArg *arg, char *data, int len)
{
  usbAccessRead = 1;
  iTotalBytesRead = iTotalBytesRead + len;
  return Orig_IoRead(arg, data, len);
}

void disableUsb(void) 
{ 
  if(usbStatus) 
  {
    sceUsbDeactivate(0);
    sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
    sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);

    msstor_driver->funcs->IoOpen = Orig_IoOpen;
    msstor_driver->funcs->IoClose = Orig_IoClose;
    msstor_driver->funcs->IoRead = Orig_IoRead;
    msstor_driver->funcs->IoWrite = Orig_IoWrite;
    msstor_driver->funcs->IoLseek = Orig_IoLseek;
    msstor_driver->funcs->IoIoctl = Orig_IoIoctl;
    msstor_driver->funcs->IoDevctl = Orig_IoDevctl;

    usbStatus = 0;
    
    iTotalBytesRead = 0;
    iTotalBytesWritten = 0;
    
    // invalidate the MS driver cache 
    sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0 );
    sceIoDevctl("flashfat0:", 0x0240D81E, NULL, 0, NULL, 0 ); 
    sceIoDevctl("flashfat1:", 0x0240D81E, NULL, 0, NULL, 0 ); 
  }
}

void enableUsb(int device) 
{
  if (usbStatus)
  {
    disableUsb();
    sceKernelDelayThread(300000);
  }

  if(!usbModuleStatus) 
  {
    pspSdkLoadStartModule("flash0:/kd/semawm.prx", PSP_MEMORY_PARTITION_KERNEL);
    pspSdkLoadStartModule("flash0:/kd/usbstor.prx", PSP_MEMORY_PARTITION_KERNEL);
    pspSdkLoadStartModule("flash0:/kd/usbstormgr.prx", PSP_MEMORY_PARTITION_KERNEL);
    pspSdkLoadStartModule("flash0:/kd/usbstorms.prx", PSP_MEMORY_PARTITION_KERNEL);
    pspSdkLoadStartModule("flash0:/kd/usbstorboot.prx", PSP_MEMORY_PARTITION_KERNEL);
    
    lflash_driver = FindDriver("lflash");
    msstor_driver = FindDriver("msstor");
    
    Orig_IoOpen = msstor_driver->funcs->IoOpen;
    Orig_IoClose = msstor_driver->funcs->IoClose;
    Orig_IoRead = msstor_driver->funcs->IoRead;
    Orig_IoWrite = msstor_driver->funcs->IoWrite;
    Orig_IoLseek = msstor_driver->funcs->IoLseek;
    Orig_IoIoctl = msstor_driver->funcs->IoIoctl;
    Orig_IoDevctl = msstor_driver->funcs->IoDevctl;

    usbModuleStatus = 1;
  }

  if (device != 0)
  {
    unit = device-1;

    msstor_driver->funcs->IoOpen = New_IoOpen;
    msstor_driver->funcs->IoClose = New_IoClose;
    msstor_driver->funcs->IoRead = New_IoRead;
    msstor_driver->funcs->IoWrite = New_IoWrite;
    msstor_driver->funcs->IoLseek = New_IoLseek;
    msstor_driver->funcs->IoIoctl = New_IoIoctl;
    msstor_driver->funcs->IoDevctl = New_IoDevctl;
  } else {
  	msstor_driver->funcs->IoRead = hcHooked_IoRead;
    msstor_driver->funcs->IoWrite = hcHooked_IoWrite;
  }
  
  sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
  sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
  sceUsbstorBootSetCapacity(0x800000);
  sceUsbActivate(0x1c8);
  usbStatus = 1;
}



void usbEnableUsbMS0(void)
{
  enableUsb(0);
}

void usbEnableUsbF0(void)
{
  enableUsb(1);
}

void usbEnableUsbF1(void)
{
  enableUsb(2);
}

void usbDisableUsb(void)
{
  disableUsb();
}

int usbGetUsbState(void)
{
	int r = 0;
	
	u32 state;
	state = sceUsbGetState();

  r = state + (usbAccessWrite * 0x00010000) + (usbAccessRead * 0x00020000);
  usbAccessRead = 0;
  usbAccessWrite = 0;
  return r;
}

int usbGetBytesRead(void)
{
	return iTotalBytesRead;
}

int usbGetBytesWritten(void)
{
	return iTotalBytesWritten;
}
