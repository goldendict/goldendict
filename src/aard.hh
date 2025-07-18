/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __AARD_HH_INCLUDED__
#define __AARD_HH_INCLUDED__

#include "dictionary.hh"

/// Support for the aard dictionaries.
namespace Aard {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing &,
                                      unsigned maxHeadwordsToExpand )
  THROW_SPEC( std::exception );

}

#endif
