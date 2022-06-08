/* This file is (c) 2010 Jennie Petoumenou <epetoumenou@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef GREEKTRANSLIT_HH
#define GREEKTRANSLIT_HH

#include "dict/dictionary.hh"

// Support for Greek transliteration
namespace GreekTranslit {

sptr< Dictionary::Class > makeDictionary() THROW_SPEC( std::exception );

}

#endif // GREEKTRANSLIT_HH
