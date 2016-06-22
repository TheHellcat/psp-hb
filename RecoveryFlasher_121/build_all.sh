mkdir _release
mkdir _release/RECOVERY
mkdir _release/Pandora
rm _release/RECOVERY/EBOOT.PBP
rm _release/Pandora/rflash.elf

cd helper
make clean
make
cp khelper.prx ../myPSAR/files
cd ..

cd myPSAR
./run.sh
mv DATA.PSAR ..
cd ..

make clean
cp Makefile.elf Makefile
make
psp-strip rflash.elf
cp rflash.elf _release/Pandora
cp README-Pandora.txt _release/Pandora
cp Makefile.EBOOT Makefile

make clean
make
cp EBOOT.PBP _release/RECOVERY
cp README.TXT _release
cp FASTLZ.TXT _release
