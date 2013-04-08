/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_NETMGR_HH_INCLUDED__
#define __ARTICLE_NETMGR_HH_INCLUDED__

#include <QtNetwork>
#include "dictionary.hh"
#include "article_maker.hh"

using std::vector;

/// A custom QNetworkAccessManager version which fetches images from the
/// dictionaries when requested.

class ArticleNetworkAccessManager: public QNetworkAccessManager
{
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

protected:

  virtual QNetworkReply * createRequest( Operation op,
                                         QNetworkRequest const & req,
                                         QIODevice * outgoingData );
};

class ArticleResourceReply: public QNetworkReply
{
  Q_OBJECT

  sptr< Dictionary::DataRequest > req;
  size_t alreadyRead;

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

#endif
