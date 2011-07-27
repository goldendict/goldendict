/* This file is (c) 2011 Tvangeste <i.4m.l33t@yandex.ru>
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
  updateTabBarVisibility();
}

void MainTabWidget::tabRemoved(int index)
{
  updateTabBarVisibility();
}

void MainTabWidget::updateTabBarVisibility()
{
  tabBar()->setVisible( !hideSingleTab || tabBar()->count() > 1 );
}

void MainTabWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
  emit doubleClicked();
}
