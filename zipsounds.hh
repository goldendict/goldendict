/* This file is part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ZIPSOUNDS_HH
#define ZIPSOUNDS_HH

#include "dictionary.hh"

/// Support for compresses audio packs (.zips).
namespace ZipSounds {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
  ;

}

#endif // ZIPSOUNDS_HH
