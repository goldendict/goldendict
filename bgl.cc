/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "bgl.hh"
#include "btreeidx.hh"
#include "bgl_babylon.hh"
#include "file.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "langcoder.hh"
#include "language.hh"
#include "dprintf.hh"
#include "fsencoding.hh"
#include "htmlescape.hh"

#include <map>
#include <set>
#include <list>
#include <zlib.h>
#include <ctype.h>
#include <string.h>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QDebug>

#include <QRegExp>

namespace Bgl {

using std::map;
using std::multimap;
using std::set;
using gd::wstring;
using gd::wchar;
using std::list;
using std::pair;
using std::string;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace
{
  enum
  {
    Signature = 0x584c4742, // BGLX on little-endian, XLGB on big-endian
    CurrentFormatVersion = 19 + BtreeIndexing::FormatVersion
  };

  struct IdxHeader
  {
    uint32_t signature; // First comes the signature, BGLX
    uint32_t formatVersion; // File format version, currently 1.
    uint32_t parserVersion; // Version of the parser used to parse the BGL file.
                            // If it's lower than the current one, the file is to
                            // be re-parsed.
    uint32_t foldingVersion; // Version of the folding algorithm used when building
                             // index. If it's different from the current one,
                             // the file is to be rebuilt.
    uint32_t articleCount; // Total number of articles, for informative purposes only
    uint32_t wordCount; // Total number of words, for informative purposes only
    /// Add more fields here, like name, description, author and such.
    uint32_t chunksOffset; // The offset to chunks' storage
    uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
    uint32_t indexRootOffset;
    uint32_t resourceListOffset; // The offset of the list of resources
    uint32_t resourcesCount; // Number of resources stored
    uint32_t langFrom;  // Source language
    uint32_t langTo;    // Target language
    uint32_t iconAddress; // Address of the icon in the chunks' storage
    uint32_t iconSize; // Size of the icon in the chunks' storage, 0 = no icon
    uint32_t descriptionAddress; // Address of the dictionary description in the chunks' storage
    uint32_t descriptionSize; // Size of the description in the chunks' storage, 0 = no description
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
           header.formatVersion != CurrentFormatVersion ||
           header.parserVersion != Babylon::ParserVersion ||
           header.foldingVersion != Folding::Version;
  }

  // Removes the $1$-like postfix
  string removePostfix( string const & in )
  {
    if ( in.size() && in[ in.size() - 1 ] == '$' )
    {
      // Find the end of it and cut it, barring any unexpectedness
      for( long x = in.size() - 2; x >= 0; x-- )
      {
        if ( in[ x ] == '$' )
          return in.substr( 0, x );
        else
        if ( !isdigit( in[ x ] ) )
          break;
      }
    }

    return in;
  }

  // Since the standard isspace() is locale-specific, we need something
  // that would never mess up our utf8 input. The stock one worked fine under
  // Linux but was messing up strings under Windows.
  bool isspace_c( int c )
  {
    switch( c )
    {
      case ' ':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
      case '\v':
        return true;

      default:
        return false;
    }
  }

  // Removes any leading or trailing whitespace
  void trimWs( string & word )
  {
    if ( word.size() )
    {
      unsigned begin = 0;

      while( begin < word.size() && isspace_c( word[ begin ] ) )
        ++begin;

      if ( begin == word.size() ) // Consists of ws entirely?
        word.clear();
      else
      {
        unsigned end = word.size();

        // Doesn't consist of ws entirely, so must end with just isspace()
        // condition.
        while( isspace_c( word[ end - 1 ] ) )
          --end;

        if ( end != word.size() || begin )
          word = string( word, begin, end - begin );
      }
    }
  }

  void addEntryToIndex( string & word,
                        uint32_t articleOffset,
                        IndexedWords & indexedWords,
                        vector< wchar > & wcharBuffer )
  {
    // Strip any leading or trailing whitespaces
    trimWs( word );

    // If the word starts with a slash, we drop it. There are quite a lot
    // of them, and they all seem to be redudant duplicates.

    if ( word.size() && word[ 0 ] == '/' )
      return;

    // Check the input word for a superscript postfix ($1$, $2$ etc), which
    // signifies different meaning in Bgl files. We emit different meaning
    // as different articles, but they appear in the index as the same word.

    if ( word.size() && word[ word.size() - 1 ] == '$' )
    {
      word = removePostfix( word );
      trimWs( word );
    }

    // Convert the word from utf8 to wide chars

    if ( wcharBuffer.size() <= word.size() )
      wcharBuffer.resize( word.size() + 1 );

    long result = Utf8::decode( word.c_str(), word.size(),
                                &wcharBuffer.front() );

    if ( result < 0 )
    {
      qWarning( "Failed to decode utf8 of headword \"%s\", skipping it.", word.c_str() );
      return;
    }

    indexedWords.addWord( wstring( &wcharBuffer.front(), result ), articleOffset );
  }


  DEF_EX( exFailedToDecompressArticle, "Failed to decompress article's body", Dictionary::Ex )
  DEF_EX( exChunkIndexOutOfRange, "Chunk index is out of range", Dictionary::Ex )

  class BglDictionary: public BtreeIndexing::BtreeDictionary
  {
    Mutex idxMutex;
    File::Class idx;
    IdxHeader idxHeader;
    string dictionaryName;
    ChunkedStorage::Reader chunks;

  public:

    BglDictionary( string const & id, string const & indexFile,
                   string const & dictionaryFile );

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

    virtual sptr< Dictionary::WordSearchRequest > findHeadwordsForSynonym( wstring const & )
      throw( std::exception );

    virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                        vector< wstring > const & alts,
                                                        wstring const & )
      throw( std::exception );

    virtual sptr< Dictionary::DataRequest > getResource( string const & name )
      throw( std::exception );

    virtual QString const& getDescription();

  protected:

    virtual void loadIcon() throw();

  private:


    /// Loads an article with the given offset, filling the given strings.
    void loadArticle( uint32_t offset, string & headword,
                      string & displayedHeadword, string & articleText );

    static void replaceCharsetEntities( string & );

    friend class BglHeadwordsRequest;
    friend class BglArticleRequest;
    friend class BglResourceRequest;
  };

