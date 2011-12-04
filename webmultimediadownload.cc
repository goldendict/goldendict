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
  //reply.reset();
  if(reply)reply->deleteLater();
  finish();
}

void WebMultimediaDownload::replyFinished( QNetworkReply * r )
{
    if ( r->error() == QNetworkReply::NoError )
    {

        QUrl redirectUrl = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if(!redirectUrl.isEmpty())
        {
            if(redirectUrl.scheme().isEmpty())
            {
                if(redirectUrl.host().isEmpty())
                {
                    if(redirectUrl.toString().indexOf("/")!=0)
                    {
                        redirectUrl.setPath(r->url().path().left(r->url().path().lastIndexOf("/"))+redirectUrl.path());
                    }
                    redirectUrl.setHost(r->url().host());
                }
                redirectUrl.setScheme(r->url().scheme());
            }
            QString nurl(redirectUrl.toString());
           //QNetworkAccessManager *mrg = reply->manager();
            //reply.reset();
            reply=r->manager()->get(QNetworkRequest(redirectUrl));
            //isRedirect = true;
            r->deleteLater();
            return;
        }
      QByteArray all = r->readAll();

      Mutex::Lock _( dataMutex );

      data.resize( all.size() );

      memcpy( data.data(), all.data(), all.size() );

      hasAnyData = true;
    }
    else
      setErrorString( r->errorString() );
    r->deleteLater();
    finish();
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
