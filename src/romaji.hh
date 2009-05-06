/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ROMAJI_HH_INCLUDED__
#define __ROMAJI_HH_INCLUDED__

#include "transliteration.hh"
#include "config.hh"

/// Japanese romanization (Romaji) support.
namespace Romaji {

using std::vector;

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Romaji const & )
  throw( std::exception );

}

#endif
