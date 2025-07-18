/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __XDXF2HTML_HH_INCLUDED__
#define __XDXF2HTML_HH_INCLUDED__

#include <string>
#include <map>
#include "dictionary.hh"
#include "indexedzip.hh"

/// Xdxf is an xml file format. Since we display html, we'd like to be able
/// to convert articles with such a markup to an html.
namespace Xdxf2Html {

enum DICT_TYPE { STARDICT, XDXF };

using std::string;
using std::map;

/// Converts the given xdxf markup to an html one. This is currently used
/// for Stardict's 'x' records.
string convert( string const &, DICT_TYPE type, map < string, string > const * pAbrv,
                Dictionary::Class *dictPtr, IndexedZip * resourceZip, bool isLogicalFormat = false,
                unsigned revisionNumber = 0, QString * headword = 0 );

}

#endif
