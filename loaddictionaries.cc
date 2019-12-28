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
#include "gddebug.hh"
#include "fsencoding.hh"
#include "xdxf.hh"
#include "sdict.hh"
#include "aard.hh"
#include "zipsounds.hh"
#include "mdx.hh"
#include "zim.hh"
#include "dictserver.hh"
#include "slob.hh"
#include "gls.hh"

#ifndef NO_EPWING_SUPPORT
#include "epwing.hh"
#endif

#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
#include "chinese.hh"
#endif
#include <QSplashScreen>
#include <QMessageBox>
#include <QDir>

#include <set>

using std::set;

using std::string;
using std::vector;

class DictNameFilter : public QStringList
{
public:
    DictNameFilter() {
        *this << "*.bgl" << "*.ifo" << "*.lsa" << "*.dat"
                      << "*.dsl" << "*.dsl.dz"  << "*.index" << "*.xdxf"
                      << "*.xdxf.dz" << "*.dct" << "*.aar" << "*.zips"
                      << "*.mdx" << "*.gls" << "*.gls.dz"
        #ifdef MAKE_SLOB_SUPPORT
                      << "*.slob"
        #endif
        #ifdef MAKE_ZIM_SUPPORT
                      << "*.zim" << "*.zimaa"
        #endif
        #ifndef NO_EPWING_SUPPORT
                      << "*catalogs"
        #endif
        ;
    }
    ~DictNameFilter(){}
};

LoadDictionaries::LoadDictionaries(Config::Class const & cfg , QElapsedTimer const & timer_ , std::vector<sptr<Dictionary::Class> > &dicts )
: dictionaries(dicts), timer(timer_), cfg_(cfg)
#ifdef DICTS_LOADING_CONCURRENT
  ,sWait(0), ref(0)
#endif
{
}
static const QString rn("\n");

void LoadDictionaries::run()
{
  emit showMessage(tr("Start Handling Dictionaries ..."));
  const QString tes = tr("Time elapsed: %2 s");
  try
  {
    for( Config::Paths::const_iterator i = cfg_.paths.begin(); i != cfg_.paths.end(); ++i )
      handlePath( *i );
#ifdef DICTS_LOADING_CONCURRENT
    do
    {
      int left = Qt4x5::AtomicInt::loadAcquire(ref);
      if(left < 1)
        break;
      while(!sWait.tryAcquire(1, 1000))
        emit showMessage(tr("Handling User's Dictionary%1%3%5%7").arg(rn).
                         arg(tr("%1 left").arg(left)).arg(rn).
                         arg(tes.arg(timer.elapsed() / 1000)) );
    }while(true);
#endif
    if(!exceptionText.empty() && dictionaries.empty())
    {
        emit showMessage(tr("Failed to Handle User's Dictionaries%1%3").arg(rn).
                         arg(QString::fromUtf8(exceptionText.c_str())));
        return;
    }
#ifndef DICTS_LOADING_CONCURRENT
    else
    {
        emit showMessage(tr("%1 User's Dictionaries Handled%3%5").arg(rn).
                         arg(tes.arg(timer.elapsed() / 1000)) );
    }
#endif

    // Make soundDirs
    {
      vector< sptr< Dictionary::Class > > soundDirDictionaries =
        SoundDir::makeDictionaries( cfg_.soundDirs, FsEncoding::encode( Config::getIndexDir() ), *this );

      dictionaries.insert( dictionaries.end(), soundDirDictionaries.begin(),
                           soundDirDictionaries.end() );
      if(!soundDirDictionaries.empty())
          emit showMessage(tr("%1 SoundDir Dictionaries Handled%3%5").
                           arg(soundDirDictionaries.size()).arg(rn).
                           arg(tes.arg(timer.elapsed() / 1000)) );
    }

    // Make hunspells
    {
      vector< sptr< Dictionary::Class > > hunspellDictionaries =
        HunspellMorpho::makeDictionaries( cfg_.hunspell );

      dictionaries.insert( dictionaries.end(), hunspellDictionaries.begin(),
                           hunspellDictionaries.end() );
      if(!hunspellDictionaries.empty())
          emit showMessage(tr("%1 HunspellMorpho Dictionaries Handled%3%5").
                           arg(hunspellDictionaries.size()).arg(rn).
                           arg(tes.arg(timer.elapsed() / 1000)) );
    }
  }
  catch( std::exception & e )
  {
    exceptionText.append(e.what());
  }
  emit showMessage(tr("Finished Handling Dictionaries: %1%3%5").
                   arg(dictionaries.size()).arg(rn).
                   arg(tes.arg(timer.elapsed() / 1000)) );
}

