/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEWEBVIEW_HH_INCLUDED__
#define __ARTICLEWEBVIEW_HH_INCLUDED__

#include "config.hh"
#include "webkit_or_webengine.hh"

#ifdef USE_QTWEBKIT
class ArticleInspector;
#endif

// TODO (Qt WebEngine): this class is pretty useless in the Qt WebEngine version right now
// because of QTBUG-43602 - it receives no mouse events. Will have to set an event filter
// on QWidget children of QWebEngineView and handle the mouse events.

/// A thin wrapper around WebView to accommodate to some ArticleView's needs.
/// Currently the only added features:
/// 1. Ability to know if the middle mouse button is pressed or not according
///    to the view's current state. This is used to open links in new tabs when
///    they are clicked with middle button. There's also an added possibility to
///    get double-click events after the fact with the doubleClicked() signal.
/// 2. Manage our own QWebInspector instance. In order to show inspector correctly,
///    use triggerPageAction( QWebPage::InspectElement ) instead.
class ArticleWebView: public WebView
{
  Q_OBJECT

public:

  ArticleWebView( QWidget * parent );
  ~ArticleWebView();

  void setUp( Config::Class * cfg );

  bool isMidButtonPressed() const
  { return midButtonPressed; }
  void setSelectionBySingleClick( bool set )
  { selectionBySingleClick = set; }

#ifdef USE_QTWEBKIT
  void triggerPageAction( QWebPage::WebAction action, bool checked = false );
#endif

signals:

  /// Signals that the user has just double-clicked. The signal is delivered
  /// after the event was processed by the view -- that's the difference from
  /// installing an event filter. This is used for translating the double-clicked
  /// word, which gets selected by the view in response to double-click.
  void doubleClicked( QPoint pos );

protected:

#ifdef USE_QTWEBKIT
  bool event( QEvent * event );
#endif
  void mousePressEvent( QMouseEvent * event );
  void mouseReleaseEvent( QMouseEvent * event );
  void mouseDoubleClickEvent( QMouseEvent * event );
#ifdef USE_QTWEBKIT
  void focusInEvent( QFocusEvent * event );
  void wheelEvent( QWheelEvent * event );
#endif

private:

  Config::Class * cfg;
#ifdef USE_QTWEBKIT
  ArticleInspector * inspector;
#endif

  bool midButtonPressed;
  bool selectionBySingleClick;
  bool showInspectorDirectly;
};

#endif
