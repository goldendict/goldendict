## Introduction

<b>GoldenDict</b> is a feature-rich dictionary lookup program, supporting multiple dictionary formats (StarDict/Babylon/Lingvo/Dictd/AARD/MDict/SDict) and online dictionaries, featuring perfect article rendering with the complete markup, illustrations and other content retained, and allowing you to type in words without any accents or correct case.

## Requirements

This code has been run and tested on Windows XP/Vista/7, Ubuntu Linux, Mac OS X.

### External Deps

* Make, GCC, Git
* Qt framework. Minimum required version is 4.6 for Windows, 4.5 for all other platforms. But Qt 4.7 or 4.8 is recommended.
* If you want to use Qt 5.x then use branch qt4x5
* Qt Creator IDE is recommended for development
* Various libraries on Linux (png, zlib, etc)
* On Mac and Windows all the libraries are already included in the repository

### Installing External Deps on Ubuntu Linux

    sudo apt-get install git pkg-config build-essential qt4-qmake \
         libvorbis-dev zlib1g-dev libhunspell-dev x11proto-record-dev \
         libqt4-dev libqtwebkit-dev libxtst-dev liblzo2-dev libbz2-dev \
         libao-dev libavutil-dev libavformat-dev libtiff5-dev libeb16-dev

## How to build

First, clone this repository, e.g.:

    git clone git://github.com/goldendict/goldendict.git

And then invoke `qmake-qt4` and `make`:

    cd goldendict && qmake-qt4 && make

In case when qmake-qt4 does not exist, try using `qmake` but make sure it is indeed from the Qt 4 installation.
Alternatively, you might want to load `goldendict.pro` file from within Qt Creator, especially on Windows.

### Building with Chinese conversion support

To add Chinese conversion support you need at first install libopencc-dev package:

    sudo apt-get install libopencc-dev

Then pass `"CONFIG+=chinese_conversion_support"` to `qmake`

    qmake "CONFIG+=chinese_conversion_support"

### Building with Zim dictionaries support

To add Zim and Slob formats support you need at first install lzma-dev package:

    sudo apt-get install liblzma-dev

Then pass `"CONFIG+=zim_support"` to `qmake`

    qmake "CONFIG+=zim_support"

### Building without extra tiff handler

If you have problem building with libtiff5-dev package, you can pass
`"CONFIG+=no_extra_tiff_handler"` to `qmake` in order to disable extra tiff support
(without such extra support some b/w tiff images will not be displayed):

    qmake "CONFIG+=no_extra_tiff_handler"

### Building without Epwing format support

If you have problem building with libeb-dev package, you can pass
`"CONFIG+=no_epwing_support"` to `qmake` in order to disable Epwing format support

    qmake "CONFIG+=no_epwing_support"

### Building without internal audio player

If you have problem building with FFmpeg/libao (for example, Ubuntu older than 12.04), you can pass
`"DISABLE_INTERNAL_PLAYER=1"` to `qmake` in order to disable internal audio player completely:

    qmake "DISABLE_INTERNAL_PLAYER=1"

<b>NB:</b> All additional settings for `qmake` that you need must be combined in one `qmake` launch, for example:

    qmake "CONFIG+=zim_support" "CONFIG+=no_extra_tiff_handler" "DISABLE_INTERNAL_PLAYER=1"


Then, invoke `make clean` before `make` because the setting change:

    make clean && make

### Building under Windows with MS Visual Studio

To build GoldenDict with Visual Studio take one of next library packs and unpack it to `"winlibs/lib/msvc"` folder in GoldenDict sources folder.  
[GoldenDict_libs_VS2013_x86_v2.7z](http://www.mediafire.com/download/lu73ykqmjf7bqjs/GoldenDict_libs_VS2013_x86_v2.7z) - for MS Visual Studio 2013, 32 bit  
[GoldenDict_libs_VS2013_x64_v2.7z](http://www.mediafire.com/download/wcwx8041ixixq3w/GoldenDict_libs_VS2013_x64_v2.7z) - for MS Visual Studio 2013, 64 bit  
[GoldenDict_libs_VS2015_x86_v2.7z](http://www.mediafire.com/download/qu7xh7v6ar0x2oc/GoldenDict_libs_VS2015_x86_v2.7z) - for MS Visual Studio 2015, 32 bit  
[GoldenDict_libs_VS2015_x64_v2.7z](http://www.mediafire.com/download/3jk6j55j6l9w902/GoldenDict_libs_VS2015_x64_v2.7z) - for MS Visual Studio 2015, 64 bit  

To create project files for Visual Studio you can pass `"-tp vc"` option to `qmake`.

Note: In Qt 5.6.0 and later the `Webkit` module was removed from official release builds. You should to build it from sources to compile GoldenDict.


## Installation

Installation is an optional step since the built binary can be used as-is without installation. But you can properly install via:

    make install

<b>NB:</b> Don't do that on Windows!

## License

This project is licensed under the <b>GNU GPLv3+</b> license, a copy of which can be found in the `LICENSE.txt` file.

## Support

Users looking for support should file an issue in the official [GoldenDict issue tracker](https://github.com/goldendict/goldendict/issues),
or even better: submit a [pull request](https://github.com/goldendict/goldendict/pulls) if you have a fix available.
General questions should be asked on the [official GoldenDict forum](http://goldendict.org/forum/).