  BglDictionary::BglDictionary( string const & id, string const & indexFile,
                                string const & dictionaryFile ):
    BtreeDictionary( id, vector< string >( 1, dictionaryFile ) ),
    idx( indexFile, "rb" ),
    idxHeader( idx.read< IdxHeader >() ),
    chunks( idx, idxHeader.chunksOffset )
  {
    idx.seek( sizeof( idxHeader ) );

    // Read the dictionary's name

    size_t len = idx.read< uint32_t >();

    vector< char > nameBuf( len );

    idx.read( &nameBuf.front(), len );

    dictionaryName = string( &nameBuf.front(), len );

    // Initialize the index

    openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
               idx, idxMutex );
  }

  void BglDictionary::loadIcon() throw()
  {
    if ( dictionaryIconLoaded )
      return;

    QString fileName =
      QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

    // Remove the extension
    fileName.chop( 3 );

    if( !loadIconFromFile( fileName ) )
    {
      if( idxHeader.iconSize )
      {

        // Try loading icon now

        vector< char > chunk;

        Mutex::Lock _( idxMutex );

        char * iconData = chunks.getBlock( idxHeader.iconAddress, chunk );

        QImage img;

        if (img.loadFromData( ( unsigned char *) iconData, idxHeader.iconSize  ) )
        {
          // Load successful

          dictionaryNativeIcon = QIcon( QPixmap::fromImage( img ) );

          // Transform it to be square
          int max = img.width() > img.height() ? img.width() : img.height();

          QImage result( max, max, QImage::Format_ARGB32 );
          result.fill( 0 ); // Black transparent

          QPainter painter( &result );

          painter.drawImage( QPoint( img.width() == max ? 0 : ( max - img.width() ) / 2,
                                     img.height() == max ? 0 : ( max - img.height() ) / 2 ),
                             img );

          painter.end();

          dictionaryIcon = QIcon( QPixmap::fromImage( result ) );
        }
      }

      if ( dictionaryIcon.isNull() )
        dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/icon32_bgl.png");
    }

    dictionaryIconLoaded = true;
  }

  void BglDictionary::loadArticle( uint32_t offset, string & headword,
                                   string & displayedHeadword,
                                   string & articleText )
  {
    vector< char > chunk;

    Mutex::Lock _( idxMutex );

    char * articleData = chunks.getBlock( offset, chunk );

    headword = articleData;

    displayedHeadword = articleData + headword.size() + 1;

    articleText =
      string( articleData + headword.size() +
                displayedHeadword.size() + 2 );
  }

  QString const& BglDictionary::getDescription()
  {
    if( !dictionaryDescription.isEmpty() )
      return dictionaryDescription;

    if( idxHeader.descriptionSize == 0 )
      dictionaryDescription = "NONE";
    else
    {
      vector< char > chunk;
      char * dictDescription = chunks.getBlock( idxHeader.descriptionAddress, chunk );
      string str( dictDescription );
      if( !str.empty() )
        dictionaryDescription += "Copyright: " + Html::unescape( QString::fromUtf8( str.data(), str.size() ) ) + "\n\n";
      dictDescription += str.size() + 1;

      str = string( dictDescription );
      if( !str.empty() )
        dictionaryDescription += "Author: " + QString::fromUtf8( str.data(), str.size() ) + "\n\n";
      dictDescription += str.size() + 1;

      str = string( dictDescription );
      if( !str.empty() )
        dictionaryDescription += "E-mail: " + QString::fromUtf8( str.data(), str.size() ) + "\n\n";
      dictDescription += str.size() + 1;

      str = string( dictDescription );
      if( !str.empty() )
        dictionaryDescription += Html::unescape( QString::fromUtf8( str.data(), str.size() ) );
    }

    return dictionaryDescription;
  }

