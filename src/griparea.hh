/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __GRIPAREA_HH_INCLUDED__
#define __GRIPAREA_HH_INCLUDED__

#include <QWidget>
#include <QStylePainter>
#include <QStyleOptionDockWidget>

/// A grip area to move a window, looking like a dock widget's title area.
class GripArea: public QWidget
{
  Q_OBJECT

public:

  GripArea( QWidget * parent );

protected:

  virtual void paintEvent( QPaintEvent * );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );

private:

  QPoint startPos;
};

#endif
