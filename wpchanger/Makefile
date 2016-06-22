TARGET = wpchanger
OBJS = main.o

INCDIR =
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = wpcexp.exp
PSP_FW_VERSION = 300

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = 
LIBS = -lpsprtc_driver
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