/// BglDictionary::findHeadwordsForSynonym()

class BglHeadwordsRequest;

class BglHeadwordsRequestRunnable: public QRunnable
{
  BglHeadwordsRequest & r;
  QSemaphore & hasExited;

public:

  BglHeadwordsRequestRunnable( BglHeadwordsRequest & r_,
                               QSemaphore & hasExited_ ): r( r_ ),
                                                          hasExited( hasExited_ )
  {}

  ~BglHeadwordsRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class BglHeadwordsRequest: public Dictionary::WordSearchRequest
{
  friend class BglHeadwordsRequestRunnable;

  wstring str;
  BglDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  BglHeadwordsRequest( wstring const & word_,
                       BglDictionary & dict_ ):
    str( word_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new BglHeadwordsRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by BglHeadwordsRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~BglHeadwordsRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void BglHeadwordsRequestRunnable::run()
{
  r.run();
}

void BglHeadwordsRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  vector< WordArticleLink > chain = dict.findArticles( str );

  wstring caseFolded = Folding::applySimpleCaseOnly( str );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( isCancelled )
    {
      finish();
      return;
    }

    string headword, displayedHeadword, articleText;

    dict.loadArticle( chain[ x ].articleOffset,
                      headword, displayedHeadword, articleText );

    wstring headwordDecoded;
    try
    {
      headwordDecoded = Utf8::decode( removePostfix(  headword ) );
    }
    catch( Utf8::exCantDecode )
    {
    }

    if ( caseFolded != Folding::applySimpleCaseOnly( headwordDecoded ) && !headwordDecoded.empty() )
    {
      // The headword seems to differ from the input word, which makes the
      // input word its synonym.
      Mutex::Lock _( dataMutex );

      matches.push_back( headwordDecoded );
    }
  }

  finish();
}

sptr< Dictionary::WordSearchRequest >
  BglDictionary::findHeadwordsForSynonym( wstring const & word )
  throw( std::exception )
{
  return new BglHeadwordsRequest( word, *this );
}

// Converts a $1$-like postfix to a <sup>1</sup> one
string postfixToSuperscript( string const & in )
{
  if ( !in.size() || in[ in.size() - 1 ] != '$' )
    return in;

  for( long x = in.size() - 2; x >= 0; x-- )
  {
    if ( in[ x ] == '$' )
    {
      if ( in.size() - x - 2 > 2 )
      {
        // Large postfixes seem like something we wouldn't want to show --
        // some dictionaries seem to have each word numbered using the
        // postfix.
        return in.substr( 0, x );
      }
      else
        return in.substr( 0, x ) + "<sup>" + in.substr( x + 1, in.size() - x - 2 ) + "</sup>";
    }
    else
    if ( !isdigit( in[ x ] ) )
      break;
  }

  return in;
}


/// BglDictionary::getArticle()

class BglArticleRequest;

class BglArticleRequestRunnable: public QRunnable
{
  BglArticleRequest & r;
  QSemaphore & hasExited;

public:

