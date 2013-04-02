/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "aard.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "langcoder.hh"
#include "dprintf.hh"
#include "fsencoding.hh"
#include "decompress.hh"

#include <map>
#include <set>
#include <string>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QString>
#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QDomDocument>
#include <QUrl>

#include "ufile.hh"
#include "wstring_qt.hh"

namespace Aard {

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

DEF_EX_STR( exNotDctFile, "Not an Sdictionary file", Dictionary::Ex )
DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )
DEF_EX_STR( exWordIsTooLarge, "Enountered a word that is too large:", Dictionary::Ex )
DEF_EX_STR( exSuddenEndOfFile, "Sudden end of file", Dictionary::Ex )

#ifdef _MSC_VER
#pragma pack( push, 1 )
#endif

// Big-Endian template
// http://habrahabr.ru/blogs/cpp/121811/

template<typename T>
struct BigEndian
{
    union
    {
        unsigned char bytes[sizeof(T)];
        T raw_value;
    };

    BigEndian(T t = T())
    {
        operator =(t);
    }

    BigEndian(const BigEndian<T> & t)
    {
        raw_value = t.raw_value;
    }

    operator const T() const
    {
        T t = T();
        for (unsigned i = 0; i < sizeof(T); i++)
            t |= T(bytes[sizeof(T) - 1 - i]) << (i << 3);
        return t;
    }

    const T operator = (const T t)
    {
        for (unsigned i = 0; i < sizeof(T); i++)
            bytes[sizeof(T) - 1 - i] = (unsigned char)( t >> (i << 3) );
        return t;
    }

}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

typedef BigEndian< uint16_t > uint16_be;
typedef BigEndian< uint32_t > uint32_be;
typedef BigEndian< uint64_t > uint64_be;

