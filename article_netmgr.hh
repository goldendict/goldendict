/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLE_NETMGR_HH_INCLUDED__
#define __ARTICLE_NETMGR_HH_INCLUDED__

#include <QtNetwork>

#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
#include <QWebSecurityOrigin>
#include <QSet>
#include <QMap>
#include <QPair>
#endif

#include "dictionary.hh"
#include "article_maker.hh"

using std::vector;

/// A custom QNetworkAccessManager version which fetches images from the
/// dictionaries when requested.

#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)

// White lists for QWebSecurityOrigin
struct SecurityWhiteList
{
  QWebSecurityOrigin * origin;
  QString originUri;
  QSet< QPair< QString, QString > > hostsToAccess;

  SecurityWhiteList() :
    origin( 0 )
  {}

  ~SecurityWhiteList()
  {
    swlDelete();
  }
  SecurityWhiteList( SecurityWhiteList const & swl ) :
    origin( 0 )
  {
    swlCopy( swl );
  }

  SecurityWhiteList & operator=( SecurityWhiteList const & swl )
  {
    swlDelete();
    swlCopy( swl );
    return *this;
  }

  QWebSecurityOrigin * setOrigin( QUrl const & url )
  {
    swlDelete();
    originUri = url.toString( QUrl::FullyDecoded );
    origin = new QWebSecurityOrigin( url );
    return origin;
  }

private:

  void swlCopy( SecurityWhiteList const & swl )
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

  void swlDelete()
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
};

  typedef QMap< QString, SecurityWhiteList > Origins;

#endif

class ArticleNetworkAccessManager: public QNetworkAccessManager
{
  vector< sptr< Dictionary::Class > > const & dictionaries;
  ArticleMaker const & articleMaker;
  bool const & disallowContentFromOtherSites;
  bool const & hideGoldenDictHeader;
#if QT_VERSION >= QT_VERSION_CHECK(5,2,0)
  Origins allOrigins;
#endif
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

#endif
