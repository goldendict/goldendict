/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "loaddictionaries.hh"
#include "initializing.hh"
#include "bgl.hh"
#include "stardict.hh"
#include "lsa.hh"
#include "dsl.hh"
#include "mediawiki.hh"
#include "sounddir.hh"
#include "hunspell.hh"
#include "dictdfiles.hh"
#include "romaji.hh"
#include "russiantranslit.hh"
#include "german.hh"
#include "greektranslit.hh"
#include "belarusiantranslit.hh"
#include "website.hh"
#include "forvo.hh"
#include "programs.hh"
#include "voiceengines.hh"
#include "dprintf.hh"
#include "fsencoding.hh"
#include "xdxf.hh"
#include "sdict.hh"
#include "aard.hh"
#include "zipsounds.hh"
#include "mdx.hh"
#include "zim.hh"

#include <QMessageBox>
#include <QDir>

#include <set>

using std::set;

using std::string;
using std::vector;

LoadDictionaries::LoadDictionaries( Config::Class const & cfg ):
  paths( cfg.paths ), soundDirs( cfg.soundDirs ), hunspell( cfg.hunspell ),
  transliteration( cfg.transliteration ),
  exceptionText( "Load did not finish" ), // Will be cleared upon success
  maxPictureWidth( cfg.maxPictureWidth ),
  maxHeadwordSize( cfg.maxHeadwordSize )
{
  // Populate name filters

  nameFilters << "*.bgl" << "*.ifo" << "*.lsa" << "*.dat"
              << "*.dsl" << "*.dsl.dz"  << "*.index" << "*.xdxf"
              << "*.xdxf.dz" << "*.dct" << "*.aar" << "*.zips"
              << "*.mdx"
#ifdef MAKE_ZIM_SUPPORT
              << "*.zim" << "*.zimaa"
#endif
;
}

void LoadDictionaries::run()
{
  try
  {
    for( Config::Paths::const_iterator i = paths.begin(); i != paths.end(); ++i )
      handlePath( *i );

    // Make soundDirs
    {
      vector< sptr< Dictionary::Class > > soundDirDictionaries =
        SoundDir::makeDictionaries( soundDirs, FsEncoding::encode( Config::getIndexDir() ), *this );

      dictionaries.insert( dictionaries.end(), soundDirDictionaries.begin(),
                           soundDirDictionaries.end() );
    }

    // Make hunspells
    {
      vector< sptr< Dictionary::Class > > hunspellDictionaries =
        HunspellMorpho::makeDictionaries( hunspell );

      dictionaries.insert( dictionaries.end(), hunspellDictionaries.begin(),
                           hunspellDictionaries.end() );
    }

    exceptionText.clear();
  }
  catch( std::exception & e )
  {
    exceptionText = e.what();
  }
}

void LoadDictionaries::handlePath( Config::Path const & path )
{
  vector< string > allFiles;

  QDir dir( path.path );

  QFileInfoList entries = dir.entryInfoList( nameFilters, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );

  for( QFileInfoList::const_iterator i = entries.constBegin();
       i != entries.constEnd(); ++i )
  {
    QString fullName = i->canonicalFilePath();

    if ( path.recursive && i->isDir() )
    {
      // Make sure the path doesn't look like with dsl resources
      if ( !fullName.endsWith( ".dsl.files", Qt::CaseInsensitive ) &&
           !fullName.endsWith( ".dsl.dz.files", Qt::CaseInsensitive ) )
        handlePath( Config::Path( fullName, true ) );
    }

    if ( !i->isDir() )
      allFiles.push_back( FsEncoding::encode( QDir::toNativeSeparators( fullName ) ) );
  }

  {
    vector< sptr< Dictionary::Class > > bglDictionaries =
      Bgl::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), bglDictionaries.begin(),
                         bglDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > stardictDictionaries =
      Stardict::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), stardictDictionaries.begin(),
                         stardictDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > lsaDictionaries =
      Lsa::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), lsaDictionaries.begin(),
                         lsaDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > dslDictionaries =
      Dsl::makeDictionaries(
          allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, maxPictureWidth, maxHeadwordSize );

    dictionaries.insert( dictionaries.end(), dslDictionaries.begin(),
                         dslDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > dictdDictionaries =
      DictdFiles::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), dictdDictionaries.begin(),
                         dictdDictionaries.end() );
  }
  {
    vector< sptr< Dictionary::Class > > xdxfDictionaries =
      Xdxf::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), xdxfDictionaries.begin(),
                         xdxfDictionaries.end() );
  }
  {
    vector< sptr< Dictionary::Class > > sdictDictionaries =
      Sdict::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), sdictDictionaries.begin(),
                         sdictDictionaries.end() );
  }
  {
    vector< sptr< Dictionary::Class > > aardDictionaries =
      Aard::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), aardDictionaries.begin(),
                         aardDictionaries.end() );
  }
  {
    vector< sptr< Dictionary::Class > > zipSoundsDictionaries =
      ZipSounds::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), zipSoundsDictionaries.begin(),
                         zipSoundsDictionaries.end() );
  }
  {
    vector< sptr< Dictionary::Class > > mdxDictionaries =
      Mdx::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), mdxDictionaries.begin(),
                         mdxDictionaries.end() );
  }
