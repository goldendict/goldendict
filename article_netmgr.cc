/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#if defined( _MSC_VER ) && _MSC_VER < 1800 // VS2012 and older
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

#include <QUrl>

#include "article_netmgr.hh"
#include "article_maker.hh"
#include "wstring_qt.hh"
#include "gddebug.hh"
#include "qt4x5.hh"

#ifndef USE_QTWEBKIT
#include <QThread>
#endif

using std::string;

#ifdef USE_QTWEBKIT
#if QT_VERSION >= 0x050300 // Qt 5.3+

  // SecurityWhiteList

  SecurityWhiteList & SecurityWhiteList::operator=( SecurityWhiteList const & swl )
  {
    swlDelete();
    swlCopy( swl );
    return *this;
  }

  QWebSecurityOrigin * SecurityWhiteList::setOrigin( QUrl const & url )
  {
    swlDelete();
    originUri = url.toString( QUrl::PrettyDecoded );
    origin = new QWebSecurityOrigin( url );
    return origin;
  }

  void SecurityWhiteList::swlCopy( SecurityWhiteList const & swl )
  {
    if( swl.origin )
    {
      hostsToAccess = swl.hostsToAccess;
      originUri = swl.originUri;
      origin = new QWebSecurityOrigin( QUrl( originUri ) );

      for( QSet< QPair< QString, QString > >::iterator it = hostsToAccess.begin();
           it != hostsToAccess.end(); ++it )
        origin->addAccessWhitelistEntry( it->first, it->second, QWebSecurityOrigin::AllowSubdomains );
    }
  }

  void SecurityWhiteList::swlDelete()
  {
    if( origin )
    {
      for( QSet< QPair< QString, QString > >::iterator it = hostsToAccess.begin();
           it != hostsToAccess.end(); ++it )
        origin->removeAccessWhitelistEntry( it->first, it->second, QWebSecurityOrigin::AllowSubdomains );

      delete origin;
      origin = 0;
    }
    hostsToAccess.clear();
    originUri.clear();
  }

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

    connect( baseReply, SIGNAL( finished() ), this, SLOT( finishedSlot() ) );

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
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    emit errorOccurred( code );
#else
    emit error( code );
#endif
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

  void AllowFrameReply::finishedSlot()
  {
#if QT_VERSION >= QT_VERSION_CHECK( 4, 8, 0 )
    setFinished( true );
#endif
    emit finished();
  }

#endif // QT_VERSION
#endif // USE_QTWEBKIT

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

string ArticleNetworkAccessManager::makeBlankPage( QColor * pageBackgroundColor ) const
{
  return articleMaker.makeBlankPageHtmlCode( pageBackgroundColor );
}

