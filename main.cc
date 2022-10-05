/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <stdio.h>
#include <QIcon>
#include "gdappstyle.hh"
#include "article_urlschemehandler.hh"
#include "mainwindow.hh"
#include "config.hh"

#include "processwrapper.hh"
#include "hotkeywrapper.hh"
#ifdef HAVE_X11
#include <fixx11h.h>
#endif

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

#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <QString>
#include <QTextStream>
#include <QUrl>

#include "gddebug.hh"

#ifdef USE_QTWEBKIT
#include <QWebSecurityOrigin>
#endif

#if defined( Q_OS_MAC ) && QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include "lionsupport.h"
#endif

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) )

void gdMessageHandler( QtMsgType type, const QMessageLogContext &context, const QString &mess )
{
  Q_UNUSED( context );
  QString message( mess );
  QByteArray msg = message.toUtf8().constData();

#else

void gdMessageHandler( QtMsgType type, const char *msg_ )
{
  QString message = QString::fromUtf8( msg_ );
  QByteArray msg = QByteArray::fromRawData( msg_, strlen( msg_ ) );

#endif

  switch (type) {

    case QtDebugMsg:
      if( logFilePtr && logFilePtr->isOpen() )
        message.insert( 0, "Debug: " );
      else
        fprintf(stderr, "Debug: %s\n", msg.constData());
      break;

    case QtWarningMsg:
      if( logFilePtr && logFilePtr->isOpen() )
        message.insert( 0, "Warning: " );
      else
        fprintf(stderr, "Warning: %s\n", msg.constData());
      break;

    case QtCriticalMsg:
      if( logFilePtr && logFilePtr->isOpen() )
        message.insert( 0, "Critical: " );
      else
        fprintf(stderr, "Critical: %s\n", msg.constData());
      break;

    case QtFatalMsg:
      if( logFilePtr && logFilePtr->isOpen() )
      {
        logFilePtr->write( "Fatal: " );
        logFilePtr->write( msg );
        logFilePtr->flush();
      }
      else
        fprintf(stderr, "Fatal: %s\n", msg.constData());
      abort();

#if QT_VERSION >= QT_VERSION_CHECK( 5, 5, 0 )
    case QtInfoMsg:
      if( logFilePtr && logFilePtr->isOpen() )
        message.insert( 0, "Info: " );
      else
        fprintf(stderr, "Info: %s\n", msg.constData());
      break;
#endif
  }

  if( logFilePtr && logFilePtr->isOpen() )
  {
    message.append( "\n" );
    logFilePtr->write( message.toUtf8() );
    logFilePtr->flush();
  }
}

class GDCommandLine
{
  bool showHelp, toggleScanPopup, logFile;
  QString word, groupName, popupGroupName;
  QVector< QString > arguments;
public:
  GDCommandLine( int argc, char **argv );

  bool needShowHelp() const
  { return showHelp; }

  inline bool needToggleScanPopup()
  { return toggleScanPopup; }

  inline bool needSetGroup()
  { return !groupName.isEmpty(); }

  inline QString getGroupName()
  { return groupName; }

  inline bool needSetPopupGroup()
  { return !popupGroupName.isEmpty(); }

  inline QString getPopupGroupName()
  { return popupGroupName; }

  inline bool needLogFile()
  { return logFile; }

  inline bool needTranslateWord()
  { return !word.isEmpty(); }

  inline QString wordToTranslate()
  { return word; }

private:
  void handleUriSchemes();
};

GDCommandLine::GDCommandLine( int argc, char **argv ):
showHelp( false ),
toggleScanPopup( false ),
logFile( false )
{
  if( argc > 1 )
  {
#ifdef Q_OS_WIN32
    (void) argv;
    int num;
    LPWSTR *pstr = CommandLineToArgvW( GetCommandLineW(), &num );
    if( pstr && num > 1 )
    {
      for( int i = 1; i < num; i++ )
        arguments.push_back( QString::fromWCharArray( pstr[ i ] ) );
    }
#else
    for( int i = 1; i < argc; i++ )
      arguments.push_back( QString::fromLocal8Bit( argv[ i ] ) );
#endif
    // Parse command line
    for( int i = 0; i < arguments.size(); i++ )
    {
      if( arguments[ i ].compare( "-h" ) == 0 || arguments[ i ].compare( "--help" ) == 0 )
      {
        showHelp = true;
        continue;
      }
      else
      if( arguments[ i ].compare( "--toggle-scan-popup" ) == 0 )
      {
        toggleScanPopup = true;
        continue;
      }
      else
      if( arguments[ i ].compare( "--log-to-file" ) == 0 )
      {
        logFile = true;
        continue;
      }
      else
      if( arguments[ i ].startsWith( "--group-name=" ) )
      {
        groupName = arguments[ i ].mid( arguments[ i ].indexOf( '=' ) + 1 );
        continue;
      }
      else
      if( arguments[ i ].startsWith( "--popup-group-name=" ) )
      {
        popupGroupName = arguments[ i ].mid( arguments[ i ].indexOf( '=' ) + 1 );
        continue;
      }
      else
        word = arguments[ i ];
    }

    handleUriSchemes();
  }
}

void GDCommandLine::handleUriSchemes()
{
  if( word.isEmpty() )
    return;

  static QLatin1String const uriSchemes[] = { QLatin1String( "goldendict://" ),
                                              QLatin1String( "dict://" ) };
  static size_t const uriSchemeCount = sizeof( uriSchemes ) / sizeof( uriSchemes[ 0 ] );
  for( size_t i = 0; i < uriSchemeCount; ++i )
  {
    QLatin1String const scheme = uriSchemes[ i ];
    if( !word.startsWith( scheme ) )
      continue;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    int schemeSize = scheme.size();
#else
    int schemeSize = strlen( scheme.latin1() );
#endif
    if( word.size() > schemeSize && word.at( schemeSize ) == QLatin1Char( '/' ) )
      ++schemeSize; // support dict:///word as well

    word.remove( 0, schemeSize );

    // An URI can end with a trailing slash, which has to be removed here. This should be more common
    // than a deliberate lookup of the slash character or a string that ends with a slash.
    // If word equals '/' (size 1), then word_ ends with 4 slashes, in which case translate the slash.
    if( word.size() > 1 && word.at( word.size() - 1 ) == QLatin1Char( '/' ) )
      word.chop( 1 );

    word = QUrl::fromPercentEncoding( word.toUtf8() );
    break;
  }
}

class LogFilePtrGuard
{
  QFile logFile;
  Q_DISABLE_COPY( LogFilePtrGuard )  
public:
  LogFilePtrGuard() { logFilePtr = &logFile; }
  ~LogFilePtrGuard() { logFilePtr = 0; }
};

int main( int argc, char ** argv )
{
#ifdef Q_OS_UNIX
    // GoldenDict use lots of X11 functions and it currently cannot work
    // natively on Wayland. This workaround will force GoldenDict to use
    // XWayland.
    char * xdg_envc = getenv("XDG_SESSION_TYPE");
    QString xdg_session = xdg_envc ? QString::fromLatin1(xdg_envc) : QString();
    if (!QString::compare(xdg_session, QString("wayland"), Qt::CaseInsensitive))
    {
        setenv("QT_QPA_PLATFORM", "xcb", 1);
    }
#endif
  #ifdef Q_OS_MAC
    setenv("LANG", "en_US.UTF-8", 1);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
   // Check for retina display
   if( LionSupport::isRetinaDisplay() )
     QApplication::setGraphicsSystem( "native" );
   else
     QApplication::setGraphicsSystem( "raster" );
#endif
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

#if defined( Q_OS_UNIX )
  setlocale( LC_ALL, "" ); // use correct char set mapping
#endif

  // --show-error-file is an undocumented, hidden option passed on exit by a crashing GoldenDict instance in its
  // terminate handler. Therefore, we can assume that, if present, --show-error-file is the first command-line
  // argument, that the second argument is the error file name and that there are no more arguments.
  if( argc == 3 && strcmp( argv[ 1 ], "--show-error-file" ) == 0 )
  {
    // The program has crashed -- show a message about it

#ifdef Q_OS_WIN32
    int numArgs;
    LPWSTR *pstr = CommandLineToArgvW( GetCommandLineW(), &numArgs );
    Q_ASSERT( pstr );
    Q_ASSERT( numArgs == 3 );
    QString const errorFileName = QString::fromWCharArray( pstr[ 2 ] );
#else
    QString const errorFileName = QString::fromLocal8Bit( argv[ 2 ] );
#endif

    argc = 1; // the two arguments are parsed just above
    QApplication app( argc, argv );

    QFile errFile( errorFileName );

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

#ifndef USE_QTWEBKIT
  registerArticleUrlSchemes();
#endif

  QHotkeyApplication app( "GoldenDict", argc, argv );
  LogFilePtrGuard logFilePtrGuard;

  GDCommandLine gdcl( argc, argv );

  bool const showHelpAndExit = gdcl.needShowHelp();
  // Ignore other command-line arguments if help is requested.
  if( !showHelpAndExit && app.isRunning() )
  {
    bool wasMessage = false;

    if( gdcl.needToggleScanPopup() )
    {
      app.sendMessage( "toggleScanPopup" );
      wasMessage = true;
    }

    if( gdcl.needSetGroup() )
    {
      app.sendMessage( QString( "setGroup: " ) + gdcl.getGroupName() );
      wasMessage = true;
    }

    if( gdcl.needSetPopupGroup() )
    {
      app.sendMessage( QString( "setPopupGroup: " ) + gdcl.getPopupGroupName() );
      wasMessage = true;
    }

    if( gdcl.needTranslateWord() )
    {
      app.sendMessage( QString( "translateWord: " ) + gdcl.wordToTranslate() );
      wasMessage = true;
    }

    if( !wasMessage )
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

#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
  // OpenCC needs to load it's data files by relative path on Windows and OS X
  QDir::setCurrent( Config::getProgramDataDir() );
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

  // Show help after loading a system locale translator to translate the help message.
  // Show help before loading Config to avoid conflicts with a possible another running
  // GoldenDict instance. An unfortunate consequence of not loading Config is that the
  // interface language configured in GoldenDict Preferences does not affect the help message.
  if( showHelpAndExit )
  {
    QString const helpMessage = QCoreApplication::translate( "CommandLineHelp",
        "Usage:\n"
        "  goldendict [OPTION]... [WORD|URI]\n\n"
        "GoldenDict dictionary lookup program\n\n"
        "  WORD\t\t\t\tA word or quoted phrase to translate\n"
        "  URI\t\t\t\t\"[golden]dict://[/]word or phrase to translate[/]\"\n\n"
        "Options:\n"
        "  -h, --help\t\t\tShow command-line help and exit\n"
        "  --log-to-file\t\t\tTurn on debug mode\n"
        "  --toggle-scan-popup\t\tToggle scanning mode on/off\n"
        "  --group-name=GROUP\t\tSet current group of dictionaries in the main window to GROUP\n"
        "  --popup-group-name=GROUP\tSet current group of dictionaries in the popup window to GROUP\n\n"
        "If another GoldenDict instance is running, the second instance exits immediately, the options"
        " --toggle-scan-popup, --group-name, --popup-group-name affect the running instance,"
        " WORD or URI is translated in the running instance's popup window.\n"
        "Otherwise, all options affect the initial state of the new instance,"
        " WORD or URI is translated in the new instance's main window.\n\n"
        "Assigning a global keyboard shortcut to the \"goldendict --toggle-scan-popup\""
        " command in system preferences emulates a scanning-mode-toggling global hotkey."
                                                            );

#ifdef Q_OS_WIN32
    // GoldenDict has no console under Windows. Show help in a message box
    // on this platform, as QCommandLineParser documentation recommends.
    QMessageBox::information( 0, QCoreApplication::translate( "CommandLineHelp", "GoldenDict command-line help" ),
                              helpMessage );
#else
    QTextStream out( stdout, QIODevice::WriteOnly );
    out << helpMessage << '\n';
#endif

    return 0;
  }

  Config::Class cfg;
  for( ; ; )
  {
    try
    {
      cfg = Config::load();
    }
    catch( Config::exError & )
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

  if( gdcl.needLogFile() )
  {
    // Open log file
    logFilePtr->setFileName( Config::getConfigDir() + "gd_log.txt" );
    logFilePtr->remove();
    logFilePtr->open( QFile::ReadWrite );

    // Write UTF-8 BOM
    QByteArray line;
    line.append( 0xEF ).append( 0xBB ).append( 0xBF );
    logFilePtr->write( line );

    // Install message handler
#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) )
    qInstallMessageHandler( gdMessageHandler );
#else
    qInstallMsgHandler( gdMessageHandler );
#endif
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

#ifdef USE_QTWEBKIT
  // registerArticleUrlSchemes() is responsible for similar configuration in the Qt WebEngine version.
  // Add the dictionary schemes we use as local to make file:// links work in articles.
  QWebSecurityOrigin::addLocalScheme( "gdlookup" );
  QWebSecurityOrigin::addLocalScheme( "gdpicture" );
#endif

  MainWindow m( cfg );

  app.addDataCommiter( m );

  QObject::connect( &app, SIGNAL(messageReceived(const QString&)),
    &m, SLOT(messageFromAnotherInstanceReceived(const QString&)));

  if( gdcl.needToggleScanPopup() )
    m.toggleScanPopup();

  if( gdcl.needSetGroup() )
    m.setGroupByName( gdcl.getGroupName(), true );

  if( gdcl.needSetPopupGroup() )
    m.setGroupByName( gdcl.getPopupGroupName(), false );

  if( gdcl.needTranslateWord() )
    m.wordReceived( gdcl.wordToTranslate() );

  int r = app.exec();

  app.removeDataCommiter( m );

  if( logFilePtr->isOpen() )
    logFilePtr->close();

  return r;
}