#ifdef MAKE_ZIM_SUPPORT
  {
    vector< sptr< Dictionary::Class > > zimDictionaries =
      Zim::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), zimDictionaries.begin(),
                         zimDictionaries.end() );
  }
#endif
}

void LoadDictionaries::indexingDictionary( string const & dictionaryName ) throw()
{
  emit indexingDictionarySignal( QString::fromUtf8( dictionaryName.c_str() ) );
}


void loadDictionaries( QWidget * parent, bool showInitially,
                       Config::Class const & cfg,
                       std::vector< sptr< Dictionary::Class > > & dictionaries,
                       QNetworkAccessManager & dictNetMgr,
                       bool doDeferredInit_ )
{
  dictionaries.clear();

  ::Initializing init( parent, showInitially );

  // Start a thread to load all the dictionaries

  LoadDictionaries loadDicts( cfg );

  QObject::connect( &loadDicts, SIGNAL( indexingDictionarySignal( QString const & ) ),
                    &init, SLOT( indexing( QString const & ) ) );

  QEventLoop localLoop;

  QObject::connect( &loadDicts, SIGNAL( finished() ),
                    &localLoop, SLOT( quit() ) );

  loadDicts.start();

  localLoop.exec();

  loadDicts.wait();

  if ( loadDicts.getExceptionText().size() )
  {
    QMessageBox::critical( parent, QCoreApplication::translate( "LoadDictionaries", "Error loading dictionaries" ),
                           QString::fromUtf8( loadDicts.getExceptionText().c_str() ) );

    return;
  }

  dictionaries = loadDicts.getDictionaries();

  ///// We create transliterations syncronously since they are very simple

  // Make Romaji
  {
    vector< sptr< Dictionary::Class > > romajiDictionaries =
      Romaji::makeDictionaries( cfg.transliteration.romaji );

    dictionaries.insert( dictionaries.end(), romajiDictionaries.begin(),
                         romajiDictionaries.end() );
  }

  // Make Russian transliteration
  if ( cfg.transliteration.enableRussianTransliteration )
    dictionaries.push_back( RussianTranslit::makeDictionary() );

  // Make German transliteration
  if ( cfg.transliteration.enableGermanTransliteration )
    dictionaries.push_back( GermanTranslit::makeDictionary() );

  // Make Greek transliteration
  if ( cfg.transliteration.enableGreekTransliteration )
    dictionaries.push_back( GreekTranslit::makeDictionary() );

  // Make Belarusian transliteration
  if ( cfg.transliteration.enableBelarusianTransliteration )
  {
    vector< sptr< Dictionary::Class > > dicts = BelarusianTranslit::makeDictionaries();
    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  ///// We create MediaWiki dicts syncronously, since they use netmgr

  {
    vector< sptr< Dictionary::Class > > dicts =
      MediaWiki::makeDictionaries( loadDicts, cfg.mediawikis, dictNetMgr );

    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  ///// WebSites are very simple, no need to create them asyncronously
  {
    vector< sptr< Dictionary::Class > > dicts =
      WebSite::makeDictionaries( cfg.webSites );

    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  //// Forvo dictionaries

  {
    vector< sptr< Dictionary::Class > > dicts =
      Forvo::makeDictionaries( loadDicts, cfg.forvo, dictNetMgr );

    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  //// Programs
  {
    vector< sptr< Dictionary::Class > > dicts =
      Programs::makeDictionaries( cfg.programs );

    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  //// Text to Speech
  {
    vector< sptr< Dictionary::Class > > dicts =
      VoiceEngines::makeDictionaries( cfg.voiceEngines );

    dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
  }

  DPRINTF( "Load done\n" );

  // Remove any stale index files

  set< string > ids;

  for( unsigned x = dictionaries.size(); x--; )
    ids.insert( dictionaries[ x ]->getId() );

  QDir indexDir( Config::getIndexDir() );

  QStringList allIdxFiles = indexDir.entryList( QDir::Files );

  for( QStringList::const_iterator i = allIdxFiles.constBegin();
       i != allIdxFiles.constEnd(); ++i )
  {
    if ( ids.find( FsEncoding::encode( *i ) ) == ids.end() &&
         i->size() == 32 )
      indexDir.remove( *i );
  }

  // Run deferred inits

  if ( doDeferredInit_ )
    doDeferredInit( dictionaries );
}

void doDeferredInit( std::vector< sptr< Dictionary::Class > > & dictionaries )
{
  for( unsigned x = 0; x < dictionaries.size(); ++x )
    dictionaries[ x ]->deferredInit();
}
