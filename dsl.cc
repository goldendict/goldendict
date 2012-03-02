/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dsl.hh"
#include "dsl_details.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "dictzip.h"
#include "htmlescape.hh"
#include "iconv.hh"
#include "filetype.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "langcoder.hh"
#include "wstring_qt.hh"
#include "zipfile.hh"
#include "indexedzip.hh"
#include "dprintf.hh"

#include <zlib.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <wctype.h>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QSemaphore>
#include <QThreadPool>
#include <QAtomicInt>
#include <QUrl>

#include <QDir>
#include <QFileInfo>
#include <QPainter>

// For TIFF conversion
#include <QImage>
#include <QByteArray>
#include <QBuffer>

namespace Dsl {

using namespace Details;

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using gd::wstring;
using gd::wchar;
using std::vector;
using std::list;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )

enum
{
  Signature = 0x584c5344, // DSLX on little-endian, XLSD on big-endian
  CurrentFormatVersion = 15 + BtreeIndexing::FormatVersion + Folding::Version,
  CurrentZipSupportVersion = 2
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, DSLX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t zipSupportVersion; // Zip support version -- narrows down reindexing
                              // when it changes only for dictionaries with the
                              // zip files
  int dslEncoding; // Which encoding is used for the file indexed
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t hasAbrv; // Non-zero means file has abrvs at abrvAddress
  uint32_t abrvAddress; // Address of abrv map in the chunked storage
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t articleCount; // Number of articles this dictionary has
  uint32_t wordCount; // Number of headwords this dictionary has
  uint32_t langFrom;  // Source language
  uint32_t langTo;    // Target language
  uint32_t hasZipFile; // Non-zero means there's a zip file with resources
                       // present
  uint32_t zipIndexBtreeMaxElements; // Two fields from IndexInfo of the zip
                                     // resource index.
  uint32_t zipIndexRootOffset;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

bool indexIsOldOrBad( string const & indexFile, bool hasZipFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion ||
         (bool) header.hasZipFile != hasZipFile ||
         ( hasZipFile && header.zipSupportVersion != CurrentZipSupportVersion );
}

class DslDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  sptr< ChunkedStorage::Reader > chunks;
  string dictionaryName;
  map< string, string > abrv;
  Mutex dzMutex;
  dictData * dz;
  Mutex resourceZipMutex;
  IndexedZip resourceZip;
  BtreeIndex resourceZipIndex;
  QIcon dictionaryNativeIcon, dictionaryIcon;
  bool dictionaryIconLoaded;

  QAtomicInt deferredInitDone;
  Mutex deferredInitMutex;
  bool deferredInitRunnableStarted;
  QSemaphore deferredInitRunnableExited;

  string initError;

public:

  DslDictionary( string const & id, string const & indexFile,
                 vector< string > const & dictionaryFiles );

  virtual void deferredInit();

  ~DslDictionary();

  virtual string getName() throw()
  { return dictionaryName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.articleCount; }

  virtual unsigned long getWordCount() throw()
  { return idxHeader.wordCount; }

  virtual QIcon getIcon() throw();

  virtual QIcon getNativeIcon() throw();

  inline virtual quint32 getLangFrom() const
  { return idxHeader.langFrom; }

  inline virtual quint32 getLangTo() const
  { return idxHeader.langTo; }

  #if 0
  virtual vector< wstring > findHeadwordsForSynonym( wstring const & )
    throw( std::exception )
  {
    return vector< wstring >();
  }
  #endif

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

private:

  virtual string const & ensureInitDone();
  void doDeferredInit();

  void loadIcon();

  /// Loads the article. Does not process the DSL language.
  void loadArticle( uint32_t address,
                    wstring const & requestedHeadwordFolded,
                    wstring & tildeValue,
                    wstring & displayedHeadword,
                    unsigned & headwordIndex,
                    wstring & articleText );

  /// Converts DSL language to an Html.
  string dslToHtml( wstring const & );

  // Parts of dslToHtml()
  string nodeToHtml( ArticleDom::Node const & );
  string processNodeChildren( ArticleDom::Node const & node );

  friend class DslArticleRequest;
  friend class DslResourceRequest;
  friend class DslDeferredInitRunnable;
};

