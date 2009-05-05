/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mediawiki.hh"
#include "wstring_qt.hh"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QtXml>
#include <list>

namespace MediaWiki {

using namespace Dictionary;

namespace {

class MediaWikiDictionary: public Dictionary::Class
{
  string name;
  QString url;
  QNetworkAccessManager & netMgr;

public:

  MediaWikiDictionary( string const & id, string const & name_,
                       QString const & url_,
                       QNetworkAccessManager & netMgr_ ):
    Dictionary::Class( id, vector< string >() ),
    name( name_ ),
    url( url_ ),
    netMgr( netMgr_ )
  {
  }

  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual QIcon getIcon() throw()
  { return QIcon(":/icons/icon32_wiki.png"); }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const &,
                                                 unsigned long maxResults ) throw( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &, vector< wstring > const & alts )
    throw( std::exception );
};

class MediaWikiWordSearchRequest: public MediaWikiWordSearchRequestSlots
{
  sptr< QNetworkReply > netReply;
  bool livedLongEnough; // Indicates that the request has lived long enough
                        // to be destroyed prematurely. Used to prevent excessive
                        // network loads when typing search terms rapidly.
  bool isCancelling;

public:

  MediaWikiWordSearchRequest( wstring const &,
                              QString const & url, QNetworkAccessManager & mgr );

  ~MediaWikiWordSearchRequest();

  virtual void cancel();

protected:

  virtual void timerEvent( QTimerEvent * );

private:

  virtual void downloadFinished();
};

MediaWikiWordSearchRequest::MediaWikiWordSearchRequest( wstring const & str,
                                                        QString const & url,
                                                        QNetworkAccessManager & mgr ):
  livedLongEnough( false ), isCancelling( false )
{
  printf( "request begin\n" );
  QUrl reqUrl( url + "/api.php?action=query&list=allpages&aplimit=40&format=xml" );

  reqUrl.addQueryItem( "apfrom", gd::toQString( str ) );

  netReply = mgr.get( QNetworkRequest( reqUrl ) );

  connect( netReply.get(), SIGNAL( finished() ),
           this, SLOT( downloadFinished() ) );

  // We start a timer to postpone early destruction, so a rapid type won't make
  // unnecessary network load
  startTimer( 200 );
}

void MediaWikiWordSearchRequest::timerEvent( QTimerEvent * ev )
{
  killTimer( ev->timerId() );
  livedLongEnough = true;

  if ( isCancelling )
    finish();
}

MediaWikiWordSearchRequest::~MediaWikiWordSearchRequest()
{
  printf( "request end\n" );
}

void MediaWikiWordSearchRequest::cancel()
{
  // We either finish it in place, or in the timer handler
  isCancelling = true;

  if ( netReply.get() )
    netReply.reset();

  if ( livedLongEnough )
  {
    finish();
  }
  else
    printf("not long enough\n" );
}

void MediaWikiWordSearchRequest::downloadFinished()
{
  if ( isCancelling || isFinished() ) // Was cancelled
    return;

  if ( netReply->error() == QNetworkReply::NoError )
  {
    QDomDocument dd;

    QString errorStr;
    int errorLine, errorColumn;

    if ( !dd.setContent( netReply.get(), false, &errorStr, &errorLine, &errorColumn  ) )
    {
      setErrorString( QString( tr( "XML parse error: %1 at %2,%3" ).
                               arg( errorStr ).arg( errorLine ).arg( errorColumn ) ) );
    }
    else
    {
      QDomNode pages = dd.namedItem( "api" ).namedItem( "query" ).namedItem( "allpages" );

      if ( !pages.isNull() )
      {
        QDomNodeList nl = pages.toElement().elementsByTagName( "p" );

        Mutex::Lock _( dataMutex );

        for( unsigned x = 0; x < nl.length(); ++x )
          matches.push_back( gd::toWString( nl.item( x ).toElement().attribute( "title" ) ) );
      }
    }
    printf( "done.\n" );
  }
  else
    setErrorString( netReply->errorString() );

  finish();
}

class MediaWikiArticleRequest: public MediaWikiDataRequestSlots
{
  typedef std::list< std::pair< sptr< QNetworkReply >, bool > > NetReplies;
  NetReplies netReplies;
  QString url;

public:

  MediaWikiArticleRequest( wstring const & word, vector< wstring > const & alts,
                           QString const & url, QNetworkAccessManager & mgr );

  virtual void cancel();

private:

  void addQuery( QNetworkAccessManager & mgr, wstring const & word );

