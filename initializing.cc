/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QIcon>
#include "initializing.hh"
#include <QCloseEvent>

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )
#include <qt_windows.h>
#include <uxtheme.h>

WindowsStyle::WindowsStyle()
{
  style = QStyleFactory::create( "windows" );
}

WindowsStyle & WindowsStyle::instance()
{
  static WindowsStyle ws;
  return ws;
}

#endif
/*
Initializing::Initializing( QWidget * parent, bool showOnStartup ): QDialog( parent )
{
  ui.setupUi( this );
  setWindowFlags( Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
                  Qt::WindowMinimizeButtonHint );

  #ifndef Q_OS_MAC
    setWindowIcon( QIcon( ":/icons/programicon.png" ) );
  #else
    setWindowIcon( QIcon( ":/icons/macicon.png" ) );
  #endif

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

  // Style "windowsvista" in Qt5 turn off progress bar animation for classic appearance
  // We use simply "windows" style instead for this case

  oldBarStyle = 0;

  if( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA
      && ( QSysInfo::windowsVersion() & QSysInfo::WV_NT_based )
      && !IsThemeActive() )
  {
    QStyle * barStyle = WindowsStyle::instance().getStyle();

    if( barStyle )
    {
      oldBarStyle = ui.progressBar->style();
      ui.progressBar->setStyle( barStyle );
    }
  }

#endif

  if ( showOnStartup )
  {
    ui.operation->setText( tr( "Please wait..." ) );
    ui.dictionary->hide();
    ui.progressBar->hide();
    show();
  }
}

void Initializing::indexing( QString const & dictionaryName )
{
  ui.operation->setText( tr( "Please wait while indexing dictionary" ) );
  ui.dictionary->setText( dictionaryName );
  ui.dictionary->show();
  ui.progressBar->show();
  adjustSize();
  show();
}

void Initializing::closeEvent( QCloseEvent * ev )
{
  ev->ignore();
}

void Initializing::reject()
{
}

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

Initializing::~Initializing()
{
  if( oldBarStyle )
    ui.progressBar->setStyle( oldBarStyle );
}
#endif
*/