  BglArticleRequestRunnable( BglArticleRequest & r_,
                                  QSemaphore & hasExited_ ): r( r_ ),
                                                             hasExited( hasExited_ )
  {}

  ~BglArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class BglArticleRequest: public Dictionary::DataRequest
{
  friend class BglArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  BglDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  BglArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     BglDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new BglArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by BglArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  void fixHebString(string & hebStr); // Hebrew support
  void fixHebArticle(string & hebArticle); // Hebrew support

  ~BglArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void BglArticleRequestRunnable::run()
{
  r.run();
}

void BglArticleRequest::fixHebString(string & hebStr) // Hebrew support - convert non-unicode to unicode
{
  wstring hebWStr;
  try
  {
    hebWStr = Utf8::decode(hebStr);
  }
  catch( Utf8::exCantDecode )
  {
    hebStr = "Utf-8 decoding error";
    return;
  }

  for (unsigned int i=0; i<hebWStr.size();i++)
  {
    if (hebWStr[i]>=224 && hebWStr[i]<=250) // Hebrew chars encoded ecoded as windows-1255 or ISO-8859-8
        hebWStr[i]+=1488-224; // Convert to Hebrew unicode
  }
  hebStr=Utf8::encode(hebWStr);
}

void BglArticleRequest::fixHebArticle(string & hebArticle) // Hebrew support - remove extra chars at the end
{
  unsigned nulls;

  for ( nulls = hebArticle.size(); nulls > 0 &&
        ( ( hebArticle[ nulls - 1 ] <= 32 &&
            hebArticle[ nulls - 1 ] >= 0 ) ||
          ( hebArticle[ nulls - 1 ] >= 65 &&
            hebArticle[ nulls - 1 ] <= 90 ) ); --nulls ) ; //special chars and A-Z

  hebArticle.resize( nulls );
}

void BglArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  vector< WordArticleLink > chain = dict.findArticles( word );

  static Language::Id hebrew = LangCoder::code2toInt( "he" ); // Hebrew support

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
  // Sometimes the articles are physically duplicated. We store hashes of
  // the bodies to account for this.
  set< QByteArray > articleBodiesIncluded;

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( isCancelled )
    {
      finish();
      return;
    }

    try
    {

    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword, displayedHeadword, articleText;

    dict.loadArticle( chain[ x ].articleOffset,
                      headword, displayedHeadword, articleText );

    // Ok. Now, does it go to main articles, or to alternate ones? We list
    // main ones first, and alternates after.

    // We do the case-folded and postfix-less comparison here.

    wstring headwordStripped =
      Folding::applySimpleCaseOnly( Utf8::decode( removePostfix( headword ) ) );

	// Hebrew support - fix Hebrew text
    if (dict.idxHeader.langFrom == hebrew)
    {
        displayedHeadword= displayedHeadword.size() ? displayedHeadword : headword;
        fixHebString(articleText);
        fixHebArticle(articleText);
        fixHebString(displayedHeadword);
    }

    string const & targetHeadword = displayedHeadword.size() ?
                                    displayedHeadword : headword;

    QCryptographicHash hash( QCryptographicHash::Md5 );
    hash.addData( targetHeadword.data(), targetHeadword.size() + 1 ); // with 0
    hash.addData( articleText.data(), articleText.size() );

    if ( !articleBodiesIncluded.insert( hash.result() ).second )
      continue; // Already had this body

    multimap< wstring, pair< string, string > > & mapToUse =
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( pair< wstring, pair< string, string > >(
      Folding::applySimpleCaseOnly( Utf8::decode( headword ) ),
      pair< string, string >( targetHeadword, articleText ) ) );

    articlesIncluded.insert( chain[ x ].articleOffset );

    } // try
    catch( Utf8::exCantDecode )
    {
    }
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
      if (dict.isFromLanguageRTL() ) // RTL support
        result += "<h3 style=\"text-align:right;direction:rtl\">";
      else
        result += "<h3>";
      result += postfixToSuperscript( i->second.first );
      result += "</h3>";
      if ( dict.isToLanguageRTL() )
        result += "<div class=\"bglrtl\">" + i->second.second + "</div>";
      else
        result += "<div>" + i->second.second + "</div>";
      result += cleaner;
  }

 
  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      if (dict.isFromLanguageRTL() ) // RTL support
        result += "<h3 style=\"text-align:right;direction:rtl\">";
      else
        result += "<h3>";
      result += postfixToSuperscript( i->second.first );
      result += "</h3>";
      if ( dict.isToLanguageRTL() )
        result += "<div class=\"bglrtl\">" + i->second.second + "</div>";
      else
        result += "<div>" + i->second.second + "</div>";
      result += cleaner;
  }
  // Do some cleanups in the text

