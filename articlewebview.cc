/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"

#ifdef USE_QTWEBKIT

#include "articleinspector.hh"
#include "qt4x5.hh"

#include <QMouseEvent>
#include <QApplication>
#include <QWebFrame>

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif

ArticleWebView::ArticleWebView( QWidget *parent ):
  QWebView( parent ),
  inspector( NULL ),
  midButtonPressed( false ),
  selectionBySingleClick( false ),
  showInspectorDirectly( true )
{
}

ArticleWebView::~ArticleWebView()
{
  if ( inspector )
    inspector->deleteLater();
}

void ArticleWebView::setUp( Config::Class * cfg )
{
  this->cfg = cfg;
}

void ArticleWebView::saveConfigData() const
{
  if( inspector )
    cfg->inspectorGeometry = inspector->saveGeometry();
}

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

void ArticleWebView::mousePressEvent( QMouseEvent * event )
{
  if ( event->buttons() & Qt4x5::middleButton() )
    midButtonPressed = true;

  QWebView::mousePressEvent( event );

  if( selectionBySingleClick && ( event->buttons() & Qt::LeftButton ) && !isOnScrollBar( *event ) )
  {
    findText(""); // clear the selection first, if any
    QMouseEvent ev( QEvent::MouseButtonDblClick, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers() );
    QApplication::sendEvent( page(), &ev );
  }
}

void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  QWebView::mouseReleaseEvent( event );

  // QWebPage::linkClicked() signal is emitted during the above call to QWebView::mouseReleaseEvent(),
  // so midButtonPressed has been used already and can be unset now.
  if( midButtonPressed && !( event->buttons() & Qt4x5::middleButton() ) )
    midButtonPressed = false;
}

void ArticleWebView::mouseDoubleClickEvent( QMouseEvent * event )
{
  QWebView::mouseDoubleClickEvent( event );

  // emit the signal only if we are not double-clicking on scrollbars
  if( !isOnScrollBar( *event ) )
    emit doubleClicked( event->pos() );
}

// TODO (Qt WebEngine): port if this code is useful in the Qt WebEngine version.
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

  // TODO (Qt WebEngine): port if this code is useful in the Qt WebEngine version.
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

bool ArticleWebView::isOnScrollBar( QMouseEvent const & event ) const
{
  int const scrollBarWidth = page()->mainFrame()->scrollBarGeometry( Qt::Vertical ).width();
  int const scrollBarHeight = page()->mainFrame()->scrollBarGeometry( Qt::Horizontal ).height();

  return event.x() >= width() - scrollBarWidth || event.y() >= height() - scrollBarHeight;
}

#else // USE_QTWEBKIT

#include "gddebug.hh"

#include <QChildEvent>

void ArticleWebView::setEventFilter( QObject * filterObject )
{
  eventFilterObject = filterObject;
}

bool ArticleWebView::isWatched( QObject * object ) const
{
  return object->parent() == this && object->isWidgetType();
}

QCursor ArticleWebView::cursor() const
{
  return childWidget ? childWidget->cursor() : QCursor{};
}

void ArticleWebView::setCursor( QCursor const & cursor )
{
  if( childWidget )
    childWidget->setCursor( cursor );
}

void ArticleWebView::unsetCursor()
{
  if( childWidget )
    childWidget->unsetCursor();
}

void ArticleWebView::childEvent( QChildEvent * event )
{
  auto * const child = event->child();
  if( child->isWidgetType() )
  {
    // Empirical observation: in Qt 5.15 QWebEngineView has a single widget child, which is added
    // soon after construction and is never removed. Remove the event filter from a removed child
    // anyway in case this occurs under some circumstances or in a future Qt version.
    switch( event->type() )
    {
      case QEvent::ChildAdded:
        child->installEventFilter( eventFilterObject );

        if( childWidget )
          gdWarning( "A second widget child is added to an article web view. Replacing the first one." );
        childWidget = qobject_cast< QWidget * >( child );
        break;

      case QEvent::ChildRemoved:
        child->removeEventFilter( eventFilterObject );

        if( childWidget == child )
          childWidget = nullptr;
        break;

      default:
        break;
    }
  }

  return QWebEngineView::childEvent( event );
}

#endif // USE_QTWEBKIT
