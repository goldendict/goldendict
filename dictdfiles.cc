/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dictdfiles.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "dictzip.h"
#include "htmlescape.hh"
#include "fsencoding.hh"
#include "langcoder.hh"
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <wctype.h>
#include <stdlib.h>
#include "dprintf.hh"

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

namespace DictdFiles {

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using gd::wstring;
using std::vector;
using std::list;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX( exFailedToReadLineFromIndex, "Failed to read line from index file", Dictionary::Ex )
DEF_EX( exMalformedIndexFileLine, "Malformed index file line encountered", Dictionary::Ex )
DEF_EX( exInvalidBase64, "Invalid base64 sequence encountered", Dictionary::Ex )

enum
{
  Signature = 0x58444344, // DCDX on little-endian, XDCD on big-endian
  CurrentFormatVersion = 3 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, DCDX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t wordCount; // Total number of words
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t langFrom;  // Source language
  uint32_t langTo;    // Target language
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

class DictdDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx, indexFile; // The later is .index file
  IdxHeader idxHeader;
  dictData * dz;

public:

  DictdDictionary( string const & id, string const & indexFile,
                   vector< string > const & dictionaryFiles );

  ~DictdDictionary();

  virtual string getName() throw();

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.wordCount; }

  virtual unsigned long getWordCount() throw()
  { return idxHeader.wordCount; }

  virtual void loadIcon() throw();

  inline virtual quint32 getLangFrom() const
  { return idxHeader.langFrom; }

  inline virtual quint32 getLangTo() const
  { return idxHeader.langTo; }

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );
};

DictdDictionary::DictdDictionary( string const & id,
                                  string const & indexFile,
                                  vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  indexFile( dictionaryFiles[ 0 ], "rb" ),
  idxHeader( idx.read< IdxHeader >() )
{
  // Open the .dict file

  dz = dict_data_open( dictionaryFiles[ 1 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 1 ] );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );
}

DictdDictionary::~DictdDictionary()
{
  if ( dz )
    dict_data_close( dz );
}

string nameFromFileName( string const & indexFileName )
{
  if ( indexFileName.empty() )
    return string();

  char const * sep = strrchr( indexFileName.c_str(), FsEncoding::separator() );

  if ( !sep )
    sep = indexFileName.c_str();

  char const * dot = strrchr( sep, '.' );

  if ( !dot )
    dot = indexFileName.c_str() + indexFileName.size();

  return Utf8::encode( FsEncoding::decode( string( sep + 1, dot - sep - 1 ) ) );
}

void DictdDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 5 );

  if( !loadIconFromFile( fileName ) )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_dictd.png");
  }

  dictionaryIconLoaded = true;
}

string DictdDictionary::getName() throw()
{
  return nameFromFileName( getDictionaryFilenames()[ 0 ] );
}

uint32_t decodeBase64( string const & str )
{
  static char const digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  uint32_t number = 0;

  for( char const * next = str.c_str(); *next; ++next )
  {
    char const * d = strchr( digits, *next );

    if ( !d )
      throw exInvalidBase64();

    number = number * 64 + ( d - digits );
  }

  return number;
}

