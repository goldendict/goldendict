#include "weburlrequestinterceptor.h"
#include <QDebug>
#include "utils.hh"
#include "globalbroadcaster.h"

WebUrlRequestInterceptor::WebUrlRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}
void WebUrlRequestInterceptor::interceptRequest( QWebEngineUrlRequestInfo &info) {
  if(  GlobalBroadcaster::instance()->getPreference()->disallowContentFromOtherSites && Utils::isExternalLink( info.requestUrl() ) )
  {
    //file:// link ,pass
    if(info.requestUrl().scheme()=="file"){
      return;
    }
    auto hostBase = getHostBase( info.requestUrl().host() );
    if( GlobalBroadcaster::instance()->existedInWhitelist( hostBase ) )
    {
      //whitelist url does not block
      return;
    }
    if(Utils::isHtmlResources(info.requestUrl())){
      //let throuth the resources file.
      return;
    }

    // block external links
    {
      info.block( true );
      return;
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

  //window.location=audio link
  if( Utils::Url::isAudioUrl(info.requestUrl()) && info.navigationType()==QWebEngineUrlRequestInfo::NavigationTypeRedirect )
  {
    qDebug() << "blocked audio url from page redirect" << info.requestUrl().url();
    info.block( true );
  }
}
