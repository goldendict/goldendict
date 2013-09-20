/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "sounddir.hh"
#include "file.hh"
#include "folding.hh"
#include "utf8.hh"
#include "btreeidx.hh"
#include "chunkedstorage.hh"
#include "filetype.hh"
#include "htmlescape.hh"
#include "audiolink.hh"
#include "wstring_qt.hh"
#include "fsencoding.hh"
#include <set>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QDebug>

namespace SoundDir {

using std::string;
using gd::wstring;
using std::map;
using std::multimap;
using std::set;
using BtreeIndexing::WordArticleLink;
using BtreeIndexing::IndexedWords;
using BtreeIndexing::IndexInfo;

namespace {

enum
{
  Signature = 0x58524453, // SDRX on little-endian, XRDS on big-endian
  CurrentFormatVersion = 1+ BtreeIndexing::FormatVersion + Folding::Version
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, SDRX
  uint32_t formatVersion; // File format version, is to be CurrentFormatVersion
  uint32_t soundsCount; // Total number of sounds, for informative purposes only
  uint32_t chunksOffset; // The offset to chunks' storage
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

class SoundDirDictionary: public BtreeIndexing::BtreeDictionary
{
  string name;
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
  ChunkedStorage::Reader chunks;
  QString iconFilename;

public:

  SoundDirDictionary( string const & id, string const & name,
                      string const & indexFile,
                      vector< string > const & dictionaryFiles,
                      QString const & iconFilename_ );

  virtual string getName() throw()
  { return name; }

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

SoundDirDictionary::SoundDirDictionary( string const & id,
                                        string const & name_,
                                        string const & indexFile,
                                        vector< string > const & dictionaryFiles,
                                        QString const & iconFilename_ ):
  BtreeDictionary( id, dictionaryFiles ),
  name( name_ ),
  idx( indexFile, "rb" ),
  idxHeader( idx.read< IdxHeader >() ),
  chunks( idx, idxHeader.chunksOffset ),
  iconFilename( iconFilename_ )
{
  // Initialize the index

  openIndex( IndexInfo( idxHeader.indexBtreeMaxElements,
                        idxHeader.indexRootOffset ),
             idx, idxMutex );
}

sptr< Dictionary::DataRequest > SoundDirDictionary::getArticle( wstring const & word,
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

  // maps to the chain number
  multimap< wstring, unsigned > mainArticles, alternateArticles;

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

    multimap< wstring, unsigned > & mapToUse =
      ( wordCaseFolded == headwordStripped ) ?
        mainArticles : alternateArticles;

    mapToUse.insert( std::pair< wstring, uint32_t >(
      Folding::applySimpleCaseOnly( Utf8::decode( chain[ x ].word ) ), x ) );

    articlesIncluded.insert( chain[ x ].articleOffset );
  }

  if ( mainArticles.empty() && alternateArticles.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such word

  string result;

  multimap< wstring, uint32_t >::const_iterator i;

  result += "<table class=\"lsa_play\">";
  for( i = mainArticles.begin(); i != mainArticles.end(); ++i )
  {
    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( QString::number( chain[ i->second ].articleOffset ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + Html::escape( chain[ i->second ].word ) + "</a></td>";
    result += "</tr>";
  }

  for( i = alternateArticles.begin(); i != alternateArticles.end(); ++i )
  {
    result += "<tr>";

    QUrl url;
    url.setScheme( "gdau" );
    url.setHost( QString::fromUtf8( getId().c_str() ) );
    url.setPath( QString::number( chain[ i->second ].articleOffset ) );

    string ref = string( "\"" ) + url.toEncoded().data() + "\"";

    result += addAudioLink( ref, getId() );

    result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
    result += "<td><a href=" + ref + ">" + Html::escape( chain[ i->second ].word ) + "</a></td>";
    result += "</tr>";
  }

  result += "</table>";

  Dictionary::DataRequestInstant * ret =
    new Dictionary::DataRequestInstant( true );

  ret->getData().resize( result.size() );

  memcpy( &(ret->getData().front()), result.data(), result.size() );

  return ret;
}

void SoundDirDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  if( !iconFilename.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), iconFilename );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/playsound.png");
  dictionaryIconLoaded = true;
}

sptr< Dictionary::DataRequest > SoundDirDictionary::getResource( string const & name )
  throw( std::exception )
{
  bool isNumber = false;

  uint32_t articleOffset = QString::fromUtf8( name.c_str() ).toULong( &isNumber );

  if ( !isNumber )
    return new Dictionary::DataRequestInstant( false ); // No such resource

  vector< char > chunk;
  char * articleData;

  try
  {
    Mutex::Lock _( idxMutex );

    articleData = chunks.getBlock( articleOffset, chunk );

    if ( articleData >= &chunk.front() + chunk.size() )
    {
      // chunks reader thinks it's okay since zero-sized records can exist,
      // but we don't allow that.
      throw ChunkedStorage::exAddressOutOfRange();
    }
  }
  catch(  ChunkedStorage::exAddressOutOfRange & )
  {
    // Bad address
    return new Dictionary::DataRequestInstant( false ); // No such resource
  }

  chunk.back() = 0; // It must end with 0 anyway, but just in case

  QDir dir( QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) ) );

