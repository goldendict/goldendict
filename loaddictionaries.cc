/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "loaddictionaries.hh"
#include "initializing.hh"
#include "config.hh"
#ifdef GD_BGL_SUPPORT
#include "bgl.hh"
#endif
#ifdef GD_STARDICT_SUPPORT
#include "stardict.hh"
#endif
#ifdef GD_LSA_SUPPORT
#include "lsa.hh"
#endif
#ifdef GD_DSL_SUPPORT
#include "dsl.hh"
#endif
#ifdef GD_MEDIAWIKI_SUPPORT
#include "mediawiki.hh"
#endif
#ifdef GD_SOUND_DIRS_SUPPORT
#include "sounddir.hh"
#endif
#ifdef GD_HUNSPELL_SUPPORT
#include "hunspell.hh"
#endif
#ifdef GD_DICTD_SUPPORT
#include "dictdfiles.hh"
#endif
#ifdef GD_TRANSLITERATION_SUPPORT
#include "chinese.hh"
#include "romaji.hh"
#include "russiantranslit.hh"
#include "german.hh"
#include "greektranslit.hh"
#include "belarusiantranslit.hh"
#endif
#ifdef GD_WEBSITE_SUPPORT
#include "website.hh"
#endif
#ifdef GD_FORVO_API_SUPPORT
#include "forvo.hh"
#endif
#ifdef GD_PROGRAM_SUPPORT
#include "programs.hh"
#endif
#ifdef GD_VOICE_ENGINE_SUPPORT
#include "voiceengines.hh"
#endif
#include "gddebug.hh"
#include "fsencoding.hh"
#ifdef GD_XDXF_SUPPORT
#include "xdxf.hh"
#endif
#ifdef GD_SDICT_SUPPORT
#include "sdict.hh"
#endif
#ifdef GD_AARD_SUPPORT
#include "aard.hh"
#endif
#ifdef GD_ZIPSOUNDS_SUPPORT
#include "zipsounds.hh"
#endif
#ifdef GD_MDICT_SUPPORT
#include "mdx.hh"
#endif
#ifdef GD_ZIM_SUPPORT
#include "zim.hh"
#endif
#ifdef GD_DICTSERVER_SUPPORT
#include "dictserver.hh"
#endif
#ifdef GD_SLOB_SUPPORT
#include "slob.hh"
#endif
#ifdef GD_GLS_SUPPORT
#include "gls.hh"
#endif

#ifdef GD_EPWING_SUPPORT
#include "epwing.hh"
#endif

#include <QSplashScreen>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QDir>
#include <QThreadPool>
#include <QEventLoop>
#include <QApplication>

#include <set>

using std::set;

using std::string;
using std::vector;

class DictNameFilter : public QStringList
{
public:
    DictNameFilter() {
        *this
#ifdef GD_BGL_SUPPORT
        << "*.bgl"
#endif
#ifdef GD_STARDICT_SUPPORT
        << "*.ifo"
#endif
#ifdef GD_SOUND_DIRS_SUPPORT
        << "*.lsa" << "*.dat" // ABBYY Lingvo audio files (.lsa and .dat)
#endif
#ifdef GD_DSL_SUPPORT
        << "*.dsl" << "*.dsl.dz"
#endif
#ifdef GD_DICTD_SUPPORT
        << "*.index"
#endif
#ifdef GD_XDXF_SUPPORT
        << "*.xdxf" << "*.xdxf.dz"
#endif
#ifdef GD_SDICT_SUPPORT
        << "*.dct"
#endif
#ifdef GD_AARD_SUPPORT
        << "*.aar"
#endif
        << "*.zips"  // Compressed sound packs (zips)
#ifdef GD_MDICT_SUPPORT
        << "*.mdx"
#endif
#ifdef GD_GLS_SUPPORT
        << "*.gls" << "*.gls.dz"
#endif
#ifdef GD_SLOB_SUPPORT
        << "*.slob"
#endif
#ifdef GD_ZIM_SUPPORT
        << "*.zim" << "*.zimaa"
#endif
#ifdef GD_EPWING_SUPPORT
        << "*catalogs"
#endif
        ;
    }
    ~DictNameFilter(){}
};

LoadDictionaries::LoadDictionaries( Config::Class const & cfg ,
                                    QElapsedTimer const & timer_ ,
                                    const bool doDeferredInit,
                                    QNetworkAccessManager &dtNetMgr,
                                    std::vector<sptr<Dictionary::Class> > &dicts )
    : dictionaries(dicts), timer(timer_), doDeferredInit_(doDeferredInit),
      cfg_(cfg), dictNetMgr(dtNetMgr)
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

        if(!exceptionText.empty() && dictionaries.empty())
        {
            emit showMessage(tr("Failed to Handle User's Dictionaries%1%3").arg(rn).
                             arg(QString::fromUtf8(exceptionText.c_str())));
            return;
        }
