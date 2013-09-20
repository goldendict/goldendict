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

#include <set>
#include <string>
#include <QFile>
#include <QDir>
#include <QDebug>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <QUrl>

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
  CurrentFormatVersion = 1 + BtreeIndexing::FormatVersion
};

struct IdxHeader
{
  uint32_t signature; // First comes the signature, ZIPS
  uint32_t formatVersion; // File format version, currently 1.
  uint32_t soundsCount; // Total number of sounds, for informative purposes only
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

wstring stripExtension( string const & str )
{
  wstring name;
  try
  {
    name = Utf8::decode( str );
  }
  catch( Utf8::exCantDecode )
  {
    return name;
  }

  if( Filetype::isNameOfSound( str ) )
  {
    wstring::size_type pos = name.rfind( L'.' );
    if ( pos != wstring::npos )
      name.erase( pos );
  }
  return name;
}

class ZipSoundsDictionary: public BtreeIndexing::BtreeDictionary
{
  Mutex idxMutex;
  File::Class idx;
  IdxHeader idxHeader;
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
                                                      wstring const & )
    throw( std::exception );

  virtual sptr< Dictionary::DataRequest > getResource( string const & name )
    throw( std::exception );

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

sptr< Dictionary::DataRequest > ZipSoundsDictionary::getResource( string const & name )
  throw( std::exception )
{
  // Remove extension for sound files (like in sound dirs)

  wstring strippedName = stripExtension( name );

  vector< WordArticleLink > chain = findArticles( strippedName );

  if ( chain.empty() )
    return new Dictionary::DataRequestInstant( false ); // No such resource

  sptr< Dictionary::DataRequestInstant > dr = new
    Dictionary::DataRequestInstant( true );

  if ( zipsFile.loadFile( chain[ 0 ].articleOffset, dr->getData() ) )
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
  throw( std::exception )
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
        qDebug( "Zips: Building the index for dictionary: %s\n", i->c_str() );

        File::Class idx( indexFile, "wb" );
        IdxHeader idxHeader;

        memset( &idxHeader, 0, sizeof( idxHeader ) );

        // We write a dummy header first. At the end of the process the header
        // will be rewritten with the right values.

        idx.write( idxHeader );

        IndexedWords names, zipFileNames;
        IndexedZip zipFile;
        if( zipFile.openZipFile( QDir::fromNativeSeparators(
                                 FsEncoding::decode( i->c_str() ) ) ) )
          zipFile.indexFile( zipFileNames );

        if( !zipFileNames.empty() )
        {
          // Remove extension for sound files (like in sound dirs)

          for( IndexedWords::iterator i = zipFileNames.begin(); i != zipFileNames.end(); ++i )
          {
            vector< WordArticleLink > links = i->second;
            for( unsigned x = 0; x < links.size(); x++ )
            {
              wstring word = stripExtension( links[ x ].word );
              if( !word.empty() )
                names.addSingleWord( word, links[ x ].articleOffset );
            }
          }

          // Build the resulting zip file index

          IndexInfo idxInfo = BtreeIndexing::buildIndex( names, idx );

          // That concludes it. Update the header.

          idxHeader.indexBtreeMaxElements = idxInfo.btreeMaxElements;
          idxHeader.indexRootOffset = idxInfo.rootOffset;

          idxHeader.signature = Signature;
          idxHeader.formatVersion = CurrentFormatVersion;

          idxHeader.soundsCount = names.size();

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
      qWarning( "Zipped sounds pack reading failed: %s, error: %s\n",
                i->c_str(), e.what() );
    }
  }

  return dictionaries;
}

}
