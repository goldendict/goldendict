/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
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
#include <zlib.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>
#include <wctype.h>

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
using std::wstring;
using std::vector;
using std::list;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;

namespace {

DEF_EX_STR( exCantReadFile, "Can't read file", Dictionary::Ex )

enum
{
  Signature = 0x584c5344, // DSLX on little-endian, XLSD on big-endian
  CurrentFormatVersion = 6 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, DSLX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  int dslEncoding; // Which encoding is used for the file indexed
  uint32_t chunksOffset; // The offset to chunks' storage
  uint32_t hasAbrv; // Non-zero means file has abrvs at abrvAddress
  uint32_t abrvAddress; // Address of abrv map in the chunked storage
  uint32_t indexOffset; // The offset of the index in the file
} __attribute__((packed));

bool indexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  IdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != Signature ||
         header.formatVersion != CurrentFormatVersion;
}

class DslDictionary: public BtreeIndexing::BtreeDictionary
{
  File::Class idx;
  IdxHeader idxHeader;
  ChunkedStorage::Reader chunks;
  string dictionaryName;
  map< string, string > abrv;
  dictData * dz;

public:

  DslDictionary( string const & id, string const & indexFile,
                      vector< string > const & dictionaryFiles );

  ~DslDictionary();

  virtual string getName() throw()
  { return dictionaryName; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual vector< wstring > findHeadwordsForSynonym( wstring const & )
    throw( std::exception )
  {
    return vector< wstring >();
  }

  virtual string getArticle( wstring const &, vector< wstring > const & alts )
    throw( Dictionary::exNoSuchWord, std::exception );

  virtual void getResource( string const & name,
                            vector< char > & data )
    throw( Dictionary::exNoSuchResource, std::exception );

private:

  /// Loads the article. Does not process the DSL language.
  void loadArticle( uint32_t address,
                    string & headword,
                    list< wstring > & displayedHeadwords,
                    wstring & articleText );

  /// Converts DSL language to an Html.
  string dslToHtml( wstring const & );

  // Parts of dslToHtml()
  string nodeToHtml( ArticleDom::Node const & );
  string processNodeChildren( ArticleDom::Node const & node );
};

DslDictionary::DslDictionary( string const & id,
                              string const & indexFile,
                              vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset )
{
  // Open the .dict file

  dz = dict_data_open( dictionaryFiles[ 0 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 0 ] );

  // Read the dictionary name

  idx.seek( sizeof( idxHeader ) );

  vector< char > dName( idx.read< uint32_t >() );
  idx.read( &dName.front(), dName.size() );
  dictionaryName = string( &dName.front(), dName.size() );

  // Read the abrv, if any

  if ( idxHeader.hasAbrv )
  {
    vector< char > chunk;

    char * abrvBlock = chunks.getBlock( idxHeader.abrvAddress, chunk );

    uint32_t total;
    memcpy( &total, abrvBlock, sizeof( uint32_t ) );
    abrvBlock += sizeof( uint32_t );

    printf( "Loading %u abbrv\n", total );

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

  idx.seek( idxHeader.indexOffset );

  openIndex( idx );
}

DslDictionary::~DslDictionary()
{
  if ( dz )
    dict_data_close( dz );
}

void DslDictionary::loadArticle( uint32_t address,
                                 string & headword,
                                 list< wstring > & displayedHeadwords,
                                 wstring & articleText )
{
  wstring articleData;

  {
    vector< char > chunk;
  
    char * articleProps = chunks.getBlock( address, chunk );
  
    uint32_t articleOffset, articleSize;
  
    memcpy( &articleOffset, articleProps, sizeof( articleOffset ) );
    memcpy( &articleSize, articleProps + sizeof( articleOffset ),
            sizeof( articleSize ) );

    printf( "offset = %x\n", articleOffset );

    char * articleBody = dict_data_read_( dz, articleOffset, articleSize, 0, 0 );
  
    if ( !articleBody )
      throw exCantReadFile( getDictionaryFilenames()[ 0 ] );

    try
    {
      articleData =
        DslIconv::toWstring(
          DslIconv::getEncodingNameFor( DslEncoding( idxHeader.dslEncoding ) ),
          articleBody, articleSize );
    }
    catch( ... )
    {
      free( articleBody );
      throw;
    }
  }

  size_t pos = articleData.find_first_of( L"\n\r" );

  if ( pos == wstring::npos )
    pos = articleData.size();

  wstring firstHeadword( articleData, 0, pos );

  printf( "first headword = %ls\n", firstHeadword.c_str() );

  // Make a headword
  {
    wstring str( firstHeadword );
    list< wstring > lst;

    processUnsortedParts( str, true );
    expandOptionalParts( str, lst );

    headword = Utf8::encode( lst.front() );
  }

  // Generate displayed headwords

  displayedHeadwords.clear();

  processUnsortedParts( firstHeadword, false );
  expandOptionalParts( firstHeadword, displayedHeadwords );

  // Now skip alts until we reach the body itself
  while ( pos != articleData.size() )
  {
    if ( articleData[ pos ] == '\r' )
      ++pos;

    if ( pos != articleData.size() )
    {
      if ( articleData[ pos ] == '\n' )
        ++pos;
    }

    if ( pos != articleData.size() && !iswblank( articleData[ pos ] ) )
    {
      // Skip any alt headwords
      pos = articleData.find_first_of( L"\n\r", pos );

      if ( pos == wstring::npos )
        pos = articleData.size();
    }
    else
      break;
  }

  if ( pos != articleData.size() )
    articleText = wstring( articleData, pos );
  else
    articleText = L"";
}

string DslDictionary::dslToHtml( wstring const & str )
{
  ArticleDom dom( str );

  string html = processNodeChildren( dom.root );

  // Lines seem to indicate paragraphs in Dsls, so we enclose each line within
  // a <p></p>.

  for( size_t x = html.size(); x--; )
    if ( html[ x ] == '\n' )
      html.insert( x + 1, "</p><p>" );

  return "<!-- DSL Source:\n" + Utf8::encode( str ) + "\n-->"
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

  if ( node.tagName == L"b" )
    result += "<b class=\"dsl_b\">" + processNodeChildren( node ) + "</b>";
  else
  if ( node.tagName == L"i" )
    result += "<i class=\"dsl_i\">" + processNodeChildren( node ) + "</i>";
  else
  if ( node.tagName == L"u" )
    result += "<span class=\"dsl_u\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == L"c" )
  {
    result += "<font color=\"" + ( node.tagAttrs.size() ?
      Html::escape( Utf8::encode( node.tagAttrs ) ) : string( "c_default_color" ) )
      + "\">" + processNodeChildren( node ) + "</font>";
  }
  else
  if ( node.tagName == L"*" )
    result += "<span class=\"dsl_opt\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName.size() == 2 && node.tagName[ 0 ] == L'm' &&
       iswdigit( node.tagName[ 1 ] ) )
    result += "<div class=\"dsl_" + Utf8::encode( node.tagName ) + "\">" + processNodeChildren( node ) + "</div>";
  else
  if ( node.tagName == L"trn" )
    result += "<span class=\"dsl_trn\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == L"ex" )
    result += "<span class=\"dsl_ex\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == L"com" )
    result += "<span class=\"dsl_com\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == L"s" )
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

      bool search = true;

      try
      {
        File::Class f( n, "r" );

        search = false;
      }
      catch( File::Ex & )
      {
      }

      string ref = "\"gdau://" + ( search ? string( "search" ) : getId() ) +
                   "/" + Html::escape( filename ) +"\"";
  
      result += "<span class=\"dsl_s_wav\"><a href=" + ref
         + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a></span>";
    }
    else
    if ( Filetype::isNameOfPicture( filename ) )
    {
      result += "<img src=\"bres://" + getId() + "/" + Html::escape( filename )
             + "\" alt=\"" + Html::escape( filename ) + "\"/>";
    }
    else
    {
      // Unknown file type, downgrade to a hyperlink
      result += "<a class=\"dsl_s\" href=\"bres://" + getId() + "/" + Html::escape( filename )
             + "\">" + processNodeChildren( node ) + "</a>";
    }
  }
  else
  if ( node.tagName == L"url" )
    result += "<a class=\"dsl_url\" href=\"" + Html::escape( Utf8::encode( node.renderAsText() ) ) +"\">" + processNodeChildren( node ) + "</a>";
  else
  if ( node.tagName == L"!trs" )
    result += "<span class=\"dsl_trs\">" + processNodeChildren( node ) + "</span>";
  else
  if ( node.tagName == L"p" )
  {
    result += "<span class=\"dsl_p\"";

    string val = Utf8::encode( node.renderAsText() );

    // If we have such a key, display a title

    map< string, string >::const_iterator i = abrv.find( val );

    if ( i != abrv.end() )
      result += " title=\"" + Html::escape( i->second ) + "\"";

    result += ">" + processNodeChildren( node ) + "</span>";
  }
  else
  if ( node.tagName == L"'" )
  {
    result += "<span class=\"dsl_stress\">" + processNodeChildren( node ) + "<span class=\"dsl_stacc\">" + Utf8::encode( wstring( 1, 0x301 ) ) + "</span></span>";
  }
  else
  if ( node.tagName == L"lang" )
  {
    result += "<span class=\"dsl_lang\">" + processNodeChildren( node ) + "</span>";
  }
  else
  if ( node.tagName == L"ref" )
  {
    result += "<a class=\"dsl_ref\" href=\"bword://" + Html::escape( Utf8::encode( node.renderAsText() ) ) +"\">" + processNodeChildren( node ) + "</a>";
  }
  else
  if ( node.tagName == L"sub" )
  {
    result += "<sub>" + processNodeChildren( node ) + "</sub>";
  }
  else
  if ( node.tagName == L"sup" )
  {
    result += "<sup>" + processNodeChildren( node ) + "</sup>";
  }
  else
  if ( node.tagName == L"t" )
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


