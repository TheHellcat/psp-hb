TARGET = HCDemo1
OBJS = main.o pspcore.o common.o part0.o

INCDIR = $(PSPDEV)/psp/sdk/include/
LIBS = -lstdc++ -lm
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = /usr/local/pspdev/psp/sdk/lib/
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
# PSP_EBOOT_TITLE = HELLCATs Crappy Demo 1
PSP_EBOOT_TITLE = testing....
PSP_EBOOT_ICON  = ICON0.PNG
# PSP_EBOOT_PIC1  = PIC1.PNG 
# PSP_EBOOT_SND0  = SND0.AT3

include $(PSPDEV)/psp/sdk/lib/build.mak

./main.o: ./main.cpp
	$(CXX) $(CXXFLAGS) -c ./main.cpp -o ./main.o

./pspcore.o: ./pspcore.c
	$(CXX) $(CXXFLAGS) -c ./pspcore.c -o ./pspcore.o

./common.o: ./common.cpp
	$(CXX) $(CXXFLAGS) -c ./common.cpp -o ./common.o

./part0.o: ./part0.cpp
	$(CXX) $(CXXFLAGS) -c ./part0.cpp -o ./part0.o


install:
	mkdir -p "/cygdrive/p/PSP/GAME150/$(TARGET)"
	cp EBOOT.PBP "/cygdrive/p/PSP/GAME150/$(TARGET)/EBOOT.PBP"

kinstall:
	mkdir -p "/cygdrive/p/PSP/GAME150/$(TARGET)"
	mkdir -p "/cygdrive/p/PSP/GAME150/$(TARGET)%"
	cp "$(TARGET)/EBOOT.PBP" "/cygdrive/p/PSP/GAME150/$(TARGET)/EBOOT.PBP"
	cp "$(TARGET)%/EBOOT.PBP" "/cygdrive/p/PSP/GAME150/$(TARGET)%/EBOOT.PBP"

SCEkinstall:
	mkdir -p "/cygdrive/p/PSP/GAME150/__SCE__$(TARGET)"
	mkdir -p "/cygdrive/p/PSP/GAME150/%__SCE__$(TARGET)"
	cp "__SCE__$(TARGET)/EBOOT.PBP" "/cygdrive/p/PSP/GAME150/__SCE__$(TARGET)/EBOOT.PBP"
	cp "%__SCE__$(TARGET)/EBOOT.PBP" "/cygdrive/p/PSP/GAME150/%__SCE__$(TARGET)/EBOOT.PBP"

