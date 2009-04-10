/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __AUDIOLINK_HH_INCLUDED__
#define __AUDIOLINK_HH_INCLUDED__


#include <string>

/// Adds a piece of javascript to save the given audiolink to a special
/// javascript variable. Embed this into article's html to enable the
/// 'say sound' functionality.
/// The url should be escaped and surrounded by quotes.
std::string addAudioLink( std::string const & url );

#endif