#endif
        handleOthers();
    }
    catch( std::exception & e )
    {
        exceptionText.append(e.what());
    }

    emit showMessage(tr("Finished Handling Dictionaries: %1%3%5").
                     arg(dictionaries.size()).arg(rn).
                     arg(tes.arg(timer.elapsed() / 1000)) );

    try {
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

        // Run deferred inits
        if ( doDeferredInit_ )
        {
            emit showMessage(tr("Init Dictionaries ..."));
            doDeferredInit( dictionaries );
        }
    }
    catch( std::exception & e )
    {
        exceptionText.append(e.what());
    }
}

void LoadDictionaries::handleOthers()
{
    const QString hmsg = tr("%1 %3 Dictionaries Handled");
#ifdef GD_SOUND_DIRS_SUPPORT
    // Make soundDirs
    {
        vector< sptr< Dictionary::Class > > soundDirDictionaries =
                SoundDir::makeDictionaries( cfg_.soundDirs, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), soundDirDictionaries.begin(),
                             soundDirDictionaries.end() );
        if(!soundDirDictionaries.empty())
            emit showMessage(hmsg.arg(soundDirDictionaries.size()).arg("SoundDir"));
    }
#endif
#ifdef GD_HUNSPELL_SUPPORT
    // Make hunspells
    {
        vector< sptr< Dictionary::Class > > hunspellDictionaries =
                HunspellMorpho::makeDictionaries( cfg_.hunspell );

        dictionaries.insert( dictionaries.end(), hunspellDictionaries.begin(),
                             hunspellDictionaries.end() );
        if(!hunspellDictionaries.empty())
            emit showMessage(hmsg.arg(hunspellDictionaries.size()).arg("HunspellMorpho"));
    }
#endif
    ////////////////////////////////////////////////////////////////////////////
    ///// We create transliterations synchronously since they are very simple
#ifdef GD_TRANSLITERATION_SUPPORT
    // Make Chinese conversion
    {
        vector< sptr< Dictionary::Class > > chineseDictionaries =
                Chinese::makeDictionaries( cfg_.transliteration.chinese );

        if(!chineseDictionaries.empty())
        {
            dictionaries.insert( dictionaries.end(), chineseDictionaries.begin(),
                                 chineseDictionaries.end() );
            emit showMessage(hmsg.arg(chineseDictionaries.size()).arg("Chinese-conversion"));
        }
    }

    // Make Romaji
    {
        vector< sptr< Dictionary::Class > > romajiDictionaries =
                Romaji::makeDictionaries( cfg_.transliteration.romaji );

        if(!romajiDictionaries.empty())
        {
            dictionaries.insert( dictionaries.end(), romajiDictionaries.begin(),
                                 romajiDictionaries.end() );
            emit showMessage(hmsg.arg(romajiDictionaries.size()).arg("Romaji"));
        }
    }

    size_t transliteration_dc = dictionaries.size();
    // Make Russian transliteration
    if ( cfg_.transliteration.enableRussianTransliteration )
        dictionaries.push_back( RussianTranslit::makeDictionary() );

    // Make German transliteration
    if ( cfg_.transliteration.enableGermanTransliteration )
        dictionaries.push_back( GermanTranslit::makeDictionary() );

    // Make Greek transliteration
    if ( cfg_.transliteration.enableGreekTransliteration )
        dictionaries.push_back( GreekTranslit::makeDictionary() );

    // Make Belarusian transliteration
    if ( cfg_.transliteration.enableBelarusianTransliteration )
    {
        vector< sptr< Dictionary::Class > > dicts = BelarusianTranslit::makeDictionaries();
        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
        }
    }

    transliteration_dc =  dictionaries.size() - transliteration_dc;
    if(transliteration_dc)
        emit showMessage(hmsg.arg(transliteration_dc).arg("Transliteration"));
#endif

#ifdef GD_MEDIAWIKI_SUPPORT
    ///// We create MediaWiki dicts synchronously, since they use netmgr
    {
        vector< sptr< Dictionary::Class > > dicts =
                MediaWiki::makeDictionaries( *this, cfg_.mediawikis, dictNetMgr );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("MediaWiki"));
        }
    }
#endif
#ifdef GD_WEBSITE_SUPPORT
    ///// WebSites are very simple, no need to create them asynchronously
    {
        vector< sptr< Dictionary::Class > > dicts =
                WebSite::makeDictionaries( cfg_.webSites, dictNetMgr );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("WebSite"));
        }
    }
#endif
#ifdef GD_FORVO_API_SUPPORT
    //// Forvo dictionaries
    {
        vector< sptr< Dictionary::Class > > dicts =
                Forvo::makeDictionaries( *this, cfg_.forvo, dictNetMgr );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("Forvo"));
        }
    }
#endif
#ifdef GD_PROGRAM_SUPPORT
    //// Programs
    {
        vector< sptr< Dictionary::Class > > dicts =
                Programs::makeDictionaries( cfg_.programs );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("Programs"));
        }
    }
#endif
#ifdef GD_VOICE_ENGINE_SUPPORT
    //// Text to Speech
    {
        vector< sptr< Dictionary::Class > > dicts =
                VoiceEngines::makeDictionaries( cfg_.voiceEngines );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("Text-to-Speech"));
        }
    }
