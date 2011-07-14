/* This file is (c) 2011 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mainstatusbar.hh"

#include <Qt>
#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QApplication>

MainStatusBar::MainStatusBar( QWidget *parent ) : QWidget( parent )
{
  textWidget = new QLabel( QString(), this );
  textWidget->setObjectName( "text" );
  textWidget->setFont( QApplication::font( "QStatusBar" ) );
  textWidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

  picWidget = new QLabel( QString(), this );
  picWidget->setObjectName( "icon" );
  picWidget->setPixmap( QPixmap() );
  picWidget->setScaledContents( true );
  picWidget->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Ignored );

  timer = new QTimer( this );
  timer->setSingleShot( true );

  // layout
  QHBoxLayout * layout = new QHBoxLayout;
  layout->setSpacing( 0 );
  layout->setSizeConstraint( QLayout::SetFixedSize );
  layout->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( picWidget );
  layout->addWidget( textWidget );
  setLayout( layout );

  parentWidget()->installEventFilter( this );

  connect( timer, SIGNAL( timeout() ), SLOT( clearMessage() ) );
}

bool MainStatusBar::hasImage() const
{
  return !picWidget->pixmap()->isNull();
}

void MainStatusBar::clearMessage()
{
  textWidget->setText( QString() );
  picWidget->setPixmap( QPixmap() );
  timer->stop();
  refresh();
}

QString MainStatusBar::currentMessage() const
{
  return textWidget->text();
}

void MainStatusBar::showMessage(const QString & str, int timeout, const QPixmap & pixmap)
{
  textWidget->setText( str );
  picWidget->setPixmap( pixmap );

  // reload stylesheet
  setStyleSheet( styleSheet() );

  if ( timeout > 0 )
  {
    timer->start( timeout );
  }

  refresh();
}

void MainStatusBar::refresh()
{
  if ( !currentMessage().isEmpty() )
  {
    adjustSize();

    if ( !picWidget->pixmap()->isNull() )
    {
      picWidget->setFixedSize( textWidget->height(), textWidget->height() );
    }
    else
    {
      picWidget->setFixedSize( 0, 0 );
    }

    adjustSize();

    move( QPoint( 0, parentWidget()->height() - height() ) );

    show();
    raise();
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

bool MainStatusBar::eventFilter( QObject *, QEvent * e )
{
  switch ( e->type() ) {
    case QEvent::Resize:
      refresh();
      break;
    default:
      break;
  };

  return false;
}
