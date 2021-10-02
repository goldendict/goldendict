/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_NETMGR_HH_INCLUDED__
#define __ARTICLE_NETMGR_HH_INCLUDED__

#include <QtNetwork>

#if QT_VERSION >= 0x050300  // Qt 5.3+
#include <QSet>
#include <QMap>
#include <QPair>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QNetworkAccessManager>
#endif

#include "dictionary.hh"
#include "article_maker.hh"

using std::vector;

/// A custom QNetworkAccessManager version which fetches images from the
/// dictionaries when requested.

// Proxy class for QNetworkReply to remove X-Frame-Options header
// It allow to show websites in <iframe> tag like Qt 4.x

class AllowFrameReply : public QNetworkReply
{
  Q_OBJECT
private:
  QNetworkReply * baseReply;
  QByteArray buffer;

  AllowFrameReply();
  AllowFrameReply( AllowFrameReply const & );

public:
  explicit AllowFrameReply( QNetworkReply * _reply );
  ~AllowFrameReply()
  { delete baseReply; }

  // QNetworkReply virtual functions
  void setReadBufferSize( qint64 size );
  void close()
  { baseReply->close(); }

  // QIODevice virtual functions
  qint64 bytesAvailable() const;
  bool atEnd() const
  { return baseReply->atEnd(); }
  qint64 bytesToWrite() const
  { return baseReply->bytesToWrite(); }
  bool canReadLine() const
  { return baseReply->canReadLine(); }
  bool isSequential() const
  { return baseReply->isSequential(); }
  bool waitForReadyRead( int msecs )
  { return baseReply->waitForReadyRead( msecs ); }
  bool waitForBytesWritten( int msecs )
  { return baseReply->waitForBytesWritten( msecs ); }
  bool reset()
  { return baseReply->reset(); }

public slots:

  // Own AllowFrameReply slots
  void applyMetaData();
  void applyError( QNetworkReply::NetworkError code );
  void readDataFromBase();

  // Redirect QNetworkReply slots
  virtual void abort()
  { baseReply->abort(); }
  virtual void ignoreSslErrors()
  { baseReply->ignoreSslErrors(); }

protected:
  // QNetworkReply virtual functions
  void ignoreSslErrorsImplementation( const QList< QSslError > & errors )
  { baseReply->ignoreSslErrors( errors ); }
  void setSslConfigurationImplementation( const QSslConfiguration & configuration )
  { baseReply->setSslConfiguration( configuration ); }
  void sslConfigurationImplementation( QSslConfiguration & configuration ) const
  { configuration = baseReply->sslConfiguration(); }

  // QIODevice virtual functions
  qint64 readData( char * data, qint64 maxSize );
  qint64 readLineData( char * data, qint64 maxSize )
  { return baseReply->readLine( data, maxSize ); }
  qint64 writeData( const char * data, qint64 maxSize )
  { return baseReply->write( data, maxSize ); }
};


class ArticleNetworkAccessManager: public QNetworkAccessManager
{
    Q_OBJECT
  vector< sptr< Dictionary::Class > > const & dictionaries;
  ArticleMaker const & articleMaker;
  bool const & disallowContentFromOtherSites;
  bool const & hideGoldenDictHeader;

public:

  ArticleNetworkAccessManager( QObject * parent,
                               vector< sptr< Dictionary::Class > > const &
                               dictionaries_,
                               ArticleMaker const & articleMaker_,
                               bool const & disallowContentFromOtherSites_,
                               bool const & hideGoldenDictHeader_ ):
    QNetworkAccessManager( parent ), dictionaries( dictionaries_ ),
    articleMaker( articleMaker_ ),
    disallowContentFromOtherSites( disallowContentFromOtherSites_ ),
    hideGoldenDictHeader( hideGoldenDictHeader_ )
  {}

  /// Tries handling any kind of internal resources referenced by dictionaries.
  /// If it succeeds, the result is a dictionary request object. Otherwise, an
  /// empty pointer is returned.
  /// The function can optionally set the Content-Type header correspondingly.
  sptr< Dictionary::DataRequest > getResource( QUrl const & url,
                                               QString & contentType );

//protected:

  virtual QNetworkReply * createRequest( Operation op,
                                         QNetworkRequest const & req,
                                         QIODevice * outgoingData );

};

class ArticleResourceReply: public QNetworkReply
{
  Q_OBJECT

  sptr< Dictionary::DataRequest > req;
  qint64 alreadyRead;

public:

  ArticleResourceReply( QObject * parent,
                        QNetworkRequest const &,
                        sptr< Dictionary::DataRequest > const &,
                        QString const & contentType );

  ~ArticleResourceReply();

protected:

  virtual qint64 bytesAvailable() const;

  virtual void abort()
  {}
  virtual qint64 readData( char * data, qint64 maxSize );

  // We use the hackery below to work around the fact that we need to emit
  // ready/finish signals after we've been constructed.
signals:

  void readyReadSignal();
  void finishedSignal();

private slots:

  void reqUpdated();
  void reqFinished();
  
  void readyReadSlot();
  void finishedSlot();
};

class BlockedNetworkReply: public QNetworkReply
{
  Q_OBJECT

public:

  BlockedNetworkReply( QObject * parent );

  virtual qint64 readData( char *, qint64 )
  {
    return -1;
  }

  virtual void abort()
  {}

protected:

  // We use the hackery below to work around the fact that we need to emit
  // ready/finish signals after we've been constructed.

signals:

  void finishedSignal();

private slots:

  void finishedSlot();
};


class LocalSchemeHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    LocalSchemeHandler(ArticleNetworkAccessManager &articleNetMgr);
    void requestStarted(QWebEngineUrlRequestJob *requestJob);

protected:

private:
    ArticleNetworkAccessManager& mManager;
};
#endif
