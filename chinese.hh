/* This file is (c) 2015 Zhe Wang <0x1997@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CHINESE_HH_INCLUDED__
#define __CHINESE_HH_INCLUDED__

#include <map>
#include "config.hh"
#include "dictionary.hh"

/// Chinese character conversion support.
namespace Chinese {

std::vector< sptr< Dictionary::Class > > makeDictionaries( Config::Chinese const & )
  ;

}

#endif