void LoadDictionaries::handlePath( Config::Path const & path )
{
  static const DictNameFilter nameFilters;
  vector< string > allFiles;

  QDir dir( path.path );

  QFileInfoList entries = dir.entryInfoList( nameFilters, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );

  for( QFileInfoList::const_iterator i = entries.constBegin();
       i != entries.constEnd(); ++i )
  {
    QString fullName = i->absoluteFilePath();

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
  if(allFiles.empty())
     return;
  emit showMessage(tr("Handling User's Dictionary%1%3").arg(rn).arg(path.path),
                   Qt::AlignCenter, Qt::darkBlue);
#ifdef DICTS_LOADING_CONCURRENT
  QThreadPool *tp = QThreadPool::globalInstance();
  int maxThreadCount = tp->maxThreadCount();
  if(maxThreadCount <= 1 )
  {
      handleFiles(dictionaries, allFiles);
      return;
  }

  QRunnable *r = new LoadDictionariesRunnable(*this, allFiles);
  while( Qt4x5::AtomicInt::loadAcquire(ref) > maxThreadCount )
    tp->waitForDone(100);
  ++ref;
  tp->start(r);
#else
  handleFiles(allFiles);
#endif
}

#ifdef DICTS_LOADING_CONCURRENT
void LoadDictionaries::handleFiles(std::vector< sptr< Dictionary::Class > > &dictionaries,
                                   const std::vector< std::string > &allFiles)
#else
void LoadDictionaries::handleFiles(const std::vector< std::string > &allFiles)
#endif
{
  {
    vector< sptr< Dictionary::Class > > bglDictionaries =
      Bgl::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), bglDictionaries.begin(),
                         bglDictionaries.end() );
  }

  {
    vector< sptr< Dictionary::Class > > stardictDictionaries =
      Stardict::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

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
          allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxPictureWidth, cfg_.maxHeadwordSize );

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
      Aard::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

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
  {
    vector< sptr< Dictionary::Class > > glsDictionaries =
      Gls::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), glsDictionaries.begin(),
                         glsDictionaries.end() );
  }
#ifdef MAKE_ZIM_SUPPORT
  {
    vector< sptr< Dictionary::Class > > zimDictionaries =
      Zim::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

    dictionaries.insert( dictionaries.end(), zimDictionaries.begin(),
                         zimDictionaries.end() );
  }
#endif
#ifdef MAKE_SLOB_SUPPORT
  {
    vector< sptr< Dictionary::Class > > slobDictionaries =
      Slob::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

    dictionaries.insert( dictionaries.end(), slobDictionaries.begin(),
                         slobDictionaries.end() );
  }
#endif
#ifndef NO_EPWING_SUPPORT
  {
    vector< sptr< Dictionary::Class > > epwingDictionaries =
      Epwing::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

    dictionaries.insert( dictionaries.end(), epwingDictionaries.begin(),
                         epwingDictionaries.end() );
  }
#endif
}

void LoadDictionaries::indexingDictionary( string const & dictionaryName ) throw()
{
  emit showMessage( tr("Indexing Dictionary%1%3").arg(rn).arg(QString::fromUtf8( dictionaryName.c_str() )) );
}

#ifdef DICTS_LOADING_CONCURRENT
void LoadDictionariesRunnable::run()
{
    std::string exceptionText;
    std::vector< sptr< Dictionary::Class > > dictionaries;
    try
    {
        ld.handleFiles(dictionaries, allFiles);
    }
    catch( std::exception & e )
    {
      exceptionText = e.what();
    }
    ld.addDictionaries(dictionaries, exceptionText);
}
#endif

