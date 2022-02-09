#ifndef RESOURCESCHEMEHANDLER_H
#define RESOURCESCHEMEHANDLER_H

#include "article_netmgr.hh"

class ResourceSchemeHandler : public QWebEngineUrlSchemeHandler
{
  Q_OBJECT
public:
  ResourceSchemeHandler(ArticleNetworkAccessManager &articleNetMgr);
  void requestStarted(QWebEngineUrlRequestJob *requestJob);

protected:
private:
  ArticleNetworkAccessManager &mManager;
  QMimeDatabase db;
};

#endif // RESOURCESCHEMEHANDLER_H
