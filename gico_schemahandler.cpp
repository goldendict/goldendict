#include "gico_schemahandler.h"

GicoSchemeHandler::GicoSchemeHandler(ArticleNetworkAccessManager& articleNetMgr):mManager(articleNetMgr){

}
void GicoSchemeHandler::requestStarted(QWebEngineUrlRequestJob *requestJob)
{
    QUrl url = requestJob->requestUrl();

    QNetworkRequest request;
    request.setUrl( url );

    QNetworkReply* reply=this->mManager.createRequest(QNetworkAccessManager::GetOperation,request,NULL);

    QString contentType="image/png";
    if(url.scheme()=="gadu"){
        contentType="audio/wav";
    }
    // Reply segment
    requestJob->reply(contentType.toLatin1(), reply);
}
