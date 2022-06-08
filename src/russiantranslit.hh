/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __RUSSIANTRANSLIT_HH_INCLUDED__
#define __RUSSIANTRANSLIT_HH_INCLUDED__

#include "dict/dictionary.hh"

// Support for Russian transliteration
namespace RussianTranslit {

sptr< Dictionary::Class > makeDictionary() THROW_SPEC( std::exception );

}

#endif

