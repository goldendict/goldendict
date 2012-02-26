/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "audiolink.hh"

std::string addAudioLink( std::string const & url,
                          std::string const & dictionaryId )
{
    return std::string( "<script language=\"JavaScript\">" +
                        makeAudioLinkScript( url, dictionaryId ) +
                        "</script>" );
}

std::string makeAudioLinkScript( std::string const & url,
                                 std::string const & dictionaryId )
{
  return std::string( "var gdAudioLink; "
                      "if ( !gdAudioLink ) gdAudioLink=" ) + url +
         "; if ( typeof gdActivateAudioLink_" + dictionaryId + " != 'function' ) {"
         "eval( 'function gdActivateAudioLink_" + dictionaryId + "() {"
         "gdAudioLink=" + url + "; }' ); }";
}
