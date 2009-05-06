/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "transliteration.hh"
#include "utf8.hh"
#include "folding.hh"

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
                                                      Table const & table_ ):
  Dictionary::Class( id, vector< string >() ),
  name( name_ ), table( table_ )
{}

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
                                                    vector< wstring > const & )
  throw( std::exception )
{ return new Dictionary::DataRequestInstant( false ); }


vector< wstring > TransliterationDictionary::getAlternateWritings( wstring const & str )
  throw()
{
  vector< wstring > results;
  
  wstring folded = Folding::apply( str );

  if ( folded.empty() )
    return results;

  wstring result;

  wchar const * ptr = folded.c_str();

  size_t left = folded.size();

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
      // No matches -- skip one char
      --left;
      ++ptr;
    }
  }
  
  if ( result.size() )
    results.push_back( result );

  return results;
}

sptr< Dictionary::WordSearchRequest > TransliterationDictionary::findHeadwordsForSynonym( wstring const & str )
  throw( std::exception )
{
  sptr< Dictionary::WordSearchRequestInstant > result = new Dictionary::WordSearchRequestInstant();

  vector< wstring > alts = getAlternateWritings( str );

  printf( "alts = %u\n", alts.size() );
  
  for( unsigned x = 0; x < alts.size(); ++x )
    result->getMatches().push_back( alts[ x ] );

  return result;
}

}
