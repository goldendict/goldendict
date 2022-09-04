#include "iframeschemehandler.h"

#include <QTextCodec>

IframeSchemeHandler::IframeSchemeHandler(QObject * parent):QWebEngineUrlSchemeHandler(parent){

}
void IframeSchemeHandler::requestStarted(QWebEngineUrlRequestJob *requestJob)
{
  QUrl url = requestJob->requestUrl();

  // website dictionary iframe url
  url = QUrl( Utils::Url::queryItemValue( url, "url" ) );
  QNetworkRequest request;
  request.setUrl( url );
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);

  QNetworkReply * reply = mgr.get( request );

  auto finishAction     = [ = ]() -> void
  {
    QByteArray contentType = "text/html;charset=UTF-8";

    QBuffer * buffer = new QBuffer( requestJob );
    // Handle reply data
    if( reply->error() != QNetworkReply::NoError )
    {
      QString emptyHtml = QString( "<html><body>%1</body></html>" ).arg( reply->errorString() );
      buffer->setData( emptyHtml.toUtf8() );
      requestJob->reply( contentType, buffer );
      return;
    }
    QByteArray replyData = reply->readAll();
    QString articleString;

    QTextCodec * codec = QTextCodec::codecForHtml( replyData, QTextCodec::codecForName( "UTF-8" ) );
    articleString      = codec->toUnicode( replyData );

    // Change links from relative to absolute

    QString root = reply->url().scheme() + "://" + reply->url().host();
    QString base = root + reply->url().path();

    QRegularExpression baseTag( "<base\\s+.*?>",
                             QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption );
    
    QString baseTagHtml = "<base href=\"" + base + "\">";

    QString depressionFocus ="<script type=\"application/javascript\"> HTMLElement.prototype.focus=function(){console.log(\"focus() has been disabled.\");}</script>"
                      "<script type=\"text/javascript\" src=\"qrc:///scripts/iframeResizer.contentWindow.min.js\"></script>"
                      "<script type=\"text/javascript\" src=\"qrc:///scripts/iframe-defer.js\"></script>";
    
    // remove existed base tag
    articleString.remove( baseTag ) ;

    QRegularExpression headTag( "<head\\b.*?>",
                                QRegularExpression::CaseInsensitiveOption
                                  | QRegularExpression::DotMatchesEverythingOption );
    auto match = headTag.match( articleString, 0 );
    if( match.hasMatch() )
    {
      articleString.insert( match.capturedEnd(), baseTagHtml );
      articleString.insert( match.capturedEnd(), depressionFocus );
    }
    else
    {
      // the html contain no head element
      // just insert at the beginning of the html ,and leave it at the mercy of browser(chrome webengine)
      articleString.insert( 0, baseTagHtml );
      articleString.insert( 0, depressionFocus );
    }

    buffer->setData(codec->fromUnicode(articleString));

    requestJob->reply(contentType , buffer );
  };
  connect( reply, &QNetworkReply::finished, requestJob, finishAction );

  connect( requestJob, &QObject::destroyed, reply, &QObject::deleteLater );
}