  BglDictionary::replaceCharsetEntities( result );
  result = QString::fromUtf8( result.c_str() )
          // onclick location to link
          .replace( QRegExp( "<([a-z0-9]+)\\s+[^>]*onclick=\"[a-z.]*location(?:\\.href)\\s*=\\s*'([^']+)[^>]*>([^<]+)</\\1>", Qt::CaseInsensitive ),
                    "<a href=\"\\2\">\\3</a>")
           .replace( QRegExp( "(<\\s*a\\s+[^>]*href\\s*=\\s*[\"']\\s*)bword://", Qt::CaseInsensitive ),
                     "\\1bword:" )
          //remove invalid width, height attrs
          .replace(QRegExp( "(width)|(height)\\s*=\\s*[\"']\\d{7,}[\"'']" ),
                   "" )
          //remove invalid <br> tag
          .replace( QRegExp( "<br>(<div|<table|<tbody|<tr|<td|</div>|</table>|</tbody>|</tr>|</td>|function addScript|var scNode|scNode|var atag|while\\(atag|atag=atag|document\\.getElementsByTagName|addScript|src=\"bres|<a onmouseover=\"return overlib|onclick=\"return overlib)", Qt::CaseInsensitive ),
                    "\\1" )
          .replace( QRegExp( "(AUTOSTATUS, WRAP\\);\" |</DIV>|addScript\\('JS_FILE_PHONG_VT_45634'\\);|appendChild\\(scNode\\);|atag\\.firstChild;)<br>", Qt::CaseInsensitive ),
                    " \\1 " )
           .toUtf8().data();

  Mutex::Lock _( dataMutex );

  data.resize( result.size() );

  memcpy( &data.front(), result.data(), result.size() );

  hasAnyData = true;

  finish();
}

sptr< Dictionary::DataRequest > BglDictionary::getArticle( wstring const & word,
                                                           vector< wstring > const & alts,
                                                           wstring const & )
  throw( std::exception )
{
  return new BglArticleRequest( word, alts, *this );
}


//// BglDictionary::getResource()

class BglResourceRequest;

class BglResourceRequestRunnable: public QRunnable
{
  BglResourceRequest & r;
  QSemaphore & hasExited;

public:

