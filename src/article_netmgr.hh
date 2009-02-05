/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
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

public:

  ArticleNetworkAccessManager( QObject * parent,
                               vector< sptr< Dictionary::Class > > const &
                               dictionaries_,
                               ArticleMaker const & articleMaker_ ):
    QNetworkAccessManager( parent ), dictionaries( dictionaries_ ),
    articleMaker( articleMaker_ )
  {}

  /// Tries reading a resource referenced by a "bres://" url. If it succeeds,
  /// the vector is filled with data, and true is returned. If it doesn't
  /// succeed, it returns false. The function can optionally set the Content-Type
  /// header correspondingly.
  bool getResource( QUrl const & url, vector< char > & data,
                    QString & contentType );

protected:

  virtual QNetworkReply * createRequest( Operation op,
                                         QNetworkRequest const & req,
                                         QIODevice * outgoingData );
};

class ArticleResourceReply: public QNetworkReply
{
  Q_OBJECT

  vector< char > data;

  size_t left;

public:

  ArticleResourceReply( QObject * parent,
                        QNetworkRequest const &,
                        vector< char > const & data,
                        QString const & contentType );

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

  void readyReadSlot();
  void finishedSlot();
};

#endif
