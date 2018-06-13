/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "zipsounds.hh"
#include "file.hh"
#include "folding.hh"
#include "utf8.hh"
#include "btreeidx.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "indexedzip.hh"
#include "filetype.hh"
#include "gddebug.hh"
#include "chunkedstorage.hh"
#include "htmlescape.hh"

#include <set>
#include <string>
#include <QFile>
#include <QDir>
#include <QDebug>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include "qt4x5.hh"

namespace ZipSounds {

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

enum
{
  Signature = 0x5350495a, // ZIPS on little-endian, SPIZ on big-endian
  CurrentFormatVersion = 5 + BtreeIndexing::FormatVersion
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, ZIPS
  uint32_t formatVersion; // File format version, currently 1.
  uint32_t soundsCount; // Total number of sounds, for informative purposes only
  uint32_t indexBtreeMaxElements; // Two fields from IndexInfo
  uint32_t indexRootOffset;
  uint32_t chunksOffset; // The offset to chunks' storage
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

wstring stripExtension( string const & str )
{
  wstring name;
  try
  {
    name = Utf8::decode( str );
  }
  catch( Utf8::exCantDecode & )
  {
    return name;
  }

  if( Filetype::isNameOfSound( str ) )
  {
    wstring::size_type pos = name.rfind( L'.' );
    if ( pos != wstring::npos )
      name.erase( pos );

    // Strip spaces at the end of name
    string::size_type n = name.length();
    while( n && name.at( n - 1 ) == L' ' )
      n--;

    if( n != name.length() )
      name.erase( n );
  }
  return name;
}

class ZipSoundsDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  sptr< ChunkedStorage::Reader > chunks;
  IndexedZip zipsFile;

public:

  ZipSoundsDictionary( string const & id, string const & indexFile,
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
                                                      wstring const &,
                                                      bool ignoreDiacritics )
    THROW_SPEC( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    THROW_SPEC( std::exception );

protected:

  virtual void loadIcon() throw();
};

ZipSoundsDictionary::ZipSoundsDictionary( string const & id,
                                          string const & indexFile,
                                          vector< string > const & dictionaryFiles ):
  BtreeDictionary( id, dictionaryFiles ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() )
{
  chunks = new ChunkedStorage::Reader( idx, idxHeader.chunksOffset );

  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
                        idx, idxMutex );

  QString zipName = QDir::fromNativeSeparators(
                    FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  zipsFile.openZipFile( zipName );
  zipsFile.openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                                 idxHeader.indexRootOffset ),
                                 idx, idxMutex );

}

string ZipSoundsDictionary::getName() throw()
{
  string result = FsEncoding::basename( getDictionaryFilenames()[ 0 ] );

  // Strip the extension
  result.erase( result.rfind( '.' ) );

  return result;
}

sptr< Dictionary::DataRequest > ZipSoundsDictionary::getArticle( wstring const & word,
                                                                 vector< wstring > const & alts,
                                                                 wstring const &,
                                                                 bool ignoreDiacritics )
  THROW_SPEC( std::exception )
{
  vector< WordArticleLink > chain = findArticles( word, ignoreDiacritics );

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    /// Make an additional query for each alt

    vector< WordArticleLink > altChain = findArticles( alts[ x ], ignoreDiacritics );

    chain.insert( chain.end(), altChain.begin(), altChain.end() );
  }

  multimap< wstring, uint32_t > mainArticles, alternateArticles;

  set< uint32_t > articlesIncluded; // Some synonims make it that the articles
                                    // appear several times. We combat this
                                    // by only allowing them to appear once.

  wstring wordCaseFolded = Folding::applySimpleCaseOnly( word );
  if( ignoreDiacritics )
    wordCaseFolded = Folding::applyDiacriticsOnly( wordCaseFolded );

  for( unsigned x = 0; x < chain.size(); ++x )
  {
    if ( articlesIncluded.find( chain[ x ].articleOffset ) != articlesIncluded.end() )
      continue; // We already have this article in the body.

    // Ok. Now, does it go to main articles, or to alternate ones? We list
    // main ones first, and alternates after.

    // We do the case-folded comparison here.

    wstring headwordStripped =
      Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) );
    if( ignoreDiacritics )
      headwordStripped = Folding::applyDiacriticsOnly( headwordStripped );

    multimap< wstring, uint32_t > & mapToUse =
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( std::pair< wstring, uint32_t >(
      Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) ), chain[ x ].articleOffset ) );

    articlesIncluded.insert( chain[ x ].articleOffset );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such word

  string result;

  multimap< wstring, uint32_t >::const_iterator i;

  result += "<table class=\"lsa_play\">";

  vector< char > chunk;
  char * nameBlock;

  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
    try
    {
      Mutex::Lock _( idxMutex );
      nameBlock = chunks->getBlock( i->second, chunk );

      if ( nameBlock >= &chunk.front() + chunk.size() )
      {
        // chunks reader thinks it's okay since zero-sized records can exist,
        // but we don't allow that.
        throw ChunkedStorage::exAddressOutOfRange();
      }
    }
    catch(  ChunkedStorage::exAddressOutOfRange & )
    {
      // Bad address
      continue;
    }

    uint16_t sz;
    memcpy( &sz, nameBlock, sizeof( uint16_t ) );
    nameBlock += sizeof( uint16_t );

    string name( nameBlock, sz );
    nameBlock += sz;

    string displayedName = mainArticles.size() + alternateArticles.size() > 1 ?
           name : Utf8::encode( stripExtension( name ) );

    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( name.c_str() ) ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + Html::escape( displayedName ) + "</a></td>";
    result += "</tr>";
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
    try
    {
      Mutex::Lock _( idxMutex );
      nameBlock = chunks->getBlock( i->second, chunk );

      if ( nameBlock >= &chunk.front() + chunk.size() )
      {
        // chunks reader thinks it's okay since zero-sized records can exist,
        // but we don't allow that.
        throw ChunkedStorage::exAddressOutOfRange();
      }
    }
    catch(  ChunkedStorage::exAddressOutOfRange & )
    {
      // Bad address
      continue;
    }

    uint16_t sz;
    memcpy( &sz, nameBlock, sizeof( uint16_t ) );
    nameBlock += sizeof( uint16_t );

    string name( nameBlock, sz );
    nameBlock += sz;

    string displayedName = mainArticles.size() + alternateArticles.size() > 1 ?
           name : Utf8::encode( stripExtension( name ) );

    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( Qt4x5::Url::ensureLeadingSlash( QString::fromUtf8( name.c_str() ) ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + Html::escape( displayedName ) + "</a></td>";
    result += "</tr>";
  }

  result += "</table>";

  Dictionary::DataRequestInstant * ret =
    new Dictionary::DataRequestInstant( true );

  ret->getData().resize( result.size() );

  memcpy( &(ret->getData().front()), result.data(), result.size() );

  return ret;
}

