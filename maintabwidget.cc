/* This file is (c) 2012 Tvangeste <i.4m.l33t@yandex.ru>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "maintabwidget.hh"
#include <QDebug>

MainTabWidget::MainTabWidget( QWidget * parent) : QTabWidget( parent ) {
  hideSingleTab = false;
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

void MainTabWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
  (void) event;
  emit doubleClicked();
}
