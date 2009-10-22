/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEWEBVIEW_HH_INCLUDED__
#define __ARTICLEWEBVIEW_HH_INCLUDED__

#include <QWebView>

/// A thin wrapper around QWebView to accomodate to some ArticleView's needs.
/// Currently the only added feature is an ability to know if the middle mouse
/// button is pressed or not according to the view's current state. This is used
/// to open links in new tabs when they are clicked with middle button.
class ArticleWebView: public QWebView
{
public:

  ArticleWebView( QWidget * parent ): QWebView( parent ),
                                      midButtonPressed( false )
  {}

  bool isMidButtonPressed() const
  { return midButtonPressed; }

protected:

  void mousePressEvent( QMouseEvent * event );
  void mouseReleaseEvent( QMouseEvent * event );

private:

  bool midButtonPressed;
};

#endif
