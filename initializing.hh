/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __INITIALIZING_HH_INCLUDED__
#define __INITIALIZING_HH_INCLUDED__
#include <QSplashScreen>
#include <QMouseEvent>

/*
#include <QDialog>
#include "ui_initializing.h"

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 ) ) && defined( Q_OS_WIN32 )

#include <QtWidgets/QStyleFactory>

class WindowsStyle {
public:
  /// The class is a singleton.
  static WindowsStyle & instance();

  QStyle * getStyle()
  { return style; }

private:
  WindowsStyle();

  QStyle * style;
};

#endif
*/

class GDSplash : public QSplashScreen
{
    Q_OBJECT
public:
    GDSplash() : QSplashScreen(QPixmap("./splash.png"), Qt::WindowStaysOnTopHint)
    {
        setMinimumSize(QSize(600, 400));
    }
    ~GDSplash() {}

    void showUiMsg(const QString &msg, const QColor &color = Qt::black);
protected Q_SLOTS:
    void showMessage(const QString &msg, const QColor &color = Qt::black);

protected:
    void mousePressEvent(QMouseEvent *e)
    {
        e->ignore();
    }
};

#endif