sptr< Dictionary::DataRequest > DictdDictionary::getArticle( wstring const & word,
                                                             vector< wstring > const & alts,
                                                             wstring const & )
  throw( std::exception )
{
  try
  {
    vector< WordArticleLink > chain = findArticles( word );

    for( unsigned x = 0; x < alts.size(); ++x )
    {
      /// Make an additional query for each alt

      vector< WordArticleLink > altChain = findArticles( alts[ x ] );

      chain.insert( chain.end(), altChain.begin(), altChain.end() );
    }

    multimap< wstring, string > mainArticles, alternateArticles;

    set< uint32_t > articlesIncluded; // Some synonyms make it that the articles
                                      // appear several times. We combat this
                                      // by only allowing them to appear once.

    wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

    char buf[ 16384 ];

    for( unsigned x = 0; x < chain.size(); ++x )
    {
      if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
        continue; // We already have this article in the body.

      // Now load that article

      indexFile.seek( chain[ x ].articleOffset );

      if ( !indexFile.gets( buf, sizeof( buf ), true ) )
        throw exFailedToReadLineFromIndex();

      char * tab1 = strchr( buf, '\t' );

      if ( !tab1 )
        throw exMalformedIndexFileLine();

      char * tab2 = strchr( tab1 + 1, '\t' );

      if ( !tab2 )
        throw exMalformedIndexFileLine();

      // After tab1 should be article offset, after tab2 -- article size

      uint32_t articleOffset = decodeBase64( string( tab1 + 1, tab2 - tab1 - 1 ) );
      uint32_t articleSize = decodeBase64( tab2 + 1 );

      char * articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );

      if ( !articleBody )
        throw exCantReadFile( getDictionaryFilenames()[ 1 ] );

      //sprintf( buf, "Offset: %u, Size: %u\n", articleOffset, articleSize );

      string articleText = string( "<div class=\"dictd_article\">" ) +
        Html::preformat( articleBody ) + "</div>";

      free( articleBody );

      // Ok. Now, does it go to main articles, or to alternate ones? We list
      // main ones first, and alternates after.

      // We do the case-folded comparison here.

      wstring headwordStripped =
        Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) );

      multimap< wstring, string > & mapToUse =
        ( wordCaseFolded == headwordStripped ) ?
          mainArticles : alternateArticles;

      mapToUse.insert( pair< wstring, string >(
        Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) ),
        articleText ) );

      articlesIncluded.insert( chain[ x ].articleOffset );
    }

    if ( mainArticles.empty() && alternateArticles.empty() )
      return new Dictionary::DataRequestInstant( false );

    string result;

    multimap< wstring, string >::const_iterator i;

    for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
      result += i->second;

    for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
      result += i->second;

    sptr< Dictionary::DataRequestInstant > ret =
      new Dictionary::DataRequestInstant( true );

    ret->getData().resize( result.size() );

    memcpy( &(ret->getData().front()), result.data(), result.size() );

    return ret;
  }
  catch( std::exception & e )
  {
    return new Dictionary::DataRequestInstant( QString( e.what() ) );
  }
}

} // anonymous namespace

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      vector< string > const & fileNames,
                                      string const & indicesDir,
                                      Dictionary::Initializing & initializing )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
    // Only allow .index suffixes

    if ( i->size() < 6 ||
         strcasecmp( i->c_str() + ( i->size() - 6 ), ".index" ) != 0 )
      continue;

    try
    {
      vector< string > dictFiles( 1, *i );

      // Check if there is an 'abrv' file present
      string baseName( *i, 0, i->size() - 5 );

      dictFiles.push_back( string() );

      if ( !File::tryPossibleName( baseName + "dict", dictFiles[ 1 ] ) &&
           !File::tryPossibleName( baseName + "dict.dz", dictFiles[ 1 ] ) )
      {
        // No corresponding .dict file, skipping
        continue;
      }

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        // Building the index
        initializing.indexingDictionary( nameFromFileName( dictFiles[ 0 ] ) );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        IndexedWords indexedWords;

        File::Class indexFile( dictFiles[ 0 ], "rb" );

        // Read words from index until none's left.

        char buf[ 16384 ];

        do
        {
          uint32_t curOffset = indexFile.tell();

          if ( !indexFile.gets( buf, sizeof( buf ), true ) )
            break;

          // Check that there are exactly two tabs in the record.

          char * tab = strchr( buf, '\t' );

          if ( !tab || ! ( tab = strchr( tab + 1, '\t' ) ) || strchr( tab + 1, '\t' ) )
          {
            DPRINTF( "Warning: incorrect amount of tabs in a line, skipping: %s\n", buf );
            continue;
          }

          indexedWords.addWord( Utf8::decode( string( buf, strchr( buf, '\t' ) - buf ) ), curOffset );

          ++idxHeader.wordCount;

        } while( !indexFile.eof() );

        // Build index

        IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

        idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
        idxHeader.indexRootOffset = idxInfo.rootOffset;

        // That concludes it. Update the header.

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;

        // read languages
        QPair<quint32,quint32> langs =
            LangCoder::findIdsForFilename( QString::fromStdString( dictFiles[ 0 ] ) );

        // if no languages found, try dictionary's name
        if ( langs.first == 0 || langs.second == 0 )
        {
          langs =
            LangCoder::findIdsForFilename( QString::fromStdString( nameFromFileName( dictFiles[ 0 ] ) ) );
        }

        idxHeader.langFrom = langs.first;
        idxHeader.langTo = langs.second;

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );
      }

      dictionaries.push_back( new DictdDictionary( dictId,
                                                   indexFile,
                                                   dictFiles ) );
    }
    catch( std::exception & e )
    {
      FDPRINTF( stderr, "Dictd dictionary reading failed: %s, error: %s\n",
        i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
