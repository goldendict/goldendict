/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QIcon>
#include "mainwindow.hh"
#include "config.hh"

#include "processwrapper.hh"
#include "hotkeywrapper.hh"

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

  QHotkeyApplication app( argc, argv );

  app.setApplicationName( "GoldenDict" );
  app.setOrganizationDomain( "http://goldendict.berlios.de/" );

  app.setWindowIcon( QIcon( ":/icons/programicon.png" ) );

  Config::Class cfg( Config::load() );


  // Prevent execution of the 2nd copy

  // get pid
  quint64 current_pid = QCoreApplication::applicationPid();

//  QString app_fname = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
//  quint64 pid = ProcessWrapper::findProcess(
//          app_fname.toAscii().data(),
//          current_pid);
//
//  qDebug() << "pid " << pid;
//  qDebug() << "current_pid " << current_pid;

  // check pid file
  QFile pid_file( Config::getPidFileName() );

  if ( pid_file.exists() ) // pid file exists, check it
  {
    pid_file.open( QIODevice::ReadWrite );
    QDataStream ds( &pid_file );
    quint64 tmp; ds >> tmp;
    pid_file.close();

    bool isExist = ProcessWrapper::processExists(tmp);
    if ( isExist )
    {
      puts( "Another GoldenDict copy started already." );
      return 1;
    }

//    if ( tmp == pid ) // it is active - exiting
//    {
//      // to do: switch to pid ?
//      return 1;
//    }
  }

  pid_file.open( QIODevice::WriteOnly );
  QDataStream ds( &pid_file );
  ds << current_pid;
  pid_file.close();


  // Load translations

  QTranslator qtTranslator;

  QString localeName = cfg.preferences.interfaceLanguage;

  if ( localeName.isEmpty() )
    localeName = QLocale::system().name();

  qtTranslator.load( "qt_" + localeName,
                     QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
  app.installTranslator( &qtTranslator );

  QTranslator translator;
  translator.load( QString( Config::getProgramDataDir() ) + "/locale/" + localeName );
  app.installTranslator( &translator );

  MainWindow m( cfg );

  int r = app.exec();

  // remove pid file
  pid_file.remove();

  return r;
}