QNetworkReply * ArticleNetworkAccessManager::createRequest( Operation op,
                                                            QNetworkRequest const & req,
                                                            QIODevice * outgoingData )
{
  // Don't wrap in AllowFrameReply replies for local URL schemes used by the initial blank and Welcome! pages
  // to prevent the warning "QIODevice::read (QNetworkReplyFileImpl): device not open" at GoldenDict start
  // as AllowFrameReply::baseReply is not open when AllowFrameReply::readDataFromBase() is invoked then.

  if( req.url().scheme() == QLatin1String( "qrc" ) )
    return QNetworkAccessManager::createRequest( op, req, outgoingData ); // bypass AllowFrameReply

  QNetworkRequest localReq( req );

  if( ( localReq.url().scheme() == "gdlookup" || localReq.url().scheme() == "http" ) && localReq.url().host() == "upload.wikimedia.org" )
  {
    // Handle some requests from offline wikipedia/wiktionary without scheme or with "http" scheme

    QUrl newUrl( req.url() );
    newUrl.setScheme( "https" );
    localReq.setUrl( newUrl );
  }

  if ( op == GetOperation )
  {
    if ( localReq.url().scheme() == "qrcx" )
    {
      // We had to override the local load policy for the qrc URL scheme until QWebSecurityOrigin::addLocalScheme() was
      // introduced in Qt 4.6. Hence we used a custom qrcx URL scheme and redirected it here back to qrc. Qt versions
      // older than 4.6 are no longer supported, so GoldenDict itself no longer uses the qrcx scheme. However, qrcx has
      // been used for many years in our built-in article styles, and so may appear in user-defined article styles.
      // TODO: deprecate (print a warning or show a warning message box) and eventually remove support for the obsolete
      // qrcx URL scheme. A recent commit "Add support for qrc:// URL scheme" is the first one where the qrc scheme
      // works correctly. So the deprecation has to wait until older GoldenDict versions become rarely used.

      QUrl newUrl( localReq.url() );

      newUrl.setScheme( "qrc" );
      newUrl.setHost( "" );

      localReq.setUrl( newUrl );

      return QNetworkAccessManager::createRequest( op, localReq, outgoingData );
    }

#ifdef USE_QTWEBKIT
#if QT_VERSION >= 0x050300 // Qt 5.3+
    // Workaround of same-origin policy
    if( ( localReq.url().scheme().startsWith( "http" ) || localReq.url().scheme() == "ftp" )
        && localReq.hasRawHeader( "Referer" ) )
    {
      QByteArray referer = localReq.rawHeader( "Referer" );
      QUrl refererUrl = QUrl::fromEncoded( referer );

      if( refererUrl.scheme().startsWith( "http") || refererUrl.scheme() == "ftp" )
      {
        // Only for pages from network resources
        if ( !localReq.url().host().endsWith( refererUrl.host() ) )
        {
          QUrl frameUrl;
          frameUrl.setScheme( refererUrl.scheme() );
          frameUrl.setHost( refererUrl.host() );
          QString frameStr = frameUrl.toString( QUrl::PrettyDecoded );

          SecurityWhiteList & value = allOrigins[ frameStr ];
          if( !value.origin )
            value.setOrigin( frameUrl );

          QPair< QString, QString > target( localReq.url().scheme(), localReq.url().host() );
          if( value.hostsToAccess.find( target ) == value.hostsToAccess.end() )
          {
            value.hostsToAccess.insert( target );
            value.origin->addAccessWhitelistEntry( target.first, target.second,
                                                   QWebSecurityOrigin::AllowSubdomains );
          }
        }
      }
    }
#endif // QT_VERSION
#endif // USE_QTWEBKIT

    QString contentType;

    sptr< Dictionary::DataRequest > dr = getResource( localReq.url(), contentType );

    if ( dr.get() )
    {
      ArticleResourceReply * const reply = new ArticleResourceReply( this, localReq, dr, contentType );
#ifndef USE_QTWEBKIT
      reply->setStreamingDeviceWorkarounds( streamingDeviceWorkarounds );
#endif
      return reply;
    }
  }

  // Check the Referer. If the user has opted-in to block elements from external
  // pages, we block them.

  if ( disallowContentFromOtherSites && localReq.hasRawHeader( "Referer" ) )
  {
    QByteArray referer = localReq.rawHeader( "Referer" );

    //DPRINTF( "Referer: %s\n", referer.data() );

    QUrl refererUrl = QUrl::fromEncoded( referer );

    //DPRINTF( "Considering %s vs %s\n", getHostBase( localReq.url() ).toUtf8().data(),
    //        getHostBase( refererUrl ).toUtf8().data() );

    if ( !localReq.url().host().endsWith( refererUrl.host() ) &&
         getHostBase( localReq.url() ) != getHostBase( refererUrl ) && !localReq.url().scheme().startsWith("data") )
    {
      gdWarning( "Blocking element \"%s\"\n", localReq.url().toEncoded().data() );

      return new BlockedNetworkReply( localReq, this );
    }
  }

  // TODO (Qt WebEngine): obtain a dictionary that contains file:// links for testing and
  // make this code work in the Qt WebEngine version. Currently it does not work because
  // GoldenDict does not install an URL scheme handler for the standard "file" scheme.
  // This looks like an adjustment of a relative path to a dictionary in the portable version.
  if( localReq.url().scheme() == "file" )
  {
    // Check file presence and adjust path if necessary
    QString fileName = localReq.url().toLocalFile();
    if( localReq.url().host().isEmpty() && articleMaker.adjustFilePath( fileName ) )
    {
      QUrl newUrl( localReq.url() );
      QUrl localUrl = QUrl::fromLocalFile( fileName );

      newUrl.setHost( localUrl.host() );
      newUrl.setPath( Qt4x5::Url::ensureLeadingSlash( localUrl.path() ) );

      localReq.setUrl( newUrl );

      return QNetworkAccessManager::createRequest( op, localReq, outgoingData );
    }

    return QNetworkAccessManager::createRequest( op, localReq, outgoingData ); // bypass AllowFrameReply
  }

  QNetworkReply *reply = 0;

  // spoof User-Agent
  if ( hideGoldenDictHeader && localReq.url().scheme().startsWith("http", Qt::CaseInsensitive))
  {
    QByteArray const userAgentHeader = "User-Agent";
    localReq.setRawHeader( userAgentHeader,
                           localReq.rawHeader( userAgentHeader ).replace( qApp->applicationName().toUtf8(), "" ) );
    reply = QNetworkAccessManager::createRequest( op, localReq, outgoingData );
  }

  if( !reply )
    reply = QNetworkAccessManager::createRequest( op, localReq, outgoingData );

  if( localReq.url().scheme() == "https")
  {
#ifndef QT_NO_OPENSSL
    connect( reply, SIGNAL( sslErrors( QList< QSslError > ) ),
             reply, SLOT( ignoreSslErrors() ) );
#endif
  }

#if defined( USE_QTWEBKIT ) && QT_VERSION >= 0x050300 // Qt 5.3+
  return op == QNetworkAccessManager::GetOperation
         || op == QNetworkAccessManager::HeadOperation ? new AllowFrameReply( reply ) : reply;
#else
  return reply;
#endif
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
    {
      // This branch is never taken in the Qt WebEngine and Qt 4 WebKit versions.
      // It is taken in the Qt 5 WebKit version only when the initial blank page is reloaded.
      return articleMaker.makeBlankPage();
    }

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

    QStringList const mutedDictList = Qt4x5::Url::queryItemValue( url, "muted" ).split( ',' );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 14, 0 )
    QSet< QString > const mutedDicts( mutedDictList.cbegin(), mutedDictList.cend() );
