/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "mediawiki.hh"
#include "wstring_qt.hh"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QtXml>
#include <algorithm>
#include <list>
#include <queue>
#include <memory>
#include "gddebug.hh"
#include "audiolink.hh"
#include "langcoder.hh"
#include "qt4x5.hh"

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#endif

namespace MediaWiki {

using namespace Dictionary;

namespace {

class MediaWikiFactory;

class MediaWikiDictionary: public Dictionary::Class
{
  string name;
  QString url, icon;
  QNetworkAccessManager & netMgr;
  quint32 langId;
#if __cplusplus > 199711L
  std::unique_ptr< MediaWikiFactory > factory;
#else
  std::auto_ptr< MediaWikiFactory > factory;
#endif

public:

  MediaWikiDictionary( string const & id, string const & name_,
                       QString const & url_,
                       QString const & icon_,
                       QNetworkAccessManager & netMgr_ ):
    Dictionary::Class( id, vector< string >() ),
    name( name_ ),
    url( url_ ),
    icon( icon_ ),
    netMgr( netMgr_ ),
    langId( 0 )
  {
    initializeFactory();

    int n = url.indexOf( "." );
    if( n == 2 || ( n > 3 && url[ n-3 ] == '/' ) )
      langId = LangCoder::code2toInt( url.mid( n - 2, 2 ).toLatin1().data() );
  }

  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const &,
                                                 unsigned long maxResults ) THROW_SPEC( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &, vector< wstring > const & alts,
                                          wstring const &, bool )
    THROW_SPEC( std::exception );

  virtual quint32 getLangFrom() const
  { return langId; }

  virtual quint32 getLangTo() const
  { return langId; }

protected:

  virtual void loadIcon() throw();

private:

  void initializeFactory();
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
  GD_DPRINTF( "request begin\n" );
  QUrl reqUrl( url + "/api.php?action=query&list=allpages&aplimit=40&format=xml" );

#if IS_QT_5
  Qt4x5::Url::addQueryItem( reqUrl, "apfrom", gd::toQString( str ).replace( '+', "%2B" ) );
#else
  reqUrl.addEncodedQueryItem( "apfrom", QUrl::toPercentEncoding( gd::toQString( str ) ) );
#endif

  netReply = mgr.get( QNetworkRequest( reqUrl ) );

  connect( netReply.get(), SIGNAL( finished() ),
           this, SLOT( downloadFinished() ) );

#ifndef QT_NO_OPENSSL

  connect( netReply.get(), SIGNAL( sslErrors( QList< QSslError > ) ),
           netReply.get(), SLOT( ignoreSslErrors() ) );

#endif

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
  GD_DPRINTF( "request end\n" );
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
  {
    GD_DPRINTF("not long enough\n" );
  }
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

        for( Qt4x5::Dom::size_type x = 0; x < nl.length(); ++x )
          matches.push_back( gd::toWString( nl.item( x ).toElement().attribute( "title" ) ) );
      }
    }
    GD_DPRINTF( "done.\n" );
  }
  else
    setErrorString( netReply->errorString() );

  finish();
}

class MediaWikiArticleRequest: public MediaWikiDataRequestSlots
{
public:

  /// None of the data members in this struct may be null.
  struct InitData
  {
    QString url;
    QNetworkAccessManager * netMgr;
    Class * dictPtr;
  };

  explicit MediaWikiArticleRequest( InitData const & data );

  void addQuery( wstring const & word ) { doAddQuery( word ); }

  virtual void cancel();

protected:

  QNetworkReply * createQuery( wstring const & word );

  virtual QNetworkReply const * doAddQuery( wstring const & word );

  virtual bool preprocessArticle( QString & articleString )
  {
    Q_UNUSED( articleString )
    return true;
  }

  typedef std::list< std::pair< QNetworkReply *, bool > > NetReplies;
  NetReplies netReplies;
  Class * const dictPtr;

private:

  virtual void requestFinished( QNetworkReply * );

  virtual void replyHandled( QNetworkReply const * reply, bool textFound )
  {
    Q_UNUSED( reply )
    Q_UNUSED( textFound )
  }

  virtual bool needNonessentialReplacements() const { return true; }

  void processArticle( QString & articleString ) const;
  void appendArticleToData( QString const & articleString );

  /// This simple set implementation should be much more efficient than tree-
  /// and hash-based standard/Qt containers when there are very few elements.
  template< typename T >
  class SmallSet {
  public:
    bool insert( T x )
    {
      if( std::find( elements.begin(), elements.end(), x ) != elements.end() )
        return false;
      elements.push_back( x );
      return true;
    }
  private:
    std::vector< T > elements;
  };

  /// The page id set allows to filter out duplicate articles in case MediaWiki
  /// redirects the main word and words in the alts collection to the same page.
  SmallSet< long long > addedPageIds;
  const QString url;
  QNetworkAccessManager & netMgr;
};

void MediaWikiArticleRequest::cancel()
{
  finish();
}

MediaWikiArticleRequest::MediaWikiArticleRequest( InitData const & data ):
  dictPtr( data.dictPtr ), url( data.url ), netMgr( *data.netMgr )
{
  connect( &netMgr, SIGNAL( finished( QNetworkReply * ) ),
           this, SLOT( requestFinished( QNetworkReply * ) ),
           Qt::QueuedConnection );
}