DslDictionary::DslDictionary( string const & id,
                              string const & indexFile,
                              vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  dz( 0 ),
  dictionaryIconLoaded( false ),
  deferredInitRunnableStarted( false )
{
  // Read the dictionary name

  idx.seek( sizeof( idxHeader ) );

  vector< char > dName( idx.read< uint32_t >() );
  idx.read( &dName.front(), dName.size() );
  dictionaryName = string( &dName.front(), dName.size() );

  // Everything else would be done in deferred init
}

DslDictionary::~DslDictionary()
{
  Mutex::Lock _( deferredInitMutex );

  // Wait for init runnable to complete if it was ever started
  if ( deferredInitRunnableStarted )
    deferredInitRunnableExited.acquire();

  if ( dz )
    dict_data_close( dz );
}

//////// DslDictionary::deferredInit()

class DslDeferredInitRunnable: public QRunnable
{
  DslDictionary & dictionary;
  QSemaphore & hasExited;

public:

  DslDeferredInitRunnable( DslDictionary & dictionary_,
                           QSemaphore & hasExited_ ):
    dictionary( dictionary_ ), hasExited( hasExited_ )
  {}

  ~DslDeferredInitRunnable()
  {
    hasExited.release();
  }

  virtual void run()
  {
    dictionary.doDeferredInit();
  }
};

void DslDictionary::deferredInit()
{
  if ( !deferredInitDone )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( deferredInitDone )
      return;

    if ( !deferredInitRunnableStarted )
    {
      QThreadPool::globalInstance()->start(
        new DslDeferredInitRunnable( *this, deferredInitRunnableExited ),
        -1000 );
      deferredInitRunnableStarted = true;
    }
  }
}


string const & DslDictionary::ensureInitDone()
{
  // Simple, really.
  doDeferredInit();

  return initError;
}

