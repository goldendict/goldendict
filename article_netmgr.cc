/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

#include "article_netmgr.hh"
#include "wstring_qt.hh"
#include "dprintf.hh"
using std::string;

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

    QString contentType;

    sptr< Dictionary::DataRequest > dr = getResource( req.url(), contentType );

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
      qWarning( "Blocking element \"%s\"\n", req.url().toEncoded().data() );

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
      newUrl.setPath( QUrl::fromLocalFile( fileName ).path() );

      QNetworkRequest newReq( req );
      newReq.setUrl( newUrl );

      return QNetworkAccessManager::createRequest( op, newReq, outgoingData );
    }
  }

  // spoof User-Agent
  if ( hideGoldenDictHeader && req.url().scheme().startsWith("http", Qt::CaseInsensitive))
  {
    QNetworkRequest newReq( req );
    newReq.setRawHeader("User-Agent", req.rawHeader("User-Agent").replace(qApp->applicationName(), ""));
    return QNetworkAccessManager::createRequest( op, newReq, outgoingData );
  }

  return QNetworkAccessManager::createRequest( op, req, outgoingData );
}

sptr< Dictionary::DataRequest > ArticleNetworkAccessManager::getResource(
  QUrl const & url, QString & contentType )
{
  DPRINTF( "getResource: %ls\n", url.toString().toStdWString().c_str() );
  DPRINTF( "scheme: %ls\n", url.scheme().toStdWString().c_str() );
  DPRINTF( "host: %ls\n", url.host().toStdWString().c_str() );

  if ( url.scheme() == "gdlookup" )
  {
    contentType = "text/html";

    if ( url.queryItemValue( "blank" ) == "1" )
      return articleMaker.makeEmptyPage();

    bool groupIsValid = false;

    QString word = url.queryItemValue( "word" );
    unsigned group = url.queryItemValue( "group" ).toUInt( &groupIsValid );

    // See if we have some dictionaries muted

    QSet< QString > mutedDicts =
        QSet< QString >::fromList( url.queryItemValue( "muted" ).split( ',' ) );

    // Unpack contexts

    QMap< QString, QString > contexts;

    QString contextsEncoded = url.queryItemValue( "contexts" );

    if ( contextsEncoded.size() )
    {
      QByteArray ba = QByteArray::fromBase64( contextsEncoded.toLatin1() );

      QBuffer buf( & ba );

      buf.open( QBuffer::ReadOnly );

      QDataStream stream( &buf );

      stream >> contexts;
    }

    if ( groupIsValid && word.size() ) // Require group and word to be passed
      return articleMaker.makeDefinitionFor( word, group, contexts, mutedDicts );
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
              return  dictionaries[ x ]->getResource( url.path().mid( 1 ).toUtf8().data() );
            }
            catch( std::exception & e )
            {
              qWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
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
      DPRINTF( "In-place finish.\n" );
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
  long avail = req->dataSize();
  
  if ( avail < 0 )
    return 0;
  
  return (size_t) avail - alreadyRead +  QNetworkReply::bytesAvailable();
}

qint64 ArticleResourceReply::readData( char * out, qint64 maxSize )
{
  // From the doc: "This function might be called with a maxSize of 0,
  // which can be used to perform post-reading operations".
  if ( maxSize == 0 )
    return 0;

  DPRINTF( "====reading %d bytes\n", (int)maxSize );

  bool finished = req->isFinished();
  
  long avail = req->dataSize();

  if ( avail < 0 )
    return finished ? -1 : 0;

  size_t left = (size_t) avail - alreadyRead;
  
  size_t toRead = maxSize < left ? maxSize : left;

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
    error( ContentNotFoundError );

  finished();
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
