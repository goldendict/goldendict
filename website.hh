/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WEBSITE_HH_INCLUDED__
#define __WEBSITE_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"

/// Support for any web sites via a templated url.
namespace WebSite {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries( Config::WebSites const & ,Config::WebTtss const &)
    throw( std::exception );

}

#endif
