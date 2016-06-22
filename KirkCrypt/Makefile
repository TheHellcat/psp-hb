TARGET = kcrypter
OBJS = main.o intercom.o misc.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 300

LIBDIR =
LIBS =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
# PSP_EBOOT_ICON = ICON0_NUL.PNG
PSP_EBOOT_TITLE = Kirk Crypter Test
# PSP_EBOOT_ICON = ICON0.PNG
# PSP_EBOOT_PIC1 = PIC1.PNG
# PSP_EBOOT_PSAR = DATA.PSAR

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

md5:
	openssl md5 _release/*.rar
