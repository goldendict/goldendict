/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "lsa.hh"
#include "file.hh"
#include "iconv.hh"
#include "folding.hh"
#include "utf8.hh"
#include "btreeidx.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "dprintf.hh"

#include <set>
#include <string>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

#include <QUrl>
#include <QDir>
#include <QDebug>

namespace Lsa {

using std::string;
using gd::wstring;
using std::map;
using std::multimap;
using std::set;
using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

DEF_EX( exInvalidData, "Invalid data encountered", Dictionary::Ex )
DEF_EX( exFailedToOpenVorbisData, "Failed to open Vorbis data", Dictionary::Ex )
DEF_EX( exFailedToSeekInVorbisData, "Failed to seek in Vorbis data", Dictionary::Ex )
DEF_EX( exFailedToRetrieveVorbisInfo, "Failed to retrieve Vorbis info", Dictionary::Ex )

enum
{
  Signature = 0x5841534c, // LSAX on little-endian, XASL on big-endian
  CurrentFormatVersion = 5
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, BGLX
  uint32_t formatVersion; // File format version, currently 1.
  uint32_t soundsCount; // Total number of sounds, for informative purposes only
  uint32_t vorbisOffset; // Offset of the vorbis file which contains all snds
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
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

string stripExtension( string const & str )
{
  if ( str.size() > 3 &&
      ( strcasecmp( str.c_str() + ( str.size() - 4 ), ".wav" ) == 0 ) )
    return string( str, 0, str.size() - 4 );
  else
    return str;
}

struct Entry
{
  string name;

  uint32_t samplesLength;
  uint32_t samplesOffset;
public:

  // Reads an entry from the file's current position
  Entry( File::Class & f );
};

Entry::Entry( File::Class & f )
{
  bool firstEntry = ( f.tell() == 13 );
  // Read the entry's filename
  size_t read = 0;

  vector< uint16_t > filenameBuffer( 64 );

  for( ; ; ++read )
  {
    if ( filenameBuffer.size() <= read )
      filenameBuffer.resize( read + 64 );

    f.read( &filenameBuffer[ read ], 2 );

    if ( filenameBuffer[ read ] == 0xD )
    {
      if ( f.read< uint16_t >() != 0xA )
        throw exInvalidData();

      // Filename ending marker
      break;
    }
  }

  // Skip zero or ff, or just ff.

  if ( uint8_t x = f.read< uint8_t >() )
  {
    if ( x != 0xFF )
      throw exInvalidData();
  }
  else
  if ( f.read< uint8_t >() != 0xFF )
    throw exInvalidData();


  if ( !firstEntry )
  {
    // For all entries but the first one, read its offset in
    // samples.
    samplesOffset  = f.read< uint32_t >();

    if ( f.read< uint8_t >() != 0xFF )
      throw exInvalidData();
  }
  else
    samplesOffset = 0;

  // Read the size of the recording, in samples
  samplesLength = f.read< uint32_t >();

  name = Iconv::toUtf8( Iconv::Utf16Le, &filenameBuffer.front(),
                        read * sizeof( uint16_t ) );
}

class LsaDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;

public:

  LsaDictionary( string const & id, string const & indexFile,
                 vector< string > const & dictionaryFiles );

  virtual string getName() throw();

  virtual map< Dictionary::Property, string > getProperties() throw()
  { return map< Dictionary::Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return idxHeader.soundsCount; }

  virtual unsigned long getWordCount() throw()
  { return getArticleCount(); }

  virtual sptr< Dictionary::DataRequest > getArticle( wstring const &,
                                                      vector< wstring > const & alts,
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

protected:

  virtual void loadIcon() throw();
};

string LsaDictionary::getName() throw()
{
  string result = FsEncoding::basename( getDictionaryFilenames()[ 0 ] );

  // Strip the extension
  result.erase( result.rfind( '.' ) );

  return result;
}

LsaDictionary::LsaDictionary( string const & id,
                              string const & indexFile,
                              vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() )
{
  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );
}

sptr< Dictionary::DataRequest > LsaDictionary::getArticle( wstring const & word,
                                                           vector< wstring > const & alts,
                                                           wstring const & )
  throw( std::exception )
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

    // Ok. Now, does it go to main articles, or to alternate ones? We list
    // main ones first, and alternates after.

    // We do the case-folded comparison here.

