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
#include "gddebug.hh"
#include "ftshelpers.hh"

#include <QDebug>

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
DEF_EX_STR( exDictzipError, "DICTZIP error", Dictionary::Ex )

enum
{
  Signature = 0x58444344, // DCDX on little-endian, XDCD on big-endian
  CurrentFormatVersion = 5 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, DCDX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t wordCount; // Total number of words
  uint32_t articleCount; // Total number of articles
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
  string dictionaryName;
  Mutex indexFileMutex, dzMutex;

public:

  DictdDictionary( string const & id, string const & indexFile,
                   vector< string > const & dictionaryFiles );

  ~DictdDictionary();

  virtual string getName() throw()
  { return dictionaryName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.articleCount; }

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

  virtual QString const& getDescription();

  virtual sptr< Dictionary::DataRequest > getSearchResults( QString const & searchString,
                                                            int searchMode, bool matchCase,
                                                            int distanceBetweenWords,
                                                            int maxResults );
  void getArticleText( uint32_t articleAddress, QString & headword, QString & text );

  virtual void makeFTSIndex(QAtomicInt & isCancelled, bool firstIteration );

  virtual void setFTSParameters( Config::FullTextSearch const & fts )
  {
    can_FTS = fts.enabled
              && !fts.disabledTypes.contains( "DICTD", Qt::CaseInsensitive )
              && ( fts.maxDictionarySize == 0 || getArticleCount() <= fts.maxDictionarySize );
  }
};

DictdDictionary::DictdDictionary( string const & id,
                                  string const & indexFile,
                                  vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  indexFile( dictionaryFiles[ 0 ], "rb" ),
  idxHeader( idx.read< IdxHeader >() )
{

  // Read the dictionary name
  idx.seek( sizeof( idxHeader ) );

  vector< char > dName( idx.read< uint32_t >() );
  idx.read( &dName.front(), dName.size() );
  dictionaryName = string( &dName.front(), dName.size() );

  // Open the .dict file

  DZ_ERRORS error;
  dz = dict_data_open( dictionaryFiles[ 1 ].c_str(), &error, 0 );

  if ( !dz )
    throw exDictzipError( string( dz_error_str( error ) )
                          + "(" + getDictionaryFilenames()[ 1 ] + ")" );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );

  // Full-text search parameters

  can_FTS = true;

  ftsIdxName = indexFile + "_FTS";

  if( !Dictionary::needToRebuildIndex( dictionaryFiles, ftsIdxName )
      && !FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) )
    FTS_index_completed.ref();
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

      {
        Mutex::Lock _( indexFileMutex );
        indexFile.seek( chain[ x ].articleOffset );

        if ( !indexFile.gets( buf, sizeof( buf ), true ) )
          throw exFailedToReadLineFromIndex();
      }

      char * tab1 = strchr( buf, '\t' );

      if ( !tab1 )
        throw exMalformedIndexFileLine();

      char * tab2 = strchr( tab1 + 1, '\t' );

      if ( !tab2 )
        throw exMalformedIndexFileLine();

      // After tab1 should be article offset, after tab2 -- article size

      uint32_t articleOffset = decodeBase64( string( tab1 + 1, tab2 - tab1 - 1 ) );

      char * tab3 = strchr( tab2 + 1, '\t');

      uint32_t articleSize;
      if ( tab3 )
      {
         articleSize = decodeBase64( string( tab2 + 1, tab3 - tab2 - 1 ) );
      }
      else
      {
        articleSize = decodeBase64( tab2 + 1 );
      }

      string articleText;

      char * articleBody;
      {
        Mutex::Lock _( dzMutex );
        articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
      }

      if ( !articleBody )
      {
        articleText = string( "<div class=\"dictd_article\">DICTZIP error: " )
                      + dict_error_str( dz ) + "</div>";
      }
      else
      {
        static QRegExp phonetic( "\\\\([^\\\\]+)\\\\", Qt::CaseInsensitive ); // phonetics: \stuff\ ...
        static QRegExp refs( "\\{([^\\{\\}]+)\\}", Qt::CaseInsensitive );     // links: {stuff}

        articleText = string( "<div class=\"dictd_article\"" );
        if( isToLanguageRTL() )
          articleText += " dir=\"rtl\"";
        articleText += ">";

        string convertedText = Html::preformat( articleBody, isToLanguageRTL() );
        free( articleBody );

        articleText += QString::fromUtf8( convertedText.c_str() )
              .replace(phonetic, "<span class=\"dictd_phonetic\">\\1</span>")
              .replace(refs, "<a href=\"gdlookup://localhost/\\1\">\\1</a>")
            .toUtf8().data();
        articleText += "</div>";
      }

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

