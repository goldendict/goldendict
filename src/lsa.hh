/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __L9SA_HH_INCLUDED__
#define __L9SA_HH_INCLUDED__

#include "dictionary.hh"

/// Support for LSA (.dat, .lsa) audio archives of the Lingo program.
namespace Lsa {

using std::vector;
using std::string;

class Format: public Dictionary::Format
{
public:

  virtual vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & )
    throw( std::exception );
};

}

#endif
