/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "transliteration.hh"
#include "utf8.hh"
#include "folding.hh"
#include "dprintf.hh"

namespace Transliteration {

using gd::wchar;

void Table::ins( char const * from, char const * to )
{
  wstring fr = Utf8::decode( std::string( from ) );

  if ( fr.size() > maxEntrySize )
    maxEntrySize = fr.size();
  
  insert( std::pair< wstring, wstring >( fr,
                                         Utf8::decode( std::string( to ) ) ) );
}
  
TransliterationDictionary::TransliterationDictionary( string const & id,
                                                      string const & name_,
                                                      QIcon icon_,
                                                      Table const & table_,
                                                      bool caseSensitive_ ):
  Dictionary::Class( id, vector< string >() ),
  name( name_ ), table( table_ ),
  caseSensitive( caseSensitive_ )
{
  dictionaryIcon = dictionaryNativeIcon = icon_;
  dictionaryIconLoaded = true;
}

string TransliterationDictionary::getName() throw()
{ return name; }

map< Dictionary::Property, string > TransliterationDictionary::getProperties() throw()
{ return map< Dictionary::Property, string >(); }

unsigned long TransliterationDictionary::getArticleCount() throw()
{ return 0; }

unsigned long TransliterationDictionary::getWordCount() throw()
{ return 0; }

sptr< Dictionary::WordSearchRequest > TransliterationDictionary::prefixMatch( wstring const &,
                                                           unsigned long ) throw( std::exception )
{ return new Dictionary::WordSearchRequestInstant(); }

sptr< Dictionary::DataRequest > TransliterationDictionary::getArticle( wstring const &,
                                                                       vector< wstring > const &,
                                                                       wstring const & )
  throw( std::exception )
{ return new Dictionary::DataRequestInstant( false ); }


vector< wstring > TransliterationDictionary::getAlternateWritings( wstring const & str )
  throw()
{
  vector< wstring > results;

  wstring result, folded;
  wstring const * target;

  if ( caseSensitive )
  {
    // Don't do any transform -- the transliteration is case-sensitive
    target = &str;
  }
  else
  {
    folded = Folding::applySimpleCaseOnly( str );
    target = &folded;
  }

  wchar const * ptr = target->c_str();
  size_t left = target->size();

  Table::const_iterator i;
  
  while( left )
  {
    unsigned x;
    
    for( x = table.getMaxEntrySize(); x >= 1; --x )
    {
      if ( left >= x )
      {
        i = table.find( wstring( ptr, x ) );
  
        if ( i != table.end() )
        {
          result.append( i->second );
          ptr += x;
          left -= x;
          break;
        }
      }
    }

    if ( !x )
    {
      // No matches -- add this char as it is
      result.push_back( *ptr++ );
      --left;
    }
  }
  
  if ( result != *target )
    results.push_back( result );

  return results;
}

sptr< Dictionary::WordSearchRequest > TransliterationDictionary::findHeadwordsForSynonym( wstring const & str )
  throw( std::exception )
{
  sptr< Dictionary::WordSearchRequestInstant > result = new Dictionary::WordSearchRequestInstant();

  vector< wstring > alts = getAlternateWritings( str );

  DPRINTF( "alts = %u\n", (unsigned) alts.size() );
  
  for( unsigned x = 0; x < alts.size(); ++x )
    result->getMatches().push_back( alts[ x ] );

  return result;
}

}
