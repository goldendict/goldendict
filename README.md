[![Codacy Badge](https://api.codacy.com/project/badge/Grade/e507f9bf83bd48f7a5b76f71dfe9f0dd)](https://app.codacy.com/gh/xiaoyifang/goldendict?utm_source=github.com&utm_medium=referral&utm_content=xiaoyifang/goldendict&utm_campaign=Badge_Grade_Settings)
[![Windows](https://github.com/xiaoyifang/goldendict/actions/workflows/windows.yml/badge.svg)](https://github.com/xiaoyifang/goldendict/actions/workflows/windows.yml) [![Ubuntu](https://github.com/xiaoyifang/goldendict/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/xiaoyifang/goldendict/actions/workflows/ubuntu.yml)
[![macos](https://github.com/xiaoyifang/goldendict/actions/workflows/macos.yml/badge.svg)](https://github.com/xiaoyifang/goldendict/actions/workflows/macos.yml)

## Introduction

<b>GoldenDict</b> is a feature-rich dictionary lookup program, supporting multiple dictionary formats (StarDict/Babylon/Lingvo/Dictd/AARD/MDict/SDict) and online dictionaries, featuring perfect article rendering with the complete markup, illustrations and other content retained, and allowing you to type in words without any accents or correct case.

## Requirements

This code has been run and tested on Windows 10/11, Ubuntu Linux, Mac OS X.

### External Deps

* Make, GCC, Git
* Qt framework. Minimum required version is 5.15 and support the latest QT version 6.2.4
* Qt Creator IDE is recommended for development
* Various libraries on Linux (png, zlib, etc)
* On Mac and Windows all the libraries are already included in the repository


#### Installing External Deps on Ubuntu Linux for Qt5

    sudo apt-get install git pkg-config build-essential qt5-qmake \
         libvorbis-dev zlib1g-dev libhunspell-dev x11proto-record-dev \
         qtdeclarative5-dev libxtst-dev liblzo2-dev libbz2-dev \
         libao-dev libavutil-dev libavformat-dev libtiff5-dev libeb16-dev \
         libqt5svg5-dev libqt5x11extras5-dev qttools5-dev \
         qttools5-dev-tools qtmultimedia5-dev libqt5multimedia5-plugins
         
### Fedora 35
```
sudo dnf install git pkg-config \
     libvorbis-devel zlib-devel hunspell-devel lzo-devel bzip2-devel \
     libao-devel ffmpeg-devel libtiff-devel eb-devel qt5-qtx11extras-devel libXtst-devel \
     libxkbcommon-devel 


```

## How to build

First, clone this repository, e.g.:

    git clone https://github.com/goldendict/goldendict.git


### Linux 
And then invoke `qmake-qt5` and `make`:

    cd goldendict && qmake-qt5 && make

### macOS
```
brew install qt # or use official offline installer
qmake CONFIG+=release   CONFIG+=zim_support   CONFIG+=chinese_conversion_support QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64"
make 
make install
```
### Windows

Alternatively, you might want to load `goldendict.pro` file from within Qt Creator, especially on Windows.


### Building with Chinese conversion support

To add Chinese conversion support you need at first install libopencc-dev package:

    sudo apt-get install libopencc-dev

Then pass `"CONFIG+=chinese_conversion_support"` to `qmake`

    qmake "CONFIG+=chinese_conversion_support"

### Building with Zim dictionaries support

To add Zim and Slob formats support you need at first install lzma-dev and zstd-dev packages:

    sudo apt-get install liblzma-dev libzstd-dev

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

### Building without internal audio players

If you have problem building with FFmpeg/libao (for example, Ubuntu older than 12.04), you can pass
`"CONFIG+=no_ffmpeg_player"` to `qmake` in order to disable FFmpeg+libao internal audio player back end:

    qmake "CONFIG+=no_ffmpeg_player"

If you have problem building with Qt5 Multimedia or experience GStreamer run-time errors (for example, Ubuntu 14.04), you can pass
`"CONFIG+=no_qtmultimedia_player"` to `qmake` in order to disable Qt Multimedia internal audio player back end:

    qmake "CONFIG+=no_qtmultimedia_player"

<b>NB:</b> All additional settings for `qmake` that you need must be combined in one `qmake` launch, for example:

    qmake "CONFIG+=zim_support" "CONFIG+=no_extra_tiff_handler" "CONFIG+=no_ffmpeg_player"


Then, invoke `make clean` before `make` because the setting change:

    make clean && make

### Building under Windows with MS Visual Studio

the source code has offered precompile x64 windows libs on winlibs/lib/msvc. you can build your own version either.

To build with Visual Studio.
check this [how to build with visual studio](howto/how%20to%20build%20and%20debug%20with%20VS2019.md)


## Installation

Installation is an optional step since the built binary can be used as-is without installation. But you can properly install via:

    make install

<b>NB:</b> Don't do that on Windows!

You can uninstall via:

    make uninstall

## License

This project is licensed under the <b>GNU GPLv3+</b> license, a copy of which can be found in the `LICENSE.txt` file.

## Support

Users looking for support should file an issue in the official [GoldenDict issue tracker](https://github.com/goldendict/goldendict/issues),
or even better: submit a [pull request](https://github.com/goldendict/goldendict/pulls) if you have a fix available.
General questions should be asked on the [official GoldenDict forum](http://goldendict.org/forum/).
