/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
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


/// This is a base dictionary class for simple transliteratons
class BaseTransliterationDictionary: public Dictionary::Class
{
  string name;

protected:
  bool caseSensitive;

public:

  BaseTransliterationDictionary( string const & id, string const & name,
                                 QIcon icon, bool caseSensitive = true );

  virtual string getName() noexcept;

  virtual map< Dictionary::Property, string > getProperties() noexcept;

  virtual unsigned long getArticleCount() noexcept;

  virtual unsigned long getWordCount() noexcept;

  virtual vector< wstring > getAlternateWritings( wstring const & )
    noexcept = 0;

  virtual sptr< Dictionary::WordSearchRequest > findHeadwordsForSynonym( wstring const & )
    ;

  virtual sptr< Dictionary::WordSearchRequest > prefixMatch( wstring const &,
                                                             unsigned long ) ;

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const &,
                                                      wstring const &, bool )
    ;
};


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


/// A base dictionary class for table based transliteratons
class TransliterationDictionary: public BaseTransliterationDictionary
{
  Table const & table;

public:

  TransliterationDictionary( string const & id, string const & name,
                             QIcon icon,
                             Table const & table,
                             bool caseSensitive = true );

  virtual vector< wstring > getAlternateWritings( wstring const & )
    noexcept;
};

}

#endif
