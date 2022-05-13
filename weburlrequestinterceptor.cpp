#include "weburlrequestinterceptor.h"
#include <QDebug>
#include "utils.hh"
#include "globalbroadcaster.h"

WebUrlRequestInterceptor::WebUrlRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}
void WebUrlRequestInterceptor::interceptRequest( QWebEngineUrlRequestInfo &info) {
  if( Utils::isExternalLink( info.requestUrl() ) )
  {
    if(!GlobalBroadcaster::instance()-> existedInWhitelist(info.requestUrl().host()))
    {
      info.block( true );
    }
  }

  if (QWebEngineUrlRequestInfo::NavigationTypeLink == info.navigationType() && info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame) {
    //workaround to fix devtool "Switch devtool to chinese" interface was blocked.
    if( info.requestUrl().scheme() == "devtools" )
    {
      return;
    }
    emit linkClicked( info.requestUrl() );
    info.block(true);
  }
}
