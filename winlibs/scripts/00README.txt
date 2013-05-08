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

* First, build the libspeex as static library:
  * Copy "libspeex-configure-mingw32.sh" to the speex source folder.
  * ./libspeex-configure-mingw32.sh
  * make && make install

* Build ffmpeg:
  * Copy "ffmpeg-configure-mingw32.sh" to the ffmpeg source folder.
  * ./ffmpeg-configure-mingw32.sh
  * make && make install

* Build libao:
  * Copy "libao-configure-mingw32.sh" to the libao source folder.
  * ./libao-configure-mingw32.sh
  * make && make install

* Build zlib:
  * make -f./win32/makefile.gcc && make -f./win32/makefile.gcc install

binaries (dlls):  ${PREFIX}/bin
C headers:        ${PREFIX}/include
Linker files:     ${PREFIX}/lib
