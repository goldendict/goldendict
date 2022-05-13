#include "IframeSchemeHandler.h"

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

  QNetworkReply * reply = mgr.get( request );
  auto finishAction     = [ = ]() -> void
  {
    // Handle reply data

    QByteArray replyData = reply->readAll();
    QString articleString;

    QTextCodec * codec = QTextCodec::codecForHtml( replyData );
    if( codec )
      articleString = codec->toUnicode( replyData );
    else
      articleString = QString::fromUtf8( replyData );

    // Change links from relative to absolute

    QString root = reply->url().scheme() + "://" + reply->url().host();
    QString base = root + reply->url().path();
    while( !base.isEmpty() && !base.endsWith( "/" ) )
      base.chop( 1 );

    QRegularExpression tags( "<\\s*(a|link|img|script)\\s+[^>]*(src|href)\\s*=\\s*['\"][^>]+>",
                             QRegularExpression::CaseInsensitiveOption );
    QRegularExpression links( "\\b(src|href)\\s*=\\s*(['\"])([^'\"]+['\"])",
                              QRegularExpression::CaseInsensitiveOption );
    int pos = 0;
    QString articleNewString;
    QRegularExpressionMatchIterator it = tags.globalMatch( articleString );
    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();
      articleNewString += articleString.mid( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QString tag = match.captured();

      QRegularExpressionMatch match_links = links.match( tag );
      if( !match_links.hasMatch() )
      {
        articleNewString += tag;
        continue;
      }

      QString url = match_links.captured( 3 );

      if( url.indexOf( ":/" ) >= 0 || url.indexOf( "data:" ) >= 0 || url.indexOf( "mailto:" ) >= 0 ||
          url.startsWith( "#" ) || url.startsWith( "javascript:" ) )
      {
        // External link, anchor or base64-encoded data
        articleNewString += tag;
        continue;
      }

      QString newUrl = match_links.captured( 1 ) + "=" + match_links.captured( 2 );
      if( url.startsWith( "//" ) )
        newUrl += reply->url().scheme() + ":";
      else if( url.startsWith( "/" ) )
        newUrl += root;
      else
        newUrl += base;
      newUrl += match_links.captured( 3 );

      tag.replace( match_links.capturedStart(), match_links.capturedLength(), newUrl );
      articleNewString += tag;
    }
    if( pos )
    {
      articleNewString += articleString.mid( pos );
      articleString = articleNewString;
      articleNewString.clear();
    }

    sptr< Dictionary::DataRequestInstant > response = new Dictionary::DataRequestInstant( true );

    auto content = articleString.toStdString();
    response->getData().resize( content.size() );
    memcpy( &( response->getData().front() ), content.data(), content.size() );

    auto contentType="text/html";
    auto newReply = new ArticleResourceReply( this, request, response, contentType );

    requestJob->reply( contentType, newReply );
    connect( requestJob, &QObject::destroyed, newReply, &QObject::deleteLater );
  };
  connect( reply, &QNetworkReply::finished, requestJob, finishAction );

  connect( requestJob, &QObject::destroyed, reply, &QObject::deleteLater );
}
