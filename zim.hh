#ifndef __ZIM_HH_INCLUDED__
#define __ZIM_HH_INCLUDED__

#ifdef MAKE_ZIM_SUPPORT

#include "dictionary.hh"

/// Support for the Zim dictionaries.
namespace Zim {

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

#endif
