# use vcpkg to build the ffmpeg.
- folow the instructions https://trac.ffmpeg.org/wiki/CompilationGuide/vcpkg
- run command 
```
vcpkg.exe install ffmpeg[core,avcodec,avdevice,avfilter,avformat,speex,avresample,mp3lame,opus,sdl2,swresample,vorbis]:x64-windows-rel 
```
- copy dll and libs in vcpkg\installed\x64-windows-rel to goldendict's winlibs\lib\msvc

**pros**: can be compiled with speex.

# alternative Method :
just download the ffmpeg from official website: https://github.com/BtbN/FFmpeg-Builds/releases
can replace the dlls and libs in the winlibs\lib\msvc

**cons**: seems lacks libspeex or I just download the wrong package.

**pros**: easy to manage.


# I have tried the following methods no luck.
## - use the scripts provided by the following url

https://github.com/Microsoft/FFmpegInterop/issues/67

https://github.com/Microsoft/FFmpegInterop   


did not know how to link with libspeex. 

## - conan
  
  conan seems has not included libspeex option up to now.


# links worth checking:
https://stackoverflow.com/a/44556505/968188

# notes

winlib/scripts/ffmpeg-configure-mingw32.sh is the script provided by goldendict .
though I have compiled it wiht mingw32 ,I do not know how to use it to compile for visual studio .
