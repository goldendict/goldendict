/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebview.hh"
#include <QMouseEvent>
#include <QWebEngineView>
#include <QApplication>
#include <QTimer>

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

bool ArticleWebView::event(QEvent *event)
{
  if (event->type() == QEvent::ChildAdded)
  {
    QChildEvent *child_ev = static_cast<QChildEvent *>(event);

    // should restrict the child event type?
    child_ev->child()->installEventFilter(this);
  }

  return QWebEngineView::event(event);
}

bool ArticleWebView::eventFilter(QObject *obj, QEvent *ev)
{
  if (ev->type() == QEvent::MouseButtonDblClick)
  {
    // QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
    firstClicked = false;
  }
  if (ev->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
    if (pe->buttons() & Qt::LeftButton)
    {
      firstClicked = true;
    }
    mousePressEvent(pe);
  }
  if (ev->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
    mouseReleaseEvent(pe);
    if (firstClicked)
    {
      QTimer::singleShot(QApplication::doubleClickInterval(), this, [=]()
                         { singleClickAction(obj, pe); });
    }
    else
    {
      doubleClickAction(pe);
    }
  }
  if (ev->type() == QEvent::Wheel)
  {
    QWheelEvent *pe = static_cast<QWheelEvent *>(ev);
    wheelEvent(pe);
    // return true;
  }
  if (ev->type() == QEvent::FocusIn)
  {
    QFocusEvent *pe = static_cast<QFocusEvent *>(ev);
    focusInEvent(pe);
    // return true;
  }

  return QWebEngineView::eventFilter(obj, ev);
}

void ArticleWebView::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::MidButton)
    midButtonPressed = true;
}

void ArticleWebView::singleClickAction(QObject *obj, QMouseEvent *event)
{
  if (!firstClicked)
    return;

  if (selectionBySingleClick)
  {
    findText(""); // clear the selection first, if any
    sendCustomMouseEvent(obj, QEvent::MouseButtonDblClick, event);
    sendCustomMouseEvent(obj, QEvent::MouseButtonRelease, event);
  }
}

void ArticleWebView::sendCustomMouseEvent(QObject* obj,QEvent::Type type,QMouseEvent * event){
  QMouseEvent ev( QEvent::MouseButtonDblClick,event->localPos (),event->windowPos (),event->screenPos (), Qt::LeftButton, Qt::LeftButton, event->modifiers(), Qt::MouseEventSynthesizedByApplication );
  QApplication::sendEvent(obj, &ev );
}


void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  bool noMidButton = !( event->buttons() & Qt::MidButton );

  //QWebEngineView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::doubleClickAction(QMouseEvent *event) {
  // QWebEngineView::mouseDoubleClickEvent( event );

  // emit the signal only if we are not double-clicking on scrollbars
  if (Qt::MouseEventSynthesizedByApplication != event->source()) {
    emit doubleClicked(event->pos());
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