string DslDictionary::getArticle( wstring const & word,
                                       vector< wstring > const & alts )
  throw( Dictionary::exNoSuchWord, std::exception )
{
  vector< WordArticleLink > chain = findArticles( word );

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt

    vector< WordArticleLink > altChain = findArticles( alts[ x ] );

    chain.insert( chain.end(), altChain.begin(), altChain.end() );
  }

  multimap< wstring, string > mainArticles, alternateArticles;

  set< uint32_t > articlesIncluded; // Some synonims make it that the articles
                                    // appear several times. We combat this
                                    // by only allowing them to appear once.

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword;

    list< wstring > displayedHeadwords;
    wstring articleBody;

    loadArticle( chain[ x ].articleOffset, headword, displayedHeadwords,
                 articleBody );

    string articleText;

    articleText += "<span class=\"dsl_article\">";
    articleText += "<div class=\"dsl_headwords\">";

    for( list< wstring >::const_iterator i = displayedHeadwords.begin();
         i != displayedHeadwords.end(); ++i )
      articleText += dslToHtml( *i );

    articleText += "</div>";

    if ( displayedHeadwords.size() )
      expandTildes( articleBody, displayedHeadwords.front() );

    articleText += "<div class=\"dsl_definition\">";
    articleText += dslToHtml( articleBody );
    articleText += "</div>";
    articleText += "</span>";
    
    // Ok. Now, does it go to main articles, or to alternate ones? We list
    // main ones first, and alternates after.

    // We do the case-folded comparison here.

    wstring headwordStripped =
      Folding::applySimpleCaseOnly( Utf8::decode( headword ) );

    multimap< wstring, string > & mapToUse = 
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( pair< wstring, string >(
      Folding::applySimpleCaseOnly( Utf8::decode( headword ) ),
      articleText ) );

    articlesIncluded.insert( chain[ x ].articleOffset );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
    throw Dictionary::exNoSuchWord();

  string result;

  multimap< wstring, string >::const_iterator i;

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
    result += i->second;

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
    result += i->second;

  return result;
}

