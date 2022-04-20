#include "weburlrequestinterceptor.h"
#include <QDebug>
#include "utils.hh"

WebUrlRequestInterceptor::WebUrlRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}
void WebUrlRequestInterceptor::interceptRequest( QWebEngineUrlRequestInfo &info) {
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
