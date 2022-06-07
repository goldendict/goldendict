/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SEARCHPANEWIDGET_HH_INCLUDED__
#define __SEARCHPANEWIDGET_HH_INCLUDED__

#include <QWidget>
#include <QSize>

/// A widget holding the contents of the 'Search pane' docklet.
class SearchPaneWidget: public QWidget
{
public:

  SearchPaneWidget( QWidget * parent = 0 ): QWidget( parent )
  {}

  virtual QSize sizeHint() const
  { return QSize( 204, 204 ); }

};

#endif

