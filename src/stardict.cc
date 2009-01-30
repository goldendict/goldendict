/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "stardict.hh"
#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include "chunkedstorage.hh"
#include "dictzip.h"
#include "xdxf2html.hh"
#include "htmlescape.hh"
#include <zlib.h>
#include <map>
#include <set>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>

#include <QString>

namespace Stardict {

using std::map;
using std::multimap;
using std::pair;
using std::set;
using std::string;
using std::wstring;

using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;

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
  string sametypesequence, dicttype;

  Ifo( File::Class & );
};

enum
{
  Signature = 0x58444953, // SIDX on little-endian, XDIS on big-endian
  CurrentFormatVersion = 4 + BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, SIDX
  uint32_t formatVersion; // File format version (CurrentFormatVersion)
  uint32_t chunksOffset; // The offset to chunks' storage
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


class StardictDictionary: public BtreeIndexing::BtreeDictionary
{
  Ifo ifo;
  File::Class idx;
  IdxHeader idxHeader;
  ChunkedStorage::Reader chunks;
  dictData * dz;

public:

  StardictDictionary( string const & id, string const & indexFile,
                      vector< string > const & dictionaryFiles,
                      Ifo const & );

  ~StardictDictionary();

  virtual string getName() throw()
  { return ifo.bookname; }

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return ifo.wordcount; }

  virtual unsigned long getWordCount() throw()
  { return ifo.wordcount + ifo.synwordcount; }

  virtual vector< wstring > findHeadwordsForSynonym( wstring const & )
    throw( std::exception );

  virtual string getArticle( wstring const &, vector< wstring > const & alts )
    throw( Dictionary::exNoSuchWord, std::exception );

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
};

StardictDictionary::StardictDictionary( string const & id,
                                        string const & indexFile,
                                        vector< string > const & dictionaryFiles,
                                        Ifo const & ifo_ ):
  BtreeDictionary( id, dictionaryFiles ),
  ifo( ifo_ ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset )
{
  // Open the .dict file

  dz = dict_data_open( dictionaryFiles[ 2 ].c_str(), 0 );

  if ( !dz )
    throw exCantReadFile( dictionaryFiles[ 2 ] );

  // Initialize the index

  idx.seek( idxHeader.indexOffset );

  openIndex( idx );
}

StardictDictionary::~StardictDictionary()
{
  if ( dz )
    dict_data_close( dz );
}

void StardictDictionary::getArticleProps( uint32_t articleAddress,
                                          string & headword,
                                          uint32_t & offset, uint32_t & size )
{
  vector< char > chunk;

  char * articleData = chunks.getBlock( articleAddress, chunk );

  memcpy( &offset, articleData, sizeof( uint32_t ) );
  articleData += sizeof( uint32_t );
  memcpy( &size, articleData, sizeof( uint32_t ) );
  articleData += sizeof( uint32_t );

  headword = articleData;
}

