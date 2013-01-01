/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEWEBVIEW_HH_INCLUDED__
#define __ARTICLEWEBVIEW_HH_INCLUDED__

#include <QWebView>

/// A thin wrapper around QWebView to accomodate to some ArticleView's needs.
/// Currently the only added feature is an ability to know if the middle mouse
/// button is pressed or not according to the view's current state. This is used
/// to open links in new tabs when they are clicked with middle button.
/// There's also an added posibility to get double-click events after the fact
/// with the doubleClicked() signal.
class ArticleWebView: public QWebView
{
  Q_OBJECT

public:

  ArticleWebView( QWidget * parent ): QWebView( parent ),
                                      midButtonPressed( false ),
                                      selectionBySingleClick( false )
  {}

  bool isMidButtonPressed() const
  { return midButtonPressed; }
  void setSelectionBySingleClick( bool set )
  { selectionBySingleClick = set; }

signals:

  /// Signals that the user has just double-clicked. The signal is delivered
  /// after the event was processed by the view -- that's the difference from
  /// installing an event filter. This is used for translating the double-clicked
  /// word, which gets selected by the view in response to double-click.
  void doubleClicked();

protected:

  void mousePressEvent( QMouseEvent * event );
  void mouseReleaseEvent( QMouseEvent * event );
  void mouseDoubleClickEvent( QMouseEvent * event );
  void focusInEvent( QFocusEvent * event );
  void wheelEvent( QWheelEvent * event );

private:

  bool midButtonPressed;
  bool selectionBySingleClick;
};

#endif
