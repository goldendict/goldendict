#ifndef GICO_SCHEMAHANDLER_H
#define GICO_SCHEMAHANDLER_H

#include"article_netmgr.hh"

class GicoSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    GicoSchemeHandler(ArticleNetworkAccessManager& articleNetMgr);
    void requestStarted(QWebEngineUrlRequestJob *requestJob);

protected:

private:
    ArticleNetworkAccessManager& mManager;
    QMimeDatabase db;
};

#endif // GICO_SCHEMAHANDLER_H
