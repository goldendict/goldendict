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

        //should restrict the child event type?
        child_ev->child()->installEventFilter(this);
    }

    return QWebEngineView::event(event);
}

bool ArticleWebView::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonDblClick) {
        //QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
        firstClicked=false;
    }
    if (ev->type() == QEvent::MouseButtonPress) {
        firstClicked=true;
    }
    if (ev->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *pe = static_cast<QMouseEvent *>(ev);
        mouseReleaseEvent(pe);
        if(firstClicked){
            QTimer::singleShot(QApplication::doubleClickInterval(),this,[=](){
                singleClickAction(pe);
            });
        }
        else{
            doubleClickAction(pe);
        }
    }
    if (ev->type() == QEvent::Wheel) {
        QWheelEvent *pe = static_cast<QWheelEvent *>(ev);
        wheelEvent(pe);
        //return true;
    }
    if (ev->type() == QEvent::FocusIn) {
        QFocusEvent *pe = static_cast<QFocusEvent *>(ev);
        focusInEvent(pe);
        //return true;
    }

    return QWebEngineView::eventFilter(obj, ev);
}

void ArticleWebView::singleClickAction( QMouseEvent * event )
{
  if(!firstClicked)
    return;
  if ( event->buttons() & Qt::MidButton )
    midButtonPressed = true;

  //QWebEngineView::mousePressEvent(event);

  if (selectionBySingleClick && (event->button() & Qt::LeftButton)) {
         findText(""); // clear the selection first, if any
         page()->runJavaScript(QString(""
"  var s = window.getSelection();  "
"  var range = s.getRangeAt(0);  "
"  var node = s.anchorNode;  "
"  while (range.toString().indexOf(' ') != 0) {  "
"    range.setStart(node, (range.startOffset - 1));  "
"  }  "
"  range.setStart(node, range.startOffset + 1);  "
"  do {  "
"    range.setEnd(node, range.endOffset + 1);  "
"  }  "
"  while (range.toString().indexOf(' ') == -1 && range.toString().trim() != '');  "
"  var str = range.toString().trim();  "
"  console.log(str);"));
//         QMouseEvent ev( QEvent::MouseButtonDblClick, event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers() );
//         QApplication::sendEvent(page(), &ev );
  }
}

void ArticleWebView::mouseReleaseEvent( QMouseEvent * event )
{
  bool noMidButton = !( event->buttons() & Qt::MidButton );

  //QWebEngineView::mouseReleaseEvent( event );

  if ( midButtonPressed & noMidButton )
    midButtonPressed = false;
}

void ArticleWebView::doubleClickAction( QMouseEvent * event )
{
    //QWebEngineView::mouseDoubleClickEvent( event );

    int scrollBarWidth = 0;
    int scrollBarHeight = 0;

    // emit the signal only if we are not double-clicking on scrollbars
    if ((event->x() < width() - scrollBarWidth) && (event->y() < height() - scrollBarHeight)) {
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