sptr< Dictionary::DataRequest > ZipSoundsDictionary::getResource( string const & name )
  THROW_SPEC( std::exception )
{
  // Remove extension for sound files (like in sound dirs)

  wstring strippedName = stripExtension( name );

  vector< WordArticleLink > chain = findArticles( strippedName );

  if ( chain.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such resource

  // Find sound

  uint32_t dataOffset = 0;
  for( int x = chain.size() - 1; x >= 0 ; x-- )
  {
    vector< char > chunk;
    char * nameBlock = chunks->getBlock( chain[ x ].articleOffset, chunk );

    uint16_t sz;
    memcpy( &sz, nameBlock, sizeof( uint16_t ) );
    nameBlock += sizeof( uint16_t );

    string fileName( nameBlock, sz );
    nameBlock += sz;

    memcpy( &dataOffset, nameBlock, sizeof( uint32_t ) );

    if( name.compare( fileName ) == 0 )
      break;
  }

  sptr< Dictionary::DataRequestInstant > dr = new
    Dictionary::DataRequestInstant( true );

  if ( zipsFile.loadFile( dataOffset, dr->getData() ) )
    return dr;

  return new Dictionary::DataRequestInstant( false );
}

void ZipSoundsDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 4 );

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
  THROW_SPEC( std::exception )
{
  (void) initializing;
  vector< sptr< Dictionary::Class > > dictionaries;

  for( vector< string >::const_iterator i = fileNames.begin(); i != fileNames.end();
       ++i )
  {
    /// Only allow .zips extension
    if ( i->size() < 5 || strcasecmp( i->c_str() + ( i->size() - 5 ), ".zips" ) != 0 )
      continue;

    try
    {
      vector< string > dictFiles( 1, *i );
      string dictId = Dictionary::makeDictionaryId( dictFiles );
      string indexFile = indicesDir + dictId;

      if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) ||
           indexIsOldOrBad( indexFile ) )
      {
        gdDebug( "Zips: Building the index for dictionary: %s\n", i->c_str() );

        File::Class idx( indexFile, "wb" );
        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        IndexedWords names, zipFileNames;
        ChunkedStorage::Writer chunks( idx );
        quint32 namesCount;

        IndexedZip zipFile;
        if( zipFile.openZipFile( QDir::fromNativeSeparators(
                                 FsEncoding::decode( i->c_str() ) ) ) )
          zipFile.indexFile( zipFileNames, &namesCount );

        if( !zipFileNames.empty() )
        {
          for( IndexedWords::iterator i = zipFileNames.begin(); i != zipFileNames.end(); ++i )
          {
            vector< WordArticleLink > links = i->second;
            for( unsigned x = 0; x < links.size(); x++ )
            {
              // Save original name

              uint32_t offset = chunks.startNewBlock();
              uint16_t sz = links[ x ].word.size();
              chunks.addToBlock( &sz, sizeof(uint16_t) );
              chunks.addToBlock( links[ x ].word.c_str(), sz );
              chunks.addToBlock( &links[ x ].articleOffset, sizeof( uint32_t ) );

              // Remove extension for sound files (like in sound dirs)

              wstring word = stripExtension( links[ x ].word );
              if( !word.empty() )
                names.addSingleWord( word, offset );
            }
          }

          // Finish with the chunks

          idxHeader.chunksOffset = chunks.finish();

          // Build the resulting zip file index

          IndexInfo idxInfo = BtreeIndexing::buildIndex( names, idx );

          // That concludes it. Update the header.

          idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
          idxHeader.indexRootOffset = idxInfo.rootOffset;

          idxHeader.signature = Signature;
          idxHeader.formatVersion = CurrentFormatVersion;

          idxHeader.soundsCount = namesCount;

          idx.rewind();

          idx.write( &idxHeader, sizeof( idxHeader ) );
        }
        else
        {
          idx.close();
          QFile::remove( QDir::fromNativeSeparators(
                         FsEncoding::decode( indexFile.c_str() ) ) );
          throw exInvalidData();
        }
      }

      dictionaries.push_back( new ZipSoundsDictionary( dictId,
                                                       indexFile,
                                                       dictFiles ) );
    }
    catch( std::exception & e )
    {
      gdWarning( "Zipped sounds pack reading failed: %s, error: %s\n",
                 i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
