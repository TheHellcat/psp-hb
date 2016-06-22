TARGET = rflash
OBJS = main.o func.o helper.o pspPSAR.o pspDecrypt.o fastlz.o psardumper.o iplupdate.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 300

LIBDIR =
LIBS = -lpsppower -lpsprtc
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
# PSP_EBOOT_TITLE = WIP: Re-Recording Flashgiver 1.60
# PSP_EBOOT_TITLE   = B3:Resumption Lightning Giver 1.60
PSP_EBOOT_ICON = ICON0_NUL.PNG
PSP_EBOOT_TITLE = HC Recovery Flasher
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG
PSP_EBOOT_PSAR = DATA.PSAR

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

md5:
	openssl md5 _release/*.rar
