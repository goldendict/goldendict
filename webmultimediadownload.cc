#include "webmultimediadownload.hh"
#include "filetype.hh"

namespace Dictionary {

WebMultimediaDownload::WebMultimediaDownload( QUrl const & url,
                                              QNetworkAccessManager & mgr ):isRedirect(false)
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
   QUrl redirectUrl = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();

   if(!redirectUrl.isEmpty())
   {
       //CHECK THAT URL IS VALID
       if(redirectUrl.scheme().isEmpty())
       {
           redirectUrl.setScheme(r->url().scheme());
           if(redirectUrl.host().isEmpty())
           {
            redirectUrl.setHost(r->url().host());
           }
       }

       //reply.reset();
      r->manager()->get(QNetworkRequest (redirectUrl));
      if (r != reply.get()  )
        r->deleteLater();
      // todel->deleteLater();
      isRedirect =true;
       return;
   }
  if (!isRedirect && r != reply.get()  )
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

  finish();
  if(isRedirect)
  {
    isRedirect =false;
    r->deleteLater();
  }
  else
  reply.reset();

}

bool WebMultimediaDownload::isAudioUrl( QUrl const & url )
{
  // Note: we check for forvo sound links explicitly, as they don't have extensions

  return (url.scheme() == "http"  && (
              Filetype::isNameOfSound( url.path().toUtf8().data() ) || url.host() == "apifree.forvo.com" ))
          || (url.scheme() == "file" && Filetype::isNameOfSound(url.path().toUtf8().data() ))
            ||  url.hasQueryItem("webtts");
}

}