void LoadDictionaries::loadDictionaries( QWidget * parent, bool canHideParent,
                       Config::Class const & cfg,
                       std::vector< sptr< Dictionary::Class > > & dictionaries,
                       QNetworkAccessManager & dictNetMgr,
                       bool doDeferredInit_ )
{
  QElapsedTimer timer;
  timer.start();

  GDSplash splash;
  splash.show();
  splash.showUiMsg(LoadDictionaries::tr("Start Loading Dictionaries"));

  if(canHideParent)
    parent->hide();

  const int expiryTimeout = QThreadPool::globalInstance()->expiryTimeout();
  QThreadPool::globalInstance()->setExpiryTimeout(-1);

  dictionaries.clear();

  // Start a thread to load all the dictionaries

  LoadDictionaries loadDicts( cfg, timer, dictionaries );
  QObject::connect(&loadDicts, SIGNAL(showMessage(const QString &, int, const QColor &)),
                   &splash, SLOT(showMessage(const QString &, int, const QColor &)), Qt::QueuedConnection  );

  QEventLoop localLoop;

  QObject::connect( &loadDicts, SIGNAL( finished() ),
                    &localLoop, SLOT( quit() ) );

  loadDicts.start();

  localLoop.exec();

  loadDicts.wait();

  QThreadPool::globalInstance()->setExpiryTimeout(expiryTimeout);

  const string &err = loadDicts.getExceptionText();
  if ( !err.empty() )
  {
    QMessageBox::critical( &splash, QCoreApplication::translate( "LoadDictionaries", "Error loading dictionaries" ),
                           QString::fromUtf8( err.c_str() ) );
    if( dictionaries.empty() )
    {
      if(canHideParent)
        parent->show();
      splash.finish(parent);
      return;
    }
  }

  try{
  ///// We create transliterations synchronously since they are very simple
  const QString hmsg = tr("%1 %3 Dictionaries Handled");
#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
  // Make Chinese conversion
  {
    vector< sptr< Dictionary::Class > > chineseDictionaries =
      Chinese::makeDictionaries( cfg.transliteration.chinese );

    if(!chineseDictionaries.empty())
    {
        dictionaries.insert( dictionaries.end(), chineseDictionaries.begin(),
                             chineseDictionaries.end() );
        splash.showUiMsg(hmsg.arg(chineseDictionaries.size()).arg("Chinese-conversion"));
    }
  }
#endif

  // Make Romaji
  {
    vector< sptr< Dictionary::Class > > romajiDictionaries =
      Romaji::makeDictionaries( cfg.transliteration.romaji );

    if(!romajiDictionaries.empty())
    {
        dictionaries.insert( dictionaries.end(), romajiDictionaries.begin(),
                             romajiDictionaries.end() );
        splash.showUiMsg(hmsg.arg(romajiDictionaries.size()).arg("Romaji"));
    }
  }

  size_t transliteration_dc = dictionaries.size();
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
    if(!dicts.empty())
    {
      dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
    }
  }

  transliteration_dc =  dictionaries.size() - transliteration_dc;
  if(transliteration_dc)
    splash.showUiMsg(hmsg.arg(transliteration_dc).arg("Transliteration"));

  ///// We create MediaWiki dicts synchronously, since they use netmgr
  {
    vector< sptr< Dictionary::Class > > dicts =
      MediaWiki::makeDictionaries( loadDicts, cfg.mediawikis, dictNetMgr );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("MediaWiki"));
    }
  }

  ///// WebSites are very simple, no need to create them asynchronously
  {
    vector< sptr< Dictionary::Class > > dicts =
      WebSite::makeDictionaries( cfg.webSites, dictNetMgr );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("WebSite"));
    }
  }

  //// Forvo dictionaries
  {
    vector< sptr< Dictionary::Class > > dicts =
      Forvo::makeDictionaries( loadDicts, cfg.forvo, dictNetMgr );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("Forvo"));
    }
  }

  //// Programs
  {
    vector< sptr< Dictionary::Class > > dicts =
      Programs::makeDictionaries( cfg.programs );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("Programs"));
    }
  }

  //// Text to Speech
  {
    vector< sptr< Dictionary::Class > > dicts =
      VoiceEngines::makeDictionaries( cfg.voiceEngines );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("Text-to-Speech"));
    }
  }

  {
    vector< sptr< Dictionary::Class > > dicts =
      DictServer::makeDictionaries( cfg.dictServers );

    if(!dicts.empty())
    {
        dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        splash.showUiMsg(hmsg.arg(dicts.size()).arg("DictServer"));
    }
  }

  GD_DPRINTF( "Load done\n" );

  // Remove any stale index files
  {
  set< string > ids;
  std::pair< std::set< string >::iterator, bool > ret;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QTextCodec::setCodecForCStrings( QTextCodec::codecForName( "UTF8" ) );
#endif

  for( size_t x = dictionaries.size(); x--; )
  {
    ret = ids.insert( dictionaries[ x ]->getId() );
    if( !ret.second )
    {
      gdWarning( "Duplicate dictionary ID found: ID=%s, name=\"%s\", path=\"%s\"",
                 dictionaries[ x ]->getId().c_str(),
                 dictionaries[ x ]->getName().c_str(),
                 dictionaries[ x ]->getDictionaryFilenames().empty() ?
                   "" : dictionaries[ x ]->getDictionaryFilenames()[ 0 ].c_str()
                );
    }
  }

  QDir indexDir( Config::getIndexDir() );

  QStringList allIdxFiles = indexDir.entryList( QDir::Files );

  for( QStringList::const_iterator i = allIdxFiles.constBegin();
       i != allIdxFiles.constEnd(); ++i )
  {
    if ( ids.find( FsEncoding::encode( *i ) ) == ids.end()
         && i->size() == 32 )
      indexDir.remove( *i );
    else
    if ( i->endsWith( "_FTS" )
         && i->size() == 36
         && ids.find( FsEncoding::encode( i->left( 32 ) ) ) == ids.end() )
      indexDir.remove( *i );
  }
  }
  // Run deferred inits

  if ( doDeferredInit_ )
  {
    splash.showUiMsg(LoadDictionaries::tr("Init Dictionaries ..."));
    doDeferredInit( dictionaries );
  }
  }
  catch( std::exception & e )
  {
  }

  if(canHideParent)
    parent->show();
  splash.showUiMsg(LoadDictionaries::tr("Loading Done.%1%3 Dictionaries Handled%5%7").
                   arg(rn).arg(dictionaries.size()).arg(rn).arg(LoadDictionaries::tr("Time elapsed: %2 s").arg(timer.elapsed() / 1000)));
  splash.finish(parent);
}

void LoadDictionaries::doDeferredInit( std::vector< sptr< Dictionary::Class > > & dictionaries )
{
  for( unsigned x = 0; x < dictionaries.size(); ++x )
    dictionaries[ x ]->deferredInit();
}
