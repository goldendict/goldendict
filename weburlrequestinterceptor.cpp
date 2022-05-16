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
    auto hostBase = getHostBase( info.requestUrl().host() );
    if( GlobalBroadcaster::instance()->existedInWhitelist( hostBase ) )
    {
      //whitelist url does not block
      return;
    }
    if(Utils::isCssFontImage(info.requestUrl())){
      //let throuth the resources file.
      return;
    }

    // configure should the gd block external links
//    if( GlobalBroadcaster::instance()->getPreference()->disallowContentFromOtherSites )
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