  virtual void requestFinished( QNetworkReply * );
};

void MediaWikiArticleRequest::cancel()
{
  finish();
}

MediaWikiArticleRequest::MediaWikiArticleRequest( wstring const & str,
                                                  vector< wstring > const & alts,
                                                  QString const & url_,
                                                  QNetworkAccessManager & mgr ):
  url( url_ )
{
  connect( &mgr, SIGNAL( finished( QNetworkReply * ) ),
           this, SLOT( requestFinished( QNetworkReply * ) ) );
  
  addQuery(  mgr, str );

  for( unsigned x = 0; x < alts.size(); ++x )
    addQuery( mgr, alts[ x ] );
}

void MediaWikiArticleRequest::addQuery( QNetworkAccessManager & mgr,
                                        wstring const & str )
{
  printf( "Requesting article %ls\n", str.c_str() );

  QUrl reqUrl( url + "/api.php?action=parse&prop=text|revid&format=xml&redirects" );

  reqUrl.addQueryItem( "page", gd::toQString( str ) );

  sptr< QNetworkReply > netReply = mgr.get( QNetworkRequest( reqUrl ) );
  
  netReplies.push_back( std::make_pair( netReply, false ) );
}

void MediaWikiArticleRequest::requestFinished( QNetworkReply * r )
{
  printf( "Finished.\n" );

  if ( isFinished() ) // Was cancelled
    return;

  // Find this reply

  bool found = false;
  
  for( NetReplies::iterator i = netReplies.begin(); i != netReplies.end(); ++i )
  {
    if ( i->first.get() == r )
    {
      i->second = true; // Mark as finished
      found = true;
      break;
    }
  }

  if ( !found )
  {
    // Well, that's not our reply, don't do anything
    return;
  }
  
  bool updated = false;

  for( ; netReplies.size() && netReplies.front().second; netReplies.pop_front() )
  {
    sptr< QNetworkReply > netReply = netReplies.front().first;
    
    if ( netReply->error() == QNetworkReply::NoError )
    {
      QDomDocument dd;
  
      QString errorStr;
      int errorLine, errorColumn;
  
      if ( !dd.setContent( netReply.get(), false, &errorStr, &errorLine, &errorColumn  ) )
      {
        setErrorString( QString( tr( "XML parse error: %1 at %2,%3" ).
                                 arg( errorStr ).arg( errorLine ).arg( errorColumn ) ) );
      }
      else
      {
        QDomNode parseNode = dd.namedItem( "api" ).namedItem( "parse" );
  
        if ( !parseNode.isNull() && parseNode.toElement().attribute( "revid" ) != "0" )
        {
          QDomNode textNode = parseNode.namedItem( "text" );
  
          if ( !textNode.isNull() )
          {
            QString articleString = textNode.toElement().text();
  
            QUrl wikiUrl( url );
            wikiUrl.setPath( "" );
  
            // Update any special index.php pages to be absolute
            articleString.replace( QRegExp( "<a\\shref=\"(/(\\w*/)*index.php\\?)" ),
                                   QString( "<a href=\"%1\\1" ).arg( wikiUrl.toString() ) );
            // Replace the href="/foo/bar/Baz" to just href="Baz".
            articleString.replace( QRegExp( "<a\\shref=\"/(\\w*/)*" ), "<a href=\"" );
  
            // In those strings, change any underscores to spaces
            for( ; ; )
            {
              QString before = articleString;
              articleString.replace( QRegExp( "<a href=\"([^/:\">#]*)_" ), "<a href=\"\\1 " );
  
              if ( articleString == before )
                break;
            }
  
            QByteArray articleBody = articleString.toUtf8();
  
            printf( "Article body after: %s\n", articleBody.data() );
  
            articleBody.prepend( "<div class=\"mwiki\">" );
            articleBody.append( "</div>" );
  
            Mutex::Lock _( dataMutex );

            size_t prevSize = data.size();
            
            data.resize( prevSize + articleBody.size() );
  
            memcpy( &data.front() + prevSize, articleBody.data(), articleBody.size() );
  
            hasAnyData = true;

            updated = true;
          }
        }
      }
      printf( "done.\n" );
    }
    else
      setErrorString( netReply->errorString() );
  }

  if ( netReplies.empty() )
    finish();
  else
  if ( updated )
    update();
}

sptr< WordSearchRequest > MediaWikiDictionary::prefixMatch( wstring const & word,
                                                            unsigned long maxResults )
  throw( std::exception )
{
  return new MediaWikiWordSearchRequest( word, url, netMgr );
}

sptr< DataRequest > MediaWikiDictionary::getArticle( wstring const & word, vector< wstring > const & alts )
  throw( std::exception )
{
  return new MediaWikiArticleRequest( word, alts, url, netMgr );
}

}

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Dictionary::Initializing &,
                                      Config::MediaWikis const & wikis,
                                      QNetworkAccessManager & mgr )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  for( unsigned x = 0; x < wikis.size(); ++x )
  {
    if ( wikis[ x ].enabled )
      result.push_back( new MediaWikiDictionary( wikis[ x ].id.toStdString(),
                                                 wikis[ x ].name.toUtf8().data(),
                                                 wikis[ x ].url,
                                                 mgr ) );
  }

  return result;
}

}