void DslDictionary::doDeferredInit()
{
  if ( !deferredInitDone )
  {
    Mutex::Lock _( deferredInitMutex );

    if ( deferredInitDone )
      return;

    // Do deferred init

    try
    {
      // Don't lock index file - no one should be working with it until
      // the init is complete.
      //Mutex::Lock _( idxMutex );

      chunks = new ChunkedStorage::Reader( idx, idxHeader.chunksOffset );

      // Open the .dict file

      dz = dict_data_open( getDictionaryFilenames()[ 0 ].c_str(), 0 );

      if ( !dz )
        throw exCantReadFile( getDictionaryFilenames()[ 0 ] );

      // Read the abrv, if any

      if ( idxHeader.hasAbrv )
      {
        vector< char > chunk;

        char * abrvBlock = chunks->getBlock( idxHeader.abrvAddress, chunk );

        uint32_t total;
        memcpy( &total, abrvBlock, sizeof( uint32_t ) );
        abrvBlock += sizeof( uint32_t );

        DPRINTF( "Loading %u abbrv\n", total );

        while( total-- )
        {
          uint32_t keySz;
          memcpy( &keySz, abrvBlock, sizeof( uint32_t ) );
          abrvBlock += sizeof( uint32_t );

          char * key = abrvBlock;

          abrvBlock += keySz;

          uint32_t valueSz;
          memcpy( &valueSz, abrvBlock, sizeof( uint32_t ) );
          abrvBlock += sizeof( uint32_t );

          abrv[ string( key, keySz ) ] = string( abrvBlock, valueSz );

          abrvBlock += valueSz;
        }
      }

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
    catch( std::exception & e )
    {
      initError = e.what();
    }
    catch( ... )
    {
      initError = "Unknown error";
    }

    deferredInitDone.ref();
  }
}



QIcon DslDictionary::getNativeIcon() throw()
{
  loadIcon();

  return dictionaryNativeIcon;
}

QIcon DslDictionary::getIcon() throw()
{
  loadIcon();

  return dictionaryIcon;
}

void DslDictionary::loadIcon()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension

  QString lc = fileName.toLower();

  if ( fileName.endsWith( ".dsl.dz", Qt::CaseInsensitive ) )
    fileName.chop( 6 );
  else
    fileName.chop( 3 );

  fileName += "bmp";

  QFileInfo info( fileName );

  if ( info.exists() )
  {
    QImage img( fileName );

    if ( !img.isNull() )
    {
      // Load successful

      // Apply the color key

      img.setAlphaChannel( img.createMaskFromColor( QColor( 192, 192, 192 ).rgb(),
                                                    Qt::MaskOutColor ) );

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
  {
    // Load failed -- use default icons

    dictionaryIcon = QIcon(":/icons/icon32_dsl.png");
    dictionaryNativeIcon = QIcon(":/icons/icon_dsl_native.png");
  }

  dictionaryIconLoaded = true;
}

/// Determines whether or not this char is treated as whitespace for dsl
/// parsing or not. We can't rely on any Unicode standards here, since the
/// only standard that matters here is the original Dsl compiler's insides.
/// Some dictionaries, for instance, are known to specifically use a non-
/// breakable space (0xa0) to indicate that a headword begins with a space,
/// so nbsp is not a whitespace character for Dsl compiler.
/// For now we have only space and tab, since those are most likely the only
/// ones recognized as spaces by that compiler.
bool isDslWs( wchar ch )
{
  switch( ch )
  {
    case ' ':
    case '\t':
      return true;
    default:
      return false;
  }
}

void DslDictionary::loadArticle( uint32_t address,
                                 wstring const & requestedHeadwordFolded,
                                 wstring & tildeValue,
                                 wstring & displayedHeadword,
                                 unsigned & headwordIndex,
                                 wstring & articleText )
{
  wstring articleData;

  {
    vector< char > chunk;

    char * articleProps;

    {
      Mutex::Lock _( idxMutex );

      articleProps = chunks->getBlock( address, chunk );
    }

    uint32_t articleOffset, articleSize;

    memcpy( &articleOffset, articleProps, sizeof( articleOffset ) );
    memcpy( &articleSize, articleProps + sizeof( articleOffset ),
            sizeof( articleSize ) );

    DPRINTF( "offset = %x\n", articleOffset );


    char * articleBody;

    {
      Mutex::Lock _( dzMutex );

      articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
    }

    if ( !articleBody )
      throw exCantReadFile( getDictionaryFilenames()[ 0 ] );

    try
    {
      articleData =
        DslIconv::toWstring(
          DslIconv::getEncodingNameFor( DslEncoding( idxHeader.dslEncoding ) ),
          articleBody, articleSize );
      free( articleBody );
    }
    catch( ... )
    {
      free( articleBody );
      throw;
    }
  }

  size_t pos = 0;
  bool hadFirstHeadword = false;
  bool foundDisplayedHeadword = false;

  wstring tildeValueWithUnsorted; // This one has unsorted parts left
  for( headwordIndex = 0; ; )
  {
    size_t begin = pos;

    pos = articleData.find_first_of( GD_NATIVE_TO_WS( L"\n\r" ), begin );

    if ( pos == wstring::npos )
      pos = articleData.size();

    if ( !foundDisplayedHeadword )
    {
      // Process the headword

      wstring rawHeadword = wstring( articleData, begin, pos - begin );

      if ( !hadFirstHeadword )
      {
        // We need our tilde expansion value
        tildeValue = rawHeadword;

        list< wstring > lst;

        expandOptionalParts( tildeValue, lst );

        if ( lst.size() ) // Should always be
          tildeValue = lst.front();

        tildeValueWithUnsorted = tildeValue;

        processUnsortedParts( tildeValue, false );
      }

      wstring str = rawHeadword;

      if ( hadFirstHeadword )
        expandTildes( str, tildeValueWithUnsorted );

      processUnsortedParts( str, true );

      str = Folding::applySimpleCaseOnly( str );

      list< wstring > lst;
      expandOptionalParts( str, lst );

      // Does one of the results match the requested word? If so, we'd choose
      // it as our headword.

      for( list< wstring >::iterator i = lst.begin(); i != lst.end(); ++i )
      {
        unescapeDsl( *i );
        normalizeHeadword( *i );

        if ( Folding::trimWhitespace( *i ) == requestedHeadwordFolded )
        {
          // Found it. Now we should make a displayed headword for it.
          if ( hadFirstHeadword )
            expandTildes( rawHeadword, tildeValueWithUnsorted );

          processUnsortedParts( rawHeadword, false );

          displayedHeadword = rawHeadword;

          foundDisplayedHeadword = true;
          break;
        }
      }

      if ( !foundDisplayedHeadword )
      {
        ++headwordIndex;
        hadFirstHeadword = true;
      }
    }

    if ( pos == articleData.size() )
      break;

    // Skip \n\r

    if ( articleData[ pos ] == '\r' )
      ++pos;

    if ( pos != articleData.size() )
    {
      if ( articleData[ pos ] == '\n' )
        ++pos;
    }

    if ( pos == articleData.size() || isDslWs( articleData[ pos ] ) )
    {
      // Ok, it's either end of article, or the begining of the article's text
      break;
    }
  }

  if ( !foundDisplayedHeadword )
  {
    // This is strange. Anyway, use tilde expansion value, it's better
    // than nothing.
    displayedHeadword = tildeValue;
  }

  if ( pos != articleData.size() )
    articleText = wstring( articleData, pos );
  else
    articleText.clear();
}

string DslDictionary::dslToHtml( wstring const & str )
{
 // Normalize the string
  wstring normalizedStr = gd::toWString( gd::toQString( str ).normalized( QString::NormalizationForm_C ) );

  ArticleDom dom( normalizedStr );

  string html = processNodeChildren( dom.root );

  // Lines seem to indicate paragraphs in Dsls, so we enclose each line within
  // a <p></p>.

  for( size_t x = html.size(); x--; )
    if ( html[ x ] == '\n' )
      html.insert( x + 1, "</p><p>" );

  return
#if 0 // Enable this to enable dsl source in html as a comment
      "<!-- DSL Source:\n" + Utf8::encode( str ) + "\n-->"
#endif
         "<p>" + html + "</p>";
}

string DslDictionary::processNodeChildren( ArticleDom::Node const & node )
{
  string result;

  for( ArticleDom::Node::const_iterator i = node.begin(); i != node.end();
       ++i )
    result += nodeToHtml( *i );

  return result;
}
string DslDictionary::nodeToHtml( ArticleDom::Node const & node )
{
  if ( !node.isTag )
    return Html::escape( Utf8::encode( node.text ) );

  string result;

  if ( node.tagName == GD_NATIVE_TO_WS( L"b" ) )
    result += "<b class=\"dsl_b\">" + processNodeChildren( node ) + "</b>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"i" ) )
    result += "<i class=\"dsl_i\">" + processNodeChildren( node ) + "</i>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"u" ) )
  {
    string nodeText = processNodeChildren( node );

    if ( nodeText.size() && isDslWs( nodeText[ 0 ] ) )
      result.push_back( ' ' ); // Fix a common problem where in "foo[i] bar[/i]"
                               // the space before "bar" gets underlined.

    result += "<span class=\"dsl_u\">" + nodeText + "</span>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"c" ) )
  {
    result += "<font color=\"" + ( node.tagAttrs.size() ?
      Html::escape( Utf8::encode( node.tagAttrs ) ) : string( "c_default_color" ) )
      + "\">" + processNodeChildren( node ) + "</font>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"*" ) )
    result += "<span class=\"dsl_opt\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName.size() == 2 && node.tagName[ 0 ] == L'm' &&
       iswdigit( node.tagName[ 1 ] ) )
    result += "<div class=\"dsl_" + Utf8::encode( node.tagName ) + "\">" + processNodeChildren( node ) + "</div>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"trn" ) )
    result += "<span class=\"dsl_trn\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"ex" ) )
    result += "<span class=\"dsl_ex\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"com" ) )
    result += "<span class=\"dsl_com\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"s" ) )
  {
    string filename = Utf8::encode( node.renderAsText() );

    if ( Filetype::isNameOfSound( filename ) )
    {
      // If we have the file here, do the exact reference to this dictionary.
      // Otherwise, make a global 'search' one.

      string n =
        FsEncoding::dirname( getDictionaryFilenames()[ 0 ] ) +
        FsEncoding::separator() +
        FsEncoding::encode( filename );

      bool search =

        !File::exists( n ) && !File::exists( getDictionaryFilenames()[ 0 ] + ".files" +
                                             FsEncoding::separator() +
                                             FsEncoding::encode( filename ) ) &&
          ( !resourceZip.isOpen() ||
            !resourceZip.hasFile( Utf8::decode( filename ) ) );

      QUrl url;
      url.setScheme( "gdau" );
      url.setHost( QString::fromUtf8( search ? "search" : getId().c_str() ) );
      url.setPath( QString::fromUtf8( filename.c_str() ) );

      string ref = string( "\"" ) + url.toEncoded().data() + "\"";

      result += addAudioLink( ref, getId() );

      result += "<span class=\"dsl_s_wav\"><a href=" + ref
         + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a></span>";
    }
    else
    if ( Filetype::isNameOfPicture( filename ) )
    {
      QUrl url;
      url.setScheme( "bres" );
      url.setHost( QString::fromUtf8( getId().c_str() ) );
      url.setPath( QString::fromUtf8( filename.c_str() ) );

      result += string( "<img src=\"" ) + url.toEncoded().data()
             + "\" alt=\"" + Html::escape( filename ) + "\"/>";
    }
    else
    {
      // Unknown file type, downgrade to a hyperlink

      QUrl url;
      url.setScheme( "bres" );
      url.setHost( QString::fromUtf8( getId().c_str() ) );
      url.setPath( QString::fromUtf8( filename.c_str() ) );

      result += string( "<a class=\"dsl_s\" href=\"" ) + url.toEncoded().data()
             + "\">" + processNodeChildren( node ) + "</a>";
    }
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"url" ) )
    result += "<a class=\"dsl_url\" href=\"" + Html::escape( Utf8::encode( node.renderAsText() ) ) +"\">" + processNodeChildren( node ) + "</a>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"!trs" ) )
    result += "<span class=\"dsl_trs\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"p") )
  {
    result += "<span class=\"dsl_p\"";

    string val = Utf8::encode( node.renderAsText() );

    // If we have such a key, display a title

    map< string, string >::const_iterator i = abrv.find( val );

    if ( i != abrv.end() )
    {
      string title;

      if ( Utf8::decode( i->second ).size() < 70 )
      {
        // Replace all spaces with non-breakable ones, since that's how
        // Lingvo shows tooltips
        title.reserve( i->second.size() );

        for( char const * c = i->second.c_str(); *c; ++c )
          if ( *c == ' ' || *c == '\t' )
          {
            // u00A0 in utf8
            title.push_back( 0xC2 );
            title.push_back( 0xA0 );
          }
          else
            title.push_back( *c );
      }
      else
        title = i->second;

      result += " title=\"" + Html::escape( title ) + "\"";
    }

    result += ">" + processNodeChildren( node ) + "</span>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"'" ) )
  {
    // There are two ways to display the stress: by adding an accent sign or via font styles.
    // We generate two spans, one with accented data and another one without it, so the
    // user could pick up the best suitable option.
    string data = processNodeChildren( node );
    result += "<span class=\"dsl_stress\"><span class=\"dsl_stress_without_accent\">" + data + "</span>"
        + "<span class=\"dsl_stress_with_accent\">" + data + Utf8::encode( wstring( 1, 0x301 ) )
        + "</span></span>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"lang" ) )
  {
    result += "<span class=\"dsl_lang\">" + processNodeChildren( node ) + "</span>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"ref" ) )
  {
    QUrl url;

    url.setScheme( "gdlookup" );
    url.setHost( "localhost" );
    url.setPath( gd::toQString( node.renderAsText() ) );

    result += string( "<a class=\"dsl_ref\" href=\"" ) + url.toEncoded().data() +"\">" + processNodeChildren( node ) + "</a>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"sub" ) )
  {
    result += "<sub>" + processNodeChildren( node ) + "</sub>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"sup" ) )
  {
    result += "<sup>" + processNodeChildren( node ) + "</sup>";
  }
  else
  if ( node.tagName == GD_NATIVE_TO_WS( L"t" ) )
  {
    result += "<span class=\"dsl_t\">" + processNodeChildren( node ) + "</span>";
  }
  else
    result += "<span class=\"dsl_unknown\">" + processNodeChildren( node ) + "</span>";

  return result;
}

#if 0
vector< wstring > StardictDictionary::findHeadwordsForSynonym( wstring const & str )
  throw( std::exception )
{
  vector< wstring > result;

  vector< WordArticleLink > chain = findArticles( str );

  wstring caseFolded = Folding::applySimpleCaseOnly( str );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    string headword, articleText;

    loadArticle( chain[ x ].articleOffset,
                 headword, articleText );

    wstring headwordDecoded = Utf8::decode( headword );

    if ( caseFolded != Folding::applySimpleCaseOnly( headwordDecoded ) )
    {
      // The headword seems to differ from the input word, which makes the
      // input word its synonym.
      result.push_back( headwordDecoded );
    }
  }

  return result;
}
#endif

/// DslDictionary::getArticle()

class DslArticleRequest;

class DslArticleRequestRunnable: public QRunnable
{
  DslArticleRequest & r;
  QSemaphore & hasExited;

public:

  DslArticleRequestRunnable( DslArticleRequest & r_,
                             QSemaphore & hasExited_ ): r( r_ ),
                                                        hasExited( hasExited_ )
  {}

  ~DslArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class DslArticleRequest: public Dictionary::DataRequest
{
  friend class DslArticleRequestRunnable;

  wstring word;
  vector< wstring > alts;
  DslDictionary & dict;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  DslArticleRequest( wstring const & word_,
                     vector< wstring > const & alts_,
                     DslDictionary & dict_ ):
    word( word_ ), alts( alts_ ), dict( dict_ )
  {
    QThreadPool::globalInstance()->start(
      new DslArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by DslArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~DslArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void DslArticleRequestRunnable::run()
{
  r.run();
}

void DslArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  if ( dict.ensureInitDone().size() )
  {
    setErrorString( QString::fromUtf8( dict.ensureInitDone().c_str() ) );
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

  // Some synonyms make it that the articles appear several times. We combat
  // this by only allowing them to appear once. Dsl treats different headwords
  // of the same article as different articles, so we also include headword
  // index here.
  set< pair< uint32_t, unsigned > > articlesIncluded;

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    // Check if we're cancelled occasionally
    if ( isCancelled )
    {
      finish();
      return;
    }

    // Grab that article

    wstring tildeValue;
    wstring displayedHeadword;
    wstring articleBody;
    unsigned headwordIndex;

    dict.loadArticle( chain[ x ].articleOffset, wordCaseFolded, tildeValue,
                      displayedHeadword, headwordIndex, articleBody );

    if ( !articlesIncluded.insert( std::make_pair( chain[ x ].articleOffset,
                                                   headwordIndex ) ).second )
      continue; // We already have this article in the body.

    string articleText;

    articleText += "<span class=\"dsl_article\">";
    articleText += "<div class=\"dsl_headwords\">";

    articleText += dict.dslToHtml( displayedHeadword );

    articleText += "</div>";

    expandTildes( articleBody, tildeValue );

    articleText += "<div class=\"dsl_definition\">";
    articleText += dict.dslToHtml( articleBody );
    articleText += "</div>";
    articleText += "</span>";

    Mutex::Lock _( dataMutex );

    data.resize( data.size() + articleText.size() );

    memcpy( &data.front() + data.size() - articleText.size(),
            articleText.data(), articleText.size() );

    hasAnyData = true;
  }

  finish();
}

sptr< Dictionary::DataRequest > DslDictionary::getArticle( wstring const & word,
                                                           vector< wstring > const & alts,
                                                           wstring const & )
  throw( std::exception )
{
  return new DslArticleRequest( word, alts, *this );
}

//// DslDictionary::getResource()

class DslResourceRequest;

class DslResourceRequestRunnable: public QRunnable
{
  DslResourceRequest & r;
  QSemaphore & hasExited;

public:

  DslResourceRequestRunnable( DslResourceRequest & r_,
                              QSemaphore & hasExited_ ): r( r_ ),
                                                         hasExited( hasExited_ )
  {}

  ~DslResourceRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class DslResourceRequest: public Dictionary::DataRequest
{
  friend class DslResourceRequestRunnable;

  DslDictionary & dict;

  string resourceName;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  DslResourceRequest( DslDictionary & dict_,
                      string const & resourceName_ ):
    dict( dict_ ),
    resourceName( resourceName_ )
  {
    QThreadPool::globalInstance()->start(
      new DslResourceRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by DslResourceRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }

  ~DslResourceRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void DslResourceRequestRunnable::run()
{
  r.run();
}

void DslResourceRequest::run()
{
  // Some runnables linger enough that they are cancelled before they start
  if ( isCancelled )
  {
    finish();
    return;
  }

  if ( dict.ensureInitDone().size() )
  {
    setErrorString( QString::fromUtf8( dict.ensureInitDone().c_str() ) );
    finish();
    return;
  }

  string n =
    FsEncoding::dirname( dict.getDictionaryFilenames()[ 0 ] ) +
    FsEncoding::separator() +
    FsEncoding::encode( resourceName );

  DPRINTF( "n is %s\n", n.c_str() );

  try
  {
    try
    {
      Mutex::Lock _( dataMutex );

      File::loadFromFile( n, data );
    }
    catch( File::exCantOpen & )
    {
      n = dict.getDictionaryFilenames()[ 0 ] + ".files" +
          FsEncoding::separator() +
          FsEncoding::encode( resourceName );

      try
      {
        Mutex::Lock _( dataMutex );

        File::loadFromFile( n, data );
      }
      catch( File::exCantOpen & )
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

    Mutex::Lock _( dataMutex );

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

  finish();
}

sptr< Dictionary::DataRequest > DslDictionary::getResource( string const & name )
  throw( std::exception )
{
  return new DslResourceRequest( *this, name );
}

} // anonymous namespace

#if 0
static void findCorrespondingFiles( string const & ifo,
                                    string & idx, string & dict, string & syn,
                                    bool needSyn )
{
  string base( ifo, 0, ifo.size() - 3 );

  if ( !(
          tryPossibleName( base + "idx", idx ) ||
          tryPossibleName( base + "idx.gz", idx ) ||
          tryPossibleName( base + "idx.dz", idx ) ||
          tryPossibleName( base + "IDX", idx ) ||
          tryPossibleName( base + "IDX.GZ", idx ) ||
          tryPossibleName( base + "IDX.DZ", idx )
      ) )
    throw exNoIdxFile( ifo );

  if ( !(
          tryPossibleName( base + "dict", dict ) ||
          tryPossibleName( base + "dict.dz", dict ) ||
          tryPossibleName( base + "DICT", dict ) ||
          tryPossibleName( base + "dict.DZ", dict )
      ) )
    throw exNoDictFile( ifo );

  if ( needSyn && !(
                     tryPossibleName( base + "syn", syn ) ||
                     tryPossibleName( base + "syn.gz", syn ) ||
                     tryPossibleName( base + "syn.dz", syn ) ||
                     tryPossibleName( base + "SYN", syn ) ||
                     tryPossibleName( base + "SYN.GZ", syn ) ||
                     tryPossibleName( base + "SYN.DZ", syn )
     ) )
    throw exNoSynFile( ifo );
}
#endif

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
    // Try .dsl and .dsl.dz suffixes

    bool uncompressedDsl = ( i->size() >= 4 &&
                             strcasecmp( i->c_str() + ( i->size() - 4 ), ".dsl" ) == 0 );
    if ( !uncompressedDsl &&
         ( i->size() < 7 ||
           strcasecmp( i->c_str() + ( i->size() - 7 ), ".dsl.dz" ) != 0 ) )
      continue;

    // Make sure it's not an abbreviation file

    int extSize = ( uncompressedDsl ? 4 : 7 );
    if ( i->size() - extSize >= 5 &&
         strncasecmp( i->c_str() + i->size() - extSize - 5, "_abrv", 5 ) == 0 )
    {
      // It is, skip it
      continue;
    }

    unsigned atLine = 0; // Indicates current line in .dsl, for debug purposes

    try
    {
      vector< string > dictFiles( 1, *i );

      // Check if there is an 'abrv' file present
      string baseName = ( (*i)[ i->size() - 4 ] == '.' ) ?
               string( *i, 0, i->size() - 4 ) : string( *i, 0, i->size() - 7 );

      string abrvFileName;

      if ( File::tryPossibleName( baseName + "_abrv.dsl", abrvFileName ) ||
           File::tryPossibleName( baseName + "_abrv.dsl.dz", abrvFileName ) ||
           File::tryPossibleName( baseName + "_ABRV.DSL", abrvFileName ) ||
           File::tryPossibleName( baseName + "_ABRV.DSL.DZ", abrvFileName ) ||
           File::tryPossibleName( baseName + "_ABRV.DSL.dz", abrvFileName ) )
        dictFiles.push_back( abrvFileName );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      // See if there's a zip file with resources present. If so, include it.

      string zipFileName;

      if ( File::tryPossibleName( baseName + ".dsl.files.zip", zipFileName ) ||
           File::tryPossibleName( baseName + ".dsl.dz.files.zip", zipFileName ) ||
           File::tryPossibleName( baseName + ".DSL.FILES.ZIP", zipFileName ) ||
           File::tryPossibleName( baseName + ".DSL.DZ.FILES.ZIP", zipFileName ) )
        dictFiles.push_back( zipFileName );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile, zipFileName.size() ) )
      {
        DslScanner scanner( *i );

        try { // Here we intercept any errors during the read to save line at
              // which the incident happened. We need alive scanner for that.

        if ( scanner.getDictionaryName() == GD_NATIVE_TO_WS( L"Abbrev" ) )
          continue; // For now just skip abbreviations

        // Building the index
        initializing.indexingDictionary( Utf8::encode( scanner.getDictionaryName() ) );

        DPRINTF( "Dictionary name: %ls\n", scanner.getDictionaryName().c_str() );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        string dictionaryName = Utf8::encode( scanner.getDictionaryName() );

        idx.write( (uint32_t) dictionaryName.size() );
        idx.write( dictionaryName.data(), dictionaryName.size() );

        idxHeader.dslEncoding = scanner.getEncoding();

        IndexedWords indexedWords;

        ChunkedStorage::Writer chunks( idx );

        // Read the abbreviations

        if ( abrvFileName.size() )
        {
          try
          {
            DslScanner abrvScanner( abrvFileName );

            map< string, string > abrv;

            wstring curString;
            size_t curOffset;

            for( ; ; )
            {
              // Skip any whitespace
              if ( !abrvScanner.readNextLine( curString, curOffset ) )
                break;
              if ( curString.empty() || isDslWs( curString[ 0 ] ) )
                continue;

              list< wstring > keys;

              bool eof = false;

              // Insert the key and read more, or get to the definition
              for( ; ; )
              {
                processUnsortedParts( curString, true );

                if ( keys.size() )
                  expandTildes( curString, keys.front() );

                expandOptionalParts( curString, keys );

                if ( !abrvScanner.readNextLine( curString, curOffset ) || curString.empty() )
                {
                  FDPRINTF( stderr, "Warning: premature end of file %s\n", abrvFileName.c_str() );
                  eof = true;
                  break;
                }

                if ( isDslWs( curString[ 0 ] ) )
                  break;
              }

              if ( eof )
                break;

              curString.erase( 0, curString.find_first_not_of( GD_NATIVE_TO_WS( L" \t" ) ) );

              if ( keys.size() )
                expandTildes( curString, keys.front() );

              // If the string has any dsl markup, we strip it
              string value = Utf8::encode( ArticleDom( curString ).root.renderAsText() );

              for( list< wstring >::iterator i = keys.begin(); i != keys.end();
                   ++i )
              {
                unescapeDsl( *i );
                normalizeHeadword( *i );

                abrv[ Utf8::encode( Folding::trimWhitespace( *i ) ) ] = value;
              }
            }

            idxHeader.hasAbrv = 1;
            idxHeader.abrvAddress = chunks.startNewBlock();

            uint32_t sz = abrv.size();

            chunks.addToBlock( &sz, sizeof( uint32_t ) );

            for( map< string, string >::const_iterator i = abrv.begin();
                 i != abrv.end(); ++i )
            {
//              DPRINTF( "%s:%s\n", i->first.c_str(), i->second.c_str() );

              sz = i->first.size();
              chunks.addToBlock( &sz, sizeof( uint32_t ) );
              chunks.addToBlock( i->first.data(), sz );
              sz = i->second.size();
              chunks.addToBlock( &sz, sizeof( uint32_t ) );
              chunks.addToBlock( i->second.data(), sz );
            }
          }
          catch( std::exception & e )
          {
            FDPRINTF( stderr, "Error reading abrv file %s: %s. Skipping it.\n",
                     abrvFileName.c_str(), e.what() );
          }
        }

        bool hasString = false;
        wstring curString;
        size_t curOffset;

        uint32_t articleCount = 0, wordCount = 0;

        for( ; ; )
        {
          // Find the main headword

          if ( !hasString && !scanner.readNextLine( curString, curOffset ) )
            break; // Clean end of file

          hasString = false;

          // The line read should either consist of pure whitespace, or be a
          // headword

          if ( curString.empty() )
            continue;

          if ( isDslWs( curString[ 0 ] ) )
          {
            // The first character is blank. Let's make sure that all other
            // characters are blank, too.
            for( size_t x = 1; x < curString.size(); ++x )
            {
              if ( !isDslWs( curString[ x ] ) )
              {
                FDPRINTF( stderr, "Warning: garbage string in %s at offset 0x%X\n", i->c_str(), curOffset );
                break;
              }
            }
            continue;
          }

          // Ok, got the headword

          list< wstring > allEntryWords;

          processUnsortedParts( curString, true );
          expandOptionalParts( curString, allEntryWords );

          uint32_t articleOffset = curOffset;

          //DPRINTF( "Headword: %ls\n", curString.c_str() );

          // More headwords may follow

          for( ; ; )
          {
            if ( ! ( hasString = scanner.readNextLine( curString, curOffset ) ) )
            {
              FDPRINTF( stderr, "Warning: premature end of file %s\n", i->c_str() );
              break;
            }

            // Lingvo skips empty strings between the headwords
            if ( curString.empty() )
              continue;

            if ( isDslWs( curString[ 0 ] ) )
              break; // No more headwords

            DPRINTF( "Alt headword: %ls\n", curString.c_str() );

            processUnsortedParts( curString, true );
            expandTildes( curString, allEntryWords.front() );
            expandOptionalParts( curString, allEntryWords );
          }

          if ( !hasString )
            break;

          // Insert new entry

          uint32_t descOffset = chunks.startNewBlock();

          chunks.addToBlock( &articleOffset, sizeof( articleOffset ) );

          for( list< wstring >::iterator j = allEntryWords.begin();
               j != allEntryWords.end(); ++j )
          {
            unescapeDsl( *j );
            normalizeHeadword( *j );
            indexedWords.addWord( *j, descOffset );
          }

          ++articleCount;
          wordCount += allEntryWords.size();

          // Skip the article's body
          for( ; ; )
          {
            if ( ! ( hasString = scanner.readNextLine( curString, curOffset ) ) )
              break;

            if ( curString.size() && !isDslWs( curString[ 0 ] ) )
              break;
          }

          // Now that we're having read the first string after the article
          // itself, we can use its offset to calculate the article's size.
          // An end of file works here, too.

          uint32_t articleSize = ( curOffset - articleOffset );

          chunks.addToBlock( &articleSize, sizeof( articleSize ) );

          if ( !hasString )
            break;
        }

        // Finish with the chunks

        idxHeader.chunksOffset = chunks.finish();

        // Build index

        IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

        idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
        idxHeader.indexRootOffset = idxInfo.rootOffset;

        indexedWords.clear(); // Release memory -- no need for this data

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

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;
        idxHeader.zipSupportVersion = CurrentZipSupportVersion;

        idxHeader.articleCount = articleCount;
        idxHeader.wordCount = wordCount;

        idxHeader.langFrom = dslLanguageToId( scanner.getLangFrom() );
        idxHeader.langTo = dslLanguageToId( scanner.getLangTo() );

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );

      } // In-place try for saving line count
      catch( ... )
      {
        atLine = scanner.getLinesRead();
        throw;
      }

      } // if need to rebuild

      dictionaries.push_back( new DslDictionary( dictId,
                                                 indexFile,
                                                 dictFiles ) );
    }
    catch( std::exception & e )
    {
      FDPRINTF( stderr, "DSL dictionary reading failed: %s:%u, error: %s\n",
        i->c_str(), atLine, e.what() );
    }
  }

  return dictionaries;
}


}