  BglResourceRequestRunnable( BglResourceRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~BglResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class BglResourceRequest: public Dictionary::DataRequest
{
  friend class BglResourceRequestRunnable;

  Mutex & idxMutex;
  File::Class & idx;
  uint32_t resourceListOffset, resourcesCount;
  string name;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  BglResourceRequest( Mutex & idxMutex_,
                      File::Class & idx_,
                      uint32_t resourceListOffset_,
                      uint32_t resourcesCount_,
                      string const & name_ ):
    idxMutex( idxMutex_ ),
    idx( idx_ ),
    resourceListOffset( resourceListOffset_ ),
    resourcesCount( resourcesCount_ ),
    name( name_ )
  {
    QThreadPool::globalInstance()->start(
      new BglResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by BglResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~BglResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void BglResourceRequestRunnable::run()
{
  r.run();
}

void BglResourceRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  string nameLowercased = name;

  for( string::iterator i = nameLowercased.begin(); i != nameLowercased.end();
       ++i )
    *i = tolower( *i );

  Mutex::Lock _( idxMutex );

  idx.seek( resourceListOffset );

  for( size_t count = resourcesCount; count--; )
  {
    if ( isCancelled )
      break;

    vector< char > nameData( idx.read< uint32_t >() );
    idx.read( &nameData.front(), nameData.size() );

    for( size_t x = nameData.size(); x--; )
      nameData[ x ] = tolower( nameData[ x ] );

    uint32_t offset = idx.read< uint32_t >();

    if ( string( &nameData.front(), nameData.size() ) == nameLowercased )
    {
      // We have a match.

      idx.seek( offset );

      Mutex::Lock _( dataMutex );

      data.resize( idx.read< uint32_t >() );

      vector< unsigned char > compressedData( idx.read< uint32_t >() );

      idx.read( &compressedData.front(), compressedData.size() );

      unsigned long decompressedLength = data.size();

      if ( uncompress( (unsigned char *) &data.front(),
                       &decompressedLength,
                       &compressedData.front(),
                       compressedData.size() ) != Z_OK ||
           decompressedLength != data.size() )
      {
        qWarning( "Failed to decompress resource \"%s\", ignoring it.\n", name.c_str() );
      }
      else
        hasAnyData = true;

      break;
    }
  }

  finish();
}

sptr< Dictionary::DataRequest > BglDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new BglResourceRequest( idxMutex, idx, idxHeader.resourceListOffset,
                                 idxHeader.resourcesCount, name );
}

  /// Replaces <CHARSET c="t">1234;</CHARSET> occurences with &#x1234;
  void BglDictionary::replaceCharsetEntities( string & text )
  {
    QRegExp charsetExp( "<\\s*charset\\s+c\\s*=\\s*[\"']?t[\"']?\\s*>((?:\\s*[0-9a-fA-F]+\\s*;\\s*)*)<\\s*/\\s*charset\\s*>",
                        Qt::CaseInsensitive );

    charsetExp.setMinimal( true );
    
    QRegExp oneValueExp( "\\s*([0-9a-fA-F]+)\\s*;" );

    QString str = QString::fromUtf8( text.c_str() );
    
    for( int pos = 0; ( pos = charsetExp.indexIn( str, pos ) ) != -1; )
    {
      //DPRINTF( "Match: %s\n", str.mid( pos, charsetExp.matchedLength() ).toUtf8().data() );
      
      QString out;

      for( int p = 0; ( p = oneValueExp.indexIn( charsetExp.cap( 1 ), p ) ) != -1; )
      {
        //DPRINTF( "Cap: %s\n", oneValueExp.cap( 1 ).toUtf8().data() );
        out += "&#x" + oneValueExp.cap( 1 ) + ";";

        p += oneValueExp.matchedLength();
      }

      str.replace( pos, charsetExp.matchedLength(), out );
    }

    text = str.toUtf8().data();
  }

  class ResourceHandler: public Babylon::ResourceHandler
  {
    File::Class & idxFile;
    list< pair< string, uint32_t > > resources;

  public:

    ResourceHandler( File::Class & idxFile_ ): idxFile( idxFile_ )
    {}

    list< pair< string, uint32_t > > const & getResources() const
    { return resources; }

  protected:
    virtual void handleBabylonResource( string const & filename,
                                        char const * data, size_t size );
  };

  void ResourceHandler::handleBabylonResource( string const & filename,
                                               char const * data, size_t size )
  {
    //DPRINTF( "Handling resource file %s (%u bytes)\n", filename.c_str(), size );

    vector< unsigned char > compressedData( compressBound( size ) );

    unsigned long compressedSize = compressedData.size();

    if ( compress( &compressedData.front(), &compressedSize,
                   (unsigned char const *) data, size ) != Z_OK )
    {
      qWarning( "Failed to compress the body of resource \"%s\", dropping it.\n", filename.c_str() );
      return;
    }

    resources.push_back( pair< string, uint32_t >( filename, idxFile.tell() ) );

    idxFile.write< uint32_t >( size );
    idxFile.write< uint32_t >( compressedSize );
    idxFile.write( &compressedData.front(), compressedSize );
  }
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
    // Skip files with the extensions different to .bgl to speed up the
    // scanning
    if ( i->size() < 4 ||
        strcasecmp( i->c_str() + ( i->size() - 4 ), ".bgl" ) != 0 )
      continue;

    // Got the file -- check if we need to rebuid the index

    vector< string > dictFiles( 1, *i );

    string dictId = Dictionary::makeDictionaryId( dictFiles );

    string indexFile = indicesDir + dictId;

    if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
         indexIsOldOrBad( indexFile ) )
    {
      // Building the index

      qDebug() << "Bgl: Building the index for dictionary: " << i->c_str();

      Babylon b( *i );

      if ( !b.open() )
        continue;

      std::string sourceCharset, targetCharset;

      if ( !b.read( sourceCharset, targetCharset ) )
      {
        qWarning( "Failed to start reading from %s, skipping it\n", i->c_str() );
        continue;
      }

      initializing.indexingDictionary( b.title() );

      File::Class idx( indexFile, "wb" );

      IdxHeader idxHeader;

      memset( &idxHeader, 0, sizeof( idxHeader ) );

      // We write a dummy header first. At the end of the process the header
      // will be rewritten with the right values.

      idx.write( idxHeader );

      idx.write< uint32_t >( b.title().size() );
      idx.write( b.title().data(), b.title().size() );

      // This is our index data that we accumulate during the loading process.
      // For each new word encountered, we emit the article's body to the file
      // immediately, inserting the word itself and its offset in this map.
      // This map maps folded words to the original words and the corresponding
      // articles' offsets.
      IndexedWords indexedWords;

      // We use this buffer to decode utf8 into it.
      vector< wchar > wcharBuffer;

      ChunkedStorage::Writer chunks( idx );

      uint32_t articleCount = 0, wordCount = 0;

      ResourceHandler resourceHandler( idx );

      b.setResourcePrefix( string( "bres://" ) + dictId + "/" );

      // Save icon if there's one
      if ( size_t sz = b.getIcon().size() )
      {
        idxHeader.iconAddress = chunks.startNewBlock();
        chunks.addToBlock( &b.getIcon().front(), sz );
        idxHeader.iconSize = sz;
      }
      
      // Save dictionary description if there's one
      idxHeader.descriptionSize = 0;
      idxHeader.descriptionAddress = chunks.startNewBlock();

      chunks.addToBlock( b.copyright().c_str(), b.copyright().size() + 1 );
      idxHeader.descriptionSize += b.copyright().size() + 1;

      chunks.addToBlock( b.author().c_str(), b.author().size() + 1 );
      idxHeader.descriptionSize += b.author().size() + 1;

      chunks.addToBlock( b.email().c_str(), b.email().size() + 1 );
      idxHeader.descriptionSize += b.email().size() + 1;

      chunks.addToBlock( b.description().c_str(), b.description().size() + 1 );
      idxHeader.descriptionSize += b.description().size() + 1;

      for( ; ; )
      {
        bgl_entry e = b.readEntry( &resourceHandler );

        if ( e.headword.empty() )
          break;

        // Save the article's body itself first

        uint32_t articleAddress = chunks.startNewBlock();

        chunks.addToBlock( e.headword.c_str(), e.headword.size() + 1 );
        chunks.addToBlock( e.displayedHeadword.c_str(), e.displayedHeadword.size() + 1 );
        chunks.addToBlock( e.definition.c_str(), e.definition.size() + 1 );

        // Add entries to the index

        addEntryToIndex( e.headword, articleAddress, indexedWords, wcharBuffer );

        for( unsigned x = 0; x < e.alternates.size(); ++x )
          addEntryToIndex( e.alternates[ x ], articleAddress, indexedWords, wcharBuffer );

        wordCount += 1 + e.alternates.size();
        ++articleCount;
      }

      // Finish with the chunks

      idxHeader.chunksOffset = chunks.finish();

      DPRINTF( "Writing index...\n" );

      // Good. Now build the index

      IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

      idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
      idxHeader.indexRootOffset = idxInfo.rootOffset;

      // Save the resource's list.

      idxHeader.resourceListOffset = idx.tell();
      idxHeader.resourcesCount = resourceHandler.getResources().size();

      for( list< pair< string, uint32_t > >::const_iterator j =
          resourceHandler.getResources().begin();
           j != resourceHandler.getResources().end(); ++j )
      {
        idx.write< uint32_t >( j->first.size() );
        idx.write( j->first.data(), j->first.size() );
        idx.write< uint32_t >( j->second );
      }

      // That concludes it. Update the header.

      idxHeader.signature = Signature;
      idxHeader.formatVersion = CurrentFormatVersion;
      idxHeader.parserVersion = Babylon::ParserVersion;
      idxHeader.foldingVersion = Folding::Version;
      idxHeader.articleCount = articleCount;
      idxHeader.wordCount = wordCount;
      idxHeader.langFrom = b.sourceLang();//LangCoder::findIdForLanguage( Utf8::decode( b.sourceLang() ) );
      idxHeader.langTo = b.targetLang();//LangCoder::findIdForLanguage( Utf8::decode( b.targetLang() ) );

      idx.rewind();

      idx.write( &idxHeader, sizeof( idxHeader ) );
    }

    dictionaries.push_back( new BglDictionary( dictId,
                                               indexFile,
                                               *i ) );
  }

  return dictionaries;
}

}
