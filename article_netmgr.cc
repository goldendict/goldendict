/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#if defined( _MSC_VER ) && _MSC_VER < 1800 // VS2012 and older
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

#include <QUrl>

#include "article_netmgr.hh"
#include "wstring_qt.hh"
#include "gddebug.hh"
#include "qt4x5.hh"
#include <QNetworkAccessManager>

using std::string;

  // AllowFrameReply

  AllowFrameReply::AllowFrameReply( QNetworkReply * _reply ) :
    baseReply( _reply )
  {
    // Set base data

    setOperation( baseReply->operation() );
    setRequest( baseReply->request() );
    setUrl( baseReply->url() );

    // Signals to own slots

    connect( baseReply, SIGNAL( metaDataChanged() ), this, SLOT( applyMetaData() ) );

    connect( baseReply, SIGNAL( error( QNetworkReply::NetworkError) ),
             this, SLOT( applyError( QNetworkReply::NetworkError ) ) );

    connect( baseReply, SIGNAL( readyRead() ), this, SLOT( readDataFromBase() ) );

    // Redirect QNetworkReply signals

    connect( baseReply, SIGNAL( downloadProgress( qint64, qint64 ) ),
             this, SIGNAL( downloadProgress( qint64, qint64 ) ) );

    connect( baseReply, SIGNAL( encrypted() ), this, SIGNAL( encrypted() ) );

    connect( baseReply, SIGNAL( finished() ), this, SIGNAL( finished() ) );

    connect( baseReply, SIGNAL( preSharedKeyAuthenticationRequired( QSslPreSharedKeyAuthenticator * ) ),
             this, SIGNAL( preSharedKeyAuthenticationRequired( QSslPreSharedKeyAuthenticator * ) ) );

    connect( baseReply, SIGNAL( redirected( const QUrl & ) ), this, SIGNAL( redirected( const QUrl & ) ) );

    connect( baseReply, SIGNAL( sslErrors( const QList< QSslError > & ) ),
             this, SIGNAL( sslErrors( const QList< QSslError > & ) ) );

    connect( baseReply, SIGNAL( uploadProgress( qint64, qint64 ) ),
             this, SIGNAL( uploadProgress( qint64, qint64 ) ) );

    // Redirect QIODevice signals

    connect( baseReply, SIGNAL( aboutToClose() ), this, SIGNAL( aboutToClose() ) );

    connect( baseReply, SIGNAL( bytesWritten( qint64 ) ), this, SIGNAL( bytesWritten( qint64 ) ) );

    connect( baseReply, SIGNAL( readChannelFinished() ), this, SIGNAL( readChannelFinished() ) );

    setOpenMode( QIODevice::ReadOnly );
  }

  void AllowFrameReply::applyMetaData()
  {
    // Set raw headers except X-Frame-Options
    QList< QByteArray > rawHeaders = baseReply->rawHeaderList();
    for( QList< QByteArray >::iterator it = rawHeaders.begin(); it != rawHeaders.end(); ++it )
    {
      if( it->toLower() != "x-frame-options" )
        setRawHeader( *it, baseReply->rawHeader( *it ) );
    }

    // Set known headers
    setHeader( QNetworkRequest::ContentDispositionHeader,
               baseReply->header( QNetworkRequest::ContentDispositionHeader ) );
    setHeader( QNetworkRequest::ContentTypeHeader,
               baseReply->header( QNetworkRequest::ContentTypeHeader ) );
    setHeader( QNetworkRequest::ContentLengthHeader,
               baseReply->header( QNetworkRequest::ContentLengthHeader ) );
    setHeader( QNetworkRequest::LocationHeader,
               baseReply->header( QNetworkRequest::LocationHeader ) );
    setHeader( QNetworkRequest::LastModifiedHeader,
               baseReply->header( QNetworkRequest::LastModifiedHeader ) );
    setHeader( QNetworkRequest::CookieHeader,
               baseReply->header( QNetworkRequest::CookieHeader ) );
    setHeader( QNetworkRequest::SetCookieHeader,
               baseReply->header( QNetworkRequest::SetCookieHeader ) );
    setHeader( QNetworkRequest::UserAgentHeader,
               baseReply->header( QNetworkRequest::UserAgentHeader ) );
    setHeader( QNetworkRequest::ServerHeader,
               baseReply->header( QNetworkRequest::ServerHeader ) );

    // Set attributes
    setAttribute( QNetworkRequest::HttpStatusCodeAttribute,
                  baseReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ) );
    setAttribute( QNetworkRequest::HttpReasonPhraseAttribute,
                  baseReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ) );
    setAttribute( QNetworkRequest::RedirectionTargetAttribute,
                  baseReply->attribute( QNetworkRequest::RedirectionTargetAttribute ) );
    setAttribute( QNetworkRequest::ConnectionEncryptedAttribute,
                  baseReply->attribute( QNetworkRequest::ConnectionEncryptedAttribute ) );
    setAttribute( QNetworkRequest::SourceIsFromCacheAttribute,
                  baseReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ) );
    setAttribute( QNetworkRequest::HttpPipeliningWasUsedAttribute,
                  baseReply->attribute( QNetworkRequest::HttpPipeliningWasUsedAttribute ) );
    setAttribute( QNetworkRequest::BackgroundRequestAttribute,
                  baseReply->attribute( QNetworkRequest::BackgroundRequestAttribute ) );
    setAttribute( QNetworkRequest::SpdyWasUsedAttribute,
                  baseReply->attribute( QNetworkRequest::SpdyWasUsedAttribute ) );

    emit metaDataChanged();
  }

  void AllowFrameReply::setReadBufferSize( qint64 size )
  {
    QNetworkReply::setReadBufferSize( size );
    baseReply->setReadBufferSize( size );
  }

  qint64 AllowFrameReply::bytesAvailable() const
  {
    return buffer.size() + QNetworkReply::bytesAvailable();
  }

  void AllowFrameReply::applyError( QNetworkReply::NetworkError code )
  {
    setError( code, baseReply->errorString() );
    emit error( code );
  }

  void AllowFrameReply::readDataFromBase()
  {
    QByteArray data;
    data.resize( baseReply->bytesAvailable() );
    baseReply->read( data.data(), data.size() );
    buffer += data;
    emit readyRead();
  }

  qint64 AllowFrameReply::readData( char * data, qint64 maxSize )
  {
    qint64 size = qMin( maxSize, qint64( buffer.size() ) );
    memcpy( data, buffer.data(), size );
    buffer.remove( 0, size );
    return size;
  }


