/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "griparea.hh"
#include <QMouseEvent>

GripArea::GripArea( QWidget * parent ): QWidget( parent )
{
  setCursor( Qt::OpenHandCursor );
}

void GripArea::paintEvent( QPaintEvent * )
{
  if ( isEnabled() )
  {
    QStylePainter p( this );
  
    QStyleOptionDockWidgetV2 opt;
  
    opt.initFrom( this );
  
    p.drawControl( QStyle::CE_DockWidgetTitle, opt );
  }
}

void GripArea::mousePressEvent( QMouseEvent * ev )
{
  startPos = ev->globalPos();
  setCursor( Qt::ClosedHandCursor );
}

void GripArea::mouseMoveEvent( QMouseEvent * ev )
{
  QPoint newPos = ev->globalPos();

  QPoint delta = newPos - startPos;

  startPos = newPos;

  // Find a top-level window

  QWidget * w = this;

  while( w && !w->isWindow() && w->windowType() != Qt::SubWindow )
      w = w->parentWidget();

  w->move( w->pos() + delta );
}

void GripArea::mouseReleaseEvent( QMouseEvent * )
{
  setCursor( Qt::OpenHandCursor );
}
