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
make
mkdir _release
mkdir _release/RECOVERY
rm _release/RECOVERY/EBOOT.PBP
cp EBOOT.PBP _release/RECOVERY
cp README.TXT _release
cp FASTLZ.TXT _release