void DslDictionary::getResource( string const & name,
                                 vector< char > & data )
  throw( Dictionary::exNoSuchResource, std::exception )
{
  string n = 
    FsEncoding::dirname( getDictionaryFilenames()[ 0 ] ) +
    FsEncoding::separator() +
    FsEncoding::encode( name );

  printf( "n is %s\n", n.c_str() );

  try
  {
    File::Class f( n, "rb" );

    f.seekEnd();

    data.resize( f.tell() );

    f.rewind();

    f.read( &data.front(), data.size() );

    if ( Filetype::isNameOfTiff( name ) )
    {
      // Convert it

      QImage img = QImage::fromData( (unsigned char *) &data.front(),
                                     data.size() );

      if ( img.isNull() )
      {
        // Failed to load, return data as is
        return;
      }

      QByteArray ba;
      QBuffer buffer( &ba );
      buffer.open( QIODevice::WriteOnly );
      img.save( &buffer, "BMP" );

      data.resize( buffer.size() );

      memcpy( &data.front(), buffer.data(), data.size() );
    }
  }
  catch( File::Ex & )
  {
    throw Dictionary::exNoSuchResource();
  }
}

} // anonymous namespace

static bool tryPossibleName( string const & name, string & copyTo )
{
  try
  {
    File::Class f( name, "rb" );

    copyTo = name;

    return true;
  }
  catch( ... )
  {
    return false;
  }
}

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

