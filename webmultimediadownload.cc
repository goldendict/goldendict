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
  reply = NULL;

  finish();
}

void WebMultimediaDownload::replyFinished( QNetworkReply * r )
{
  if ( !r || r != reply )
    return; // Not our reply

  if ( r->error() == QNetworkReply::NoError )
  {
    QByteArray all = r->readAll();

    Mutex::Lock _( dataMutex );

    data.resize( all.size() );

    memcpy( data.data(), all.data(), all.size() );

    hasAnyData = true;
  }
  else
    setErrorString( r->errorString() );

  r->deleteLater();
  reply = NULL;

  finish();
}

bool WebMultimediaDownload::isAudioUrl( QUrl const & url )
{
  // Note: we check for forvo sound links explicitly, as they don't have extensions

  return url.scheme() == "http" && (
      Filetype::isNameOfSound( url.path().toUtf8().data() ) || url.host() == "apifree.forvo.com" );
}

}
