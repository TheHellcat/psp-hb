TARGET = main
OBJS = main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PSP_FW_VERSION = 300

LIBDIR =
LIBS = -lpspgum -lpspgu -lm
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = GFX Test
# PSP_EBOOT_ICON = ICON0.PNG
# PSP_EBOOT_PIC1 = PIC1.PNG
# PSP_EBOOT_PSAR = DATA.PSAR

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# logo.o: logo.raw
# 	bin2o -i logo.raw logo.o logo

md5:
	openssl md5 _release/*.rar
