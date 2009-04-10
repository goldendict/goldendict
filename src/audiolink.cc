/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "audiolink.hh"

std::string addAudioLink( std::string const & url )
{
  return std::string( "<script language=\"JavaScript\">var gdAudioLink; "
                      "if ( !gdAudioLink ) gdAudioLink=" ) + url +
         ";" + "</script>";
}
