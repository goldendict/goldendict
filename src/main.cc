/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QApplication>
#include <QIcon>
#include "mainwindow.hh"
#include "config.hh"
#include "processwrapper.hh"

//#define __DO_DEBUG

#ifdef __DO_DEBUG
#include <sys/resource.h>
#endif

int main( int argc, char ** argv )
{
  #ifdef __DO_DEBUG
  {
    rlimit limit;

    memset( &limit, 0, sizeof( limit ) );
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit( RLIMIT_CORE, &limit );
  }
  #endif

  QApplication app( argc, argv );

  app.setApplicationName( "GoldenDict" );
  app.setOrganizationDomain( "http://goldendict.berlios.de/" );

  app.setWindowIcon( QIcon( ":/icons/programicon.png" ) );

  Config::Class cfg( Config::load() );


  // Prevent execution of the 2nd copy

  // check if 2nd copy was started
  QString app_fname = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
  unsigned int pid = ProcessWrapper::findProcess(
          app_fname.toAscii().data(),
          QCoreApplication::applicationPid());
  if (pid)
  {
    // to do: switch to pid ?
    return 1;
  }

  // Load translations

  QTranslator qtTranslator;

  QString localeName = cfg.preferences.interfaceLanguage;

  if ( localeName.isEmpty() )
    localeName = QLocale::system().name();
  {
  }

  qtTranslator.load( "qt_" + localeName,
                     QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
  app.installTranslator( &qtTranslator );

  QTranslator translator;
  translator.load( QString( Config::getProgramDataDir() ) + "/locale/" + localeName );
  app.installTranslator( &translator );

  // Apply qt stylesheet
  {
    QFile builtInCssFile( ":/qt-style.css" );
    builtInCssFile.open( QFile::ReadOnly );
    QByteArray css = builtInCssFile.readAll();

    // Try loading a style sheet if there's one
    QFile cssFile( Config::getUserQtCssFileName() );

    if ( cssFile.open( QFile::ReadOnly ) )
      css += cssFile.readAll();

    app.setStyleSheet( css );
  }

  MainWindow m( cfg );

  return app.exec();
}
