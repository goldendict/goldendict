#include "articlewebpage.h"

ArticleWebPage::ArticleWebPage(QObject *parent)
  : QWebEnginePage{parent}
{
}
bool ArticleWebPage::acceptNavigationRequest( const QUrl & url, NavigationType type, bool isMainFrame )
{
  if( type == QWebEnginePage::NavigationTypeLinkClicked )
  {
    emit linkClicked( url );
    return true;
  }
  return QWebEnginePage::acceptNavigationRequest( url, type, isMainFrame );
}
