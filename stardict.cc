/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "stardict.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "dictzip.h"
#include "xdxf2html.hh"
#include "htmlescape.hh"
#include "langcoder.hh"
#include "dprintf.hh"
#include "fsencoding.hh"
#include "filetype.hh"
#include "indexedzip.hh"

#include <zlib.h>
#include <map>
#include <set>
#include <string>
#ifndef __WIN32
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif
#include <stdlib.h>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QString>
#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QDebug>

#include "ufile.hh"

namespace Stardict {

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using gd::wstring;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX( exNotAnIfoFile, "Not an .ifo file", Dictionary::Ex )
DEF_EX_STR( exBadFieldInIfo, "Bad field in .ifo file encountered:", Dictionary::Ex )
DEF_EX_STR( exNoIdxFile, "No corresponding .idx file was found for", Dictionary::Ex )
DEF_EX_STR( exNoDictFile, "No corresponding .dict file was found for", Dictionary::Ex )
DEF_EX_STR( exNoSynFile, "No corresponding .syn file was found for", Dictionary::Ex )

DEF_EX( ex64BitsNotSupported, "64-bit indices are not presently supported, sorry", Dictionary::Ex )
DEF_EX( exDicttypeNotSupported, "Dictionaries with dicttypes are not supported, sorry", Dictionary::Ex )

DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX_STR( exWordIsTooLarge, "Enountered a word that is too large:", Dictionary::Ex )
DEF_EX_STR( exSuddenEndOfFile, "Sudden end of file", Dictionary::Ex )

DEF_EX_STR( exIncorrectOffset, "Incorrect offset encountered in file", Dictionary::Ex )

/// Contents of an ifo file
struct Ifo
{
  string version;
  string bookname;
  uint32_t wordcount, synwordcount, idxfilesize, idxoffsetbits;
  string sametypesequence, dicttype, description;
  string copyright, author, email;

  Ifo( File::Class & );
};

enum
{
  Signature = 0x58444953, // SIDX on little-endian, XDIS on big-endian
  CurrentFormatVersion = 9 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, SIDX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t wordCount; // Saved from Ifo::wordcount
  uint32_t synWordCount; // Saved from Ifo::synwordcount
  uint32_t bookNameSize; // Book name's length. Used to read it then.
  uint32_t sameTypeSequenceSize; // That string's size. Used to read it then.
  uint32_t langFrom;  // Source language
  uint32_t langTo;    // Target language
  uint32_t hasZipFile; // Non-zero means there's a zip file with resources present
  uint32_t zipIndexBtreeMaxElements; // Two fields from IndexInfo of the zip
                                     // resource index.
  uint32_t zipIndexRootOffset;
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

class StardictDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  string bookName;
  string sameTypeSequence;
  ChunkedStorage::Reader chunks;
  Mutex dzMutex;
  dictData * dz;
  Mutex resourceZipMutex;
  IndexedZip resourceZip;

public:

  StardictDictionary( string const & id, string const & indexFile,
                      vector< string > const & dictionaryFiles );

  ~StardictDictionary();

  virtual string getName() throw()
  { return bookName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.wordCount; }

  virtual unsigned long getWordCount() throw()
  { return idxHeader.wordCount + idxHeader.synWordCount; }

  inline virtual quint32 getLangFrom() const
  { return idxHeader.langFrom; }

  inline virtual quint32 getLangTo() const
  { return idxHeader.langTo; }

  virtual sptr< Dictionary::WordSearchRequest > findHeadwordsForSynonym( wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

  virtual QString const& getDescription();

  virtual QString getMainFilename();

protected:

  void loadIcon() throw();

private:

  /// Retrives the article's offset/size in .dict file, and its headword.
  void getArticleProps( uint32_t articleAddress,
                        string & headword,
                        uint32_t & offset, uint32_t & size );

  /// Loads the article, storing its headword and formatting the data it has
  /// into an html.
  void loadArticle(  uint32_t address,
                     string & headword,
                     string & articleText );

  string loadString( size_t size );

  string handleResource( char type, char const * resource, size_t size );

  friend class StardictResourceRequest;
  friend class StardictArticleRequest;
  friend class StardictHeadwordsRequest;
};

StardictDictionary::StardictDictionary( string const & id,
                                        string const & indexFile,
                                        vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  bookName( loadString( idxHeader.bookNameSize ) ),
  sameTypeSequence( loadString( idxHeader.sameTypeSequenceSize ) ),
  chunks( idx, idxHeader.chunksOffset )
{
  // Open the .dict file

  dz = dict_data_open( dictionaryFiles[ 2 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 2 ] );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );

  // Open a resource zip file, if there's one

  if ( idxHeader.hasZipFile &&
       ( idxHeader.zipIndexBtreeMaxElements ||
         idxHeader.zipIndexRootOffset ) )
  {
    resourceZip.openIndex( IndexInfo( idxHeader.zipIndexBtreeMaxElements,
                                      idxHeader.zipIndexRootOffset ),
                           idx, idxMutex );

    QString zipName = QDir::fromNativeSeparators(
        FsEncoding::decode( getDictionaryFilenames().back().c_str() ) );

    if ( zipName.endsWith( ".zip", Qt::CaseInsensitive ) ) // Sanity check
      resourceZip.openZipFile( zipName );
  }
}

StardictDictionary::~StardictDictionary()
{
  if ( dz )
    dict_data_close( dz );
}

void StardictDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 3 );

