/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"
#include <QMouseEvent>
#include <QWebFrame>
#include <QApplication>
#include "articleinspector.hh"

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif

ArticleWebView::ArticleWebView( QWidget *parent ):
  QWebView( parent ),
#if QT_VERSION >= 0x040600
  inspector( NULL ),
  showInspectorDirectly( true ),
#endif
  midButtonPressed( false ),
  selectionBySingleClick( false )
{
}

ArticleWebView::~ArticleWebView()
{
}

void ArticleWebView::setUp( Config::Class * cfg )
{
  this->cfg = cfg;
}

#if QT_VERSION >= 0x040600
void ArticleWebView::beforeClosed()
{
    cfg->inspectorGeometry = inspector->saveGeometry();
    inspector = 0;
}
#endif

void ArticleWebView::triggerPageAction( QWebPage::WebAction action, bool checked )
{
#if QT_VERSION >= 0x040600
  if ( action == QWebPage::InspectElement )
  {
    // Get or create inspector instance for current view.
    if ( !inspector )
    {
      inspector = new ArticleInspector();
      inspector->setPage( page() );
      connect( this, SIGNAL( destroyed() ), inspector, SLOT( close() ) );
      connect( inspector, SIGNAL( destroyed() ), this, SLOT( beforeClosed() ) );
      inspector->restoreGeometry( cfg->inspectorGeometry );
      inspector->setAttribute(Qt::WA_DeleteOnClose);
    }

    if ( showInspectorDirectly )
    {
      showInspectorDirectly = false;
      // Bring up the inspector window and set focus
      inspector->show();
      inspector->activateWindow();
      inspector->raise();
      return;
    }
  }
#endif

  QWebView::triggerPageAction( action, checked );
}

bool ArticleWebView::event( QEvent * event )
{
  switch ( event->type() )
  {
#if QT_VERSION >= 0x040600
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
    showInspectorDirectly = true;
    break;

  case QEvent::ContextMenu:
    showInspectorDirectly = false;
    break;
#endif
  default:
    break;
  }

  return QWebView::event( event );
}

void ArticleWebView::mousePressEvent( QMouseEvent * event )
{
  if ( event->buttons() & Qt::MidButton )
    midButtonPressed = true;

  QWebView::mousePressEvent( event );

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

  QWebView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QWebView::mouseDoubleClickEvent( event );
#if QT_VERSION >= 0x040600
  int scrollBarWidth = page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).width();
  int scrollBarHeight = page()->mainFrame()->scrollBarGeometry( Qt::Horizontal ).height();
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
#ifdef Q_OS_WIN32

  // Avoid wrong mouse wheel handling in QWebView
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
     QWebView::wheelEvent( ev );
  }

}