#else
    QSet< QString > const mutedDicts = QSet< QString >::fromList( mutedDictList );
#endif

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
                contentType = "image/png";
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
              return  dictionaries[ x ]->getResource( Qt4x5::Url::fullPath( url ).mid( 1 ).toUtf8().data() );
            }
            catch( std::exception & e )
            {
              gdWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
                         dictionaries[ x ]->getName().c_str() );
              return sptr< Dictionary::DataRequest >();
            }
        }
    }
    else
    {
      // We don't do search requests for now
#if 0
      for( unsigned x = 0; x < dictionaries.size(); ++x )
      {
        if ( search || dictionaries[ x ]->getId() == id )
        {
          try
          {
            dictionaries[ x ]->getResource( url.path().mid( 1 ).toUtf8().data(),
                                            data );

            return true;
          }
          catch( Dictionary::exNoSuchResource & )
          {
            if ( !search )
              break;
          }
        }
      }
#endif      
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
  setUrl( netReq.url() );
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

#ifndef USE_QTWEBKIT
bool ArticleResourceReply::atEnd() const
{
  if( streamingDeviceWorkarounds != StreamingDeviceWorkarounds::None )
  {
    // QWebEngineUrlRequestJob finishes and is destroyed as soon as QIODevice::atEnd() returns true.
    // QNetworkReply::atEnd() returns true while bytesAvailable() returns 0.
    // Return false if the data request is not finished to work around always-blank web page.
    return req->isFinished() && QNetworkReply::atEnd();
  }
  return QNetworkReply::atEnd();
}
#endif

void ArticleResourceReply::close()
{
  req->cancel();
  QNetworkReply::close();
}

qint64 ArticleResourceReply::readData( char * out, qint64 maxSize )
{
  // From the doc: "This function might be called with a maxSize of 0,
  // which can be used to perform post-reading operations".
  if ( maxSize == 0 )
    return 0;

  GD_DPRINTF( "====reading %d bytes\n", (int)maxSize );

  bool finished = req->isFinished();
  
  qint64 avail = req->dataSize();

  if ( avail < 0 )
    return finished ? -1 : 0;

  qint64 left = avail - alreadyRead;

#ifndef USE_QTWEBKIT
  if( streamingDeviceWorkarounds == StreamingDeviceWorkarounds::AtEndAndReadData && left == 0 && !finished )
  {
    // Work around endlessly repeated useless calls to readData(). The sleep duration is a tradeoff.
    // On the one hand, lowering the duration reduces CPU usage. On the other hand, overly long
    // sleep duration reduces page content update frequency in the web view.
    // Waiting on a condition variable is more complex and actually works worse than
    // simple fixed-duration sleeping, because the web view is not updated until
    // the data request is finished if readData() returns only when new data arrives.
    QThread::msleep( 30 );
    return 0;
  }
#endif
  
  qint64 toRead = maxSize < left ? maxSize : left;

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
  readyRead();
}

void ArticleResourceReply::finishedSlot()
{
  if ( req->dataSize() < 0 )
  {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
    emit errorOccurred( ContentNotFoundError );
#else
    emit error( ContentNotFoundError );
#endif
  }

#if QT_VERSION >= QT_VERSION_CHECK( 4, 8, 0 )
  setFinished( true );
#endif
  emit finished();
}

BlockedNetworkReply::BlockedNetworkReply( QNetworkRequest const & request, QObject * parent ):
  QNetworkReply( parent )
{
  setUrl( request.url() );
  setError( QNetworkReply::ContentOperationNotPermittedError, "Content Blocked" );

  connect( this, SIGNAL( finishedSignal() ), this, SLOT( finishedSlot() ),
           Qt::QueuedConnection );

  emit finishedSignal(); // This way we call readyRead()/finished() sometime later
}


void BlockedNetworkReply::finishedSlot()
{
  emit readyRead();

#if QT_VERSION >= QT_VERSION_CHECK( 4, 8, 0 )
  setFinished( true );
#endif
  emit finished();
}