  QString fileName = QDir::toNativeSeparators( dir.filePath( QString::fromUtf8( articleData ) ) );

  // Now try loading that file

  try
  {
    File::Class f( FsEncoding::encode( fileName ), "rb" );

    sptr< Dictionary::DataRequestInstant > dr = new
      Dictionary::DataRequestInstant( true );

    vector< char > & data = dr->getData();

    f.seekEnd();

    data.resize( f.tell() );

    f.rewind();
    f.read( &data.front(), data.size() );

    return dr;
  }
  catch( File::Ex )
  {
    return new Dictionary::DataRequestInstant( false ); // No such resource
  }
}

void addDir( QDir const & baseDir, QDir const & dir, IndexedWords & indexedWords,
             uint32_t & soundsCount, ChunkedStorage::Writer & chunks )
{
  QFileInfoList entries = dir.entryInfoList( QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot );

  for( QFileInfoList::const_iterator i = entries.constBegin();
       i != entries.constEnd(); ++i )
  {
    if ( i->isDir() )
      addDir( baseDir, QDir( i->absoluteFilePath() ), indexedWords, soundsCount, chunks );
    else
    if ( Filetype::isNameOfSound( i->fileName().toUtf8().data() ) )
    {
      // Add this sound to index

      string fileName = baseDir.relativeFilePath( i->filePath() ).toUtf8().data();

      uint32_t articleOffset = chunks.startNewBlock();
      chunks.addToBlock( fileName.c_str(), fileName.size() + 1 );

      wstring name = gd::toWString( i->fileName() );

      wstring::size_type pos = name.rfind( L'.' );

      if ( pos != wstring::npos )
        name.erase( pos );

      indexedWords.addWord( name, articleOffset );

      ++soundsCount;
    }
  }
}

}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::SoundDirs const & soundDirs,
                                                      string const & indicesDir,
                                                      Dictionary::Initializing & initializing )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > dictionaries;

  for( Config::SoundDirs::const_iterator i = soundDirs.begin(); i != soundDirs.end();
       ++i )
  {
    QDir dir( i->path );

    if ( !dir.exists() )
      continue; // No such dir, no dictionary then

    vector< string > dictFiles( 1,
      QDir::toNativeSeparators( dir.canonicalPath() ).toLocal8Bit().data() );

    dictFiles.push_back( "SoundDir" ); // A mixin

    string dictId = Dictionary::makeDictionaryId( dictFiles );

    dictFiles.pop_back(); // Remove mixin

    string indexFile = indicesDir + dictId;

    if ( Dictionary::needToRebuildIndex( dictFiles, indexFile ) || indexIsOldOrBad( indexFile ) )
    {
      // Building the index

      qDebug() << "Sounds: Building the index for directory: " << i->path;

      initializing.indexingDictionary( i->name.toUtf8().data() );

      File::Class idx( indexFile, "wb" );

      IdxHeader idxHeader;

      memset( &idxHeader, 0, sizeof( idxHeader ) );

      // We write a dummy header first. At the end of the process the header
      // will be rewritten with the right values.

      idx.write( idxHeader );

      IndexedWords indexedWords;

      ChunkedStorage::Writer chunks( idx );

      uint32_t soundsCount = 0; // Header's one is packed, we can't ref it

      addDir( dir, dir, indexedWords, soundsCount, chunks );

      idxHeader.soundsCount = soundsCount;

      // Finish with the chunks

      idxHeader.chunksOffset = chunks.finish();

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

    dictionaries.push_back( new SoundDirDictionary( dictId,
                                                    i->name.toUtf8().data(),
                                                    indexFile,
                                                    dictFiles,
                                                    i->iconFilename ) );
  }

  return dictionaries;
}


}
