/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"

#include "qt4x5.hh"

#include <QMouseEvent>
#include <QApplication>

#ifdef USE_QTWEBKIT
#include "articleinspector.hh"
#include <QWebFrame>
#endif

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif

ArticleWebView::ArticleWebView( QWidget *parent ):
  WebView( parent ),
#ifdef USE_QTWEBKIT
  inspector( NULL ),
#endif
  midButtonPressed( false ),
  selectionBySingleClick( false ),
  showInspectorDirectly( true )
{
}

ArticleWebView::~ArticleWebView()
{
#ifdef USE_QTWEBKIT
  if ( inspector )
    inspector->deleteLater();
#endif
}

void ArticleWebView::setUp( Config::Class * cfg )
{
  this->cfg = cfg;
}

#ifdef USE_QTWEBKIT
void ArticleWebView::triggerPageAction( QWebPage::WebAction action, bool checked )
{
  if ( action == QWebPage::InspectElement )
  {
    // Get or create inspector instance for current view.
    if ( !inspector )
    {
      inspector = new ArticleInspector( cfg );
      inspector->setPage( page() );
      connect( this, SIGNAL( destroyed() ), inspector, SLOT( beforeClosed() ) );
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

  QWebView::triggerPageAction( action, checked );
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

  return QWebView::event( event );
}
#endif // USE_QTWEBKIT

void ArticleWebView::mousePressEvent( QMouseEvent * event )
{
  if ( event->buttons() & Qt4x5::middleButton() )
    midButtonPressed = true;

  WebView::mousePressEvent( event );

  if ( selectionBySingleClick && ( event->buttons() & Qt::LeftButton ) )
  {
    findText(""); // clear the selection first, if any
    QMouseEvent ev( QEvent::MouseButtonDblClick, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers() );
    QApplication::sendEvent( page(), &ev );
  }
}

void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  bool noMidButton = !( event->buttons() & Qt4x5::middleButton() );

  WebView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::mouseDoubleClickEvent( QMouseEvent * event )
{
  WebView::mouseDoubleClickEvent( event );

  // TODO (Qt WebEngine): port the scrollbar checks if they are useful in the Qt WebEngine version.
#ifdef USE_QTWEBKIT
  int scrollBarWidth = page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).width();
  int scrollBarHeight = page()->mainFrame()->scrollBarGeometry( Qt::Horizontal ).height();
  // emit the signal only if we are not double-clicking on scrollbars
  if ( ( event->x() < width() - scrollBarWidth ) &&
       ( event->y() < height() - scrollBarHeight ) )
#endif
  {
    emit doubleClicked( event->pos() );
  }

}

// TODO (Qt WebEngine): port if this code is useful in the Qt WebEngine version.
#ifdef USE_QTWEBKIT

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

#endif // USE_QTWEBKIT
