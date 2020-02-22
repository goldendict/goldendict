#ifndef __ZIM_HH_INCLUDED__
#define __ZIM_HH_INCLUDED__

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
  THROW_SPEC( std::exception );

}

#endif
