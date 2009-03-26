/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
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

    QString contentType;

    sptr< Dictionary::DataRequest > dr = getResource( req.url(), contentType );

    if ( dr.get() )
      return new ArticleResourceReply( this, req, dr, contentType );
  }

  return QNetworkAccessManager::createRequest( op, req, outgoingData );
}

sptr< Dictionary::DataRequest > ArticleNetworkAccessManager::getResource(
  QUrl const & url, QString & contentType )
{
  printf( "getResource: %ls\n", url.toString().toStdWString().c_str() );
  printf( "scheme: %ls\n", url.scheme().toStdWString().c_str() );
  printf( "host: %ls\n", url.host().toStdWString().c_str() );

  if ( url.scheme() == "gdlookup" )
  {
    QString word = url.queryItemValue( "word" );
    QString group = url.queryItemValue( "group" );

    contentType = "text/html";

    return ( url.queryItemValue( "notfound" ) != "1" ) ?
      articleMaker.makeDefinitionFor( word, group ) :
      articleMaker.makeNotFoundTextFor( word, group );
  }

  if ( ( url.scheme() == "bres" || url.scheme() == "gdau" ) &&
       url.path().size() )
  {
    //printf( "Get %s\n", req.url().host().toLocal8Bit().data() );
    //printf( "Get %s\n", req.url().path().toLocal8Bit().data() );

    string id = url.host().toStdString();

    bool search = ( id == "search" );

    if ( !search )
    {
      for( unsigned x = 0; x < dictionaries.size(); ++x )
        if ( dictionaries[ x ]->getId() == id )
          return  dictionaries[ x ]->getResource( url.path().mid( 1 ).toUtf8().data() );
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
      printf( "In-place finish.\n" );
    }
  }
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
  printf( "====reading %d bytes\n", (int)maxSize );

  bool finished = req->isFinished();
  
  long avail = req->dataSize();

  if ( avail < 0 )
    return finished ? -1 : 0;

  size_t left = (size_t) avail - alreadyRead;
  
  size_t toRead = maxSize < left ? maxSize : left;

  req->getDataSlice( alreadyRead, toRead, out );

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