namespace
{
  /// Uses some heuristics to chop off the first domain name from the host name,
  /// but only if it's not too base. Returns the resulting host name.
  QString getHostBase( QUrl const & url )
  {
    QString host = url.host();

    QStringList domains = host.split( '.' );

    int left = domains.size();

    // Skip last <=3-letter domain name
    if ( left && domains[ left - 1 ].size() <= 3 )
      --left;

    // Skip another <=3-letter domain name
    if ( left && domains[ left - 1 ].size() <= 3 )
      --left;

    if ( left > 1 )
    {
      // We've got something like www.foobar.co.uk -- we can chop off the first
      // domain

      return host.mid( domains[ 0 ].size() + 1 );
    }
    else
      return host;
  }
}

QNetworkReply * ArticleNetworkAccessManager::createRequest( Operation op,
                                                            QNetworkRequest const & req,
                                                            QIODevice * outgoingData )
{
  if ( op == GetOperation )
  {
    if ( req.url().scheme() == "qrcx" )
    {
      // We have to override the local load policy for the qrc scheme, hence
      // we use qrcx and redirect it here back to qrc
      QUrl newUrl( req.url() );

      newUrl.setScheme( "qrc" );
      newUrl.setHost( "" );

      QNetworkRequest newReq( req );
      newReq.setUrl( newUrl );

      return QNetworkAccessManager::createRequest( op, newReq, outgoingData );
    }

    QUrl url=req.url();
    QMimeType mineType=db.mimeTypeForUrl (url);
    QString contentType=mineType.name();

    if(req.url().scheme()=="gdlookup"){
        QString path=url.path();
        if(!path.isEmpty()){
            Qt4x5::Url::addQueryItem(url,"word",path.mid(1));
            url.setPath("");
            Qt4x5::Url::addQueryItem(url,"group","1");

        }
    }

    sptr< Dictionary::DataRequest > dr = getResource( url, contentType );

    if ( dr.get() )
      return new ArticleResourceReply( this, req, dr, contentType );
  }

  // Check the Referer. If the user has opted-in to block elements from external
  // pages, we block them.

  if ( disallowContentFromOtherSites && req.hasRawHeader( "Referer" ) )
  {
    QByteArray referer = req.rawHeader( "Referer" );

    //DPRINTF( "Referer: %s\n", referer.data() );

    QUrl refererUrl = QUrl::fromEncoded( referer );

    //DPRINTF( "Considering %s vs %s\n", getHostBase( req.url() ).toUtf8().data(),
    //        getHostBase( refererUrl ).toUtf8().data() );

    if ( !req.url().host().endsWith( refererUrl.host() ) &&
         getHostBase( req.url() ) != getHostBase( refererUrl ) && !req.url().scheme().startsWith("data") )
    {
      gdWarning( "Blocking element \"%s\"\n", req.url().toEncoded().data() );

      return new BlockedNetworkReply( this );
    }
  }

  if( req.url().scheme() == "file" )
  {
    // Check file presence and adjust path if necessary
    QString fileName = req.url().toLocalFile();
    if( req.url().host().isEmpty() && articleMaker.adjustFilePath( fileName ) )
    {
      QUrl newUrl( req.url() );
      QUrl localUrl = QUrl::fromLocalFile( fileName );

      newUrl.setHost( localUrl.host() );
      newUrl.setPath( Qt4x5::Url::ensureLeadingSlash( localUrl.path() ) );

      QNetworkRequest newReq( req );
      newReq.setUrl( newUrl );

      return QNetworkAccessManager::createRequest( op, newReq, outgoingData );
    }
  }

  QNetworkReply *reply = 0;

  // spoof User-Agent
  if ( hideGoldenDictHeader && req.url().scheme().startsWith("http", Qt::CaseInsensitive))
  {
    QNetworkRequest newReq( req );
    newReq.setRawHeader("User-Agent", req.rawHeader("User-Agent").replace(qApp->applicationName(), ""));
    reply = QNetworkAccessManager::createRequest( op, newReq, outgoingData );
  }

  if( !reply )
    reply = QNetworkAccessManager::createRequest( op, req, outgoingData );

  if( req.url().scheme() == "https")
  {
#ifndef QT_NO_OPENSSL
    connect( reply, SIGNAL( sslErrors( QList< QSslError > ) ),
             reply, SLOT( ignoreSslErrors() ) );
#endif
  }


  return op == QNetworkAccessManager::GetOperation
         || op == QNetworkAccessManager::HeadOperation ? new AllowFrameReply( reply ) : reply;

}

