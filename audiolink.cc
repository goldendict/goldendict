/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "audiolink.hh"

std::string addAudioLink( std::string const & url,
                          std::string const & dictionaryId )
{
  return std::string( "<script language=\"JavaScript\">var gdAudioLink; "
                      "if ( !gdAudioLink ) gdAudioLink=" ) + url +
         "; if ( typeof gdActivateAudioLink_" + dictionaryId + " != 'function' ) {"
         "eval( 'function gdActivateAudioLink_" + dictionaryId + "() {"
         "gdAudioLink=" + url + "; }' ); }"
         "</script>";
}
