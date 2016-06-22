/*
 *
 *   modified version of the M33 "NEW PSAR DUMPER"
 *   as supplement for the
 *   "Pandora for 3.xx+ Installer"
 *   for extracting 3.40 modules
 *
 *   all code in here is done by the people noted in the crediting comment
 *   bellow
 *   only modifications to make it work seemlessly with pan3xx by me!
 *
 */


// PSAR dumper for Updater data
// Original author: PspPet
//
// Contributions:
//
// Vampire (bugfixes)
// Nem (ipl decryption)
// Dark_AleX (2.60-2.80 decryption)
// Noobz (3.00-3.02 decryption)
// Team C+D (3.03-3.52 decryption)
// M33 Team (3.60-3.71 decryption) + recode for 2.XX+ kernels

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "libpsardumper.h"
#include "pspdecrypt.h"

#define printf    pspDebugScreenPrintf

char* Wanted340Files[] = { "ms0:/kd/clockgen.prx",
                           "ms0:/kd/ctrl.prx",
                           "ms0:/kd/display.prx",
                           "ms0:/kd/dmacman.prx",
                           "ms0:/kd/dmacplus.prx",
                           "ms0:/kd/emc_ddr.prx",
                           "ms0:/kd/emc_sm.prx",
                           "ms0:/kd/gpio.prx",
                           "ms0:/kd/i2c.prx",
                           "ms0:/kd/lcdc.prx",
                           "ms0:/kd/led.prx",
                           "ms0:/kd/power.prx",
                           "ms0:/kd/pwm.prx",
                           "ms0:/kd/syscon.prx",
                           "ms0:/kd/sysreg.prx",
                           "ms0:/kd/systimer.prx",
                           "0" };

////////////////////////////////////////////////////////////////////
// big buffers for data. Some system calls require 64 byte alignment

static u8* g_dataPSAR;

static u8* g_dataOut;

static u8 *g_dataOut2;

void ErrorExit(int milisecs, char *fmt, ...)
{
  va_list list;
  char msg[256];

  va_start(list, fmt);
  vsprintf(msg, fmt, list);
  va_end(list);

  printf(msg);

  sceKernelDelayThread(milisecs*1000);
}

////////////////////////////////////////////////////////////////////
// File helpers

int ReadFile(char *file, int seek, void *buf, int size)
{
  SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
  if (fd < 0)
    return fd;

  if (seek > 0)
  {
    if (sceIoLseek(fd, seek, PSP_SEEK_SET) != seek)
    {
      sceIoClose(fd);
      return -1;
    }
  }

  int read = sceIoRead(fd, buf, size);

  sceIoClose(fd);
  return read;
}

int WriteFile(char *file, void *buf, int size)
{
  int i;
  int ICanHas;

  // check we want/need the requested file to be written....
  i = 0;
  ICanHas = 0;
  while( 1 )
  {
    if( strcmp( file, Wanted340Files[i] ) == 0 ) { ICanHas = 1; }
    i++;
    if( strcmp( Wanted340Files[i], "0" ) == 0 ) { break; }
  }
  if( ICanHas == 0 ) { return size; }


  SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

  if (fd < 0)
  {
    return fd;
  }

  int written = sceIoWrite(fd, buf, size);

  sceIoClose(fd);
  return written;
}

enum
{
  MODE_ENCRYPT_SIGCHECK,
  MODE_ENCRYPT,
  MODE_DECRYPT,
};

static char *GetVersion(char *buf)
{
  char *p = strrchr(buf, ',');

  if (!p)
    return NULL;

  return p+1;
}

void SupClearScreen(void)
{
  pspDebugScreenSetBackColor(0x00402000);
  pspDebugScreenSetTextColor(0x00FFFFFF);
  pspDebugScreenClear();
  printf("*** Pandora Installer for 3.xx+ Kernels ***\n");
  printf("*******************************************\n\n");
  pspDebugScreenSetTextColor(0x00FFFF80);
  printf("Stage 2 - DDC Supplement\n3.40 FW module extraction\n\nbased on modified M33 NewPSARDumper\nAll credits are belong to there ;-) (see source for details)\n\n");
  pspDebugScreenSetTextColor(0x00FFFFFF);
  printf("-----------------------------------------------------------------\n");
  printf("\n");
  pspDebugScreenSetTextColor(0x0080FFFF);
}