    wstring headwordStripped =
      Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) );

    multimap< wstring, string > & mapToUse =
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( std::pair< wstring, string >(
      Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) ), chain[ x ].word ) );

    articlesIncluded.insert( chain[ x ].articleOffset );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such word

  string result;

  multimap< wstring, string >::const_iterator i;

  result += "<table class=\"lsa_play\">";
  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( QString::fromUtf8( i->second.c_str() ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + i->second + "</a></td>";
    result += "</tr>";
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( QString::fromUtf8( i->second.c_str() ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + i->second + "</a></td>";
    result += "</tr>";
  }

  result += "</table>";

  Dictionary::DataRequestInstant * ret =
    new Dictionary::DataRequestInstant( true );

  ret->getData().resize( result.size() );

  memcpy( &(ret->getData().front()), result.data(), result.size() );

  return ret;
}

/// This wraps around file operations
struct ShiftedVorbis
{
  FILE * f;
  size_t shift;

  ShiftedVorbis( FILE * f_, size_t shift_ ): f( f_ ), shift( shift_ )
  {}

  static size_t read( void * ptr, size_t size, size_t nmemb, void * datasource );
  static int seek( void * datasource, ogg_int64_t offset, int whence );
  static long tell( void * datasource );

  static ov_callbacks callbacks;
};

size_t ShiftedVorbis::read( void * ptr, size_t size, size_t nmemb,
                            void * datasource )
{
  ShiftedVorbis * sv = ( ShiftedVorbis * ) datasource;

  return fread( ptr, size, nmemb, sv->f );
}

int ShiftedVorbis::seek( void * datasource, ogg_int64_t offset, int whence )
{
  ShiftedVorbis * sv = ( ShiftedVorbis * ) datasource;

  if ( whence == SEEK_SET )
    offset += sv->shift;

  return fseek( sv->f, offset, whence );
}

long ShiftedVorbis::tell( void * datasource )
{
  ShiftedVorbis * sv = ( ShiftedVorbis * ) datasource;
  long result = ftell( sv->f );

  if ( result != -1 )
    result -= sv->shift;

  return result;
}

ov_callbacks ShiftedVorbis::callbacks = { ShiftedVorbis::read,
                                          ShiftedVorbis::seek,
                                          NULL,
                                          ShiftedVorbis::tell };

// A crude .wav header which is sufficient for our needs
struct WavHeader
{
  char riff[ 4 ]; // RIFF
  uint32_t riffLength;
  char waveAndFmt[ 8 ]; // WAVEfmt%20
  uint32_t fmtLength; // 16
  uint16_t formatTag; // 1
  uint16_t channels; // 1 or 2
  uint32_t samplesPerSec;
  uint32_t bytesPerSec;
  uint16_t blockAlign;
  uint16_t bitsPerSample; // 16
  char data[ 4 ]; // data
  uint32_t dataLength;
} 
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

sptr< Dictionary::DataRequest > LsaDictionary::getResource( string const & name )
  throw( std::exception )
{
  // See if the name ends in .wav. Remove that extension then

  string strippedName =
    ( name.size() > 3 && ( name.compare( name.size() - 4, 4, ".wav" ) == 0 ) ) ?
      string( name, 0, name.size() - 4 ) : name;

  vector< WordArticleLink > chain = findArticles( Utf8::decode( strippedName ) );

  if ( chain.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such resource

  File::Class f( getDictionaryFilenames()[ 0 ], "rb" );

  f.seek( chain[ 0 ].articleOffset );
  Entry e( f );

  f.seek( idxHeader.vorbisOffset );

  ShiftedVorbis sv( f.file(), idxHeader.vorbisOffset );

  OggVorbis_File vf;

  int result = ov_open_callbacks( &sv, &vf, 0, 0, ShiftedVorbis::callbacks );

  if ( result )
    throw exFailedToOpenVorbisData();

  if ( ov_pcm_seek( &vf, e.samplesOffset ) )
    throw exFailedToSeekInVorbisData();

  vorbis_info * vi = ov_info( &vf, -1 );

  if ( !vi )
  {
    ov_clear( &vf );

    throw exFailedToRetrieveVorbisInfo();
  }

  sptr< Dictionary::DataRequestInstant > dr = new
    Dictionary::DataRequestInstant( true );

  vector< char > & data = dr->getData();

  data.resize( sizeof( WavHeader ) + e.samplesLength * 2 );

  WavHeader * wh = (WavHeader *)&data.front();

  memset( wh, 0, sizeof( *wh ) );

  memcpy( wh->riff, "RIFF", 4 );
  wh->riffLength = data.size() - 8;

  memcpy( wh->waveAndFmt, "WAVEfmt ", 8 );
  wh->fmtLength = 16;
  wh->formatTag = 1;
  wh->channels = vi->channels;
  wh->samplesPerSec = vi->rate;
  wh->bytesPerSec = vi->channels * vi->rate * 2;
  wh->blockAlign = vi->channels * 2;
  wh->bitsPerSample = 16;
  memcpy( wh->data, "data", 4 );
  wh->dataLength = data.size() - sizeof( *wh );

  // Now decode vorbis to the rest of the block

  char * ptr = &data.front() + sizeof( *wh );
  int left = data.size() - sizeof( *wh );
  int bitstream = 0;

  while( left )
  {
    long result = ov_read( &vf, ptr, left, 0, 2, 1, &bitstream );

    if ( result <= 0 )
    {
      qWarning( "Warning: failed to read Vorbis data (code = %ld)\n", result );
      memset( ptr, 0, left );
      break;
    }

    if ( result > left )
    {
      FDPRINTF( stderr, "Warning: Vorbis decode returned more data than requested.\n" );

      result = left;
    }

    ptr += result;
    left -= result;
  }

  ov_clear( &vf );

  return dr;
}

void LsaDictionary::loadIcon() throw()
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
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/playsound.png");
  }

  dictionaryIconLoaded = true;
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
    /// Only allow .dat and .lsa extensions to save scanning time
    if ( i->size() < 4 ||
        ( strcasecmp( i->c_str() + ( i->size() - 4 ), ".dat" ) != 0 &&
          strcasecmp( i->c_str() + ( i->size() - 4 ), ".lsa" ) != 0 ) )
      continue;

    try
    {
      File::Class f( *i, "rb" );

      /// Check the signature

      char buf[ 9 ];

      if ( f.readRecords( buf, 9, 1 ) != 1 || memcmp( buf, "L\09\0S\0A\0\xff", 9 ) != 0 )
      {
        // The file is too short or the signature doesn't match -- skip this
        // file
        continue;
      }

      vector< string > dictFiles( 1, *i );

      string dictId = Dictionary::makeDictionaryId( dictFiles );

      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) || indexIsOldOrBad( indexFile ) )
      {
        // Building the index

        qDebug( "Lsa: Building the index for dictionary: %s\n", i->c_str() );

        initializing.indexingDictionary( FsEncoding::basename( *i ) );

        File::Class idx( indexFile, "wb" );

        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        IndexedWords indexedWords;

        /// XXX handle big-endian machines here!
        uint32_t entriesCount = f.read< uint32_t >();

        DPRINTF( "%s: %u entries\n", i->c_str(), entriesCount );

        idxHeader.soundsCount = entriesCount;

        vector< uint16_t > filenameBuffer;

        while( entriesCount-- )
        {
          uint32_t offset = f.tell();

          Entry e( f );


          // Remove the extension, no need for that in the index
          e.name = stripExtension( e.name );

          DPRINTF( "Read filename %s (%u at %u)<\n", e.name.c_str(), e.samplesLength, e.samplesOffset );

          // Insert new entry into an index

          indexedWords.addWord( Utf8::decode( e.name ), offset );
        }

        idxHeader.vorbisOffset = f.tell();

        // Make sure there's really some vobis data there

        char buf[ 4 ];

        f.read( buf, sizeof( buf ) );

        if ( strncmp( buf, "OggS", 4 ) != 0 )
          throw exInvalidData();

        // Build the index

        IndexInfo idxInfo = BtreeIndexing::buildIndex( indexedWords, idx );

        idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
        idxHeader.indexRootOffset = idxInfo.rootOffset;

         // That concludes it. Update the header.

        idxHeader.signature = Signature;
        idxHeader.formatVersion = CurrentFormatVersion;

        idx.rewind();

        idx.write( &idxHeader, sizeof( idxHeader ) );
      }

      dictionaries.push_back( new LsaDictionary( dictId,
                                                 indexFile,
                                                 dictFiles ) );
    }
    catch( std::exception & e )
    {
      qWarning( "Lingvo's LSA reading failed: %s, error: %s\n",
                 i->c_str(), e.what() );
    }
  }

  return dictionaries;
}


}
