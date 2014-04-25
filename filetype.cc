/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "filetype.hh"
#include <ctype.h>

namespace Filetype {

namespace {


/// Removes any trailing or leading spaces and lowercases the string.
/// The lowercasing is done simplistically, but it is enough for file
/// extensions.
string simplifyString( string const & str )
{
  string result;

  size_t beginPos = 0;

  while( beginPos < str.size() && isspace( str[ beginPos ] ) )
    ++beginPos;

  size_t endPos = str.size();

  while( endPos && isspace( str[ endPos - 1 ] ) )
    --endPos;

  result.reserve( endPos - beginPos );

  while( beginPos < endPos )
    result.push_back( tolower( str[ beginPos++ ] ) );

  return result;
}

/// Checks if the given string ends with the given substring
bool endsWith( string const & str, string const & tail )
{
  return str.size() >= tail.size() &&
    str.compare( str.size() - tail.size(), tail.size(), tail ) == 0;
}

}

bool isNameOfSound( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".wav" ) ||
    endsWith( s, ".au" ) ||
    endsWith( s, ".voc" ) ||
    endsWith( s, ".ogg" ) ||
    endsWith( s, ".mp3" ) ||
    endsWith( s, ".m4a") ||
    endsWith( s, ".aac" ) ||
    endsWith( s, ".flac" ) ||
    endsWith( s, ".mid" ) ||
    endsWith( s, ".kar" ) ||
    endsWith( s, ".mpc" ) ||
    endsWith( s, ".wma" ) ||
    endsWith( s, ".wv" ) ||
    endsWith( s, ".ape" ) ||
    endsWith( s, ".spx" );
}

bool isNameOfVideo( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".mpg" ) ||
    endsWith( s, ".mpeg" )||
    endsWith( s, ".mpe" ) ||
    endsWith( s, ".ogv" ) ||
    endsWith( s, ".ogm" ) ||
    endsWith( s, ".avi" ) ||
    endsWith( s, ".m4v" ) ||
    endsWith( s, ".mp4" ) ||
    endsWith( s, ".mkv" ) ||
    endsWith( s, ".wmv" ) ||
    endsWith( s, ".sfw" ) ||
    endsWith( s, ".flv" ) ||
    endsWith( s, ".divx" ) ||
    endsWith( s, ".3gp" ) ||
    endsWith( s, ".mov" );
}

bool isNameOfPicture( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".jpg" ) ||
    endsWith( s, ".jpeg" ) ||
    endsWith( s, ".jpe" ) ||
    endsWith( s, ".png" ) ||
    endsWith( s, ".gif" ) ||
    endsWith( s, ".bmp" ) ||
    endsWith( s, ".tif" ) ||
    endsWith( s, ".tiff" ) ||
    endsWith( s, ".tga" ) ||
    endsWith( s, ".pcx" ) ||
    endsWith( s, ".ico" ) ||
    endsWith( s, ".svg" );
}

bool isNameOfTiff( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".tif" ) ||
    endsWith( s, ".tiff" );
}

bool isNameOfCSS( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".css" );
}

bool isNameOfSvg( string const & name )
{
  string s = simplifyString( name );

  return
    endsWith( s, ".svg" );
}

}
