/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef USE_QTWEBKIT

#include "article_urlschemehandler.hh"

#include "article_netmgr.hh"
#include "gddebug.hh"

#include <QByteArray>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QWebEngineProfile>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlScheme>

#include <array>
#include <utility>

namespace {

/// @return the names of GoldenDict's custom URL schemes handled by ArticleUrlSchemeHandler.
auto const & handledUrlSchemeNames()
{
  // Note that the remaining custom URL scheme names - "bword", "gdprg" and "gdtts" - do not
  // belong in this list, because they are handled only in ArticleView::linkHovered() and
  // ArticleView::openLink(). ArticleNetworkAccessManager does not know about these schemes.

  static std::array< QByteArray, 7 > const names {
    QByteArrayLiteral( "bres" ),
    QByteArrayLiteral( "gdau" ),
    QByteArrayLiteral( "gdlookup" ),
    QByteArrayLiteral( "gdpicture" ),
    QByteArrayLiteral( "gdvideo" ),
    QByteArrayLiteral( "gico" ),
    QByteArrayLiteral( "qrcx" )
  };
  return names;
}

QNetworkRequest networkRequestFromJob( QWebEngineUrlRequestJob & job )
{
  QNetworkRequest request( job.requestUrl() );
  auto const headers = job.requestHeaders();
  for( auto it = headers.constKeyValueBegin(), end = headers.constKeyValueEnd(); it != end; ++it )
    request.setRawHeader( it->first, it->second );
  return request;
}

QWebEngineUrlRequestJob::Error jobErrorFromNetworkError( QNetworkReply::NetworkError networkError )
{
  switch( networkError )
  {
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::ProxyNotFoundError:
    case QNetworkReply::ContentNotFoundError:
    case QNetworkReply::ContentGoneError:
      return QWebEngineUrlRequestJob::UrlNotFound;
    case QNetworkReply::ProtocolUnknownError:
    case QNetworkReply::ProtocolInvalidOperationError:
      return QWebEngineUrlRequestJob::UrlInvalid;
    case QNetworkReply::OperationCanceledError:
      return QWebEngineUrlRequestJob::RequestAborted;
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::BackgroundRequestNotAllowedError:
    case QNetworkReply::ProxyConnectionRefusedError:
    case QNetworkReply::ProxyAuthenticationRequiredError:
    case QNetworkReply::ContentAccessDenied:
    case QNetworkReply::ContentOperationNotPermittedError:
    case QNetworkReply::AuthenticationRequiredError:
      return QWebEngineUrlRequestJob::RequestDenied;
    default:
      return QWebEngineUrlRequestJob::RequestFailed;
  }
}

void reportError( QNetworkReply::NetworkError networkError, QNetworkReply const & reply, QWebEngineUrlRequestJob * job )
{
  gdWarning( "Error while requesting URL %s: %s (%s)", qUtf8Printable( reply.url().toString() ),
             qUtf8Printable( reply.errorString() ),
             QMetaEnum::fromType< decltype( networkError ) >().valueToKey( networkError ) );

  if( job )
    job->fail( jobErrorFromNetworkError( networkError ) );
}

bool respondToUrlRequest( QWebEngineUrlRequestJob * job, QNetworkReply * reply )
{
  Q_ASSERT( job );
  Q_ASSERT( reply );

  auto const networkError = reply->error();
  if( networkError != QNetworkReply::NoError )
  {
    reportError( networkError, *reply, job );
    return false;
  }

  auto contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toByteArray();
  if( contentType.isEmpty() )
  {
    // GoldenDict does not always set the Content-Type header => fall back to guessing the MIME type from the URL.
    contentType = QMimeDatabase().mimeTypeForUrl( reply->url() ).name().toUtf8();
  }

  job->reply( contentType, reply );
  return true;
}

} // unnamed namespace

void registerArticleUrlSchemes()
{
  // When unhandled URL schemes - "gdprg", "gdtts", "bword" - are registered, a blank page is
  // loaded when the user clicks on a link that points to such an URL => don't register them.

  for( auto const & name : handledUrlSchemeNames() )
  {
    QWebEngineUrlScheme scheme( name );
    scheme.setSyntax( QWebEngineUrlScheme::Syntax::Host );
    // Register dictionary schemes as local to make file:// and qrc:// links work in articles.
    scheme.setFlags( QWebEngineUrlScheme::SecureScheme
                     | QWebEngineUrlScheme::LocalScheme
                     | QWebEngineUrlScheme::LocalAccessAllowed );
    QWebEngineUrlScheme::registerScheme( scheme );
  }
}

ArticleUrlSchemeHandler::ArticleUrlSchemeHandler( ArticleNetworkAccessManager & netMgr ) :
  QWebEngineUrlSchemeHandler( &netMgr ),
  articleNetMgr( netMgr )
{
}

void ArticleUrlSchemeHandler::install( QWebEngineProfile & profile )
{
  for( auto const & name : handledUrlSchemeNames() )
    profile.installUrlSchemeHandler( name, this );
}

void ArticleUrlSchemeHandler::requestStarted( QWebEngineUrlRequestJob * job )
{
  if( job->requestMethod() != "GET" )
  {
    gdWarning( "Unsupported custom scheme request method: %s. Initiator: %s. URL: %s.",
               job->requestMethod().constData(), qUtf8Printable( job->initiator().toString() ),
               qUtf8Printable( job->requestUrl().toString() ) );
    job->fail( QWebEngineUrlRequestJob::UrlInvalid );
    return;
  }

  auto * const reply = articleNetMgr.get( networkRequestFromJob( *job ) );

  // Respond to the request immediately in order to show topmost articles as
  // soon as they become available, without waiting for all articles to load.
  // job uses the QIODevice interface of reply in another thread. *reply can be one of the following:
  // - a qrc:// QNetworkAccessManager's QNetworkReply;
  // - QNetworkAccessManager's QNetworkReply for a GoldenDict's custom URL scheme (an error page);
  // - ArticleResourceReply;
  // - BlockedNetworkReply.
  // Hopefully the QNetworkAccessManager's replies support such usage. ArticleResourceReply is designed
  // to be thread-safe in this scenario. BlockedNetworkReply is trivial and obviously thread-safe.

  if( !respondToUrlRequest( job, reply ) )
  {
    // A network error occurred and respondToUrlRequest() handled it.
    // Destroy the no longer needed reply as soon as it finishes.
    connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
    return;
  }

  QPointer< QWebEngineUrlRequestJob > jobPointer( job );

  connect( reply, &QNetworkReply::errorOccurred, reply, [ reply, jobPointer ]( QNetworkReply::NetworkError code ) {
    reportError( code, *reply, jobPointer );
  } );

  // Deliberately don't use job as context in this connection: if job is destroyed
  // before reply is finished, reply would be leaked. Using reply as context does
  // not impact behavior, but silences Clazy checker connect-3arg-lambda (level1).
  connect( reply, &QNetworkReply::finished, reply, [ reply, jobPointer = std::move( jobPointer ) ] {
    // At this point reply is no longer written to and can be safely destroyed once job ends reading from it.
    if( jobPointer )
      connect( jobPointer, &QObject::destroyed, reply, &QObject::deleteLater );
    else
      reply->deleteLater();
  } );
}

#endif // USE_QTWEBKIT
