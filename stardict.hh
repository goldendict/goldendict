/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __STARTDICT_HH_INCLUDED__
#define __STARTDICT_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the Stardict (ifo+dict+idx+syn) dictionaries.
namespace Stardict {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing &,
                                      unsigned maxHeadwordsToExpand )
  ;

}

#endif
