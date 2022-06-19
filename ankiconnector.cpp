#include "ankiconnector.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "utils.hh"
AnkiConnector::AnkiConnector( QObject * parent, Config::Class const & _cfg ) : QObject{ parent }, cfg( _cfg )
{
  mgr = new QNetworkAccessManager( this );
  connect( mgr, &QNetworkAccessManager::finished, this, &AnkiConnector::finishedSlot );
}

void AnkiConnector::sendToAnki( QString const & word, QString const & text )
{
  //for simplicity. maybe use QJsonDocument in future?
  QString postTemplate = QString( "{"
                                  "\"action\": \"addNote\","
                                  "\"version\": 6,"
                                  "\"params\": {"
                                  "   \"note\": {"
                                  "  \"deckName\": \"%1\","
                                  "  \"modelName\": \"%2\","
                                  "  \"fields\":%3,"
                                  "  \"options\": {"
                                  "    \"allowDuplicate\": true"
                                  "  },"
                                  "  \"tags\": []"
                                  "}"
                                  "}"
                                  "}"
                                  "" );

  QJsonObject fields;
  fields.insert( "Front", word );
  fields.insert( "Back", text );

  QString postData = postTemplate.arg( cfg.preferences.ankiConnectServer.deck,
                                       cfg.preferences.ankiConnectServer.model,
                                       Utils::json2String( fields ) );

//  qDebug().noquote() << postData;
  QUrl url;
  url.setScheme( "http" );
  url.setHost( cfg.preferences.ankiConnectServer.host );
  url.setPort( cfg.preferences.ankiConnectServer.port );
  QNetworkRequest request( url );
  request.setTransferTimeout( 3000 );
  //  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "applicaion/json" );
  auto reply = mgr->post( request, postData.toUtf8() );
  connect( reply,
           &QNetworkReply::errorOccurred,
           this,
           [ this ]( QNetworkReply::NetworkError e )
           {
             qWarning() << e;
             emit this->errorText( tr( "anki: post to anki failed" ) );
           } );
}

void AnkiConnector::finishedSlot( QNetworkReply * reply )
{
  if( reply->error() == QNetworkReply::NoError )
  {
    QByteArray bytes   = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson( bytes );
    auto obj           = json.object();
    if( obj.size() != 2 || !obj.contains( "error" ) || !obj.contains( "result" ) ||
        obj[ "result" ].toString().isEmpty() )
    {
      emit errorText( QObject::tr( "anki: post to anki failed" ) );
    }
    QString result = obj[ "result" ].toString();

    qDebug() << "anki result:" << result;

    emit errorText( tr( "anki: post to anki success" ) );
  }
  else
  {
    qDebug() << "anki connect error" << reply->errorString();
    emit errorText( "anki:" + reply->errorString() );
  }

  reply->deleteLater();
}