  if( !loadIconFromFile( fileName ) )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_stardict.png");
  }

  dictionaryIconLoaded = true;
}

string StardictDictionary::loadString( size_t size )
{
  vector< char > data( size );

  idx.read( &data.front(), data.size() );

  return string( &data.front(), data.size() );
}

void StardictDictionary::getArticleProps( uint32_t articleAddress,
                                          string & headword,
                                          uint32_t & offset, uint32_t & size )
{
  vector< char > chunk;

  Mutex::Lock _( idxMutex );

  char * articleData = chunks.getBlock( articleAddress, chunk );

  memcpy( &offset, articleData, sizeof( uint32_t ) );
  articleData += sizeof( uint32_t );
  memcpy( &size, articleData, sizeof( uint32_t ) );
  articleData += sizeof( uint32_t );

  headword = articleData;
}

/// This function tries to make an html of the Stardict's resource typed
/// 'type', contained in a block pointed to by 'resource', 'size' bytes long.
string StardictDictionary::handleResource( char type, char const * resource, size_t size )
{
  switch( type )
  {
    case 'x': // Xdxf content
      return Xdxf2Html::convert( string( resource, size ), Xdxf2Html::STARDICT, NULL, this );
    case 'h': // Html content
    {
      string articleText = "<div class=\"sdct_h\">" + string( resource, size ) + "</div>";

      return ( QString::fromUtf8( articleText.c_str() )
               .replace( QRegExp( "(<\\s*img\\s+[^>]*src\\s*=\\s*[\"']+)((?!data:)[^\"']*)", Qt::CaseInsensitive ),
                         "\\1bres://" + QString::fromStdString( getId() ) + "/\\2" )
               .replace( QRegExp( "(<\\s*link\\s+[^>]*href\\s*=\\s*[\"']+)((?!data:)[^\"']*)", Qt::CaseInsensitive ),
                         "\\1bres://" + QString::fromStdString( getId() ) + "/\\2" )
               .toUtf8().data() );
    }
    case 'm': // Pure meaning, usually means preformatted text
      return "<div class=\"sdct_m\">" + Html::preformat( string( resource, size ), isToLanguageRTL() ) + "</div>";
    case 'l': // Same as 'm', but not in utf8, instead in current locale's
              // encoding.
              // We just use Qt here, it should know better about system's
              // locale.
      return "<div class=\"sdct_l\">" + Html::preformat( QString::fromLocal8Bit( resource, size ).toUtf8().data(),
                                                         isToLanguageRTL() )
                                      + "</div>";
    case 'g': // Pango markup.
      return "<div class=\"sdct_g\">" + string( resource, size ) + "</div>";
    case 't': // Transcription
      return "<div class=\"sdct_t\">" + Html::escape( string( resource, size ) ) + "</div>";
    case 'y': // Chinese YinBiao or Japanese KANA. Examples are needed. For now,
              // just output as pure escaped utf8.
      return "<div class=\"sdct_y\">" + Html::escape( string( resource, size ) ) + "</div>";
    case 'k': // KingSoft PowerWord data. We don't know how to handle that.
      return "<div class=\"sdct_k\">" + Html::escape( string( resource, size ) ) + "</div>";
    case 'w': // MediaWiki markup. We don't handle this right now.
      return "<div class=\"sdct_w\">" + Html::escape( string( resource, size ) ) + "</div>";
    case 'n': // WordNet data. We don't know anything about it.
      return "<div class=\"sdct_n\">" + Html::escape( string( resource, size ) ) + "</div>";

    case 'r': // Resource file list. For now, resources aren't handled.
      return "<div class=\"sdct_r\">" + Html::escape( string( resource, size ) ) + "</div>";

    case 'W': // An embedded Wav file. Unhandled yet.
      return "<div class=\"sdct_W\">(an embedded .wav file)</div>";
    case 'P': // An embedded picture file. Unhandled yet.
      return "<div class=\"sdct_P\">(an embedded picture file)</div>";
  }

  if ( islower( type ) )
  {
    return string( "<b>Unknown textual entry type " ) + string( 1, type ) + ":</b> " + Html::escape( string( resource, size ) ) + "<br>";
  }
  else
    return string( "<b>Unknown blob entry type " ) + string( 1, type ) + "</b><br>";
}