sptr< Dictionary::DataRequest > ArticleNetworkAccessManager::getResource(
  QUrl const & url, QString & contentType )
{
  GD_DPRINTF( "getResource: %ls\n", url.toString().toStdWString().c_str() );
  GD_DPRINTF( "scheme: %ls\n", url.scheme().toStdWString().c_str() );
  GD_DPRINTF( "host: %ls\n", url.host().toStdWString().c_str() );

  if ( url.scheme() == "gdlookup" )
  {
    if( !url.host().isEmpty() && url.host() != "localhost" )
    {
      // Strange request - ignore it
      return new Dictionary::DataRequestInstant( false );
    }

    contentType = "text/html";

    if ( Qt4x5::Url::queryItemValue( url, "blank" ) == "1" )
      return articleMaker.makeEmptyPage();

    Config::InputPhrase phrase ( Qt4x5::Url::queryItemValue( url, "word" ).trimmed(),
                                 Qt4x5::Url::queryItemValue( url, "punctuation_suffix" ) );

    bool groupIsValid = false;
    unsigned group = Qt4x5::Url::queryItemValue( url, "group" ).toUInt( &groupIsValid );
   
    QString dictIDs = Qt4x5::Url::queryItemValue( url, "dictionaries" );
    if( !dictIDs.isEmpty() )
    {
      // Individual dictionaries set from full-text search
      QStringList dictIDList = dictIDs.split( "," );
      return articleMaker.makeDefinitionFor( phrase, 0, QMap< QString, QString >(), QSet< QString >(), dictIDList );
    }

    // See if we have some dictionaries muted

    QSet< QString > mutedDicts =
        QSet< QString >::fromList( Qt4x5::Url::queryItemValue( url, "muted" ).split( ',' ) );

    // Unpack contexts

    QMap< QString, QString > contexts;

    QString contextsEncoded = Qt4x5::Url::queryItemValue( url, "contexts" );

    if ( contextsEncoded.size() )
    {
      QByteArray ba = QByteArray::fromBase64( contextsEncoded.toLatin1() );

      QBuffer buf( & ba );

      buf.open( QBuffer::ReadOnly );

      QDataStream stream( &buf );

      stream >> contexts;
    }

    // See for ignore diacritics

    bool ignoreDiacritics = Qt4x5::Url::queryItemValue( url, "ignore_diacritics" ) == "1";

    if ( groupIsValid && phrase.isValid() ) // Require group and phrase to be passed
      return articleMaker.makeDefinitionFor( phrase, group, contexts, mutedDicts, QStringList(), ignoreDiacritics );
  }

  if ( ( url.scheme() == "bres" || url.scheme() == "gdau" || url.scheme() == "gdvideo" || url.scheme() == "gico" ) &&
       url.path().size() )
  {
    //DPRINTF( "Get %s\n", req.url().host().toLocal8Bit().data() );
    //DPRINTF( "Get %s\n", req.url().path().toLocal8Bit().data() );

    string id = url.host().toStdString();

    bool search = ( id == "search" );

    if ( !search )
    {
      for( unsigned x = 0; x < dictionaries.size(); ++x )
        if ( dictionaries[ x ]->getId() == id )
        {
            if( url.scheme() == "gico" )
            {
                QByteArray bytes;
                QBuffer buffer(&bytes);
                buffer.open(QIODevice::WriteOnly);
                dictionaries[ x ]->getIcon().pixmap( 16 ).save(&buffer, "PNG");
                buffer.close();
                sptr< Dictionary::DataRequestInstant > ico = new Dictionary::DataRequestInstant( true );
                ico->getData().resize( bytes.size() );
                memcpy( &( ico->getData().front() ), bytes.data(), bytes.size() );
                return ico;
            }
            try
            {
              return  dictionaries[ x ]->getResource( Qt4x5::Url::path( url ).mid( 1 ).toUtf8().data() );
            }
            catch( std::exception & e )
            {
              gdWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
                         dictionaries[ x ]->getName().c_str() );
              return sptr< Dictionary::DataRequest >();
            }
        }
    }

  }

  if ( url.scheme() == "gdpicture" )
  {
    contentType = "text/html";
    QUrl imgUrl ( url );
    imgUrl.setScheme( "bres" );
    return articleMaker.makePicturePage( imgUrl.toEncoded().data() );
  }

  return sptr< Dictionary::DataRequest >();
}