/// This function tries to make an html of the Stardict's resource typed
/// 'type', contained in a block pointed to by 'resource', 'size' bytes long.
static string handleResource( char type, char const * resource, size_t size )
{
  switch( type )
  {
    case 'x': // Xdxf content
      return Xdxf2Html::convert( string( resource, size ) );
    case 'h': // Html content
      return "<div class=\"sdct_h\">" + string( resource, size ) + "</div>";
    case 'm': // Pure meaning, usually means preformatted text
      return "<pre class=\"sdct_m\">" + Html::escape( string( resource, size ) ) + "</pre>";
    case 'l': // Same as 'm', but not in utf8, instead in current locale's
              // encoding.
              // We just use Qt here, it should know better about system's
              // locale.
      return "<pre class=\"sdct_l\">" + Html::escape( QString::fromLocal8Bit( resource, size ).toUtf8().data() ) + "</pre>";
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

  // Note that the function always zero-pads the result.
  char * articleBody = dict_data_read_( dz, offset, size, 0, 0 );

  if ( !articleBody )
    throw exCantReadFile( getDictionaryFilenames()[ 2 ] );

  articleText.clear();

  char * ptr = articleBody;

  if ( ifo.sametypesequence.size() )
  {
    /// The sequence is known, it's not stored in the article itself
    for( unsigned seq = 0; seq < ifo.sametypesequence.size(); ++seq )
    {
      // Last entry doesn't have size info -- it is inferred from
      // the bytes left
      bool entrySizeKnown = ( seq == ifo.sametypesequence.size() - 1 );

      uint32_t entrySize;

      if ( entrySizeKnown )
        entrySize = size;
      else
      if ( !size )
      {
        fprintf( stderr, "Warning: short entry for the word %s encountered.\n", headword.c_str() );
        break;
      }

      char type = ifo.sametypesequence[ seq ];

      if ( islower( type ) )
      {
        // Zero-terminated entry, unless it's the last one
        if ( !entrySizeKnown )
          entrySize = strlen( ptr );

        if ( size < entrySize )
        {
          fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
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
            fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
            break;
          }

          memcpy( &entrySize, ptr, sizeof( uint32_t ) );

          entrySize = ntohl( entrySize );

          ptr += sizeof( uint32_t );
          size -= sizeof( uint32_t );
        }

        if ( size < entrySize )
        {
          fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( type, ptr, entrySize );

        ptr += entrySize;
        size -= entrySize;
      }
      else
      {
        fprintf( stderr, "Warning: non-alpha entry type 0x%x for the word %s encountered.\n",
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
          fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
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
          fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        uint32_t entrySize;

        memcpy( &entrySize, ptr + 1, sizeof( uint32_t ) );

        entrySize = ntohl( entrySize );

        if ( size < sizeof( uint32_t ) + 1 + entrySize )
        {
          fprintf( stderr, "Warning: malformed entry for the word %s encountered.\n", headword.c_str() );
          break;
        }

        articleText += handleResource( *ptr, ptr + 1 + sizeof( uint32_t ), entrySize );

        ptr += sizeof( uint32_t ) + 1 + entrySize;
        size -= sizeof( uint32_t ) + 1 + entrySize;
      }
      else
      {
        fprintf( stderr, "Warning: non-alpha entry type 0x%x for the word %s encountered.\n",
                         (unsigned)*ptr, headword.c_str() );
        break;
      }
    }
  }

  free( articleBody );
}

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

string StardictDictionary::getArticle( wstring const & word,
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

  multimap< wstring, pair< string, string > > mainArticles, alternateArticles;

  set< uint32_t > articlesIncluded; // Some synonims make it that the articles
                                    // appear several times. We combat this
                                    // by only allowing them to appear once.

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Now grab that article

    string headword, articleText;

    loadArticle( chain[ x ].articleOffset, headword, articleText );

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
    throw Dictionary::exNoSuchWord();

  string result;

  multimap< wstring, pair< string, string > >::const_iterator i;

  string cleaner = "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</font>""</font>""</font>""</font>""</font>""</font>"
                   "</b></b></b></b></b></b></b></b>"
                   "</i></i></i></i></i></i></i></i>";

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
      result += cleaner;
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
      result += "<h3>";
      result += i->second.first;
      result += "</h3>";
      result += i->second.second;
      result += cleaner;
  }

  return result;
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

  //printf( "%s<\n", f.gets().c_str() );
  //printf( "%s<\n", f.gets().c_str() );

  if ( f.gets() != "StarDict's dict ifo file" ||
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
    }
  }
  catch( File::exReadError & )
  {
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

static void handleIdxSynFile( string const & fileName,
                              IndexedWords & indexedWords,
                              ChunkedStorage::Writer & chunks,
                              vector< uint32_t > * articleOffsets,
                              bool isSynFile )
{
  gzFile stardictIdx = gzopen( fileName.c_str(), "rb" );

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

  // We append one zero byte to catch runaway string at the end, if any

  image.back() = 0;

  // Now parse it

  for( char const * ptr = &image.front(); ptr != &image.back(); )
  {
    size_t wordLen = strlen( ptr );

    if ( ptr + wordLen + 1 + ( isSynFile ? sizeof( uint32_t ) :
                                           sizeof( uint32_t ) * 2 ) >
         &image.back() )
      throw exSuddenEndOfFile( fileName );

    char const * word = ptr;

    ptr += wordLen + 1;

    uint32_t offset;

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
    }

    wstring folded = Folding::apply( Utf8::decode( word ) );

    // Insert new entry into an index

    IndexedWords::iterator i = indexedWords.insert(
      IndexedWords::value_type( folded, vector< WordArticleLink >() ) ).first;

    // Try to conserve memory somewhat -- slow insertions are ok
    i->second.reserve( i->second.size() + 1 );

    i->second.push_back( WordArticleLink( word, offset ) );
  }

  printf( "%u entires made\n", indexedWords.size() );
}

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
    if ( i->size() < 4 ||
        strcasecmp( i->c_str() + ( i->size() - 4 ), ".ifo" ) != 0 )
      continue;

    try
    {
      File::Class ifoFile( *i, "r" );

      Ifo ifo( ifoFile );

      if ( ifo.idxoffsetbits == 64 )
        throw ex64BitsNotSupported();

      if ( ifo.dicttype.size() )
        throw exDicttypeNotSupported();

      printf( "bookname = %s\n", ifo.bookname.c_str() );
      printf( "wordcount = %u\n", ifo.wordcount );

      vector< string > dictFiles( 1, *i );

      string idxFileName, dictFileName, synFileName;

      findCorrespondingFiles( *i, idxFileName, dictFileName, synFileName,
                              ifo.synwordcount );

      dictFiles.push_back( idxFileName );
      dictFiles.push_back( dictFileName );

      if ( ifo.synwordcount )
        dictFiles.push_back( synFileName );

      string dictId = makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        // Building the index
        initializing.indexingDictionary( ifo.bookname );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

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

        idxHeader.indexOffset = BtreeIndexing::buildIndex( indexedWords, idx );

        // That concludes it. Update the header.

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );
      }

      dictionaries.push_back( new StardictDictionary( dictId,
                                                      indexFile,
                                                      dictFiles,
                                                      ifo ) );
      
    }
    catch( std::exception & e )
    {
      fprintf( stderr, "Stardict's dictionary reading failed: %s, error: %s\n",
        i->c_str(), e.what() );
    }
  }

  return dictionaries;
}


}
