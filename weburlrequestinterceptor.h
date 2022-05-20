#ifndef WEBURLREQUESTINTERCEPTOR_H
#define WEBURLREQUESTINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>

class WebUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    WebUrlRequestInterceptor(QObject *p = Q_NULLPTR);
    void interceptRequest(QWebEngineUrlRequestInfo &info);
  signals:
    void linkClicked( const QUrl & url );
};

#endif // WEBURLREQUESTINTERCEPTOR_H
