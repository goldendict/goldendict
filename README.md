## Introduction

<b>GoldenDict</b> is a feature-rich dictionary lookup program, supporting multiple dictionary formats (StarDict/Babylon/Lingvo/Dictd/AARD/MDict/SDict) and online dictionaries, featuring perfect article rendering with the complete markup, illustrations and other content retained, and allowing you to type in words without any accents or correct case.

## Requirements

This code has been run and tested on Windows XP/Vista/7, Ubuntu Linux, Mac OS X.

### External Deps

* Make, GCC, Git
* Qt framework. Minumal required version is 4.5. But Qt 4.7 or 4.8 is recommended.
* Qt Creator IDE is recommended for development
* Various libraries on Linux (png, zlib, etc)
* On Mac and Windows all the libraries are already included in the repository

### Installing External Deps on Ubuntu Linux

    sudo apt-get install git pkg-config build-essential qt4-qmake \
         libvorbis-dev zlib1g-dev libhunspell-dev x11proto-record-dev \
         libqt4-dev libqtwebkit-dev libxtst-dev liblzo2-dev libbz2-dev \
         libao-dev libavutil-dev libavformat-dev

## How to build

First, clone this repository, e.g.:

    git clone git://github.com/goldendict/goldendict.git

And then invoke `qmake-qt4` and `make`:

    cd goldendict && qmake-qt4 && make

In case when qmake-qt4 does not exist, try using `qmake` but make sure it is indeed from the Qt 4 installation.
Alternatively, you might want to load `goldendict.pro` file from within Qt Creator, especially on Windows.

### Building without internal audio player

If you have problem building with FFmpeg/libao (for example, Ubuntu older than 12.04), you can pass
`"DISABLE_INTERNAL_PLAYER=1"` to `qmake` in order to disable internal audio player completely:

    qmake "DISABLE_INTERNAL_PLAYER=1"

Then, invoke `make clean` before `make` because the setting change:

    make clean && make

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