#endif
#ifdef GD_DICTSERVER_SUPPORT
    {
        vector< sptr< Dictionary::Class > > dicts =
                DictServer::makeDictionaries( cfg_.dictServers );

        if(!dicts.empty())
        {
            dictionaries.insert( dictionaries.end(), dicts.begin(), dicts.end() );
            emit showMessage(hmsg.arg(dicts.size()).arg("DictServer"));
        }
    }
#endif
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
    int maxThreadCount = tp->maxThreadCount();;
    if( maxThreadCount <= 1 )
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
#ifdef GD_BGL_SUPPORT
    {
        vector< sptr< Dictionary::Class > > bglDictionaries =
                Bgl::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), bglDictionaries.begin(),
                             bglDictionaries.end() );
    }
#endif
#ifdef GD_STARDICT_SUPPORT
    {
        vector< sptr< Dictionary::Class > > stardictDictionaries =
                Stardict::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

        dictionaries.insert( dictionaries.end(), stardictDictionaries.begin(),
                             stardictDictionaries.end() );
    }
#endif
#ifdef GD_LSA_SUPPORT
    {
        vector< sptr< Dictionary::Class > > lsaDictionaries =
                Lsa::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), lsaDictionaries.begin(),
                             lsaDictionaries.end() );
    }
#endif
#ifdef GD_DSL_SUPPORT
    {
        vector< sptr< Dictionary::Class > > dslDictionaries =
                Dsl::makeDictionaries(
                    allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxPictureWidth, cfg_.maxHeadwordSize );

        dictionaries.insert( dictionaries.end(), dslDictionaries.begin(),
                             dslDictionaries.end() );
    }
#endif
#ifdef GD_DICTD_SUPPORT
    {
        vector< sptr< Dictionary::Class > > dictdDictionaries =
                DictdFiles::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), dictdDictionaries.begin(),
                             dictdDictionaries.end() );
    }
#endif
#ifdef GD_XDXF_SUPPORT
    {
        vector< sptr< Dictionary::Class > > xdxfDictionaries =
                Xdxf::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), xdxfDictionaries.begin(),
                             xdxfDictionaries.end() );
    }
#endif
#ifdef GD_SDICT_SUPPORT
    {
        vector< sptr< Dictionary::Class > > sdictDictionaries =
                Sdict::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), sdictDictionaries.begin(),
                             sdictDictionaries.end() );
    }
#endif
#ifdef GD_AARD_SUPPORT
    {
        vector< sptr< Dictionary::Class > > aardDictionaries =
                Aard::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

        dictionaries.insert( dictionaries.end(), aardDictionaries.begin(),
                             aardDictionaries.end() );
    }
#endif
#ifdef GD_ZIPSOUNDS_SUPPORT
    {
        vector< sptr< Dictionary::Class > > zipSoundsDictionaries =
                ZipSounds::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), zipSoundsDictionaries.begin(),
                             zipSoundsDictionaries.end() );
    }
#endif
#ifdef GD_MDICT_SUPPORT
    {
        vector< sptr< Dictionary::Class > > mdxDictionaries =
                Mdx::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), mdxDictionaries.begin(),
                             mdxDictionaries.end() );
    }
#endif
#ifdef GD_GLS_SUPPORT
    {
        vector< sptr< Dictionary::Class > > glsDictionaries =
                Gls::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this );

        dictionaries.insert( dictionaries.end(), glsDictionaries.begin(),
                             glsDictionaries.end() );
    }
#endif
#ifdef GD_ZIM_SUPPORT
    {
        vector< sptr< Dictionary::Class > > zimDictionaries =
                Zim::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

        dictionaries.insert( dictionaries.end(), zimDictionaries.begin(),
                             zimDictionaries.end() );
    }
#endif
#ifdef GD_SLOB_SUPPORT
    {
        vector< sptr< Dictionary::Class > > slobDictionaries =
                Slob::makeDictionaries( allFiles, FsEncoding::encode( Config::getIndexDir() ), *this, cfg_.maxHeadwordsToExpand );

        dictionaries.insert( dictionaries.end(), slobDictionaries.begin(),
                             slobDictionaries.end() );
    }
#endif
#ifdef GD_EPWING_SUPPORT
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

    GDSplash splash(parent, !canHideParent ? Qt::WindowStaysOnTopHint : Qt::WindowFlags());
    splash.show();
    splash.showUiMsg(LoadDictionaries::tr("Start Loading Dictionaries"));
    bool pVisible = parent->isVisible();
    if(canHideParent && pVisible)
        parent->hide();

    const int expiryTimeout = QThreadPool::globalInstance()->expiryTimeout();
    QThreadPool::globalInstance()->setExpiryTimeout(-1);

    dictionaries.clear();

    // Start a thread to load all the dictionaries

    LoadDictionaries loadDicts( cfg, timer, doDeferredInit_, dictNetMgr, dictionaries );
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
            if(canHideParent && pVisible)
                parent->show();
            splash.finish(parent);
            return;
        }
    }

    if(canHideParent && pVisible)
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