//int main(void)
int ddcsup( u8* buf1, u8* buf2, u8* buf3 )
{
  int mode=0, s;
  u8 pbp_header[0x28];

  int psarbufsize = 17000000;
  int outbufsize  =  2500000;

  g_dataPSAR = buf1;
  g_dataOut = buf2;
  g_dataOut2 = buf3;

  pspDebugScreenInit();
  SupClearScreen();

  /*if (sceKernelDevkitVersion() < 0x02070110)
  {
    ErrorExit(10000, "This program requires 2.71 or higher.\n",
                   "If you are in a cfw, please reexecute psardumper on the higher kernel.\n");
  }

  SceUID mod = pspSdkLoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    ErrorExit(5000, "Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
  }

  mod = pspSdkLoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
  if (mod < 0)
  {
    ErrorExit(5000, "Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
  } */
/*
    printf("PSAR Dumper\n");
    printf(" by PspPet\n");

  printf("2.80 Decryption by Dark_AleX.\n");
  printf("3.00 Decryption by Team Noobz.\n");
  printf("3.03 + 3.10 + 3.30 Decryption by Team C+D.\n");
  printf("3.60 + 3.71 decryption by M33 Team.\n\n");

  printf("Press cross to dump encrypted with sigcheck and decrypted reboot.bin.\n");
  printf("Press circle to dump encrypted without sigcheck and decrypted reboot.bin.\n");
  printf("Press square to decrypt all.\n\n");

  while (1)
  {
    SceCtrlData pad;

    sceCtrlReadBufferPositive(&pad, 1);

    if (pad.Buttons & PSP_CTRL_CROSS)
    {
      mode = MODE_ENCRYPT_SIGCHECK;
      break;
    }
    else if (pad.Buttons & PSP_CTRL_CIRCLE)
    {
      mode = MODE_ENCRYPT;
      break;
    }
    else if (pad.Buttons & PSP_CTRL_SQUARE)
    {
      mode = MODE_DECRYPT;
      break;
    }

    sceKernelDelayThread(10000);
  }
*/

  mode = MODE_DECRYPT;

  sceKernelVolatileMemLock(0, (void *)&g_dataOut2, &s);

  printf("Loading ms0:/340.PBP....\n");

  if (ReadFile("ms0:/340.PBP", 0, pbp_header, sizeof(pbp_header)) != sizeof(pbp_header))
  {
    ErrorExit(5000, "Cannot find 340.PBP at root.\n");
  }

  int cbFile = ReadFile("ms0:/340.PBP", *(u32 *)&pbp_header[0x24], g_dataPSAR, psarbufsize);
  if (cbFile <= 0)
  {
    ErrorExit(5000, "Error Reading 340.PBP.\n");
  }
  else if (cbFile == psarbufsize)
  {
    ErrorExit(5000, "PSAR too big. Recompile with bigger buffer.\n");
  }

  printf("PSAR file loaded (%d bytes)\n", cbFile);

    if (memcmp(g_dataPSAR, "PSAR", 4) != 0)
    {
        ErrorExit(5000, "Not a PSAR file\n");
    }

  if (pspPSARInit(g_dataPSAR, g_dataOut, g_dataOut2) < 0)
  {
    ErrorExit(5000, "pspPSARInit failed!.\n");
  }

  printf("Version: %s.\n\nExtracting required 3.40 modules, please wait....\n", GetVersion((char *)g_dataOut+0x10));

  /*  sceIoMkdir("ms0:/F0", 0777);
  sceIoMkdir("ms0:/F0/PSARDUMPER", 0777);
  sceIoMkdir("ms0:/F0/data", 0777);
  sceIoMkdir("ms0:/F0/dic", 0777);
  sceIoMkdir("ms0:/F0/font", 0777);
  sceIoMkdir("ms0:/F0/kd", 0777);
  sceIoMkdir("ms0:/F0/vsh", 0777);
  sceIoMkdir("ms0:/F0/data/cert", 0777);
  sceIoMkdir("ms0:/F0/kd/resource", 0777);
  sceIoMkdir("ms0:/F0/vsh/etc", 0777);
  sceIoMkdir("ms0:/F0/vsh/module", 0777);
  sceIoMkdir("ms0:/F0/vsh/resource", 0777); */

    while (1)
  {
    char name[128];
    int cbExpanded;
    int pos;
    int signcheck;

    int res = pspPSARGetNextFile(g_dataPSAR, cbFile, g_dataOut, g_dataOut2, name, &cbExpanded, &pos, &signcheck);

    if (res < 0)
    {
      ErrorExit(5000, "PSAR decode error, pos=0x%08X.\n", pos);
    }
    else if (res == 0) /* no more files */
    {
      break;
    }

    /*if (!strncmp(name, "com:", 4) && comtable_size > 0)
    {
      if (!FindTablePath(com_table, comtable_size, name+4, name))
      {
        ErrorExit(5000, "Error: cannot find path of %s.\n", name);
      }
    }

    else if (!strncmp(name, "01g:", 4) && _1gtable_size > 0)
    {
      if (!FindTablePath(_1g_table, _1gtable_size, name+4, name))
      {
        ErrorExit(5000, "Error: cannot find path of %s.\n", name);
      }
    }

    else if (!strncmp(name, "02g:", 4) && _2gtable_size > 0)
    {
      if (!FindTablePath(_2g_table, _2gtable_size, name+4, name))
      {
        ErrorExit(5000, "Error: cannot find path of %s.\n", name);
      }
    } */

        //printf("'%s' ", name);

    char* szFileBase = strrchr(name, '/');

    if (szFileBase != NULL)
      szFileBase++;  // after slash
    else
      szFileBase = "err.err";

    if (cbExpanded > 0)
    {
      char szDataPath[128];

      if (!strncmp(name, "flash0:/", 8))
      {
        sprintf(szDataPath, "ms0:/%s", name+8);
      }

      /*else if (!strncmp(name, "flash1:/", 8))
      {
        sprintf(szDataPath, "ms0:/F1/%s", name+8);
      }

      else if (!strcmp(name, "com:00000"))
      {
        comtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded);

        if (comtable_size <= 0)
        {
          ErrorExit(5000, "Cannot decrypt common table.\n");
        }

        if (comtable_size > sizeof(com_table))
        {
          ErrorExit(5000, "Com table buffer too small. Recompile with bigger buffer.\n");
        }

        memcpy(com_table, g_dataOut2, comtable_size);
        strcpy(szDataPath, "ms0:/F0/PSARDUMPER/common_files_table.bin");
      }

      else if (!strcmp(name, "01g:00000"))
      {
        _1gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded);

        if (_1gtable_size <= 0)
        {
          ErrorExit(5000, "Cannot decrypt 1g table.\n");
        }

        if (_1gtable_size > sizeof(_1g_table))
        {
          ErrorExit(5000, "1g table buffer too small. Recompile with bigger buffer.\n");
        }

        memcpy(_1g_table, g_dataOut2, _1gtable_size);
        strcpy(szDataPath, "ms0:/F0/PSARDUMPER/fat_files_table.bin");
      }

      else if (!strcmp(name, "02g:00000"))
      {
        _2gtable_size = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded);

        if (_2gtable_size <= 0)
        {
          ErrorExit(5000, "Cannot decrypt 2g table %08X.\n", _2gtable_size);
        }

        if (_2gtable_size > sizeof(_2g_table))
        {
          ErrorExit(5000, "2g table buffer too small. Recompile with bigger buffer.\n");
        }

        memcpy(_2g_table, g_dataOut2, _2gtable_size);
        strcpy(szDataPath, "ms0:/F0/PSARDUMPER/slim_files_table.bin");
      } */

      else
      {
        sprintf(szDataPath, "ms0:/F0/PSARDUMPER/%s", strrchr(name, '/') + 1);
      }

      //printf("expanded");

      if (signcheck && mode == MODE_ENCRYPT_SIGCHECK
        && (strcmp(name, "flash0:/kd/loadexec.prx") != 0)
        && (strcmp(name, "flash0:/kd/loadexec_02g.prx") != 0))
      {
        pspSignCheck(g_dataOut2);
      }

      if ((mode != MODE_DECRYPT) || (memcmp(g_dataOut2, "~PSP", 4) != 0))
      {
        /*if (strstr(szDataPath, "ipl") && strstr(szDataPath, "2000"))
        {
          // IPL Pre-decryption
          cbExpanded = pspDecryptPRX(g_dataOut2, g_dataOut, cbExpanded);
          if (cbExpanded <= 0)
          {
            printf("Warning: cannot pre-decrypt 2000 IPL.\n");
          }
          else
          {
            memcpy(g_dataOut2, g_dataOut, cbExpanded);
          }
        } */

        // !!! //
        if (WriteFile(szDataPath, g_dataOut2, cbExpanded) != cbExpanded)
              {
          ErrorExit(5000, "Cannot write %s.\n", szDataPath);
          break;
        }

        //printf(",saved");
      }

      if ((memcmp(g_dataOut2, "~PSP", 4) == 0) &&
        (mode == MODE_DECRYPT))
      {
        int cbDecrypted = pspDecryptPRX(g_dataOut2, g_dataOut, cbExpanded);

        // output goes back to main buffer
        // trashed 'g_dataOut2'
        if (cbDecrypted > 0)
        {
          u8* pbToSave = g_dataOut;
          int cbToSave = cbDecrypted;

          //printf(",decrypted");

          if ((g_dataOut[0] == 0x1F && g_dataOut[1] == 0x8B) ||
            memcmp(g_dataOut, "2RLZ", 4) == 0)
          {
            int cbExp = pspDecompress(g_dataOut, g_dataOut2, outbufsize);

            if (cbExp > 0)
            {
              //printf(",expanded");
              pbToSave = g_dataOut2;
              cbToSave = cbExp;
            }
            else
            {
              //printf("Decompress error\n"
              //     "File will be written compressed.\n");
            }
          }

          // !!! //
          if (WriteFile(szDataPath, pbToSave, cbToSave) != cbToSave)
          {
            ErrorExit(5000, "Error writing %s.\n", szDataPath);
          }

          //printf(",saved!");
        }
        else
        {
          ErrorExit(5000, "Error in decryption.\n");
        }
      }

      else if (strncmp(name, "ipl:", 4) == 0)
      { /*
        sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part1_%s", szFileBase);

        int cb1 = pspDecryptIPL1(g_dataOut2, g_dataOut, cbExpanded);
        if (cb1 > 0 && (WriteFile(szDataPath, g_dataOut, cb1) == cb1))
        {
          int cb2 = pspLinearizeIPL2(g_dataOut, g_dataOut2, cb1);
          sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part2_%s", szFileBase);

          WriteFile(szDataPath, g_dataOut2, cb2);

          int cb3 = pspDecryptIPL3(g_dataOut2, g_dataOut, cb2);
          sprintf(szDataPath, "ms0:/F0/PSARDUMPER/part3_%s", szFileBase);
          WriteFile(szDataPath, g_dataOut, cb3);
        } */
      }
    }
    else if (cbExpanded == 0)
    {
      //printf("empty");
    }

    //printf("\n");
    scePowerTick(0);
  }

  //ExtractReboot(mode, "ms0:/F0/kd/loadexec.prx", "ms0:/F0/reboot.bin", "reboot.bin");
  //ExtractReboot(mode, "ms0:/F0/kd/loadexec_02g.prx", "ms0:/F0/reboot_02g.bin", "reboot_02g.bin");

    scePowerTick(0);
  ErrorExit(2000, "\n\nDone.\n");

    return 0;
}

