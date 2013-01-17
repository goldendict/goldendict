/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __DICTSPANEWIDGET_HH_INCLUDED__
#define __DICTSPANEWIDGET_HH_INCLUDED__

#include <QWidget>
#include <QSize>

/// A widget holding the contents of the 'Dictionaries pane' docklet.
class DictsPaneWidget: public QWidget
{
  Q_OBJECT
public:

  DictsPaneWidget( QWidget * parent = 0 ): QWidget( parent )
  {}

  virtual QSize sizeHint() const
  { return QSize( 204, 204 ); }

};

#endif
