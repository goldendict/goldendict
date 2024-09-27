/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "audiolink.hh"

std::string addAudioLink( std::string const & url)
{
    return std::string( "<script>" +
                        makeAudioLinkScript( url ) +
                        "</script>" );
}

std::string makeAudioLinkScript( std::string const & url)
{
  /// Convert "'" to "\'" - this char broke autoplay of audiolinks

  std::string ref;
  bool escaped = false;
  for( unsigned x = 0; x < url.size(); x++ )
  {
    char ch = url[ x ];
    if( escaped )
    {
      ref += ch;
      escaped = false;
      continue;
    }
    if( ch == '\'' )
      ref += '\\';
    ref += ch;
    escaped = ( ch == '\\' );
  }

  // Assign the first audio link of each article to gdJustLoadedAudioLink JavaScript variable.
  // Once an article finishes loading, the value of gdJustLoadedAudioLink is stored away
  // and the variable is set to null, before the next article starts loading.
  return "gdJustLoadedAudioLink = gdJustLoadedAudioLink || " + ref + ';';
}
