#include "maintabwidget.hh"

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
