/*  ==========================================  *\
 *                                              *
 *  PANDORA INSTALLER FOR 3.XX KERNELS          *
 *  smashed together by ---==> HELLCAT <==---   *
 *                                              *
\*  ==========================================  */

//   The Kernel Module for stuff we can't do    //
//   in usermode                                //
//////////////////////////////////////////////////

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "pspcrypt.h"
#include "systemctrl.h"

// here come some binary files, we include into our module for later extraction
// it's all additional files that get written to the MS ontop of the 1.50 dump
#include "package/pspbtcnf.h"
#include "package/pspbtcnfgame.h"
#include "package/pspbtcnfupdater.h"
#include "package/ireg.h"
#include "package/dreg.h"
#include "package/backup.h"
#include "package/decryptprx.h"
#include "package/extprxs.h"
#include "package/flashemu.h"
#include "package/lfatfs_patched.h"
#include "package/libpsardumper.h"
#include "package/restore.h"
#include "package/ta082_driver.h"
#include "package/updateflasher.h"
#include "package/msipl.h"
#include "package/pstart.h"

PSP_MODULE_INFO("KernelStuff_driver", 0x1006, 3, 2);
PSP_MAIN_THREAD_ATTR(0);

u8 *dataPSAR, *dataOut, *dataOut2;
char *dataMisc;
int use371nids = 0;
int useDirectAccess = 0;

#include "psardump.c"

u8 sha1_150[20] =
{
  0x1A, 0x4C, 0x91, 0xE5, 0x2F, 0x67, 0x9B, 0x8B,
  0x8B, 0x29, 0xD1, 0xA2, 0x6A, 0xF8, 0xC5, 0xCA,
  0xA6, 0x04, 0xD3, 0x30
};

u8 sha1[20];

SceKernelUtilsSha1Context ctx;


// here we go....
int fnow = 0;
void SetInstallFileOffset(int o)
{
  fnow = o;
}
int DoInstall(int full)
{
  // the main installer routine....

  int r;

  if( (full == 1) || (full == 2) )
  {

    // whoa, this one is the real magic ;-)
    r = extract_psar(dataPSAR, 10149440, dataOut, dataOut2);
    if(r!=0){ return r; }

    fnow++;

    if(fnow==1)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_dreg, 0);
      if( r < 0)
      {
        fnow++;
      } else {
        WriteFile("ms0:/registry/system.dreg", dataOut2, r);
        return r;
      }
    }

    if(fnow==2)
    {
      r = sceKernelGzipDecompress(dataOut2, 2000000, f_ireg, 0);
      if( r < 0)
      {
        fnow++;
      } else {
        WriteFile("ms0:/registry/system.ireg", dataOut2, r);
        return r;
      }
    }

    fnow--;
  }

  if( full != 2)
  {
  fnow++;
  if(fnow<3){ fnow=3; }
  if(fnow==3)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_backup, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/backup.elf", dataOut2, r);
      return r;
    }
  }

  if(fnow==4)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_decryptprx, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/decryptprx.prx", dataOut2, r);
      return r;
    }
  }

  if(fnow==5)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_extprxs, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/extprxs.elf", dataOut2, r);
      return r;
    }
  }

  if(fnow==6)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_flashemu, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/flashemu.prx", dataOut2, r);
      return r;
    }
  }

  if(fnow==7)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_lfatfs_patched, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/lfatfs_patched.prx", dataOut2, r);
      return r;
    }
  }

  if(fnow==8)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_libpsardumper, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/libpsardumper.prx", dataOut2, r);
      return r;
    }
  }

  if(fnow==9)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_restore, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/restore.elf", dataOut2, r);
      return r;
    }
  }

  if(fnow==10)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_ta082_driver, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/ta082_driver.prx", dataOut2, r);
      return r;
    }
  }

  if(fnow==11)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_updateflasher, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/updateflasher.elf", dataOut2, r);
      return r;
    }
  }

  if(fnow==12)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_msipl, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/msipl.bin", dataOut2, r);
      return r;
    }
  }

  if(fnow==13)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnf, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/pspbtcnf.txt", dataOut2, r);
      return r;
    }
  }

  if(fnow==14)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfgame, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/pspbtcnf_game.txt", dataOut2, r);
      return r;
    }
  }

  if(fnow==15)
  {
    r = sceKernelGzipDecompress(dataOut2, 2000000, f_pspbtcnfupdater, 0);
    if( r < 0)
    {
      fnow++;
    } else {
      WriteFile("ms0:/kd/pspbtcnf_updater.txt", dataOut2, r);
      return r;
    }
  }
  }

  return 0;
}

int kstuffGetMsPartitionStart(void)
{
  int r;

  r = sceKernelGzipDecompress(dataOut, 2000000, bin_pstart, 0);
  if( r < 0)
  {
    return r;
  } else {
    // WriteFile("ms0:/registry/system.dreg", dataOut, r);
    return r;
  }

  return 0;
}


// write battery EEPROM
u32 sceSyscon_driver_1165C864(u8 addr, u16 data);
// read battery EEPROM
u32 sceSyscon_driver_68EF0BEF(u8 addr);
// same for 3.71
u32 sceSyscon_driver_40CBBD46(u8 addr, u16 data);
u32 sceSyscon_driver_B9C1B0AC(u8 addr);

