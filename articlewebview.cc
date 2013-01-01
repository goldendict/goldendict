/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"
#include <QMouseEvent>
#include <QWebFrame>
#include <QApplication>

void ArticleWebView::mousePressEvent( QMouseEvent * event )
{
  if ( event->buttons() & Qt::MidButton )
    midButtonPressed = true;

  QWebView::mousePressEvent( event );

  if ( selectionBySingleClick && ( event->buttons() & Qt::LeftButton ) )
  {
    QMouseEvent ev( QEvent::MouseButtonDblClick, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers() );
    QApplication::sendEvent( page(), &ev );
  }
}

void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  bool noMidButton = !( event->buttons() & Qt::MidButton );

  QWebView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QWebView::mouseDoubleClickEvent( event );

  int scrollBarWidth = page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).width();
  int scrollBarHeight = page()->mainFrame()->scrollBarGeometry( Qt::Horizontal ).height();

  // emit the signal only if we are not double-clicking on scrollbars
  if ( ( event->x() < width() - scrollBarWidth ) &&
       ( event->y() < height() - scrollBarHeight ) )
  {
    emit doubleClicked();
  }

}

void ArticleWebView::focusInEvent( QFocusEvent * event )
{
  QWebView::focusInEvent( event );

  switch( event->reason() )
  {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
      page()->mainFrame()->evaluateJavaScript("top.focus();");
      break;

    default:
      break;
  }
}

void ArticleWebView::wheelEvent( QWheelEvent *ev )
{
  if ( ev->modifiers().testFlag( Qt::ControlModifier ) )
  {
     ev->ignore();
  }
  else
  {
     QWebView::wheelEvent( ev );
  }

}
