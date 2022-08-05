/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEWEBVIEW_HH_INCLUDED__
#define __ARTICLEWEBVIEW_HH_INCLUDED__

#include "config.hh"
#include <QEvent>
#include <QMouseEvent>
#include <QWebEngineView>
#include <QPointer>

/// A thin wrapper around QWebEngineView to accommodate to some ArticleView's needs.
/// Currently the only added features:
/// 1. Ability to know if the middle mouse button is pressed or not according
///    to the view's current state. This is used to open links in new tabs when
///    they are clicked with middle button. There's also an added possibility to
///    get double-click events after the fact with the doubleClicked() signal.
class ArticleWebView: public QWebEngineView
{
  Q_OBJECT

public:

  ArticleWebView( QWidget * parent );
  ~ArticleWebView();
  void setUp( Config::Class * cfg );

  bool isMidButtonPressed() const
  { return midButtonPressed; }
  void resetMidButtonPressed()
  {
    midButtonPressed = false;
  }
  void setSelectionBySingleClick( bool set )
  { selectionBySingleClick = set; }

  bool eventFilter(QObject *obj, QEvent *ev);

  signals:

  /// Signals that the user has just double-clicked. The signal is delivered
  /// after the event was processed by the view -- that's the difference from
  /// installing an event filter. This is used for translating the double-clicked
  /// word, which gets selected by the view in response to double-click.
  void doubleClicked( QPoint pos );

  void linkClicked( const QUrl & url );

protected:
  QWebEngineView *createWindow(QWebEnginePage::WebWindowType type);
  bool event( QEvent * event );
  void singleClickAction(QMouseEvent *event);
  void sendCustomMouseEvent(QEvent::Type type);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent( QMouseEvent * event );
  void doubleClickAction( QMouseEvent * event );
  void focusInEvent( QFocusEvent * event );
  void wheelEvent( QWheelEvent * event );


private:

  Config::Class * cfg;
  //QPointer<QOpenGLWidget> child_;

  bool midButtonPressed;
  bool selectionBySingleClick;

  //MouseDbClickEvent will also emit MousePressEvent which conflict the single click event.
  //this variable used to distinguish the single click and real double click.
  bool singleClickToDbClick;
  bool dbClicked;

public slots:

    //receive signal ,a link has been clicked.
    void linkClickedInHtml(QUrl const& url);
};

#endif
