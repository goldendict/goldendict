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
    if(GlobalBroadcaster::instance()-> existedInWhitelist(info.requestUrl().host()))
    {
      //whitelist url does not block
      return;
    }
    if(Utils::isCssFontImage(info.requestUrl())){
      //let throuth the resources file.
      return;
    }
    info.block(true);
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
