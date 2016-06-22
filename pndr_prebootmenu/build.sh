clear
make clean
echo -n const char* builddate = \" > builddate.h
date +"%d-%m-%y %H:%M\";" >> builddate.h
make
make strip
cp bootmenu.elf /cygdrive/h/E/
cd installer
make clean
make
cp EBOOT.PBP /cygdrive/h/E/
cd kstuff
make clean
make
cp kstuff.prx /cygdrive/h/E/
cd ../..
cd unemu_prx
make clean
make
cp unemu.prx /cygdrive/h/E/
cd ..
cd loader_prx
make clean
make
cp loadmenu.prx /cygdrive/h/E/
cd ..
