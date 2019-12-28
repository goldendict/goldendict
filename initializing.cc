/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "initializing.hh"
#include <QApplication>
#include <QDesktopWidget>
/*
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

#endif
*/

GDSplash::GDSplash() : QSplashScreen(QPixmap("./splash.png").scaled(QApplication::desktop()->screenGeometry().size() * 0.4))
{
}

void GDSplash::showUiMsg(const QString &msg, const QColor &color, int align) {
    QSplashScreen::showMessage(msg, align, color);
    qApp->processEvents();
}

