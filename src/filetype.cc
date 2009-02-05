/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
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

  while( beginPos < str.size() && isblank( str[ beginPos ] ) );

  size_t endPos = str.size();

  while( endPos && isblank( str[ endPos - 1 ] ) )
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
    endsWith( s, ".mp3" );
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

}