/// AAR file header
struct AAR_header
{
    char signature[4];
    char checksum[40];
    uint16_be version;
    char uuid[16];
    uint16_be volume;
    uint16_be totalVolumes;
    uint32_be metaLength;
    uint32_be wordsCount;
    uint32_be articleOffset;
    char indexItemFormat[4];
    char keyLengthFormat[2];
    char articleLengthFormat[2];
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct IndexElement
{
    uint32_be wordOffset;
    uint32_be articleOffset;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct IndexElement64
{
    uint32_be wordOffset;
    uint64_be articleOffset;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

enum
{
  Signature = 0x58524141, // AARX on little-endian, XRAA on big-endian
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, AARX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t wordCount;
  uint32_t articleCount;
  uint32_t langFrom;  // Source language
  uint32_t langTo;    // Target language
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#ifdef _MSC_VER
#pragma pack( pop, 1 )
#endif

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

void readJSONValue( string const & source, string & str, string::size_type & pos)
{
    int level = 1;
    char endChar;
    str.push_back( source[pos] );
    if( source[pos] == '{')
        endChar = '}';
    else if( source[pos] == '[' )
        endChar = ']';
    else if( source[pos] == '\"' )
    {
        str.clear();
        endChar = '\"';
    }
    else
        endChar = ',';

    pos++;
    char ch = 0;
    char lastCh = 0;
    while( !( ch == endChar && lastCh != '\\' && level == 0 )
           && pos < source.size() )
    {
        lastCh = ch;
        ch = source[ pos++ ];
        if( ( ch == '{' || ch == '[' ) && lastCh != '\\' )
          level++;
        if( ( ch == '}' || ch == ']' ) && lastCh != '\\' )
          level--;

        if( ch == endChar &&
            ( ( ch == '\"' && lastCh != '\\' ) || ch == ',' )
            && level == 1)
          break;
        str.push_back( ch );
    }
}

map< string, string > parseMetaData( string const & metaData )
{
// Parsing JSON string
    map< string, string > data;
    string name, value;
    string::size_type n = 0;

    while( n < metaData.length() && metaData[n] != '{' )
        n++;
    while( n < metaData.length() )
    {
        // Skip to '"'
        while( n < metaData.length() && metaData[n] != '\"' )
            n++;
        if( ++n >= metaData.length() )
            break;

        // Read name
        while(  n < metaData.length() &&
                !( ( metaData[n] == '\"' || metaData[n] == '{' ) && metaData[n-1] != '\\' ) )
            name.push_back( metaData[n++]);

        // Skip to ':'
        if( ++n >= metaData.length() )
            break;
        while( n < metaData.length() && metaData[n] != ':' )
            n++;
        if( ++n >= metaData.length() )
            break;

        // Find value start after ':'
        while( n < metaData.length()
               && !( ( metaData[n] == '\"'
                       || metaData[n] == '{'
                       || metaData[n] == '['
                       || ( metaData[n] >= '0' && metaData[n] <= '9' ) )
               && metaData[n-1] != '\\' ) )
            n++;
        if( n >= metaData.length() )
            break;

        readJSONValue( metaData, value, n);

        data[name] = value;

        name.clear();
        value.clear();
        if( ++n >= metaData.length() )
            break;
    }
    return data;
}

class AardDictionary: public BtreeIndexing::BtreeDictionary
{
    Mutex idxMutex;
    Mutex aardMutex;
    File::Class idx;
    IdxHeader idxHeader;
    ChunkedStorage::Reader chunks;
    string dictionaryName;
    File::Class df;

  public:

    AardDictionary( string const & id, string const & indexFile,
                     vector< string > const & dictionaryFiles );

    ~AardDictionary();

    virtual string getName() throw()
    { return dictionaryName; }

    virtual map< Dictionary::Property, string > getProperties() throw()
    { return map< Dictionary::Property, string >(); }

    virtual unsigned long getArticleCount() throw()
    { return idxHeader.articleCount; }

    virtual unsigned long getWordCount() throw()
    { return idxHeader.wordCount; }

    inline virtual quint32 getLangFrom() const
    { return idxHeader.langFrom; }

    inline virtual quint32 getLangTo() const
    { return idxHeader.langTo; }

    virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                        vector< wstring > const & alts,
                                                        wstring const & )
      throw( std::exception );

    virtual QString const& getDescription();

protected:

    virtual void loadIcon() throw();

private:

    /// Loads the article.
    void loadArticle( uint32_t address,
                      string & articleText );
    string convert( string const & in_data );

    friend class AardArticleRequest;
};

AardDictionary::AardDictionary( string const & id,
                                string const & indexFile,
                                vector< string > const & dictionaryFiles ):
    BtreeDictionary( id, dictionaryFiles ),
    idx( indexFile, "rb" ),
    idxHeader( idx.read< IdxHeader >() ),
    chunks( idx, idxHeader.chunksOffset ),
    df( dictionaryFiles[ 0 ], "rb" )
{
    // Read dictionary name

    idx.seek( sizeof( idxHeader ) );
    vector< char > dName( idx.read< uint32_t >() );
    if( dName.size() )
    {
        idx.read( &dName.front(), dName.size() );
        dictionaryName = string( &dName.front(), dName.size() );
    }

    // Initialize the index

    openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                          idxHeader.indexRootOffset ),
               idx, idxMutex );

    // Read decription

}

AardDictionary::~AardDictionary()
{
    df.close();
}

void AardDictionary::loadIcon() throw()
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
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_aard.png");
  }

  dictionaryIconLoaded = true;
}