QNetworkReply * MediaWikiArticleRequest::createQuery( wstring const & str )
{
  Q_ASSERT( !isFinished() && "Finished request should not make queries!" );

  gdDebug( "MediaWiki: requesting article %s\n", gd::toQString( str ).toUtf8().data() );

  QUrl reqUrl( url + "/api.php?action=parse&prop=text|revid&format=xml&redirects" );

#if IS_QT_5
  Qt4x5::Url::addQueryItem( reqUrl, "page", gd::toQString( str ).replace( '+', "%2B" ) );
#else
  reqUrl.addEncodedQueryItem( "page", QUrl::toPercentEncoding( gd::toQString( str ) ) );
#endif

  QNetworkReply * netReply = netMgr.get( QNetworkRequest( reqUrl ) );
  
#ifndef QT_NO_OPENSSL

  connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
           netReply, SLOT( ignoreSslErrors() ) );

#endif

  return netReply;
}

QNetworkReply const * MediaWikiArticleRequest::doAddQuery( wstring const & word )
{
  QNetworkReply * const reply = createQuery( word );
  netReplies.push_back( std::make_pair( reply, false ) );
  return reply;
}

void MediaWikiArticleRequest::requestFinished( QNetworkReply * r )
{
  GD_DPRINTF( "Finished.\n" );

  if ( isFinished() ) // Was cancelled
    return;

  // Find this reply

  bool found = false;
  
  for( NetReplies::iterator i = netReplies.begin(); i != netReplies.end(); ++i )
  {
    if ( i->first == r )
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

  while( netReplies.size() && netReplies.front().second )
  {
    QNetworkReply * netReply = netReplies.front().first;
    netReplies.pop_front();

    bool textFound = false;
    
    if ( netReply->error() == QNetworkReply::NoError )
    {
      QDomDocument dd;
  
      QString errorStr;
      int errorLine, errorColumn;
  
      if ( !dd.setContent( netReply, false, &errorStr, &errorLine, &errorColumn  ) )
      {
        setErrorString( QString( tr( "XML parse error: %1 at %2,%3" ).
                                 arg( errorStr ).arg( errorLine ).arg( errorColumn ) ) );
      }
      else
      {
        QDomNode parseNode = dd.namedItem( "api" ).namedItem( "parse" );
  
        if ( !parseNode.isNull() && parseNode.toElement().attribute( "revid" ) != "0"
             // Don't show the same article more than once:
             && addedPageIds.insert( parseNode.toElement().attribute( "pageid" ).toLongLong() ) )
        {
          QDomNode textNode = parseNode.namedItem( "text" );
  
          if ( !textNode.isNull() )
          {
            QString articleString = textNode.toElement().text();
            textFound = true;
            if( preprocessArticle( articleString ) )
            {
              processArticle( articleString );
              appendArticleToData( articleString );
              updated = true;
            }
          }
        }
      }
      GD_DPRINTF( "done.\n" );
    }
    else
      setErrorString( netReply->errorString() );

    replyHandled( netReply, textFound );

    disconnect( netReply, 0, 0, 0 );
    netReply->deleteLater();
  }

  if ( netReplies.empty() )
    finish();
  else
  if ( updated )
    update();
}

void MediaWikiArticleRequest::processArticle( QString & articleString ) const
{
  // Replace all ":" in links, remove '#' part in links to other articles
  int pos = 0;
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  QRegularExpression regLinks( "<a\\s+href=\"/([^\"]+)\"" );
  QString articleNewString;
  QRegularExpressionMatchIterator it = regLinks.globalMatch( articleString );
  while( it.hasNext() )
  {
    QRegularExpressionMatch match = it.next();
    articleNewString += articleString.midRef( pos, match.capturedStart() - pos );
    pos = match.capturedEnd();

    QString link = match.captured( 1 );
#else
  QRegExp regLinks( "<a\\s+href=\"/([^\"]+)\"" );
  for( ; ; )
  {
    pos = regLinks.indexIn( articleString, pos );
    if( pos < 0 )
      break;
    QString link = regLinks.cap( 1 );
#endif
    if( link.indexOf( "://" ) >= 0 )
    {
      // External link
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
      articleNewString += match.captured();
#else
      pos += regLinks.cap().size();
#endif
      continue;
    }

    if( link.indexOf( ':' ) >= 0 )
      link.replace( ':', "%3A" );

    int n = link.indexOf( '#', 1 );
    if( n > 0 )
    {
      QString anchor = link.mid( n + 1 ).replace( '_', "%5F" );
      link.truncate( n );
      link += QString( "?gdanchor=%1" ).arg( anchor );
    }

    QString newLink = QString( "<a href=\"/%1\"" ).arg( link );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    articleNewString += newLink;
  }
  if( pos )
  {
    articleNewString += articleString.midRef( pos );
    articleString = articleNewString;
    articleNewString.clear();
  }
#else
    articleString.replace( pos, regLinks.cap().size(), newLink );
    pos += newLink.size();
  }
#endif

  QUrl wikiUrl( url );
  wikiUrl.setPath( "/" );
  const bool nonessentialReplacements = needNonessentialReplacements();

  if( nonessentialReplacements )
  {
    // Update any special index.php pages to be absolute
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    articleString.replace( QRegularExpression( "<a\\shref=\"(/([\\w]*/)*index.php\\?)" ),
                           QString( "<a href=\"%1\\1" ).arg( wikiUrl.toString() ) );
#else
    articleString.replace( QRegExp( "<a\\shref=\"(/(\\w*/)*index.php\\?)" ),
                           QString( "<a href=\"%1\\1" ).arg( wikiUrl.toString() ) );
#endif

    // audio tag
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QRegularExpression reg1( "<audio\\s.+?</audio>",
                             QRegularExpression::CaseInsensitiveOption
                             | QRegularExpression::DotMatchesEverythingOption );
    QRegularExpression reg2( "<source\\s+src=\"([^\"]+)",
                             QRegularExpression::CaseInsensitiveOption );
    pos = 0;
    it = reg1.globalMatch( articleString );
    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();
      articleNewString += articleString.midRef( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QString tag = match.captured();
      QRegularExpressionMatch match2 = reg2.match( tag );
      if( match2.hasMatch() )
      {
        QString ref = match2.captured( 1 );
        QString audio_url = "<a href=\"" + ref
                            + "\"><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a>";
        articleNewString += audio_url;
      }
      else
        articleNewString += match.captured();
    }
    if( pos )
    {
      articleNewString += articleString.midRef( pos );
      articleString = articleNewString;
      articleNewString.clear();
    }
#else
    QRegExp reg1( "<audio\\s.+</audio>", Qt::CaseInsensitive, QRegExp::RegExp2 );
    reg1.setMinimal( true );
    QRegExp reg2( "<source\\s+src=\"([^\"]+)", Qt::CaseInsensitive );
    pos = 0;
    for( ; ; )
    {
      pos = reg1.indexIn( articleString, pos );
      if( pos >= 0 )
      {
        QString tag = reg1.cap();
        if( reg2.indexIn( tag ) >= 0 )
        {
          QString ref = reg2.cap( 1 );
          QString audio_url = "<a href=\"" + ref
                              + "\"><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a>";
          articleString.replace( pos, tag.length(), audio_url );
        }
        pos += 1;
      }
      else
        break;
    }
#endif
    // audio url
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    articleString.replace( QRegularExpression( "<a\\s+href=\"(//upload\\.wikimedia\\.org/wikipedia/[^\"'&]*\\.og[ga](?:\\.mp3|))\"" ),
#else
    articleString.replace( QRegExp( "<a\\s+href=\"(//upload\\.wikimedia\\.org/wikipedia/[^\"'&]*\\.og[ga](?:\\.mp3|))\"" ),
#endif
                           QString::fromStdString( addAudioLink( string( "\"" ) + wikiUrl.scheme().toStdString() + ":\\1\"",
                                                   this->dictPtr->getId() ) + "<a href=\"" + wikiUrl.scheme().toStdString() + ":\\1\"" ) );

    // Add url scheme to image source urls
    articleString.replace( " src=\"//", " src=\"" + wikiUrl.scheme() + "://" );
    //fix src="/foo/bar/Baz.png"
    articleString.replace( "src=\"/", "src=\"" + wikiUrl.toString() );
  }

  // Remove the /wiki/ prefix from links
  articleString.replace( "<a href=\"/wiki/", "<a href=\"" );

  // In those strings, change any underscores to spaces
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  QRegularExpression rxLink( "<a\\s+href=\"[^/:\">#]+" );
  it = rxLink.globalMatch( articleString );
  while( it.hasNext() )
  {
    QRegularExpressionMatch match = it.next();
    for( int i = match.capturedStart() + 9; i < match.capturedEnd(); i++ )
      if( articleString.at( i ) == QChar( '_') )
        articleString[ i ] = ' ';
  }
#else
  for( ; ; )
  {
    QString before = articleString;
    articleString.replace( QRegExp( "<a href=\"([^/:\">#]*)_" ), "<a href=\"\\1 " );

    if ( articleString == before )
      break;
  }
#endif
  //fix file: url
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  articleString.replace( QRegularExpression( "<a\\s+href=\"([^:/\"]*file%3A[^/\"]+\")",
                                             QRegularExpression::CaseInsensitiveOption ),
#else
  articleString.replace( QRegExp("<a\\s+href=\"([^:/\"]*file%3A[^/\"]+\")", Qt::CaseInsensitive ),
#endif
                         QString( "<a href=\"%1/index.php?title=\\1" ).arg( url ));

  if( nonessentialReplacements )
  {
    // Add url scheme to other urls like  "//xxx"
    articleString.replace( " href=\"//", " href=\"" + wikiUrl.scheme() + "://" );

    // Fix urls in "srcset" attribute
    pos = 0;
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
    QRegularExpression regSrcset( " srcset\\s*=\\s*\"/[^\"]+\"" );
    it = regSrcset.globalMatch( articleString );
    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();
      articleNewString += articleString.midRef( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QString srcset = match.captured();
#else
    QRegExp regSrcset( " srcset\\s*=\\s*\"/([^\"]+)\"" );
    for( ; ; )
    {
      pos = regSrcset.indexIn( articleString, pos );
      if( pos < 0 )
        break;
      QString srcset = regSrcset.cap();
#endif
      QString newSrcset = srcset.replace( "//", wikiUrl.scheme() + "://" );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
      articleNewString += newSrcset;
    }
    if( pos )
    {
      articleNewString += articleString.midRef( pos );
      articleString = articleNewString;
      articleNewString.clear();
    }
#else
      articleString.replace( pos, regSrcset.cap().size(), newSrcset );
      pos += newSrcset.size();
    }
#endif
  }
}

void MediaWikiArticleRequest::appendArticleToData( QString const & articleString )
{
  QByteArray articleBody = articleString.toUtf8();
  articleBody.prepend( dictPtr->isToLanguageRTL() ? "<div class=\"mwiki\" dir=\"rtl\">" :
                                                    "<div class=\"mwiki\">" );
  articleBody.append( "</div>" );

  Mutex::Lock _( dataMutex );
  size_t prevSize = data.size();
  data.resize( prevSize + articleBody.size() );
  memcpy( &data.front() + prevSize, articleBody.data(), articleBody.size() );
  hasAnyData = true;
}

class MediaWikiFactory
{
  Q_DISABLE_COPY( MediaWikiFactory )
public:

  MediaWikiFactory() {}
  virtual ~MediaWikiFactory() {}

  virtual QIcon defaultIcon() const { return QIcon( ":/icons/icon32_wiki.png" ); }

  typedef MediaWikiArticleRequest::InitData InitData;

  virtual sptr< MediaWikiArticleRequest > articleRequest( InitData const & data ) const
  {
    return new MediaWikiArticleRequest( data );
  }
};

class FandomArticleRequest: public MediaWikiArticleRequest
{
public:

  explicit FandomArticleRequest( InitData const & data ):
    MediaWikiArticleRequest( data ) {}

protected:

  virtual bool preprocessArticle( QString & articleString );

  // These replacements are never applicable to Fandom articles in practice
  // -> disable them to improve performance.
  virtual bool needNonessentialReplacements() const { return false; }
};

bool FandomArticleRequest::preprocessArticle( QString & articleString )
{
  QRegExp minimalRegExp;
  minimalRegExp.setMinimal( true );

  // Lazy loading does not work in goldendict -> display these images
  // by switching to the simpler alternative format under <noscript> tag.
  const QString lazyImgTag = "<img [^>]*class=\"thumbimage lazyload\"[^>]*>";

  // This "/wiki/File:..." link leads to the entire article, not the image that
  // the user clicks on -> replace it with the img src link edited by removing the
  // "scale-to-width-down" argument so as to show the full-size image in a browser.
  // This regular expression takes care to preserve the "path-prefix=de" part of
  // the link without which some links break.
  // An example from the "Star Wars: Episode I The Phantom Menace" article in English Wookieepedia:
  // <a class="image lightbox" href="/wiki/File:Farewell_to_Jira.JPG" title="Farewell to Jira.JPG (23 KB)" style="height:50px; width:120px;"><noscript><img style="" src="https://static.wikia.nocookie.net/starwars/images/e/e3/Farewell_to_Jira.JPG/revision/latest/scale-to-width-down/120?cb=20060605192302" title="Farewell to Jira.JPG (23 KB)" class="thumbimage" alt="Farewell to Jira" data-image-name="Farewell to Jira.JPG" data-image-key="Farewell_to_Jira.JPG" data-caption="Farewell to Jira"></noscript><img style="" src="data:image/gif;base64,R0lGODlhAQABAIABAAAAAP///yH5BAEAAAEALAAAAAABAAEAQAICTAEAOw%3D%3D" title="Farewell to Jira.JPG (23 KB)" class="thumbimage lazyload" alt="Farewell to Jira" data-image-name="Farewell to Jira.JPG" data-image-key="Farewell_to_Jira.JPG" data-caption="Farewell to Jira" data-src="https://static.wikia.nocookie.net/starwars/images/e/e3/Farewell_to_Jira.JPG/revision/latest/scale-to-width-down/120?cb=20060605192302" /></a>
  // A different example from the "Episode IX – Der Aufstieg Skywalkers" article in https://jedipedia.fandom.com (German):
  // <a class="image lightbox" href="/wiki/Datei:The_Rise_of_Skywalker_Poster_D23_Expo.jpg" title="The Rise of Skywalker Poster D23 Expo.jpg (130 KB)" style="height:184px; width:124px;"><noscript><img style="" src="https://static.wikia.nocookie.net/jedipedia/images/6/6d/The_Rise_of_Skywalker_Poster_D23_Expo.jpg/revision/latest/scale-to-width-down/125?cb=20190824210240&amp;path-prefix=de" title="The Rise of Skywalker Poster D23 Expo.jpg (130 KB)" class="thumbimage" alt="The Rise of Skywalker Poster D23 Expo" data-image-name="The Rise of Skywalker Poster D23 Expo.jpg" data-image-key="The_Rise_of_Skywalker_Poster_D23_Expo.jpg"></noscript><img style="" src="data:image/gif;base64,R0lGODlhAQABAIABAAAAAP///yH5BAEAAAEALAAAAAABAAEAQAICTAEAOw%3D%3D" title="The Rise of Skywalker Poster D23 Expo.jpg (130 KB)" class="thumbimage lazyload" alt="The Rise of Skywalker Poster D23 Expo" data-image-name="The Rise of Skywalker Poster D23 Expo.jpg" data-image-key="The_Rise_of_Skywalker_Poster_D23_Expo.jpg" data-src="https://static.wikia.nocookie.net/jedipedia/images/6/6d/The_Rise_of_Skywalker_Poster_D23_Expo.jpg/revision/latest/scale-to-width-down/125?cb=20190824210240&amp;path-prefix=de" /></a>
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.replace( QRegExp( "<a class=\"image lightbox\" href=\"/wiki/[^\"]+\"([^>]*>)"
                                  "\\s*<noscript>\\s*(<img [^>]*src=)(\"[^\"]+/revision/latest)"
                                  "(/scale-to-width-down/\\d+)?([^\"]*\")([^>]+>)\\s*</noscript>\\s*"
                                  + lazyImgTag ),
                         "<a href=\\3\\5\\1\\2\\3\\4\\5\\6" );

  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <a href="https://static.wikia.nocookie.net/starwars/images/e/e2/SwKOTOR25cropped.jpg/revision/latest?cb=20190413205440" class="image"><img alt="" src="data:image/gif;base64,R0lGODlhAQABAIABAAAAAP///yH5BAEAAAEALAAAAAABAAEAQAICTAEAOw%3D%3D" decoding="async" width="150" height="158" class="thumbimage lazyload" data-image-name="SwKOTOR25cropped.jpg" data-image-key="SwKOTOR25cropped.jpg" data-src="https://static.wikia.nocookie.net/starwars/images/e/e2/SwKOTOR25cropped.jpg/revision/latest/scale-to-width-down/150?cb=20190413205440" /></a> 	<noscript><a href="https://static.wikia.nocookie.net/starwars/images/e/e2/SwKOTOR25cropped.jpg/revision/latest?cb=20190413205440" class="image"><img alt="" src="https://static.wikia.nocookie.net/starwars/images/e/e2/SwKOTOR25cropped.jpg/revision/latest/scale-to-width-down/150?cb=20190413205440" decoding="async" width="150" height="158" class="thumbimage" data-image-name="SwKOTOR25cropped.jpg" data-image-key="SwKOTOR25cropped.jpg" data-src="https://static.wikia.nocookie.net/starwars/images/e/e2/SwKOTOR25cropped.jpg/revision/latest/scale-to-width-down/150?cb=20190413205440" /></a></noscript>
  QString lazyLinkNoscript = "<a [^>]*class=\"image\">\\s*" + lazyImgTag
                              + "\\s*</a>\\s*<noscript>(.*?)</noscript>";
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  articleString.replace( QRegularExpression( lazyLinkNoscript ), "\\1" );
#else
  minimalRegExp.setPattern( lazyLinkNoscript.remove( '?' ) );
  articleString.replace( minimalRegExp, "\\1" );
#endif

  // data-src -> src for images of class="lazyload" to make era icons and other small icons visible.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <img alt="SWAJsmall.jpg" src="data:image/gif;base64,R0lGODlhAQABAIABAAAAAP///yH5BAEAAAEALAAAAAABAAEAQAICTAEAOw%3D%3D" decoding="async" width="47" height="15" data-image-name="SWAJsmall.jpg" data-image-key="SWAJsmall.jpg" data-src="https://static.wikia.nocookie.net/starwars/images/e/ee/SWAJsmall.jpg/revision/latest/scale-to-width-down/47?cb=20070219044103" class="lazyload" />
  const QString dataSrcLazyloadTag = " src=\"data:[^\"]*\"([^>]* )data-(src=\"[^\"]+\") class=\"lazyload\"";
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  articleString.replace( QRegularExpression( dataSrcLazyloadTag ),
#else
  articleString.replace( QRegExp( dataSrcLazyloadTag ),
#endif
                         "\\1\\2" );

  // This "info-icon" link shows up in GoldenDict as a large empty space under
  // most images -> remove it for compactness.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <a href="/wiki/File:AnakinShmi.jpg" class="info-icon"><svg><use xlink:href="#wds-icons-info-small"></use></svg></a>
  QString infoIconTag = "<a [^>]*class=\"info-icon\">.*?</a>";
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  articleString.remove( QRegularExpression( infoIconTag ) );
#else
  minimalRegExp.setPattern( infoIconTag.remove( '?' ) );
  articleString.remove( minimalRegExp );
#endif

  // The "wds-icon..." image inside this "wds-button..." link shows up in GoldenDict as a large
  // empty space under a sequence of images -> remove the entire useless button as clicking on
  // the link has no effect, and editing articles in GoldenDict does not make sense anyway.
  // An example from the "Star Wars: Episode I The Phantom Menace" article in English Wookieepedia:
  // <a class="wds-button wikia-photogallery-add"><svg class="wds-icon wds-icon-tiny"><use xlink:href="#wds-icons-image-small"></use></svg><span>Add a photo to this gallery</span></a>
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  minimalRegExp.setPattern( "<a class=\"wds-button wikia-photogallery-add\">.*</a>" );
  articleString.remove( minimalRegExp );

  // The "wds-icon..." image inside this "mw-editsection" span shows up in GoldenDict
  // as a large empty space above most captions -> remove the entire useless span as
  // the final GoldenDict's versions of the links uselessly point to the current
  // article, and editing articles in GoldenDict does not make sense anyway.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <span class="mw-editsection"><span class="mw-editsection-bracket">[</span><a href="/wiki/Anakin_Skywalker/Legends?veaction=edit&amp;section=3" class="mw-editsection-visualeditor" title="Edit section: Early life">edit</a><span class="mw-editsection-divider"> | </span><a href="/wiki/Anakin_Skywalker/Legends?action=edit&amp;section=3" title="Edit section: Early life"><svg class="wds-icon wds-icon-tiny section-edit-pencil-icon"><use xlink:href="#wds-icons-pencil-tiny"></use></svg>edit source</a><span class="mw-editsection-bracket">]</span></span>
  // Have to remove the entire span in two steps - first the inner spans, then the
  // outer one, because the nested span tags are closed in the same way: "</span>".
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.remove( QRegExp( "<span class=\"mw-editsection-[^<]*</span>" ) );
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  minimalRegExp.setPattern( "<span class=\"mw-editsection\".*</span>" );
  articleString.remove( minimalRegExp );

  // Detect most audio links. This replacement chops the "/revision..." ending of the
  // audio link beyond the audio file extension (usually or even always ".ogg") before
  // passing the link to addAudioLink(). Otherwise, triggering the Pronounce Word
  // action loads the audio file in a browser instead of playing it in GoldenDict.
  // The "/wiki/Media:..." button links are broken and the "...Quote-audio.png..."
  // button link leads to an uninteresting audio button image -> replace them with the
  // link that is passed to addAudioLink(). Leave the "Listen" (and "Hear Han Solo")
  // links intact to give the user an option of loading the audio file in a browser.
  // NOTE: the examples for this replacement have been modified by the earlier
  // {data-src -> src for images of class="lazyload"} replacement.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <a href="/wiki/Media:TheChoiceIsYours-TPM.ogg" title="(audio)"><img alt="(audio)" decoding="async" width="20" height="20" data-image-name="Quote-audio.png" data-image-key="Quote-audio.png" src="https://static.wikia.nocookie.net/starwars/images/e/ee/Quote-audio.png/revision/latest/scale-to-width-down/20?cb=20200426203158" /></a> <a href="https://static.wikia.nocookie.net/starwars/images/4/47/TheChoiceIsYours-TPM.ogg/revision/latest?cb=20090917130644" class="internal" title="TheChoiceIsYours-TPM.ogg">Listen</a>
  // A different example from the "Obi-Wan Kenobi/Legends" article in English Wookieepedia:
  // <a href="https://static.wikia.nocookie.net/starwars/images/e/ee/Quote-audio.png/revision/latest?cb=20200426203158" class="image" title="(audio)"><img alt="(audio)" decoding="async" width="20" height="20" data-image-name="Quote-audio.png" data-image-key="Quote-audio.png" src="https://static.wikia.nocookie.net/starwars/images/e/ee/Quote-audio.png/revision/latest/scale-to-width-down/20?cb=20200426203158" /></a> <a href="https://static.wikia.nocookie.net/starwars/images/1/1f/BadFeelingAboutThis-TPM.ogg/revision/latest?cb=20090918153842" class="internal" title="BadFeelingAboutThis-TPM.ogg">Listen</a>
  // A very rare (unique?) two-line example from the "Han Solo/Legends" article in English Wookieepedia:
  /* <a href="https://static.wikia.nocookie.net/starwars/images/e/ee/Quote-audio.png/revision/latest?cb=20200426203158" class="image" title="(audio)"><img alt="(audio)" decoding="async" width="30" height="30" data-image-name="Quote-audio.png" data-image-key="Quote-audio.png" src="https://static.wikia.nocookie.net/starwars/images/e/ee/Quote-audio.png/revision/latest/scale-to-width-down/30?cb=20200426203158" /></a>
<a href="https://static.wikia.nocookie.net/starwars/images/d/d8/Han.ogg/revision/latest?cb=20051024183653" class="internal" title="Han.ogg">Hear Han Solo</a> */
  const QString audioUrlTag = "<a href=\"(?:/wiki/Media:|https://static\\.wikia\\.nocookie\\.net"
                              "/starwars/images/e/ee/Quote-audio\\.png)[^\"]*(\"[^>]*>\\s*<img [^>]*>"
                              "\\s*</a>\\s*<a href=)(\"[^\"]+)(/revision/latest\\?cb=\\d+\")";
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  articleString.replace( QRegularExpression( audioUrlTag ),
#else
  articleString.replace( QRegExp( audioUrlTag ),
#endif
                         QString::fromStdString( addAudioLink( "\\2\"", this->dictPtr->getId() )
                                                 + "<a href=\\2\\1\\2\\3" ) );

  // Detect this rare audio link variant. Make the end result look very similar to
  // and work exactly the same as the most common audio link variant (see above).
  // The only visual difference is between the remote and local audio button image.
  // The playsound.png HTML code is a slightly modified version of the code used
  // by the "audio tag" replacement in MediaWikiArticleRequest::processArticle().
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // <span class="unicode audiolink"><a href="https://static.wikia.nocookie.net/starwars/images/8/84/AniObiDookuBanter-TGG.ogg/revision/latest?cb=20090725214441" class="internal" title="AniObiDookuBanter-TGG.ogg">Listen</a></span>
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.replace( QRegExp( "<span class=\"unicode audiolink\">\\s*<a href=(\"[^\"]+)"
                                  "(/revision/latest\\?cb=\\d+\"[^<]+</a>)\\s*</span>" ),
                         QString::fromStdString(
                           addAudioLink( "\\1\"", this->dictPtr->getId() )
                           + "<a href=\\1\"><img src=\"qrcx://localhost/icons/playsound.png\""
                             " border=\"0\" alt=\"Play\"></a> <a href=\\1\\2" ) );

  // This "/wiki/File:..." "(file info)" link occurs right after almost every audio link of
  // the most common type. This link leads to the entire article or to a different article,
  // but not to the information about the audio file -> remove the misleading link.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  //  <small>(<a href="/wiki/File:TheChoiceIsYours-TPM.ogg" title="File:TheChoiceIsYours-TPM.ogg">file info</a>)</small>
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.remove( QRegExp( " <small>\\(<a href=\"/wiki/File:[^>]+>file info</a>\\)</small>" ) );

  // This "/wiki/File:..." "info" link along with the preceding "help" link occur right
  // after every audio link of the rare "unicode audiolink" type. The "info" link leads
  // to the entire article, not to the information about the audio file -> remove the
  // misleading link. The "help" link is not particularly useful either, but at least
  // it leads to where it is supposed to -> leave it in.
  // An example from the "Anakin Skywalker/Legends" article in English Wookieepedia:
  // &#160;<span class="metadata audiolinkinfo"><small>(<a href="http://en.wikipedia.org/wiki/Wikipedia:Media_help" class="extiw" title="wikipedia:Wikipedia:Media help">help</a>·<a href="/wiki/File:AniObiDookuBanter-TGG.ogg" title="File:AniObiDookuBanter-TGG.ogg">info</a>)</small></span>
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.remove( QRegExp( "\u00b7<a href=\"/wiki/File:[^>]+>info</a>" ) );

  // Remove absolute height from scrollbox lines to ensure that everything inside
  // the scrollable container is visible and does not overlap the contents below.
  // The following line occurs multiple times in many (all?) Canon and Legends articles in English Wookieepedia:
  // <table class="scrollbox" cellpadding="0" cellspacing="0" border="0" style="width:100%;"><tbody><tr><td><div style="height:400px; width:100%;">
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  articleString.replace( QRegExp( "(<table class=\"scrollbox\"[^\\n]*[^-])height:\\d+px; ?" ),
                         "\\1" );

  return true;
}

class FandomFactory: public MediaWikiFactory
{
public:

  virtual QIcon defaultIcon() const { return QIcon( ":/icons/icon32_fandom.png" ); }

  virtual sptr< MediaWikiArticleRequest > articleRequest( InitData const & data ) const
  {
    return new FandomArticleRequest( data );
  }
};

/// @return The wiki word inside the link that contains @p linkDistinction
/// or an empty string if @p article does not contain such a link.
wstring findWikiLink( QString const & article, QString const & linkDistinction )
{
  // Searching for a simple string is much faster than for a regular expression.
  // Since the first search just below probably fails in most cases,
  // it should be optimized instead of trying to squeeze the entire regexp into it.
  const int distinctionPosition = article.indexOf( linkDistinction );
  if( distinctionPosition < 0 )
    return wstring();
  const int linkPosition = article.lastIndexOf( QRegExp( "[<>]" ), distinctionPosition );
  if( linkPosition < 0 )
    return wstring();

  const QString linkForepart = article.mid( linkPosition,
                                            distinctionPosition - linkPosition );
  const QRegExp linkPattern( "<a href=\"/wiki/([^\"]+)\".*" );
  if( linkPattern.exactMatch( linkForepart ) )
    return gd::toWString( linkPattern.cap( 1 ) );
  return wstring();
}

/// This class searches for redirectLinkDistinction in one of the links
/// inside the article text. If found, displays the definition of the word that
/// is this link's target instead of the original article text.
class RedirectingArticleRequest: public FandomArticleRequest
{
public:

  explicit RedirectingArticleRequest( InitData const & data,
                                      QString const & redirectLinkDistinction_ ):
    FandomArticleRequest( data ), redirectLinkDistinction( redirectLinkDistinction_ )
  {}

protected:

  virtual bool preprocessArticle( QString & articleString );

  void prependQuery( wstring const & str );

private:

  const QString redirectLinkDistinction;
};

bool RedirectingArticleRequest::preprocessArticle( QString & articleString )
{
  if( !redirectLinkDistinction.isEmpty() )
  {
    const wstring wikiWord = findWikiLink( articleString, redirectLinkDistinction );
    if( !wikiWord.empty() )
    {
      // Found our link distintion -> redirect.
      prependQuery( wikiWord );
      return false;
    }
  }

  return FandomArticleRequest::preprocessArticle( articleString );
}

void RedirectingArticleRequest::prependQuery( wstring const & str )
{
  netReplies.push_front( std::make_pair( createQuery( str ), false ) );
}

/// If the selected word does not end with preferableSuffix, this class requests
/// the definition of the word with preferableSuffix appended. If this modified
/// request fails, the definition of the original word is requested.
class SuffixAddingArticleRequest: public RedirectingArticleRequest
{
public:

  explicit SuffixAddingArticleRequest( InitData const & data,
                                       QString const & redirectLinkDistinction_,
                                       wstring const & preferableSuffix_ ):
    RedirectingArticleRequest( data, redirectLinkDistinction_ ),
    preferableSuffix( preferableSuffix_ )
  {}

protected:

  virtual QNetworkReply const * doAddQuery( wstring const & word );

private:

  virtual void replyHandled( QNetworkReply const * reply, bool textFound );

  bool endsWithPreferableSuffix( wstring const & word ) const;

  const wstring preferableSuffix;

  struct Replacement
  {
    QNetworkReply const * reply;
    wstring originalWord;
  };
  /// The relative QNetworkReply order in replacements is the same as in netReplies.
  std::queue< Replacement > replacements;
};

QNetworkReply const * SuffixAddingArticleRequest::doAddQuery( wstring const & word )
{
  if( endsWithPreferableSuffix( word ) )
    return RedirectingArticleRequest::doAddQuery( word );
  // Try the corresponding preferable article first.
  QNetworkReply const * const reply = RedirectingArticleRequest::doAddQuery( word + preferableSuffix );
  Replacement replacement = { reply, word };
  replacements.push( replacement );
  return reply;
}

void SuffixAddingArticleRequest::replyHandled( QNetworkReply const * reply, bool textFound )
{
  if( replacements.empty() || reply != replacements.front().reply )
    return;
  if( !textFound )
  {
    // Couldn't load the preferable article -> try the original word instead.
    prependQuery( replacements.front().originalWord );
  }
  replacements.pop();
}

bool SuffixAddingArticleRequest::endsWithPreferableSuffix( wstring const & word ) const
{
  if( word.size() < preferableSuffix.size() )
    return false;
  return std::equal( word.end() - static_cast< wstring::difference_type >( preferableSuffix.size() ),
                     word.end(), preferableSuffix.begin() );
}

/// Ensures that Wookieepedia era icons are visible at the top of the article.
/// The most important "era icon" is the Canon or Legends indicator.
/// It is not immediately obvious whether the current article is
/// the Canon or the Legends version of the subject without this indicator.
void makeEraIconsVisible( QString & article )
{
  // The following line occurs once in many (all?) Canon and Legends articles in English Wookieepedia:
  // <div id="title-eraicons" style="float:right;position:static;display:none">
  // For some reason QRegExp works faster than QRegularExpression in the replacement below on Linux.
  article.replace( QRegExp( "(id=\"title-eraicons\" style=\"[^\"]*)display:none;?" ),
                            "\\1" );
}

/// This class displays the default Wookieepedia tab for the requested word.
class WookieepediaArticleRequest: public FandomArticleRequest
{
public:

  explicit WookieepediaArticleRequest( InitData const & data ):
    FandomArticleRequest( data ) {}

protected:

  virtual bool preprocessArticle( QString & articleString );
};

bool WookieepediaArticleRequest::preprocessArticle( QString & articleString )
{
  if( !FandomArticleRequest::preprocessArticle( articleString ) )
    return false;
  makeEraIconsVisible( articleString );
  return true;
}

class WookieepediaFactory: public FandomFactory
{
public:

  virtual QIcon defaultIcon() const { return QIcon( ":/icons/icon32_wookieepedia.png" ); }

  virtual sptr< MediaWikiArticleRequest > articleRequest( InitData const & data ) const
  {
    return new WookieepediaArticleRequest( data );
  }
};

/// This class displays the Wookieepedia Legends definition of the requested
/// word if it exists. If there is no Legends version of the article,
/// the Wookieepedia Canon definition is displayed.
class WookieepediaLegendsArticleRequest: public SuffixAddingArticleRequest
{
public:

  explicit WookieepediaLegendsArticleRequest( InitData const & data ):
    SuffixAddingArticleRequest(
      data,

      // Detect inactive Legends tab. If found, discard the current article
      // and ask for its Legends version instead.
      // An example from the "Darth Sidious" Canon article in English Wookieepedia:
      // <a href="/wiki/Palpatine/Legends" title="Click here for Wookieepedia&#39;s article on the Legends version of this subject."><img alt="Click here for Wookieepedia&#39;s article on the Legends version of this subject." src="data:image/gif;base64,R0lGODlhAQABAIABAAAAAP///yH5BAEAAAEALAAAAAABAAEAQAICTAEAOw%3D%3D" decoding="async" width="170" height="30" data-image-name="Tab-legends-black.png" data-image-key="Tab-legends-black.png" data-src="https://static.wikia.nocookie.net/starwars/images/2/28/Tab-legends-black.png/revision/latest/scale-to-width-down/170?cb=20140430180745" class="lazyload" /></a>
      "title=\"Click here for Wookieepedia&#39;s article on the Legends version of this subject.\"",

      // Before searching for the original word, send a request for the word
      // with the /Legends suffix. In case of success, this saves waiting for,
      // then parsing the Canon reply (which may contain a long article),
      // and detecting the inactive Legends tab.
      // In case of failure, the penalty is smaller: one extra network request
      // and relatively quick parsing of the missing /Legends page reply.
      L"/Legends" )
  {}

protected:

  virtual bool preprocessArticle( QString & articleString );
};

bool WookieepediaLegendsArticleRequest::preprocessArticle( QString & articleString )
{
  if( !SuffixAddingArticleRequest::preprocessArticle( articleString ) )
    return false;
  makeEraIconsVisible( articleString );
  return true;
}

class WookieepediaLegendsFactory: public WookieepediaFactory
{
public:

  virtual sptr< MediaWikiArticleRequest > articleRequest( InitData const & data ) const
  {
    return new WookieepediaLegendsArticleRequest( data );
  }
};

sptr< WordSearchRequest > MediaWikiDictionary::prefixMatch( wstring const & word,
                                                            unsigned long maxResults )
  THROW_SPEC( std::exception )
{
  (void) maxResults;
  if ( word.size() > 80 )
  {
    // Don't make excessively large queries -- they're fruitless anyway

    return new WordSearchRequestInstant();
  }
  else
    return new MediaWikiWordSearchRequest( word, url, netMgr );
}

sptr< DataRequest > MediaWikiDictionary::getArticle( wstring const & word,
                                                     vector< wstring > const & alts,
                                                     wstring const &, bool )
  THROW_SPEC( std::exception )
{
  if ( word.size() > 80 )
  {
    // Don't make excessively large queries -- they're fruitless anyway

    return new DataRequestInstant( false );
  }
  else
  {
    const MediaWikiArticleRequest::InitData initData = { url, &netMgr, this };
    sptr< MediaWikiArticleRequest > request = factory->articleRequest( initData );

    request->addQuery( word );
    for( std::size_t i = 0; i < alts.size(); ++i )
      request->addQuery( alts[ i ] );

    return request;
  }
}

void MediaWikiDictionary::loadIcon() throw()
{
  if( dictionaryIconLoaded )
    return;

  if( !icon.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), icon );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = factory->defaultIcon();
  dictionaryIconLoaded = true;
}

void MediaWikiDictionary::initializeFactory()
{
  if( url.endsWith( "/starwars.fandom.com (Legends)" ) )
  {
    const int legendsSuffixLength = 10;
    url.chop( legendsSuffixLength );
    factory.reset( new WookieepediaLegendsFactory );
  }
  else if( url.endsWith( "/starwars.fandom.com" ) )
    factory.reset( new WookieepediaFactory );
  else if( url.endsWith( ".fandom.com" ) )
    factory.reset( new FandomFactory );
  else
    factory.reset( new MediaWikiFactory );
}

}

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Dictionary::Initializing &,
                                      Config::MediaWikis const & wikis,
                                      QNetworkAccessManager & mgr )
  THROW_SPEC( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  for( int x = 0; x < wikis.size(); ++x )
  {
    if ( wikis[ x ].enabled )
      result.push_back( new MediaWikiDictionary( wikis[ x ].id.toStdString(),
                                                 wikis[ x ].name.toUtf8().data(),
                                                 wikis[ x ].url,
                                                 wikis[ x ].icon,
                                                 mgr ) );
  }

  return result;
}

}
