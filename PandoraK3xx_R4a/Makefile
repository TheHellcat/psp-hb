TARGET = pinst3xx
OBJS = main.o kstuff.o misc.o ddcsup.o pspPSAR.o pspDecrypt.o fastlz.o ddc6sup.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 300

LIBDIR = 
LIBS = -lpsppower
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Pandora Installer for 3.xx+ -R4a-
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
