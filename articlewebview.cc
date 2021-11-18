/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"
#include <QMouseEvent>
#include <QWebEngineView>
#include <QApplication>

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif

ArticleWebView::ArticleWebView( QWidget *parent ):
  QWebEngineView( parent ),
  midButtonPressed( false ),
  selectionBySingleClick( false ),
  showInspectorDirectly( true )
{
}

ArticleWebView::~ArticleWebView()
{
}

void ArticleWebView::setUp( Config::Class * cfg )
{
  this->cfg = cfg;
}

void ArticleWebView::triggerPageAction( QWebEnginePage::WebAction action, bool checked )
{
  QWebEngineView::triggerPageAction( action, checked );
}

bool ArticleWebView::event( QEvent * event )
{
  switch ( event->type() )
  {
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
    showInspectorDirectly = true;
    break;

  case QEvent::ContextMenu:
    showInspectorDirectly = false;
    break;

  default:
    break;
  }

  return QWebEngineView::event( event );
}

void ArticleWebView::mousePressEvent( QMouseEvent * event )
{
  if ( event->buttons() & Qt::MidButton )
    midButtonPressed = true;

  QWebEngineView::mousePressEvent( event );

  if ( selectionBySingleClick && ( event->buttons() & Qt::LeftButton ) )
  {
    findText(""); // clear the selection first, if any
    QMouseEvent ev( QEvent::MouseButtonDblClick, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers() );
    QApplication::sendEvent( page(), &ev );
  }
}

void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  bool noMidButton = !( event->buttons() & Qt::MidButton );

  QWebEngineView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QWebEngineView::mouseDoubleClickEvent( event );
  //todo
#if QT_VERSION >= 0x040600 && QT_VERSION <0x050500
  int scrollBarWidth = page()->contentsSize().width();
  int scrollBarHeight = page()->contentsSize().height();
#else
  int scrollBarWidth = 0;
  int scrollBarHeight = 0;
#endif

  // emit the signal only if we are not double-clicking on scrollbars
  if ( ( event->x() < width() - scrollBarWidth ) &&
       ( event->y() < height() - scrollBarHeight ) )
  {
    emit doubleClicked( event->pos() );
  }

}

void ArticleWebView::focusInEvent( QFocusEvent * event )
{
  QWebEngineView::focusInEvent( event );

  switch( event->reason() )
  {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
      page()->runJavaScript("top.focus();");
      break;

    default:
      break;
  }
}

void ArticleWebView::wheelEvent( QWheelEvent *ev )
{
#ifdef Q_OS_WIN32

  // Avoid wrong mouse wheel handling in QWebEngineView
  // if system preferences is set to "scroll by page"

  if( ev->modifiers() == Qt::NoModifier )
  {
    unsigned nLines;
    SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &nLines, 0 );
    if( nLines == WHEEL_PAGESCROLL )
    {
      QKeyEvent kev( QEvent::KeyPress, ev->delta() > 0 ? Qt::Key_PageUp : Qt::Key_PageDown,
                     Qt::NoModifier );
      QApplication::sendEvent( this, &kev );

      ev->accept();
      return;
    }
  }
#endif

  if ( ev->modifiers().testFlag( Qt::ControlModifier ) )
  {
     ev->ignore();
  }
  else
  {
     QWebEngineView::wheelEvent( ev );
  }

}
