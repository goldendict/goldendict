/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <stdio.h>
#include <QIcon>
#include "gdappstyle.hh"
#include "mainwindow.hh"
#include "config.hh"

#include "processwrapper.hh"
#include "hotkeywrapper.hh"

//#define __DO_DEBUG

#define LOG_TO_FILE_KEY "--log-to-file"

#ifdef Q_OS_WIN32
#include <QtCore/qt_windows.h>
#endif

#ifdef __DO_DEBUG
#include <sys/resource.h>
#endif

#include "termination.hh"
#include "atomic_rename.hh"

#include <QWebSecurityOrigin>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QString>

#include "gddebug.hh"

#ifdef Q_OS_MAC
#include "lionsupport.h"
#endif

void gdMessageHandler( QtMsgType type, const char *msg )
{
  QString message = QString::fromUtf8( msg );
  switch (type) {

    case QtDebugMsg:
      if( logFile.isOpen() )
        message.insert( 0, "Debug: " );
      else
        fprintf(stderr, "Debug: %s\n", msg);
      break;

    case QtWarningMsg:
      if( logFile.isOpen() )
        message.insert( 0, "Warning: " );
      else
        fprintf(stderr, "Warning: %s\n", msg);
      break;

    case QtCriticalMsg:
      if( logFile.isOpen() )
        message.insert( 0, "Critical: " );
      else
        fprintf(stderr, "Critical: %s\n", msg);
      break;

    case QtFatalMsg:
      if( logFile.isOpen() )
      {
        logFile.write( "Fatal: " );
        logFile.write( msg );
        logFile.flush();
      }
      else
        fprintf(stderr, "Fatal: %s\n", msg);
      abort();
  }

  if( logFile.isOpen() )
  {
    message.append( "\n" );
    logFile.write( message.toUtf8() );
    logFile.flush();
  }
}

