#include <QCursor>
#include <QDesktopWidget>

#include "scanflag.hh"
#include "ui_scanflag.h"

static Qt::WindowFlags popupWindowFlags =

Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
#if QT_VERSION >= 0x050000
 | Qt::WindowDoesNotAcceptFocus
#endif
;

ScanFlag::ScanFlag(QWidget *parent) :
    QMainWindow(parent)
{
  ui.setupUi( this );

  setWindowFlags( popupWindowFlags );

  setAttribute(Qt::WA_X11DoNotAcceptFocus);

  hideTimer.setSingleShot( true );
  hideTimer.setInterval( 1500 );

  connect( &hideTimer, SIGNAL( timeout() ),
    this, SLOT( hideWindow() ) );

  connect( ui.pushButton, SIGNAL( clicked( bool ) ),
                this, SLOT( pushButtonClicked() ) );
}

ScanFlag::~ScanFlag()
{
}

void ScanFlag::pushButtonClicked()
{
  hideTimer.stop();
  hide();
  emit showScanPopup();
}

void ScanFlag::hideWindow()
{
  if ( isVisible() )
    hide();
}

void ScanFlag::showScanFlag()
{
  if ( isVisible() )
    hide();

  QPoint currentPos = QCursor::pos();

  QRect desktop = QApplication::desktop()->screenGeometry();

  QSize windowSize = geometry().size();

  int x, y;

  /// Try the to-the-right placement
  if ( currentPos.x() + 4 + windowSize.width() <= desktop.topRight().x() )
    x = currentPos.x() + 4;
  else
  /// Try the to-the-left placement
  if ( currentPos.x() - 4 - windowSize.width() >= desktop.x() )
    x = currentPos.x() - 4 - windowSize.width();
  else
  // Center it
    x = desktop.x() + ( desktop.width() - windowSize.width() ) / 2;

  /// Try the to-the-top placement
  if ( currentPos.y() - 15 - windowSize.height() >= desktop.y() )
    y = currentPos.y() - 15 - windowSize.height();
  else
  /// Try the to-the-bottom placement
  if ( currentPos.y() + 15 + windowSize.height() <= desktop.bottomLeft().y() )
    y = currentPos.y() + 15;
  else
  // Center it
    y = desktop.y() + ( desktop.height() - windowSize.height() ) / 2;

  move( x, y );

  show();
  hideTimer.start();
}
