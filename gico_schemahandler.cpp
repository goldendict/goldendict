#include "gico_schemahandler.h"

GicoSchemeHandler::GicoSchemeHandler(ArticleNetworkAccessManager& articleNetMgr):mManager(articleNetMgr){

}
void GicoSchemeHandler::requestStarted(QWebEngineUrlRequestJob *requestJob)
{
    QUrl url = requestJob->requestUrl();

    QNetworkRequest request;
    request.setUrl( url );

    QNetworkReply* reply=this->mManager.createRequest(QNetworkAccessManager::GetOperation,request,NULL);

    // Reply segment
    requestJob->reply("image/png", reply);
}