QString const& DictdDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() )
        return dictionaryDescription;

    sptr< Dictionary::DataRequest > req = getArticle( GD_NATIVE_TO_WS( L"00databaseinfo" ),
                                                      vector< wstring >(), wstring() );

    if( req->dataSize() > 0 )
      dictionaryDescription = Html::unescape( QString::fromUtf8( req->getFullData().data(), req->getFullData().size() ), true );
    else
      dictionaryDescription = "NONE";

    return dictionaryDescription;
}

void DictdDictionary::makeFTSIndex( QAtomicInt & isCancelled, bool firstIteration )
{
  if( !( Dictionary::needToRebuildIndex( getDictionaryFilenames(), ftsIdxName )
         || FtsHelpers::ftsIndexIsOldOrBad( ftsIdxName, this ) ) )
    FTS_index_completed.ref();

  if( haveFTSIndex() )
    return;

  if( ensureInitDone().size() )
    return;

  if( firstIteration && getArticleCount() > FTS::MaxDictionarySizeForFastSearch )
    return;

  gdDebug( "DictD: Building the full-text index for dictionary: %s\n",
           getName().c_str() );

  try
  {
    FtsHelpers::makeFTSIndex( this, isCancelled );
    FTS_index_completed.ref();
  }
  catch( std::exception &ex )
  {
    gdWarning( "DictD: Failed building full-text search index for \"%s\", reason: %s\n", getName().c_str(), ex.what() );
    QFile::remove( FsEncoding::decode( ftsIdxName.c_str() ) );
  }
}

void DictdDictionary::getArticleText( uint32_t articleAddress, QString & headword, QString & text )
{
  try
  {
    char buf[ 16384 ];
    {
      Mutex::Lock _( indexFileMutex );
      indexFile.seek( articleAddress );

      if ( !indexFile.gets( buf, sizeof( buf ), true ) )
        throw exFailedToReadLineFromIndex();
    }

    char * tab1 = strchr( buf, '\t' );

    if ( !tab1 )
      throw exMalformedIndexFileLine();

    headword = QString::fromUtf8( buf, tab1 - buf );

    char * tab2 = strchr( tab1 + 1, '\t' );

    if ( !tab2 )
      throw exMalformedIndexFileLine();

    // After tab1 should be article offset, after tab2 -- article size

    uint32_t articleOffset = decodeBase64( string( tab1 + 1, tab2 - tab1 - 1 ) );

    char * tab3 = strchr( tab2 + 1, '\t');

    uint32_t articleSize;
    if ( tab3 )
    {
      articleSize = decodeBase64( string( tab2 + 1, tab3 - tab2 - 1 ) );
    }
    else
    {
      articleSize = decodeBase64( tab2 + 1 );
    }

    string articleText;

    char * articleBody;
    {
      Mutex::Lock _( dzMutex );
      articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
    }

    if ( !articleBody )
    {
      articleText = dict_error_str( dz );
    }
    else
    {
      static QRegExp phonetic( "\\\\([^\\\\]+)\\\\", Qt::CaseInsensitive ); // phonetics: \stuff\ ...
      static QRegExp refs( "\\{([^\\{\\}]+)\\}", Qt::CaseInsensitive );     // links: {stuff}

      string convertedText = Html::preformat( articleBody, isToLanguageRTL() );
      free( articleBody );

      text = QString::fromUtf8( convertedText.data(), convertedText.size() )
            .replace(phonetic, "<span class=\"dictd_phonetic\">\\1</span>")
            .replace(refs, "<a href=\"gdlookup://localhost/\\1\">\\1</a>");

      text = Html::unescape( text );
    }
  }
  catch( std::exception &ex )
  {
    gdWarning( "DictD: Failed retrieving article from \"%s\", reason: %s\n", getName().c_str(), ex.what() );
  }
}

