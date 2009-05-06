/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __TRANSLITERATION_HH_INCLUDED__
#define __TRANSLITERATION_HH_INCLUDED__

#include "dictionary.hh"
#include <map>

namespace Transliteration {

using std::map;
using gd::wstring;
using std::string;
using std::vector;
  
class Table: public map< wstring, wstring >
{
  unsigned maxEntrySize;

public:

  Table(): maxEntrySize( 0 )
  {}

  unsigned getMaxEntrySize() const
  { return maxEntrySize; }
  
protected:

  /// Inserts new entry into index. from and to are UTF8-encoded strings.
  /// Also updates maxEntrySize.
  void ins( char const * from, char const * to );
};

/// This is a base dictionary class for simple transliteratons
class TransliterationDictionary: public Dictionary::Class
{
  string name;
  Table const & table;
  
public:

  TransliterationDictionary( string const & id, string const & name,
                             Table const & table );

  virtual string getName() throw();

  virtual map< Dictionary::Property, string > getProperties() throw();
  
  virtual unsigned long getArticleCount() throw();

  virtual unsigned long getWordCount() throw();
  
  virtual vector< wstring > getAlternateWritings( wstring const & )
    throw();

  virtual sptr< Dictionary::WordSearchRequest > findHeadwordsForSynonym( wstring const & )
    throw( std::exception );
  
  virtual sptr< Dictionary::WordSearchRequest > prefixMatch( wstring const &,
                                                             unsigned long ) throw( std::exception );
  
  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & )
    throw( std::exception );
};

}

#endif
