/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEWEBVIEW_HH_INCLUDED__
#define __ARTICLEWEBVIEW_HH_INCLUDED__

#ifdef USE_QTWEBKIT

#include "config.hh"
#include <QWebView>

class ArticleInspector;

/// A thin wrapper around QWebView to accommodate to some ArticleView's needs.
/// Currently the only added features:
/// 1. Ability to know if the middle mouse button is pressed or not according
///    to the view's current state. This is used to open links in new tabs when
///    they are clicked with middle button. There's also an added possibility to
///    get double-click events after the fact with the doubleClicked() signal.
/// 2. Manage our own QWebInspector instance. In order to show inspector correctly,
///    use triggerPageAction( QWebPage::InspectElement ) instead.
class ArticleWebView: public QWebView
{
  Q_OBJECT

public:

  ArticleWebView( QWidget * parent );
  ~ArticleWebView();

  void setUp( Config::Class * cfg );

  void saveConfigData() const;

  bool isMidButtonPressed() const
  { return midButtonPressed; }
  void setSelectionBySingleClick( bool set )
  { selectionBySingleClick = set; }

  void triggerPageAction( QWebPage::WebAction action, bool checked = false );

signals:

  /// Signals that the user has just double-clicked. The signal is delivered
  /// after the event was processed by the view -- that's the difference from
  /// installing an event filter. This is used for translating the double-clicked
  /// word, which gets selected by the view in response to double-click.
  void doubleClicked( QPoint pos );

protected:

  bool event( QEvent * event );
  void mousePressEvent( QMouseEvent * event );
  void mouseReleaseEvent( QMouseEvent * event );
  void mouseDoubleClickEvent( QMouseEvent * event );
  void focusInEvent( QFocusEvent * event );
  void wheelEvent( QWheelEvent * event );

private:

  bool isOnScrollBar( QMouseEvent const & event ) const;

  Config::Class * cfg;
  ArticleInspector * inspector;

  bool midButtonPressed;
  bool selectionBySingleClick;
  bool showInspectorDirectly;
};

#else // USE_QTWEBKIT

#include <QWebEngineView>

/// A thin wrapper around QWebEngineView that works around QTBUG-43602 by installing
/// an event filter on the widget child of QWebEngineView, which swallows mouse events.
/// In the Qt WebEngine version, ArticleView implements most features ArticleWebView provides in the Qt WebKit version.
class ArticleWebView: public QWebEngineView
{
  Q_OBJECT
public:
  using QWebEngineView::QWebEngineView;

  /// This function supersedes installEventFilter(), which doesn't work as expected because of QTBUG-43602.
  /// Call this function once and immediately after constructing this view. Otherwise, it will have no effect.
  /// Removing or replacing the event filter is currently unneeded and thus unsupported.
  void setEventFilter( QObject * filterObject );

  /// @return whether the argument to a timely setEventFilter() call was installed as event filter on @p object.
  bool isWatched( QObject * object ) const;

  // Use the following member functions, not QWidget's equivalents they hide.
  QCursor cursor() const;
  void setCursor( QCursor const & );
  void unsetCursor();

protected:
  void childEvent( QChildEvent * event ) override;

private:
  QObject * eventFilterObject = nullptr;
  QWidget * childWidget = nullptr;
};

#endif // USE_QTWEBKIT

#endif // __ARTICLEWEBVIEW_HH_INCLUDED__