ArticleResourceReply::ArticleResourceReply( QObject * parent,
  QNetworkRequest const & netReq,
  sptr< Dictionary::DataRequest > const & req_,
  QString const & contentType ):
  QNetworkReply( parent ), req( req_ ), alreadyRead( 0 )
{
  setRequest( netReq );

  setOpenMode( ReadOnly );

  if ( contentType.size() )
    setHeader( QNetworkRequest::ContentTypeHeader, contentType );

  connect( req.get(), SIGNAL( updated() ),
           this, SLOT( reqUpdated() ) );
  
  connect( req.get(), SIGNAL( finished() ),
           this, SLOT( reqFinished() ) );
  
  if ( req->isFinished() || req->dataSize() > 0 )
  {
    connect( this, SIGNAL( readyReadSignal() ),
             this, SLOT( readyReadSlot() ), Qt::QueuedConnection );
    connect( this, SIGNAL( finishedSignal() ),
             this, SLOT( finishedSlot() ), Qt::QueuedConnection );

    emit readyReadSignal();

    if ( req->isFinished() )
    {
      emit finishedSignal();
      GD_DPRINTF( "In-place finish.\n" );
    }
  }
}

ArticleResourceReply::~ArticleResourceReply()
{
  req->cancel();
}

void ArticleResourceReply::reqUpdated()
{
  emit readyRead();
}