int module_start(SceSize args, void *argp)
{
  int k = pspSdkSetK1(0);
  int kit = sceKernelDevkitVersion();
  if (kit < 0x03070110) // pre 3.71
  {
    use371nids = 0;
  }
  if (kit >= 0x03070110) { // 3.71+
    use371nids = 1;
  }
  if (kit >= 0x03080000) { // 3.80+
    use371nids = 2;
    useDirectAccess = 1;
  }

  pspSdkSetK1(k);
  return 0;
}

int module_stop()
{
  return 0;
}

int kstuffInitBuffers(u8 *b1, u8 *b2, u8 *b3, char *b4)
{
  dataPSAR = b1;
  dataOut  = b2;
  dataOut2 = b3;
  dataMisc = b4;
  return 0;
}

int kstuffMsInstallFW(void)
{
  return DoInstall(2);
}

int kstuffMsInstallAdditional(void)
{
  return DoInstall(0);
}

// ===== BEGIN of direct HW access code, credits to Silverspring for this :-) ===== //
int sceSysconCmdExec(u8* param, int unk);  // prototype for Syscon call
int batDirectWriteEeprom(u8 addr, u16 data)
{
  int res;
  u8 param[0x60];

  if (addr > 0x7F)
  return(0x80000102);

  param[0x0C] = 0x73; // write battery eeprom command
  param[0x0D] = 5; // tx packet length

  // tx data
  param[0x0E] = addr;
  param[0x0F] = data;
  param[0x10] = data>>8;

  res = sceSysconCmdExec(param, 0);

  if (res < 0)
  return(res);

  return 0;
}

int batDirectReadEeprom(u8 addr)
{
  int res;
  u8 param[0x60];

  if (addr > 0x7F)
  return(0x80000102);

  param[0x0C] = 0x74; // read battery eeprom command
  param[0x0D] = 3; // tx packet length

  // tx data
  param[0x0E] = addr;

  res = sceSysconCmdExec(param, 0);

  if (res < 0)
  return(res);

  // rx data
  return((param[0x21]<<8) | param[0x20]);
}
// ===== END of direct HW access code ===== //

int kstuffBatSetSerial(char* serial)
{
  int r=0;
  int k;
  int shi;
  int slo;

  shi = ((serial[0]&0xFF)*0x100)+(serial[1]&0xFF);
  slo = ((serial[2]&0xFF)*0x100)+(serial[3]&0xFF);

  k = pspSdkSetK1(0);
  if( use371nids == 0)
  {
    r = sceSyscon_driver_1165C864(0x07, shi);
    r = sceSyscon_driver_1165C864(0x09, slo);
  } else if ( use371nids == 1 ) {
    r = sceSyscon_driver_40CBBD46(0x07, shi);
    r = sceSyscon_driver_40CBBD46(0x09, slo);
  } else if ( useDirectAccess == 1 ) {
    r = batDirectWriteEeprom(0x07, shi);
    r = batDirectWriteEeprom(0x09, slo);
  }
  pspSdkSetK1(k);
  return r;
}

void kstuffBatGetSerial(char* serial)
{
  int r=0;
  int k;

  k = pspSdkSetK1(0);
  if( use371nids == 0)
  {
    r = sceSyscon_driver_68EF0BEF(0x07);
    serial[0] = (r / 0x100) & 0xFF;
    serial[1] = r & 0xFF;
    r = sceSyscon_driver_68EF0BEF(0x09);
    serial[2] = (r / 0x100) & 0xFF;
    serial[3] = r & 0xFF;
  } else if ( use371nids == 1 ) {
    r = sceSyscon_driver_B9C1B0AC(0x07);
    serial[0] = (r / 0x100) & 0xFF;
    serial[1] = r & 0xFF;
    r = sceSyscon_driver_B9C1B0AC(0x09);
    serial[2] = (r / 0x100) & 0xFF;
    serial[3] = r & 0xFF;
  } else if ( useDirectAccess == 1 ) {
    r = batDirectReadEeprom(0x07);
    serial[0] = (r / 0x100) & 0xFF;
    serial[1] = r & 0xFF;
    r = batDirectReadEeprom(0x09);
    serial[2] = (r / 0x100) & 0xFF;
    serial[3] = r & 0xFF;
  }
  pspSdkSetK1(k);
}

int kstuffBatMakeService(void)
{
  int r;
  char NewSer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

  r = kstuffBatSetSerial( NewSer );

  return r;
}

int kstuffBatMakeNormal(void)
{
  int r;
  char NewSer[4] = { 0x90, 0xCA, 0x08, 0x15 };

  r = kstuffBatSetSerial( NewSer );

  return r;
}

void kstuffSetUserFileWriteFunc(void* func)
{
  /*userWriteFile = func;*/
}

int sceSyscon_driver_7EC5A957(u32 *baryon);
u32 sceSysreg_driver_E2A5D1EE(void);
void kstuffGetMoboVersions(int* baryon, int* tachyon)
{
  u32 by, ty;
  int k;

  k = pspSdkSetK1(0);
  ty = sceSysreg_driver_E2A5D1EE();
  sceSyscon_driver_7EC5A957(&by);
  pspSdkSetK1(k);
  (*baryon) = by;
  (*tachyon) = ty;
}

int kstuffGetKCtrl( void )
{
  int btn;
  SceCtrlData pad;
  int k;

  k = pspSdkSetK1(0);

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

  sceCtrlReadBufferPositive( &pad, 1 );
  btn = pad.Buttons;

  pspSdkSetK1(k);

  return btn;
}

int kstuffInvalidateMSCache(void)
{
  return 0;
}

int kstuff__end(void)
{
  return 0;
}
