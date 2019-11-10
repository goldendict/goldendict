/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "transliteration.hh"
#include "utf8.hh"
#include "folding.hh"
#include "gddebug.hh"

namespace Transliteration {

using gd::wchar;

BaseTransliterationDictionary::BaseTransliterationDictionary( string const & id,
                                                              string const & name_,
                                                              QIcon icon_,
                                                              bool caseSensitive_ ):
  Dictionary::Class( id, vector< string >() ),
  caseSensitive( caseSensitive_ )
{
  setDictionaryName(name_);
  dictionaryIcon = dictionaryNativeIcon = icon_;
  dictionaryIconLoaded = true;
}

sptr< Dictionary::WordSearchRequest > BaseTransliterationDictionary::prefixMatch( wstring const &,
                                                                                  unsigned long ) THROW_SPEC( std::exception )
{ return sptr< Dictionary::WordSearchRequest >(new Dictionary::WordSearchRequestInstant()); }

sptr< Dictionary::DataRequest > BaseTransliterationDictionary::getArticle( wstring const &,
                                                                           vector< wstring > const &,
                                                                           wstring const &, bool )
  THROW_SPEC( std::exception )
{ return sptr< Dictionary::DataRequest >(new Dictionary::DataRequestInstant( false )); }

sptr< Dictionary::WordSearchRequest > BaseTransliterationDictionary::findHeadwordsForSynonym( wstring const & str )
  THROW_SPEC( std::exception )
{
  sptr< Dictionary::WordSearchRequestInstant > result(new Dictionary::WordSearchRequestInstant());

  vector< wstring > alts = getAlternateWritings( str );

  GD_DPRINTF( "alts = %u\n", (unsigned) alts.size() );

  for( unsigned x = 0; x < alts.size(); ++x )
    result->getMatches().push_back( alts[ x ] );

  return result;
}


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
  BaseTransliterationDictionary(id, name_, icon_, caseSensitive_),
  table( table_ )
{
}

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

}