string AardDictionary::convert( const string & in )
{
    string inConverted;
    char inCh, lastCh = 0;
    bool afterEol = false;

    for( string::const_iterator i = in.begin(), j = in.end(); i != j; ++i )
    {
        inCh = *i;
        if( lastCh == '\\' )
        {
            inConverted.erase( inConverted.size() - 1 );
            lastCh = 0;
            if( inCh == 'n' )
            {
                inConverted.append( "<br/>");
                afterEol = true;
                continue;
            }
            else if( inCh == 'r')
                continue;
        }
        else if( inCh == ' ' && afterEol )
        {
            inConverted.append( "&nbsp;" );
            continue;
        } else
            lastCh = inCh;
        afterEol = false;
        inConverted.push_back( inCh );
    }

    QDomDocument dd;
    QString errorStr;
    int errorLine, errorColumn;

    if( !dd.setContent( QByteArray( inConverted.c_str() ), false, &errorStr, &errorLine, &errorColumn ) )
    {
        FDPRINTF( stderr, "Aard article parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
        FDPRINTF( stderr, "The input was: %s\n", in.c_str() );
        return inConverted;
    }

    QDomNodeList nodes = dd.elementsByTagName( "a" ); // References
    for( int i = 0; i < nodes.count(); i++ )
    {
        QDomElement el = nodes.at( i ).toElement();
        QString ref = el.attribute( "href", "" );
        if( ref.size() == 0 || ref.indexOf( "http://") != -1 || ref[0] == '#' )
            continue;
        if( ref.indexOf( "w:") == 0 || ref.indexOf( "s:") == 0 )
            ref.replace( 0, 2, "bword:" );
        else
            ref.insert( 0, "bword:" );
        el.setAttribute( "href", ref );
    }

    return dd.toByteArray().data();
}

void AardDictionary::loadArticle( uint32_t address,
                                   string & articleText )
{
    uint32_t articleOffset = address;
    uint32_t articleSize;
    uint32_be size;

    vector< char > articleBody;

    articleText.clear();


    while( 1 )
    {
      articleText = "Article loading error";
      try
      {
        Mutex::Lock _( aardMutex );
        df.seek( articleOffset );
        df.read( &size, sizeof(size) );
        articleSize = size;
        articleBody.resize( articleSize );
        df.read( &articleBody.front(), articleSize );
      }
      catch(...)
      {
        break;
      }

      if ( articleBody.empty() )
        break;

      articleText.clear();

      string text = decompressBzip2( articleBody.data(), articleSize );
      if( text.empty() )
        text = decompressZlib( articleBody.data(), articleSize );
      if( text.empty() )
        text = string( articleBody.data(), articleSize );

      if( text.empty() || text[ 0 ] != '[' )
        break;

      string::size_type n = text.find( '\"' );
      if( n == string::npos )
        break;

      readJSONValue( text, articleText, n );

      if( articleText.empty() )
      {
        n = text.find( "\"r\"" );
        if( n != string::npos && n + 3 < text.size() )
        {
          n = text.find( '\"', n + 3 );
          if( n == string::npos )
            break;

          string link;
          readJSONValue( text, link, n );
          if( !link.empty() )
          {
            string encodedLink;
            encodedLink.reserve( link.size() );
            bool prev = false;
            for( string::const_iterator i = link.begin(); i != link.end(); ++i )
            {
              if( *i == '\\' )
              {
                if( !prev )
                {
                  prev = true;
                  continue;
                }
              }
              encodedLink.push_back( *i );
              prev = false;
            }
            encodedLink = string( QUrl::toPercentEncoding( QString::fromUtf8( encodedLink.c_str() ) ).data() );
            articleText = "<a href=\"" + encodedLink + "\">" + link + "</a>";
          }
        }
      }

      break;
    }

    if( !articleText.empty() )
      articleText = convert( articleText );
    else
      articleText = "Article decoding error";

    articleText = "<div class=\"aard\">" + articleText + "</div>";
}

QString const& AardDictionary::getDescription()
{
    if( !dictionaryDescription.isEmpty() )
        return dictionaryDescription;

    AAR_header dictHeader;
    uint32_t size;
    vector< char > data;

    {
        Mutex::Lock _( aardMutex );
        df.seek( 0 );
        df.read( &dictHeader, sizeof(dictHeader) );
        size = dictHeader.metaLength;
        data.resize( size );
        df.read( &data.front(), size );
    }

    string metaStr = decompressBzip2( data.data(), size );
    if( metaStr.empty() )
        metaStr = decompressZlib( data.data(), size );

    map< string, string > meta = parseMetaData( metaStr );

    if( !meta.empty() )
    {
        map< string, string >::const_iterator iter = meta.find( "copyright" );
        if( iter != meta.end() )
          dictionaryDescription = "Copyright: " + QString::fromUtf8( iter->second.c_str() ) + "\n\n";

        iter = meta.find( "version" );
        if( iter != meta.end() )
          dictionaryDescription = "Version: " + QString::fromUtf8( iter->second.c_str() ) + "\n\n";

        iter = meta.find( "description" );
        if( iter != meta.end() )
        {
          QString desc = QString::fromUtf8( iter->second.c_str() );
          desc.replace( "\\n", "\n" );
          desc.replace( "\\t", "\t" );
          dictionaryDescription += desc;
        }
    }

    if( dictionaryDescription.isEmpty() )
      dictionaryDescription = "NONE";

    return dictionaryDescription;
}

/// AardDictionary::getArticle()

class AardArticleRequest;

class AardArticleRequestRunnable: public QRunnable
{
  AardArticleRequest & r;
  QSemaphore & hasExited;

public:

  AardArticleRequestRunnable( AardArticleRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~AardArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class AardArticleRequest: public Dictionary::DataRequest
{
  friend class AardArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  AardDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  AardArticleRequest( wstring const & word_,
                      vector< wstring > const & alts_,
                      AardDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new AardArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by DslArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~AardArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void AardArticleRequestRunnable::run()
{
  r.run();
}

void AardArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

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

    headword = chain[ x ].word;
    try
    {
      dict.loadArticle( chain[ x ].articleOffset, articleText );
    }
    catch(...)
    {
    }

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

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
  }

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > AardDictionary::getArticle( wstring const & word,
                                                            vector< wstring > const & alts,
                                                            wstring const & )
  throw( std::exception )
{
  return new AardArticleRequest( word, alts, *this );
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
      // Skip files with the extensions different to .aar to speed up the
      // scanning
      if ( i->size() < 4 ||
          strcasecmp( i->c_str() + ( i->size() - 4 ), ".aar" ) != 0 )
        continue;

      // Got the file -- check if we need to rebuid the index

      vector< string > dictFiles( 1, *i );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        try
        {
          {
            QFileInfo info( FsEncoding::decode( i->c_str() ) );
            if( info.size() > ULONG_MAX )
            {
              DPRINTF( "File %s is too large", i->c_str() );
              continue;
            }
          }

          File::Class df( *i, "rb" );

          AAR_header dictHeader;

          df.read( &dictHeader, sizeof(dictHeader) );
          bool has64bitIndex = !strncmp( dictHeader.indexItemFormat, ">LQ", 4 );
          if( strncmp( dictHeader.signature, "aard", 4 )
              || ( !has64bitIndex && strncmp( dictHeader.indexItemFormat, ">LL", 4 ) )
              || strncmp( dictHeader.keyLengthFormat, ">H", 2 )
              || strncmp( dictHeader.articleLengthFormat, ">L", 2) )
          {
              DPRINTF( "File %s is not in supported aard format", i->c_str() );
              continue;
          }

          vector< char > data;
          uint32_t size = dictHeader.metaLength;

          if( size == 0 )
          {
              DPRINTF( "File %s has invalid metadata", i->c_str() );
              continue;
          }

          data.resize( size );
          df.read( &data.front(), size );
          string metaStr = decompressBzip2( data.data(), size );
          if( metaStr.empty() )
              metaStr = decompressZlib( data.data(), size );

          map< string, string > meta = parseMetaData( metaStr );

          if( meta.empty() )
          {
              DPRINTF( "File %s has invalid metadata", i->c_str() );
              continue;
          }

          string dictName;
          map< string, string >::const_iterator iter = meta.find( "title" );
          if( iter != meta.end() )
              dictName = iter->second;

          string langFrom;
          iter = meta.find( "index_language" );
          if( iter != meta.end() )
              langFrom = iter->second;

          string langTo;
          iter = meta.find( "article_language" );
          if( iter != meta.end() )
              langTo = iter->second;

          if( ( dictName.compare( "Wikipedia") == 0 || dictName.compare( "Wikiquote" ) == 0 )
              && !langTo.empty() )
          {
            dictName = dictName + " (" + langTo + ")";
          }

          uint16_t volumes = dictHeader.totalVolumes;
          if( volumes > 1 )
          {
            QString ss;
            ss.sprintf( " (%i/%i)", (uint16_t)(dictHeader.volume), volumes );
            dictName += ss.toLocal8Bit().data();
          }

          initializing.indexingDictionary( dictName );

          File::Class idx( indexFile, "wb" );
          IdxHeader idxHeader;
          memset( &idxHeader, 0, sizeof( idxHeader ) );

          // We write a dummy header first. At the end of the process the header
          // will be rewritten with the right values.

          idx.write( idxHeader );

          idx.write( (uint32_t) dictName.size() );
          if( !dictName.empty() )
              idx.write( dictName.data(), dictName.size() );

          IndexedWords indexedWords;

          ChunkedStorage::Writer chunks( idx );

          uint32_t wordCount = dictHeader.wordsCount;
          set< uint32_t > articleOffsets;
          uint32_t pos = df.tell();
          uint32_t wordsBase = pos + wordCount * ( has64bitIndex ? sizeof( IndexElement64 ) : sizeof( IndexElement ) );
          uint32_t articlesBase = dictHeader.articleOffset;

          for( uint32_t j = 0; j < wordCount; j++ )
          {
            uint32_t articleOffset;
            uint32_t wordOffset;

            if( has64bitIndex )
            {
              IndexElement64 el64;

              df.seek( pos );
              df.read( &el64, sizeof(el64) );
              articleOffset = articlesBase + el64.articleOffset;
              wordOffset = wordsBase + el64.wordOffset;
            }
            else
            {
              IndexElement el;

              df.seek( pos );
              df.read( &el, sizeof(el) );
              articleOffset = articlesBase + el.articleOffset;
              wordOffset = wordsBase + el.wordOffset;
            }

            df.seek( wordOffset );

            uint16_be sizeBE;
            df.read( &sizeBE, sizeof(sizeBE) );
            uint16_t wordSize = sizeBE;
            data.resize( wordSize );
            df.read( &data.front(), wordSize );

            if( articleOffsets.find( articleOffset ) == articleOffsets.end() )
                articleOffsets.insert( articleOffset );

            // Insert new entry
            indexedWords.addWord( Utf8::decode( string( data.data(), wordSize ) ), articleOffset);

            pos += has64bitIndex ? sizeof( IndexElement64 ) : sizeof( IndexElement );
          }
          // Finish with the chunks

          idxHeader.chunksOffset = chunks.finish();

          // Build index

          IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

          idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
          idxHeader.indexRootOffset = idxInfo.rootOffset;

          indexedWords.clear(); // Release memory -- no need for this data

          // That concludes it. Update the header.

          idxHeader.signature = Signature;
          idxHeader.formatVersion = CurrentFormatVersion;

          idxHeader.articleCount = articleOffsets.size();
          idxHeader.wordCount = wordCount;

          if( langFrom.size() == 3)
              idxHeader.langFrom = LangCoder::code3toInt( langFrom.c_str() );
          else if( langFrom.size() == 2 )
              idxHeader.langFrom = LangCoder::code2toInt( langFrom.c_str() );

          if( langTo.size() == 3)
              idxHeader.langTo = LangCoder::code3toInt( langTo.c_str() );
          else if( langTo.size() == 2 )
              idxHeader.langTo = LangCoder::code2toInt( langTo.c_str() );

          idx.rewind();

          idx.write( &idxHeader, sizeof( idxHeader ) );
        }
        catch( std::exception & e )
        {
          FDPRINTF( stderr, "Aard dictionary indexing failed: %s, error: %s\n",
            i->c_str(), e.what() );
          continue;
        }
        catch( ... )
        {
          FDPRINTF( stderr, "Aard dictionary indexing failed\n" );
          continue;
        }
      } // if need to rebuild
      dictionaries.push_back( new AardDictionary( dictId,
                                                   indexFile,
                                                   dictFiles ) );
  }
  return dictionaries;
}

}