void ArticleResourceReply::reqFinished()
{
  emit readyRead();
  finishedSlot();
}

qint64 ArticleResourceReply::bytesAvailable() const
{
  qint64 avail = req->dataSize();
  
  if ( avail < 0 )
    return 0;
  
  return avail - alreadyRead + QNetworkReply::bytesAvailable();
}

qint64 ArticleResourceReply::readData( char * out, qint64 maxSize )
{
  // From the doc: "This function might be called with a maxSize of 0,
  // which can be used to perform post-reading operations".
  if ( maxSize == 0 )
    return 0;

  bool finished = req->isFinished();
  
  qint64 avail = req->dataSize();

  if ( avail < 0 )
    return finished ? -1 : 0;

  qint64 left = avail - alreadyRead;
  
  qint64 toRead = maxSize < left ? maxSize : left;
  GD_DPRINTF( "====reading %d bytes\n", (int)toRead );

  try
  {
    req->getDataSlice( alreadyRead, toRead, out );
  }
  catch( std::exception & e )
  {
    qWarning( "getDataSlice error: %s\n", e.what() );
  }

  alreadyRead += toRead;

  if ( !toRead && finished )
    return -1;
  else
    return toRead;
}

void ArticleResourceReply::readyReadSlot()
{
  emit readyRead();
}

void ArticleResourceReply::finishedSlot()
{
  if ( req->dataSize() < 0 )
    errorOccurred( ContentNotFoundError );

  emit finished();
}

BlockedNetworkReply::BlockedNetworkReply( QObject * parent ): QNetworkReply( parent )
{
  setError( QNetworkReply::ContentOperationNotPermittedError, "Content Blocked" );

  connect( this, SIGNAL( finishedSignal() ), this, SLOT( finishedSlot() ),
           Qt::QueuedConnection );

  emit finishedSignal(); // This way we call readyRead()/finished() sometime later
}


void BlockedNetworkReply::finishedSlot()
{
  emit readyRead();
  emit finished();
}

LocalSchemeHandler::LocalSchemeHandler(ArticleNetworkAccessManager& articleNetMgr):mManager(articleNetMgr){

}

void LocalSchemeHandler::requestStarted(QWebEngineUrlRequestJob *requestJob)
{
    QUrl url = requestJob->requestUrl();

    QNetworkRequest request;
    request.setUrl( url );

    QNetworkReply* reply=this->mManager.createRequest(QNetworkAccessManager::GetOperation,request,NULL);
    connect(reply,&QNetworkReply::finished,requestJob,[=](){
        requestJob->reply("text/html",reply);
    });
}

