#include "articlewebpage.h"
#include "utils.hh"

ArticleWebPage::ArticleWebPage(QObject *parent)
  : QWebEnginePage{parent}
{
}
bool ArticleWebPage::acceptNavigationRequest( const QUrl & resUrl, NavigationType type, bool isMainFrame )
{
  QUrl url = resUrl;
  if( url.scheme() == "bword" || url.scheme() == "entry" )
  {
    url.setScheme( "gdlookup" );
    url.setHost( "localhost" );
    url.setPath( "" );
    auto [ valid, word ] = Utils::Url::getQueryWord( resUrl );
    Utils::Url::addQueryItem( url, "word", word );
    Utils::Url::addQueryItem( url, "group", lastReq.group );
    Utils::Url::addQueryItem( url, "muted", lastReq.mutedDicts );
    setUrl( url );
    return false;
  }

  //save current gdlookup's values.
  if( url.scheme() == "gdlookup" )
  {
    lastReq.group      = Utils::Url::queryItemValue( url, "group" );
    lastReq.mutedDicts = Utils::Url::queryItemValue( url, "muted" );
  }

  if( type == QWebEnginePage::NavigationTypeLinkClicked )
  {
    emit linkClicked( url );
    return false;
  }

  return QWebEnginePage::acceptNavigationRequest( url, type, isMainFrame );
}
