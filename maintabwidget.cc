/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "maintabwidget.hh"
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

MainTabWidget::MainTabWidget( QWidget * parent) : QTabWidget( parent ) {
  hideSingleTab = false;
  installEventFilter( this );
  tabBar()->installEventFilter( this );
}

void MainTabWidget::setHideSingleTab(bool hide)
{
  hideSingleTab = hide;
  updateTabBarVisibility();
}

void MainTabWidget::tabInserted(int index)
{
  (void) index;
  updateTabBarVisibility();

  // Avoid bug in Qt 4.8.0
  setUsesScrollButtons( count() > 10 );
}

void MainTabWidget::tabRemoved(int index)
{
  (void) index;
  updateTabBarVisibility();

  // Avoid bug in Qt 4.8.0
  setUsesScrollButtons( count() > 10 );
}

void MainTabWidget::updateTabBarVisibility()
{
  tabBar()->setVisible( !hideSingleTab || tabBar()->count() > 1 );
}

/*
void MainTabWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
  (void) event;
  emit doubleClicked();
}
*/

bool MainTabWidget::eventFilter( QObject * obj, QEvent * ev )
{
  // mouseDoubleClickEvent don't called under Ubuntu
  if( ev->type() == QEvent::MouseButtonDblClick )
  {
    QMouseEvent * mev = static_cast< QMouseEvent *>( ev );
    if( tabBar()->tabAt( mev->pos() ) == -1 )
    {
      emit doubleClicked();
      return true;
    }
  }

  if( obj == tabBar() && ev->type() == QEvent::MouseButtonPress )
  {
     QMouseEvent * mev = static_cast< QMouseEvent *>( ev );
     if( mev->button() == Qt::MidButton )
     {
         emit tabCloseRequested( tabBar()->tabAt( mev->pos() ) );
         return true;
     }
  }

  return QTabWidget::eventFilter( obj, ev );
}
