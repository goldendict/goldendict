/* This file is (c) 2011 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainstatusbar.hh"

#include <Qt>
#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>

MainStatusBar::MainStatusBar(QWidget *parent) : QFrame(parent)
{
  // style
  setWindowFlags( Qt::Tool | Qt::X11BypassWindowManagerHint | Qt::FramelessWindowHint );
  setFrameStyle( QFrame::Box | QFrame::Plain );
  setLineWidth(0);

  // components
  label = new QLabel(QString(), this);
  label->setTextFormat(Qt::PlainText);
  timer = new QTimer(this);

  // layout
  QVBoxLayout * layout = new QVBoxLayout;
  layout->addWidget(label);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  layout->setMargin(4);
  setLayout(layout);

  parentWidget()->installEventFilter( this );

  connect( timer, SIGNAL( timeout() ), SLOT( clearMessage() ) );
}

void MainStatusBar::clearMessage()
{
  message = QString();
  label->setText(message);
  timer->stop();
  refresh();
}

QString MainStatusBar::currentMessage() const
{
  return message;
}

void MainStatusBar::showMessage(const QString & str, int timeout)
{
    message = str;

    if ( timeout > 0 )
    {
      timer->start( timeout );
    }

    refresh();
}

void MainStatusBar::refresh()
{

  if ( !message.isEmpty() )
  {
    QRect pGeom = parentWidget()->geometry();

    int maxLabelLength = pGeom.width() - 2 * layout()->margin();
    label->setText( label->fontMetrics().elidedText( message, Qt::ElideRight, maxLabelLength ) );

    adjustSize();

    move(pGeom.left(), pGeom.bottom() - size().height() + 1 );
    show();
  }
  else
  {
    hide();
  }

}

void MainStatusBar::mousePressEvent ( QMouseEvent * )
{
  clearMessage();
}

bool MainStatusBar::eventFilter(QObject *, QEvent * event)
{
  switch ( event->type() ) {
    case QEvent::Move:
    case QEvent::Resize:
    case QEvent::FocusOut:
    case QEvent::WindowDeactivate:
    case QEvent::Hide:
    case QEvent::WindowActivate:
      refresh();
      break;
    default:
      break;
  };

  return false;
}