vector< sptr< Dictionary::Class > > Format::makeDictionaries(
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

    if ( ( i->size() < 4 ||
           strcasecmp( i->c_str() + ( i->size() - 4 ), ".dsl" ) != 0 ) &&
         ( i->size() < 7 ||
           strcasecmp( i->c_str() + ( i->size() - 7 ), ".dsl.dz" ) != 0 ) )
      continue;

    try
    {
      vector< string > dictFiles( 1, *i );

      // Check if there is an 'abrv' file present
      string baseName = ( (*i)[ i->size() - 4 ] == '.' ) ?
               string( *i, 0, i->size() - 4 ) : string( *i, 0, i->size() - 7 );

      string abrvFileName;

      if ( tryPossibleName( baseName + "_abrv.dsl", abrvFileName ) ||
           tryPossibleName( baseName + "_abrv.dsl.dz", abrvFileName ) ||
           tryPossibleName( baseName + "_ABRV.DSL", abrvFileName ) ||
           tryPossibleName( baseName + "_ABRV.DSL.DZ", abrvFileName ) ||
           tryPossibleName( baseName + "_ABRV.DSL.dz", abrvFileName ) )
        dictFiles.push_back( abrvFileName );

      string dictId = makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        DslScanner scanner( *i );

        if ( scanner.getDictionaryName() == L"Abbrev" )
          continue; // For now just skip abbreviations

        // Building the index
        initializing.indexingDictionary( Utf8::encode( scanner.getDictionaryName() ) );

        printf( "Dictionary name: %ls\n", scanner.getDictionaryName().c_str() );

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
              if ( curString.empty() || iswblank( curString[ 0 ] ) )
                continue;

              string key = Utf8::encode( curString );

              if ( !abrvScanner.readNextLine( curString, curOffset ) )
              {
                fprintf( stderr, "Warning: premature end of file %s\n", abrvFileName.c_str() );
                break;
              }

              if ( curString.empty() || !iswblank( curString[ 0 ] ) )
              {
                fprintf( stderr, "Warning: malformed file %s\n", abrvFileName.c_str() );
                break;
              }

              curString.erase( 0, curString.find_first_not_of( L" \t" ) );

              abrv[ key ] = Utf8::encode( curString );
            }

            idxHeader.hasAbrv = 1;
            idxHeader.abrvAddress = chunks.startNewBlock();

            uint32_t sz = abrv.size();

            chunks.addToBlock( &sz, sizeof( uint32_t ) );

            for( map< string, string >::const_iterator i = abrv.begin();
                 i != abrv.end(); ++i )
            {
              printf( "%s:%s\n", i->first.c_str(), i->second.c_str() );

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
            fprintf( stderr, "Error reading abrv file %s: %s. Skipping it.\n",
                     abrvFileName.c_str(), e.what() );
          }
        }

        bool hasString = false;
        wstring curString;
        size_t curOffset;

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

          if ( iswblank( curString[ 0 ] ) )
          {
            // The first character is blank. Let's make sure that all other
            // characters are blank, too.
            for( size_t x = 1; x < curString.size(); ++x )
            {
              if ( !iswblank( curString[ x ] ) )
              {
                fprintf( stderr, "Warning: garbage string in %s at offset 0x%X\n", i->c_str(), curOffset );
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

          //printf( "Headword: %ls\n", curString.c_str() );

          // More headwords may follow

          for( ; ; )
          {
            if ( ! ( hasString = scanner.readNextLine( curString, curOffset ) ) )
            {
              fprintf( stderr, "Warning: premature end of file %s\n", i->c_str() );
              exit( 0 );
              break;
            }

            if ( curString.empty() || iswblank( curString[ 0 ] ) )
              break; // No more headwords

            printf( "Alt headword: %ls\n", curString.c_str() );

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
            wstring folded = Folding::apply( *j );

            IndexedWords::iterator e = indexedWords.insert(
              IndexedWords::value_type( folded, vector< WordArticleLink >() ) ).first;

            // Try to conserve memory somewhat -- slow insertions are ok
            e->second.reserve( e->second.size() + 1 );

            e->second.push_back( WordArticleLink( Utf8::encode( *j ), descOffset ) );
          }

          // Skip the article's body
          for( ; ; )
          {
            if ( ! ( hasString = scanner.readNextLine( curString, curOffset ) ) )
              break;

            if ( curString.size() && !iswblank( curString[ 0 ] ) )
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

        idxHeader.indexOffset = BtreeIndexing::buildIndex( indexedWords, idx );

        // That concludes it. Update the header.

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );
      }

      dictionaries.push_back( new DslDictionary( dictId,
                                                 indexFile,
                                                 dictFiles ) );
    }
    catch( std::exception & e )
    {
      fprintf( stderr, "DSL dictionary reading failed: %s, error: %s\n",
        i->c_str(), e.what() );
    }
  }

  return dictionaries;
}


}
