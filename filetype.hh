/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FILETYPE_HH_INCLUDED__
#define __FILETYPE_HH_INCLUDED__

#include <string>

/// Utilities to guess file types based on their names.
namespace Filetype {

using std::string;

/// Returns true if the name resembles the one of a sound file (i.e. ends
/// with .wav, .ogg and such).
bool isNameOfSound( string const & );
/// Returns true if the name resembles the one of a video file (i.e. ends
/// with .mpg, .ogv and such).
bool isNameOfVideo( string const & );
/// Returns true if the name resembles the one of a picture file (i.e. ends
/// with .jpg, .png and such).
bool isNameOfPicture( string const & );
/// Returns true if the name resembles the one of a .tiff file (i.e. ends
/// with .tif or tiff). We have this one separately since we need to reconvert
/// TIFF files as WebKit doesn't seem to support them.
bool isNameOfTiff( string const & );
/// Returns true if the name resembles the one of a .css file
bool isNameOfCSS( string const & );
/// Returns true if the name resembles the one of a .svg file
bool isNameOfSvg( string const & name );

}

#endif