sptr< Dictionary::DataRequest > DictdDictionary::getSearchResults( QString const & searchString,
                                                                   int searchMode, bool matchCase,
                                                                   int distanceBetweenWords,
                                                                   int maxResults )
{
  return new FtsHelpers::FTSResultsRequest( *this, searchString,searchMode, matchCase, distanceBetweenWords, maxResults );
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
        string dictionaryName = nameFromFileName( dictFiles[ 0 ] );

        gdDebug( "DictD: Building the index for dictionary: %s\n", dictionaryName.c_str() );

        initializing.indexingDictionary( dictionaryName );

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

          // Check that there are exactly two or three tabs in the record.
          char * tab1 = strchr( buf, '\t' );
          if ( tab1 )
          {
            char * tab2 = strchr( tab1 + 1, '\t' );
            if ( tab2 )
            {
              char * tab3 = strchr( tab2 + 1, '\t');
              if ( tab3 )
              {
                char * tab4 = strchr( tab3 + 1, '\t');
                if ( tab4 )
                {
                  GD_DPRINTF( "Warning: too many tabs present, skipping: %s\n", buf );
                  continue;
                }

                // Handle the forth entry, if it exists. From dictfmt man:
                // When --index-keep-orig option is used fourth column is created
                // (if necessary) in .index file.
                indexedWords.addWord( Utf8::decode( string( tab3 + 1, strlen ( tab3 + 1 ) ) ), curOffset );
                ++idxHeader.wordCount;
              }
              indexedWords.addWord( Utf8::decode( string( buf, strchr( buf, '\t' ) - buf ) ), curOffset );
              ++idxHeader.wordCount;
              ++idxHeader.articleCount;

              // Check for proper dictionary name
              if ( !strncmp( buf, "00databaseshort", 15 ) || !strncmp( buf, "00-database-short", 17 ) )
              {
                // After tab1 should be article offset, after tab2 -- article size
                uint32_t articleOffset = decodeBase64( string( tab1 + 1, tab2 - tab1 - 1 ) );
                uint32_t articleSize = decodeBase64( tab2 + 1 );

                DZ_ERRORS error;
                dictData * dz = dict_data_open( dictFiles[ 1 ].c_str(), &error, 0 );

                if ( dz )
                {
                  char * articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
                  if ( articleBody )
                  {
                    char * eol = strchr( articleBody, '\n'  ); // skip the first line (headword itself)
                    if ( eol )
                    {
                      while( *eol && isspace( *eol ) ) ++eol; // skip spaces

                      // use only the single line for the dictionary title
                      char * endEol = strchr( eol, '\n' );
                      if ( endEol )
                        *endEol = 0;

                      GD_DPRINTF( "DICT NAME: '%s'\n", eol );
                      dictionaryName = eol;
                    }
                  }
                  dict_data_close( dz );
                }
                else
                  throw exDictzipError( string( dz_error_str( error ) )
                                        + "(" + dictFiles[ 1 ] + ")" );
              }
            }
            else
            {
              GD_DPRINTF( "Warning: only a single tab present, skipping: %s\n", buf );
              continue;
            }
          }
          else
          {
            GD_DPRINTF( "Warning: no tabs present, skipping: %s\n", buf );
            continue;
          }


        } while( !indexFile.eof() );


        // Write dictionary name

        idx.write( (uint32_t) dictionaryName.size() );
        idx.write( dictionaryName.data(), dictionaryName.size() );

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
      gdWarning( "Dictd dictionary \"%s\" reading failed, error: %s\n",
                 i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
