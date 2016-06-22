TARGET = deemerh
OBJS = main.o sgdrv.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = dhexp.exp
PSP_FW_VERSION = 300

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = 
LIBS =
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

exports:
	psp-build-exports -b dhexp.exp
