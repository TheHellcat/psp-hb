TARGET = arcpln
OBJS = arcpln.o

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
PSP_EBOOT_TITLE = Arcade-Plane Test
# PSP_EBOOT_ICON = ICON0.PNG
# PSP_EBOOT_PIC1 = PIC1.PNG
# PSP_EBOOT_PSAR = DATA.PSAR

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
