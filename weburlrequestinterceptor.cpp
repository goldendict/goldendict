#include "weburlrequestinterceptor.h"
#include <QDebug>

WebUrlRequestInterceptor::WebUrlRequestInterceptor(QObject *p)
  :QWebEngineUrlRequestInterceptor(p)
{

}
void WebUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info) {
    if(QWebEngineUrlRequestInfo::NavigationTypeLink==info.navigationType ()&&info.resourceType ()==QWebEngineUrlRequestInfo::ResourceTypeMainFrame)
        emit linkClicked(info.requestUrl ());

}
