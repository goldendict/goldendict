# libao build instruction
wget https://github.com/ngn999/tslab_libao_library/archive/refs/heads/master.zip
unzip master.zip
cd tslab_libao_library
./configure CC="gcc -arch arm64 -arch x86_64"
make -j 
install_name_tool -id @executable_path/../Frameworks/libao.dylib src/.libs/libao.dylib
