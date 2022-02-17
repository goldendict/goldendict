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

bool ArticleWebView::event( QEvent * event )
{
    if (event->type() == QEvent::ChildAdded) {
        QChildEvent *child_ev = static_cast<QChildEvent *>(event);

//      // there is also QObject child that should be ignored here;
//      // use only QOpenGLWidget child
//      QOpenGLWidget *w = static_cast<QOpenGLWidget*>(child_ev->child());

        child_ev->child()->installEventFilter(this);
    }

    return QWebEngineView::event(event);
}

void ArticleWebView::linkClickedInHtml(QUrl const& ){
  //disable single click to simulate dbclick action on the new loaded pages.
  singleClickToDbClick=false;
}

bool ArticleWebView::eventFilter(QObject *obj, QEvent *ev) {
    if (ev->type() == QEvent::MouseButtonDblClick) {
      QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
      if (Qt::MouseEventSynthesizedByApplication != pe->source()) {
        singleClickToDbClick = false;
        dbClicked = true;
      }
    }
    if (ev->type()==QEvent::MouseMove) {
      singleClickToDbClick=false;
    }
    if (ev->type() == QEvent::MouseButtonPress) {
        QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
        if(pe->button() == Qt::LeftButton)
        {
          singleClickToDbClick = true;
          dbClicked = false;
          QTimer::singleShot(QApplication::doubleClickInterval(),this,[=](){
            singleClickAction(pe);
          });
        }
        mousePressEvent(pe);
    }
    if (ev->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
        mouseReleaseEvent(pe);
        if (dbClicked) {
          //emit the signal after button release.emit earlier(in MouseButtonDblClick event) can not get selected text;
            doubleClickAction(pe);
        }
    }
    if (ev->type() == QEvent::Wheel) {
        QWheelEvent *pe = static_cast<QWheelEvent *>(ev);
        wheelEvent(pe);
    }
    if (ev->type() == QEvent::FocusIn) {
        QFocusEvent *pe = static_cast<QFocusEvent *>(ev);
        focusInEvent(pe);
    }

    return QWebEngineView::eventFilter(obj, ev);
}

void ArticleWebView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::MidButton)
        midButtonPressed = true;
}

void ArticleWebView::singleClickAction(QMouseEvent *event )
{
  if(!singleClickToDbClick)
    return;

  if (selectionBySingleClick) {
      findText(""); // clear the selection first, if any
      //send dbl click event twice? send one time seems not work .weird really.  need further investigate.
      sendCustomMouseEvent( QEvent::MouseButtonDblClick);
      sendCustomMouseEvent( QEvent::MouseButtonDblClick);
  }
}

void ArticleWebView::sendCustomMouseEvent( QEvent::Type type) {
  QPoint pt = mapFromGlobal(QCursor::pos());
  QMouseEvent ev(type, pt, pt, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                 Qt::MouseEventSynthesizedByApplication);

  auto childrens = this->children();
  for (auto child:childrens) {
    QApplication::sendEvent(child, &ev);
  }
}

void ArticleWebView::mouseReleaseEvent(QMouseEvent *event) {
  bool noMidButton = !( event->buttons() & Qt::MidButton );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::doubleClickAction(QMouseEvent *event) {
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
      auto childrens = this->children();
      for (auto child : childrens) {
        QApplication::sendEvent(child, &kev);
      }

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
