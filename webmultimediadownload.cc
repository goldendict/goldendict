#include "webmultimediadownload.hh"
#include "filetype.hh"

namespace Dictionary {

WebMultimediaDownload::WebMultimediaDownload( QUrl const & url,
                                              QNetworkAccessManager & mgr )
{
  connect( &mgr, SIGNAL(finished(QNetworkReply*)),
           this, SLOT(replyFinished(QNetworkReply*)), Qt::QueuedConnection );

  reply = mgr.get( QNetworkRequest( url ) );
}

void WebMultimediaDownload::cancel()
{
  reply.reset();

  finish();
}

void WebMultimediaDownload::replyFinished( QNetworkReply * r )
{
  if ( r != reply.get() )
    return; // Not our reply

  if ( reply->error() == QNetworkReply::NoError )
  {
    QByteArray all = reply->readAll();

    Mutex::Lock _( dataMutex );

    data.resize( all.size() );

    memcpy( data.data(), all.data(), all.size() );

    hasAnyData = true;
  }
  else
    setErrorString( reply->errorString() );

  finish();

  reply.reset();
}

bool WebMultimediaDownload::isAudioUrl( QUrl const & url )
{
  // Note: we check for forvo sound links explicitly, as they don't have extensions

  return (url.scheme() == "http"  && (
              Filetype::isNameOfSound( url.path().toUtf8().data() ) || url.host() == "apifree.forvo.com" ))
          || (url.scheme() == "file" && Filetype::isNameOfSound(url.path().toUtf8().data() )
            ||  url.hasQueryItem("webtts")
              );
}

}
