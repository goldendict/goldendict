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

GDSplash::GDSplash(QWidget *parent, Qt::WindowFlags f, const QString &picpath) :
    QSplashScreen(parent, QPixmap(), f)
{
    QPixmap qpm(picpath);
    const QRect dr = QApplication::desktop()->screenGeometry();
    int pw = dr.width();
    int ph = dr.height();
    QSplashScreen::setPixmap(qpm.scaled(pw * 0.4, (ph < pw ? ph : pw) * 0.25));
}

void GDSplash::showUiMsg(const QString &msg, const QColor &color, int align) {
    QSplashScreen::showMessage(msg, align, color);
    qApp->processEvents();
}

