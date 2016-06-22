TARGET = bootmenu
OBJS = main.o lflashfatfmt.o sysfunc.o scenand.o patchipl.o usb.o gfxstuff.o scepower.o
# BUILD_PRX = 1
LIBS = -lpsppower -lpspusb -lpspusb_driver -lpspusbbus_driver -lpspusbstor -lpng -lm -lz -lpspgu -lfreetype

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

# EXTRA_TARGETS = EBOOT.PBP
# PSP_EBOOT_TITLE = Extended PANDORA Bootmenu
# PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

strip:
	$(STRIP) $(TARGET).elf

test:
	make
	$(STRIP) $(TARGET).elf
	cp bootmenu.elf /cygdrive/h/E/