void StardictDictionary::loadArticle( uint32_t address,
                                      string & headword,
                                      string & articleText )
{
  uint32_t offset, size;

  getArticleProps( address, headword, offset, size );

  char * articleBody;

  {
    Mutex::Lock _( dzMutex );

    // Note that the function always zero-pads the result.
    articleBody = dict_data_read_( dz, offset, size, 0, 0 );
  }

  if ( !articleBody )
  {
//    throw exCantReadFile( getDictionaryFilenames()[ 2 ] );
    articleText = string( "<div class=\"sdict_m\">DICTZIP error: " ) + dict_error_str( dz ) + "</div>";
    return;
  }

  articleText.clear();

  char * ptr = articleBody;

  if ( sameTypeSequence.size() )
  {
    /// The sequence is known, it's not stored in the article itself
    for( unsigned seq = 0; seq < sameTypeSequence.size(); ++seq )
    {
      // Last entry doesn't have size info -- it is inferred from
      // the bytes left
      bool entrySizeKnown = ( seq == sameTypeSequence.size() - 1 );

      uint32_t entrySize = 0;

      if ( entrySizeKnown )
        entrySize = size;
      else
      if ( !size )
      {
        FDPRINTF( stderr, "Warning: short entry for the word %s encountered.\n", headword.c_str() );
        break;
      }

      char type = sameTypeSequence[ seq ];

      if ( islower( type ) )
      {
        // Zero-terminated entry, unless it's the last one
        if ( !entrySizeKnown )
          entrySize = strlen( ptr );

        if ( size < entrySize )
        {
          FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( type, ptr, entrySize );

        if ( !entrySizeKnown )
          ++entrySize; // Need to skip the zero byte

        ptr += entrySize;
        size -= entrySize;
      }
      else
      if ( isupper( *ptr ) )
      {
        // An entry which has its size before contents, unless it's the last one

        if ( !entrySizeKnown )
        {
          if ( size < sizeof( uint32_t ) )
          {
            FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
            break;
          }

          memcpy( &entrySize, ptr, sizeof( uint32_t ) );

          entrySize = ntohl( entrySize );

          ptr += sizeof( uint32_t );
          size -= sizeof( uint32_t );
        }

        if ( size < entrySize )
        {
          FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( type, ptr, entrySize );

        ptr += entrySize;
        size -= entrySize;
      }
      else
      {
        FDPRINTF( stderr, "Warning: non-alpha entry type 0x%x for the word %s encountered.\n",
                         type, headword.c_str() );
        break;
      }
    }
  }
  else
  {
    // The sequence is stored in each article separately
    while( size )
    {
      if ( islower( *ptr ) )
      {
        // Zero-terminated entry
        size_t len = strlen( ptr + 1 );

        if ( size < len + 2 )
        {
          FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( *ptr, ptr + 1, len );

        ptr += len + 2;
        size -= len + 2;
      }
      else
      if ( isupper( *ptr ) )
      {
        // An entry which havs its size before contents
        if ( size < sizeof( uint32_t ) + 1 )
        {
          FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        uint32_t entrySize;

        memcpy( &entrySize, ptr + 1, sizeof( uint32_t ) );

        entrySize = ntohl( entrySize );

        if ( size < sizeof( uint32_t ) + 1 + entrySize )
        {
          FDPRINTF( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( *ptr, ptr + 1 + sizeof( uint32_t ), entrySize );

        ptr += sizeof( uint32_t ) + 1 + entrySize;
        size -= sizeof( uint32_t ) + 1 + entrySize;
      }
      else
      {
        FDPRINTF( stderr, "Warning: non-alpha entry type 0x%x for the word %s encountered.\n",
                         (unsigned)*ptr, headword.c_str() );
        break;
      }
    }
  }

  free( articleBody );
}

QString const& StardictDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() )
        return dictionaryDescription;

    File::Class ifoFile( getDictionaryFilenames()[ 0 ], "r" );
    Ifo ifo( ifoFile );

    if( !ifo.copyright.empty() )
      dictionaryDescription += "Copyright: " + QString::fromUtf8( ifo.copyright.c_str() ) + "\n\n";

    if( !ifo.author.empty() )
      dictionaryDescription += "Author: " + QString::fromUtf8( ifo.author.c_str() ) + "\n\n";

    if( !ifo.email.empty() )
      dictionaryDescription += "E-mail: " + QString::fromUtf8( ifo.email.c_str() ) + "\n\n";

    if( !ifo.description.empty() )
    {
      QString desc = QString::fromUtf8( ifo.description.c_str() );
      desc.replace( "\t", "<br/>" );
      desc.replace( "\\n", "<br/>" );
      dictionaryDescription += Html::unescape( desc );
    }

    if( dictionaryDescription.isEmpty() )
      dictionaryDescription = "NONE";

    return dictionaryDescription;
}

QString StardictDictionary::getMainFilename()
{
  return FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() );
}

/// StardictDictionary::findHeadwordsForSynonym()

class StardictHeadwordsRequest;

class StardictHeadwordsRequestRunnable: public QRunnable
{
  StardictHeadwordsRequest & r;
  QSemaphore & hasExited;

public:

  StardictHeadwordsRequestRunnable( StardictHeadwordsRequest & r_,
                                    QSemaphore & hasExited_ ): r( r_ ),
                                                               hasExited( hasExited_ )
  {}

  ~StardictHeadwordsRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class StardictHeadwordsRequest: public Dictionary::WordSearchRequest
{
  friend class StardictHeadwordsRequestRunnable;

  wstring word;
  StardictDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  StardictHeadwordsRequest( wstring const & word_,
                            StardictDictionary & dict_ ):
    word( word_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new StardictHeadwordsRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by StardictHeadwordsRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~StardictHeadwordsRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void StardictHeadwordsRequestRunnable::run()
{
  r.run();
}

void StardictHeadwordsRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  try
  {
    vector< WordArticleLink > chain = dict.findArticles( word );

    wstring caseFolded = Folding::applySimpleCaseOnly( word );

    for( unsigned x = 0; x < chain.size(); ++x )
    {
      if ( isCancelled )
      {
        finish();
        return;
      }

      string headword, articleText;

      dict.loadArticle( chain[ x ].articleOffset,
                        headword, articleText );

      wstring headwordDecoded = Utf8::decode( headword );

      if ( caseFolded != Folding::applySimpleCaseOnly( headwordDecoded ) )
      {
        // The headword seems to differ from the input word, which makes the
        // input word its synonym.
        Mutex::Lock _( dataMutex );

        matches.push_back( headwordDecoded );
      }
    }
  }
  catch( std::exception & e )
  {
    setErrorString( QString::fromUtf8( e.what() ) );
  }

  finish();
}

sptr< Dictionary::WordSearchRequest >
  StardictDictionary::findHeadwordsForSynonym( wstring const & word )
  throw( std::exception )
{
  return new StardictHeadwordsRequest( word, *this );
}


/// StardictDictionary::getArticle()

class StardictArticleRequest;

class StardictArticleRequestRunnable: public QRunnable
{
  StardictArticleRequest & r;
  QSemaphore & hasExited;

public:

  StardictArticleRequestRunnable( StardictArticleRequest & r_,
                                  QSemaphore & hasExited_ ): r( r_ ),
                                                             hasExited( hasExited_ )
  {}

  ~StardictArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class StardictArticleRequest: public Dictionary::DataRequest
{
  friend class StardictArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  StardictDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  StardictArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     StardictDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new StardictArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by StardictArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~StardictArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void StardictArticleRequestRunnable::run()
{
  r.run();
}

void StardictArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  try
  {
    vector< WordArticleLink > chain = dict.findArticles( word );

    for( unsigned x = 0; x < alts.size(); ++x )
    {
      /// Make an additional query for each alt

      vector< WordArticleLink > altChain = dict.findArticles( alts[ x ] );

      chain.insert( chain.end(), altChain.begin(), altChain.end() );
    }

    multimap< wstring, pair< string, string > > mainArticles, alternateArticles;

    set< uint32_t > articlesIncluded; // Some synonims make it that the articles
                                      // appear several times. We combat this
                                      // by only allowing them to appear once.

    wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

    for( unsigned x = 0; x < chain.size(); ++x )
    {
      if ( isCancelled )
      {
        finish();
        return;
      }

      if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
        continue; // We already have this article in the body.

      // Now grab that article

      string headword, articleText;

      dict.loadArticle( chain[ x ].articleOffset, headword, articleText );

      // Ok. Now, does it go to main articles, or to alternate ones? We list
      // main ones first, and alternates after.

      // We do the case-folded comparison here.

      wstring headwordStripped =
        Folding::applySimpleCaseOnly( Utf8::decode( headword ) );

      multimap< wstring, pair< string, string > > & mapToUse =
        ( wordCaseFolded == headwordStripped ) ?
          mainArticles : alternateArticles;

      mapToUse.insert( pair< wstring, pair< string, string > >(
        Folding::applySimpleCaseOnly( Utf8::decode( headword ) ),
        pair< string, string >( headword, articleText ) ) );

      articlesIncluded.insert( chain[ x ].articleOffset );
    }

    if ( mainArticles.empty() && alternateArticles.empty() )
    {
      // No such word
      finish();
      return;
    }

    string result;

    multimap< wstring, pair< string, string > >::const_iterator i;

    string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</b></b></b></b></b></b></b></b>"
                     "</i></i></i></i></i></i></i></i>";

    for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
    {
        result += dict.isFromLanguageRTL() ? "<h3 dir=\"rtl\">" : "<h3>";
        result += i->second.first;
        result += "</h3>";
        if( dict.isToLanguageRTL() )
          result += "<span dir=\"rtl\">";
        result += i->second.second;
        result += cleaner;
        if( dict.isToLanguageRTL() )
          result += "</span>";
    }

    for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
    {
        result += dict.isFromLanguageRTL() ? "<h3 dir=\"rtl\">" : "<h3>";
        result += i->second.first;
        result += "</h3>";
        if( dict.isToLanguageRTL() )
          result += "<span dir=\"rtl\">";
        result += i->second.second;
        result += cleaner;
        if( dict.isToLanguageRTL() )
          result += "</span>";
    }
    result = QString::fromUtf8( result.c_str() )
             .replace( QRegExp( "(<\\s*a\\s+[^>]*href\\s*=\\s*[\"']\\s*)bword://", Qt::CaseInsensitive ),
                       "\\1bword:" )
             .toUtf8().data();

    Mutex::Lock _( dataMutex );

    data.resize( result.size() );

    memcpy( &data.front(), result.data(), result.size() );

    hasAnyData = true;
  }
  catch( std::exception & e )
  {
    setErrorString( QString::fromUtf8( e.what() ) );
  }

  finish();
}

sptr< Dictionary::DataRequest > StardictDictionary::getArticle( wstring const & word,
                                                                vector< wstring > const & alts,
                                                                wstring const & )
  throw( std::exception )
{
  return new StardictArticleRequest( word, alts, *this );
}


static char const * beginsWith( char const * substr, char const * str )
{
  size_t len = strlen( substr );

  return strncmp( str, substr, len ) == 0 ? str + len : 0;
}

Ifo::Ifo( File::Class & f ):
  wordcount( 0 ), synwordcount( 0 ), idxfilesize( 0 ), idxoffsetbits( 32 )
{
  static string const versionEq( "version=" );

  static string const booknameEq( "bookname=" );

  //DPRINTF( "%s<\n", f.gets().c_str() );
  //DPRINTF( "%s<\n", f.gets().c_str() );

  if ( QString::fromUtf8(f.gets().c_str()) != "StarDict's dict ifo file" ||
       f.gets().compare( 0, versionEq.size(), versionEq ) )
    throw exNotAnIfoFile();

  /// Now go through the file and parse options

  try
  {
    char option[ 16384 ];

    for( ; ; )
    {
      if ( !f.gets( option, sizeof( option ), true ) )
        break;

      if ( char const * val = beginsWith( "bookname=", option ) )
        bookname = val;
      else
      if ( char const * val = beginsWith( "wordcount=", option ) )
      {
        if ( sscanf( val, "%u", & wordcount ) != 1 )
          throw exBadFieldInIfo( option );
      }
      else
      if ( char const * val = beginsWith( "synwordcount=", option ) )
      {
        if ( sscanf( val, "%u", & synwordcount ) != 1 )
          throw exBadFieldInIfo( option );
      }
      else
      if ( char const * val = beginsWith( "idxfilesize=", option ) )
      {
        if ( sscanf( val, "%u", & idxfilesize ) != 1 )
          throw exBadFieldInIfo( option );
      }
      else
      if ( char const * val = beginsWith( "idxoffsetbits=", option ) )
      {
        if ( sscanf( val, "%u", & idxoffsetbits ) != 1 || ( idxoffsetbits != 32
             && idxoffsetbits != 64 ) )
          throw exBadFieldInIfo( option );
      }
      else
      if ( char const * val = beginsWith( "sametypesequence=", option ) )
        sametypesequence = val;
      else
      if ( char const * val = beginsWith( "dicttype=", option ) )
        dicttype = val;
      else
      if ( char const * val = beginsWith( "description=", option ) )
        description = val;
      else
      if ( char const * val = beginsWith( "copyright=", option ) )
        copyright = val;
      else
      if ( char const * val = beginsWith( "author=", option ) )
        author = val;
      else
      if ( char const * val = beginsWith( "email=", option ) )
        email = val;
    }
  }
  catch( File::exReadError & )
  {
  }
}

//// StardictDictionary::getResource()

class StardictResourceRequest;

class StardictResourceRequestRunnable: public QRunnable
{
  StardictResourceRequest & r;
  QSemaphore & hasExited;

public:

  StardictResourceRequestRunnable( StardictResourceRequest & r_,
                               QSemaphore & hasExited_ ): r( r_ ),
                                                          hasExited( hasExited_ )
  {}

  ~StardictResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class StardictResourceRequest: public Dictionary::DataRequest
{
  friend class StardictResourceRequestRunnable;

  StardictDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  StardictResourceRequest( StardictDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new StardictResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by StardictResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~StardictResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void StardictResourceRequestRunnable::run()
{
  r.run();
}

void StardictResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( isCancelled )
  {
    finish();
    return;
  }

  try
  {
    if( resourceName.at( 0 ) == '\x1E' )
      resourceName = resourceName.erase( 0, 1 );
    if( resourceName.at( resourceName.length() - 1 ) == '\x1F' )
      resourceName.erase( resourceName.length() - 1, 1 );

    string n =
      FsEncoding::dirname( dict.getDictionaryFilenames()[ 0 ] ) +
      FsEncoding::separator() +
      "res" +
      FsEncoding::separator() +
      FsEncoding::encode( resourceName );

    DPRINTF( "n is %s\n", n.c_str() );

    try
    {
      Mutex::Lock _( dataMutex );

      File::loadFromFile( n, data );
    }
    catch( File::exCantOpen )
    {
      // Try reading from zip file

      if ( dict.resourceZip.isOpen() )
      {
        Mutex::Lock _( dict.resourceZipMutex );

        Mutex::Lock __( dataMutex );

        if ( !dict.resourceZip.loadFile( Utf8::decode( resourceName ), data ) )
          throw; // Make it fail since we couldn't read the archive
      }
      else
        throw;
    }

    if ( Filetype::isNameOfTiff( resourceName ) )
    {
      // Convert it

      dataMutex.lock();

      QImage img = QImage::fromData( (unsigned char *) &data.front(),
                                     data.size() );

      dataMutex.unlock();

      if ( !img.isNull() )
      {
        // Managed to load -- now store it back as BMP

        QByteArray ba;
        QBuffer buffer( &ba );
        buffer.open( QIODevice::WriteOnly );
        img.save( &buffer, "BMP" );

        Mutex::Lock _( dataMutex );

        data.resize( buffer.size() );

        memcpy( &data.front(), buffer.data(), data.size() );
      }
    }

    if( Filetype::isNameOfCSS( resourceName ) )
    {
      Mutex::Lock _( dataMutex );

      QString css = QString::fromUtf8( data.data(), data.size() );
      dict.isolateCSS( css );
      QByteArray bytes = css.toUtf8();
      data.resize( bytes.size() );
      memcpy( &data.front(), bytes.constData(), bytes.size() );
    }

    hasAnyData = true;
  }
  catch( File::Ex & )
  {
    // No such resource -- we don't set the hasAnyData flag then
  }
  catch( Utf8::exCantDecode )
  {
    // Failed to decode some utf8 -- probably the resource name is no good
  }
  catch( ... )
  {
  }

  finish();
}

sptr< Dictionary::DataRequest > StardictDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new StardictResourceRequest( *this, name );
}

} // anonymous namespace

static void findCorrespondingFiles( string const & ifo,
                                    string & idx, string & dict, string & syn )
{
  string base( ifo, 0, ifo.size() - 3 );

  if ( !(
          File::tryPossibleName( base + "idx", idx ) ||
          File::tryPossibleName( base + "idx.gz", idx ) ||
          File::tryPossibleName( base + "idx.dz", idx ) ||
          File::tryPossibleName( base + "IDX", idx ) ||
          File::tryPossibleName( base + "IDX.GZ", idx ) ||
          File::tryPossibleName( base + "IDX.DZ", idx )
      ) )
    throw exNoIdxFile( ifo );

  if ( !(
          File::tryPossibleName( base + "dict", dict ) ||
          File::tryPossibleName( base + "dict.dz", dict ) ||
          File::tryPossibleName( base + "DICT", dict ) ||
          File::tryPossibleName( base + "dict.DZ", dict )
      ) )
    throw exNoDictFile( ifo );

  if ( !(
         File::tryPossibleName( base + "syn", syn ) ||
         File::tryPossibleName( base + "syn.gz", syn ) ||
         File::tryPossibleName( base + "syn.dz", syn ) ||
         File::tryPossibleName( base + "SYN", syn ) ||
         File::tryPossibleName( base + "SYN.GZ", syn ) ||
         File::tryPossibleName( base + "SYN.DZ", syn )
     ) )
    syn.clear();
}

static void handleIdxSynFile( string const & fileName,
                              IndexedWords & indexedWords,
                              ChunkedStorage::Writer & chunks,
                              vector< uint32_t > * articleOffsets,
                              bool isSynFile )
{
  gzFile stardictIdx = gd_gzopen( fileName.c_str() );
  if ( !stardictIdx )
    throw exCantReadFile( fileName );

  vector< char > image;

  for( ; ; )
  {
    size_t oldSize = image.size();

    image.resize( oldSize + 65536 );

    int rd = gzread( stardictIdx, &image.front() + oldSize, 65536 );

    if ( rd < 0 )
    {
      gzclose( stardictIdx );
      throw exCantReadFile( fileName );
    }

    if ( rd != 65536 )
    {
      image.resize( oldSize + rd + 1 );
      break;
    }
  }
  gzclose( stardictIdx );

  // We append one zero byte to catch runaway string at the end, if any

  image.back() = 0;

  // Now parse it

  for( char const * ptr = &image.front(); ptr != &image.back(); )
  {
    size_t wordLen = strlen( ptr );

    if ( ptr + wordLen + 1 + ( isSynFile ? sizeof( uint32_t ) :
                                           sizeof( uint32_t ) * 2 ) >
         &image.back() )
    {
      FDPRINTF( stderr, "Warning: sudden end of file %s\n", fileName.c_str() );
      break;
    }

    char const * word = ptr;

    ptr += wordLen + 1;

    uint32_t offset;

    if( strstr( word, "&#" ) )
    {
      // Decode some html-coded symbols in headword
      string unescapedWord = Html::unescapeUtf8( word );
      strncpy( (char *)word, unescapedWord.c_str(), wordLen );
      wordLen = strlen( word );
    }

    if ( !isSynFile )
    {
      // We're processing the .idx file
      uint32_t articleOffset, articleSize;

      memcpy( &articleOffset, ptr, sizeof( uint32_t ) );
      ptr += sizeof( uint32_t );
      memcpy( &articleSize, ptr, sizeof( uint32_t ) );
      ptr += sizeof( uint32_t );

      articleOffset = ntohl( articleOffset );
      articleSize = ntohl( articleSize );

      // Create an entry for the article in the chunked storage

      offset = chunks.startNewBlock();

      if ( articleOffsets )
        articleOffsets->push_back( offset );

      chunks.addToBlock( &articleOffset, sizeof( uint32_t ) );
      chunks.addToBlock( &articleSize, sizeof( uint32_t ) );
      chunks.addToBlock( word, wordLen + 1 );
    }
    else
    {
      // We're processing the .syn file
      uint32_t offsetInIndex;

      memcpy( &offsetInIndex, ptr, sizeof( uint32_t ) );
      ptr += sizeof( uint32_t );

      offsetInIndex = ntohl( offsetInIndex );

      if ( offsetInIndex >= articleOffsets->size() )
        throw exIncorrectOffset( fileName );

      offset = (*articleOffsets)[ offsetInIndex ];

      // Some StarDict dictionaries are in fact badly converted Babylon ones.
      // They contain a lot of superfluous slashed entries with dollar signs.
      // We try to filter them out here, since those entries become much more
      // apparent in GoldenDict than they were in StarDict because of
      // punctuation folding. Hopefully there are not a whole lot of valid
      // synonyms which really start from slash and contain dollar signs, or
      // end with dollar and contain slashes.
      if ( *word == '/' )
      {
        if ( strchr( word, '$' ) )
          continue; // Skip this entry
      }
      else
      if ( wordLen && word[ wordLen - 1 ] == '$' )
      {
        if ( strchr( word, '/' ) )
          continue; // Skip this entry
      }
    }

    // Insert new entry into an index

    indexedWords.addWord( Utf8::decode( word ), offset );
  }

  DPRINTF( "%u entires made\n", (unsigned) indexedWords.size() );
}


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
    if ( i->size() < 4 ||
        strcasecmp( i->c_str() + ( i->size() - 4 ), ".ifo" ) != 0 )
      continue;

    try
    {
      vector< string > dictFiles( 1, *i );

      string idxFileName, dictFileName, synFileName;

      findCorrespondingFiles( *i, idxFileName, dictFileName, synFileName );

      dictFiles.push_back( idxFileName );
      dictFiles.push_back( dictFileName );

      if ( synFileName.size() )
        dictFiles.push_back( synFileName );

      // See if there's a zip file with resources present. If so, include it.

      string zipFileName;
      string baseName = FsEncoding::dirname( idxFileName ) + FsEncoding::separator();

      if ( File::tryPossibleName( baseName + "res.zip", zipFileName ) ||
           File::tryPossibleName( baseName + "RES.ZIP", zipFileName ) ||
           File::tryPossibleName( baseName + "res" + FsEncoding::separator() + "res.zip", zipFileName ) )
        dictFiles.push_back( zipFileName );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        // Building the index

        File::Class ifoFile( *i, "r" );

        Ifo ifo( ifoFile );

        qDebug( "Stardict: Building the index for dictionary: %s\n", ifo.bookname.c_str() );

        if ( ifo.idxoffsetbits == 64 )
          throw ex64BitsNotSupported();

        if ( ifo.dicttype.size() )
          throw exDicttypeNotSupported();

        if( synFileName.empty() )
        {
          if ( ifo.synwordcount )
          {
            DPRINTF( "Warning: dictionary has synwordcount specified, but no "
                    "corresponding .syn file was found\n" );
            ifo.synwordcount = 0;  // Pretend it wasn't there
          }
        }
        else
        if ( !ifo.synwordcount )
        {
          DPRINTF( "Warning: ignoring .syn file %s, since there's no synwordcount in .ifo specified\n",
                  synFileName.c_str() );
        }


        DPRINTF( "bookname = %s\n", ifo.bookname.c_str() );
        DPRINTF( "wordcount = %u\n", ifo.wordcount );

        initializing.indexingDictionary( ifo.bookname );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        idx.write( ifo.bookname.data(), ifo.bookname.size() );
        idx.write( ifo.sametypesequence.data(), ifo.sametypesequence.size() );

        IndexedWords indexedWords;

        ChunkedStorage::Writer chunks( idx );

        // Load indices
        if ( !ifo.synwordcount )
          handleIdxSynFile( idxFileName, indexedWords, chunks, 0, false );
        else
        {
          vector< uint32_t > articleOffsets;

          articleOffsets.reserve( ifo.wordcount );

          handleIdxSynFile( idxFileName, indexedWords, chunks, &articleOffsets,
                            false );

          handleIdxSynFile( synFileName, indexedWords, chunks, &articleOffsets,
                            true );
        }

        // Finish with the chunks

        idxHeader.chunksOffset = chunks.finish();

        // Build index

        IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

        idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
        idxHeader.indexRootOffset = idxInfo.rootOffset;

        // That concludes it. Update the header.

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;

        idxHeader.wordCount = ifo.wordcount;
        idxHeader.synWordCount = ifo.synwordcount;
        idxHeader.bookNameSize = ifo.bookname.size();
        idxHeader.sameTypeSequenceSize = ifo.sametypesequence.size();

        // read languages
        QPair<quint32,quint32> langs =
            LangCoder::findIdsForFilename( QString::fromStdString( dictFileName ) );

        // if no languages found, try dictionary's name
        if ( langs.first == 0 || langs.second == 0 )
        {
          langs =
            LangCoder::findIdsForFilename( QString::fromStdString( ifo.bookname ) );
        }

        idxHeader.langFrom = langs.first;
        idxHeader.langTo = langs.second;

        // If there was a zip file, index it too

        if ( zipFileName.size() )
        {
          DPRINTF( "Indexing zip file\n" );

          idxHeader.hasZipFile = 1;

          IndexedWords zipFileNames;
          IndexedZip zipFile;
          if( zipFile.openZipFile( QDir::fromNativeSeparators(
                                   FsEncoding::decode( zipFileName.c_str() ) ) ) )
              zipFile.indexFile( zipFileNames );

          if( !zipFileNames.empty() )
          {
            // Build the resulting zip file index

            IndexInfo idxInfo = BtreeIndexing::buildIndex( zipFileNames, idx );

            idxHeader.zipIndexBtreeMaxElements = idxInfo.btreeMaxElements;
            idxHeader.zipIndexRootOffset = idxInfo.rootOffset;
          }
          else
          {
            // Bad zip file -- no index (though the mark that we have one
            // remains)
            idxHeader.zipIndexBtreeMaxElements = 0;
            idxHeader.zipIndexRootOffset = 0;
          }
        }
        else
          idxHeader.hasZipFile = 0;

        // That concludes it. Update the header.

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );
      }

      dictionaries.push_back( new StardictDictionary( dictId,
                                                      indexFile,
                                                      dictFiles ) );
    }
    catch( std::exception & e )
    {
      FDPRINTF( stderr, "Stardict's dictionary reading failed: %s, error: %s\n",
        i->c_str(), e.what() );
    }
  }

  return dictionaries;
}


}
