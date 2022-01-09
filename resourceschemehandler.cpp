#include "resourceschemehandler.h"

ResourceSchemeHandler::ResourceSchemeHandler(ArticleNetworkAccessManager& articleNetMgr):mManager(articleNetMgr){

}
void ResourceSchemeHandler::requestStarted(QWebEngineUrlRequestJob *requestJob)
{
    QUrl url = requestJob->requestUrl();

    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *reply = this->mManager.createRequest(QNetworkAccessManager::GetOperation, request, NULL);
    connect(reply, &QNetworkReply::finished, requestJob, [=]() {
      if (reply->error() == QNetworkReply::ContentNotFoundError) {
        requestJob->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
      }
      if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "resource handler failed:" << reply->error() << ":" << reply->request().url();
        requestJob->fail(QWebEngineUrlRequestJob::RequestFailed);
        return;
      }
      QMimeType mineType = db.mimeTypeForUrl(url);
      QString contentType = mineType.name();
      // Reply segment
      requestJob->reply(contentType.toLatin1(), reply);
    });
}
