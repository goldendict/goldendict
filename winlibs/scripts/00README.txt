FFmpeg, libao, zLib build instructions
-----------------------------------

Prerequisites
=============
* MSYS and MinGW ( 4.4 up to 4.6 )

Build
=====

* Get source tarball for libspeex, ffmpeg, libao, zlib and extract them respectively:
  * libspeex: http://www.speex.org/downloads/
  * libao: http://www.xiph.org/downloads/
  * ffmpeg: http://ffmpeg.org/download.html
  * zlib: http://zlib.net/

* Define environment variable: PREFIX, for example: export PREFIX=/usr/local
  * NOTE: You may also want to define CPATH and LIBRARY_PATH:
    export CPATH=${CPATH}:${PREFIX}/include
    export LIBRARY_PATH=${LIBRARY_PATH}:${PREFIX}/lib
 		export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:${PREFIX}/lib/pkgconfig
 		
* First, build the libspeex as static library:
  * ./configure --prefix=$PREFIX --enable-static --disable-shared
    /bin/make -j8 && /bin/make install

* Build ffmpeg:
  * Copy "ffmpeg-configure-mingw32.sh" to the ffmpeg source folder.
  * ./ffmpeg-configure-mingw32.sh && /bin/make -j8 && /bin/make install

* Build libao:
  * Copy "libao-configure-mingw32.sh" to the libao source folder.
  * ./libao-configure-mingw32.sh && /bin/make -j8 && /bin/make install
  * or use qmake

* Build zlib:
  * /bin/make -f./win32/makefile.gcc && /bin/make -f./win32/makefile.gcc install
* Build bzip2:
  * /bin/make -f./Makefile && /bin/make -f./Makefile install

* Build libOpenCC:
  * notice: do not use the cmake from msys2
  * cmake . -G "MSYS Makefiles" -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE=Release 
  * /bin/make -j8 
  or
  * cmake -H. -Bbuild -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=$PREFIX
  * cmake --build build --config Release --target install

* Build libeb:
  * ./configure --prefix=$PREFIX --enable-ebnet=no && /bin/make -j8 && /bin/make install
  * or use qmake

* Build libiconv:
  * ./configure --prefix=$PREFIX --enable-static=no
    /bin/make -j8 && /bin/make install

* Build libogg:
  * ./configure --prefix=$PREFIX --enable-shared=no && /bin/make -j8 && /bin/make install


* Build libvorbis:
  * ./configure --prefix=$PREFIX --enable-shared=no && /bin/make -j8 && /bin/make install

* Build lzo:
  * ./configure --prefix=$PREFIX --enable-shared=yes  && /bin/make -j8 && /bin/make install
  
* Build xz:
  * ./autogen.sh && ./configure --prefix=$PREFIX --enable-shared=yes && /bin/make -j8 && /bin/make install

* Build tiff:
  * ./configure --prefix=$PREFIX --enable-shared=yes  && /bin/make -j8 && /bin/make install
  
binaries (dlls):  ${PREFIX}/bin
C headers:        ${PREFIX}/include
Linker files:     ${PREFIX}/lib