int main( int argc, char ** argv )
{
  #ifdef Q_OS_MAC
    setenv("LANG", "en_US.UTF-8", 1);

   // Check for retina display
   if( LionSupport::isRetinaDisplay() )
     QApplication::setGraphicsSystem( "native" );
   else
     QApplication::setGraphicsSystem( "raster" );
  #endif

  // The following clause fixes a race in the MinGW runtime where throwing
  // exceptions for the first time in several threads simultaneously can cause
  // an abort(). This code throws first exception in a safe, single-threaded
  // manner, thus avoiding that race.
  {
    class Dummy {};

    try
    { throw Dummy(); }
    catch( Dummy )
    {}
  }

  if ( argc == 3 && strcmp( argv[ 1 ], "--show-error-file" ) == 0 )
  {
    // The program has crashed -- show a message about it

    QApplication app( argc, argv );

    QFile errFile( argv[ 2 ] );

    QString errorText;

    if ( errFile.open( QFile::ReadOnly ) )
      errorText = QString::fromUtf8( errFile.readAll() );

    errorText += "\n" + QString( "This information is located in file %1, "
                                 "which will be removed once you close this dialog.").arg( errFile.fileName() );

    QMessageBox::critical( 0, "GoldenDict has crashed", errorText );

    errFile.remove();

    return 0;
  }

  installTerminationHandler();

  #ifdef __DO_DEBUG
  {
    rlimit limit;

    memset( &limit, 0, sizeof( limit ) );
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit( RLIMIT_CORE, &limit );
  }
  #endif

#ifdef __WIN32

  // Under Windows, increase the amount of fopen()-able file descriptors from
  // the default 512 up to 2048.
  _setmaxstdio( 2048 );

#endif

  QHotkeyApplication app( "GoldenDict", argc, argv );

  if ( app.isRunning() )
  {
    if( argc == 2 && strcmp( argv[ 1 ], LOG_TO_FILE_KEY ) != 0 )
#ifdef Q_OS_WIN32
    {
      LPWSTR * pstr;
      int num;
      pstr = CommandLineToArgvW( GetCommandLineW(), &num );
      if( pstr && num > 0 )
        app.sendMessage( QString( "translateWord: " ) + QString::fromWCharArray( pstr[1] ) );
    }
#else
      app.sendMessage( QString( "translateWord: " ) + QString::fromLocal8Bit( argv[1] ) );
#endif
    else
      app.sendMessage("bringToFront");
    return 0; // Another instance is running
  }

  app.setApplicationName( "GoldenDict" );
  app.setOrganizationDomain( "http://goldendict.org/" );
#if QT_VERSION >= 0x040600
  app.setStyle(new GdAppStyle);
#endif

  #ifndef Q_OS_MAC
    app.setWindowIcon( QIcon( ":/icons/programicon.png" ) );
  #endif

  // Load translations for system locale

  QTranslator qtTranslator;

  QString localeName = QLocale::system().name();

  if ( !qtTranslator.load( "qt_" + localeName, Config::getLocDir() ) )
    qtTranslator.load( "qt_" + localeName,
                       QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );

  app.installTranslator( &qtTranslator );

  QTranslator translator;

  translator.load( Config::getLocDir() + "/" + localeName );

  app.installTranslator( &translator );

  Config::Class cfg;
  for( ; ; )
  {
    try
    {
      cfg = Config::load();
    }
    catch( Config::exError )
    {
      QMessageBox mb( QMessageBox::Warning, app.applicationName(),
                      app.translate( "Main", "Error in configuration file. Continue with default settings?" ),
                      QMessageBox::Yes | QMessageBox::No );
      mb.exec();
      if( mb.result() != QMessageBox::Yes )
        return -1;

      QString configFile = Config::getConfigFileName();
      renameAtomically( configFile, configFile + ".bad" );
      continue;
    }
    break;
  }

  if( argc == 2 && strcmp( argv[ 1 ], LOG_TO_FILE_KEY ) == 0 )
  {
    // Open log file
    logFile.setFileName( Config::getConfigDir() + "gd_log.txt" );
    logFile.remove();
    logFile.open( QFile::ReadWrite );

    // Write UTF-8 BOM
    QByteArray line;
    line.append( 0xEF ).append( 0xBB ).append( 0xBF );
    logFile.write( line );

    // Install message handler
    qInstallMsgHandler( gdMessageHandler );
  }

  if ( Config::isPortableVersion() )
  {
    // For portable version, hardcode some settings

    cfg.paths.clear();
    cfg.paths.push_back( Config::Path( Config::getPortableVersionDictionaryDir(), true ) );
    cfg.soundDirs.clear();
    cfg.hunspell.dictionariesPath = Config::getPortableVersionMorphoDir();
  }

  // Reload translations for user selected locale is nesessary

  if( !cfg.preferences.interfaceLanguage.isEmpty() && localeName != cfg.preferences.interfaceLanguage )
  {
    localeName = cfg.preferences.interfaceLanguage;

    if ( !qtTranslator.load( "qt_" + localeName, Config::getLocDir() ) )
      qtTranslator.load( "qt_" + localeName,
                                 QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );

    translator.load( Config::getLocDir() + "/" + localeName );
  }

  // Prevent app from quitting spontaneously when it works with scan popup
  // and with the main window closed.
  app.setQuitOnLastWindowClosed( false );

#if QT_VERSION >= 0x040600
  // Add the dictionary scheme we use as local, so that the file:// links would
  // work in the articles. The function was introduced in Qt 4.6.
  QWebSecurityOrigin::addLocalScheme( "gdlookup" );
#endif

  MainWindow m( cfg );

  app.addDataCommiter( m );

  QObject::connect( &app, SIGNAL(messageReceived(const QString&)),
    &m, SLOT(messageFromAnotherInstanceReceived(const QString&)));

  if( argc == 2 && strcmp( argv[ 1 ], LOG_TO_FILE_KEY ) != 0)
#ifdef Q_OS_WIN32
  {
    LPWSTR * pstr;
    int num;
    pstr = CommandLineToArgvW( GetCommandLineW(), &num );
    if( pstr && num > 0 )
      m.wordReceived( QString::fromWCharArray( pstr[1] ) );
  }
#else
    m.wordReceived( QString::fromLocal8Bit( argv[1] ) );
#endif

  int r = app.exec();

  app.removeDataCommiter( m );

  return r;
}
