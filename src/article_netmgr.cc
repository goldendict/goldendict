/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "article_netmgr.hh"

using std::string;

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

    vector< char > data;
    QString contentType;

    if ( getResource( req.url(), data, contentType ) )
      return new ArticleResourceReply( this, req, data, contentType );
  }

  return QNetworkAccessManager::createRequest( op, req, outgoingData );
}

bool ArticleNetworkAccessManager::getResource( QUrl const & url,
                                               vector< char > & data,
                                               QString & contentType )
{
  //printf( "getResource: %ls\n", url.toString().toStdWString().c_str() );
  //printf( "scheme: %ls\n", url.scheme().toStdWString().c_str() );
  //printf( "host: %ls\n", url.host().toStdWString().c_str() );

  if ( url.scheme() == "gdlookup" )
  {
    string result = articleMaker.makeDefinitionFor( url.queryItemValue( "word" ),
                                                    url.queryItemValue( "group" ) );

    data.resize( result.size() );

    memcpy( &data.front(), result.data(), data.size() );

    contentType = "text/html";

    return true;
  }

  if ( ( url.scheme() == "bres" || url.scheme() == "gdau" ) &&
       url.path().size() )
  {
    //printf( "Get %s\n", req.url().host().toLocal8Bit().data() );
    //printf( "Get %s\n", req.url().path().toLocal8Bit().data() );

    string id = url.host().toStdString();

    bool search = ( id == "search" );

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
  }

  return false;
}

ArticleResourceReply::ArticleResourceReply( QObject * parent,
  QNetworkRequest const & req,
  vector< char > const & data_,
  QString const & contentType ):
  QNetworkReply( parent ), data( data_ ), left( data.size() )
{
  setRequest( req );

  setOpenMode( ReadOnly );

  if ( contentType.size() )
    setHeader( QNetworkRequest::ContentTypeHeader, contentType );

  connect( this, SIGNAL( readyReadSignal() ),
           this, SLOT( readyReadSlot() ), Qt::QueuedConnection );
  connect( this, SIGNAL( finishedSignal() ),
           this, SLOT( finishedSlot() ), Qt::QueuedConnection );

  emit readyReadSignal();
  emit finishedSignal();
}

qint64 ArticleResourceReply::bytesAvailable() const
{
  return left + QNetworkReply::bytesAvailable();
}

qint64 ArticleResourceReply::readData( char * out, qint64 maxSize )
{
  printf( "====reading %d bytes\n", (int)maxSize );

  size_t toRead = maxSize < left ? maxSize : left;

  memcpy( out, &data[ data.size() - left ], toRead );

  left -= toRead;

  if ( !toRead )
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
  finished();
}

