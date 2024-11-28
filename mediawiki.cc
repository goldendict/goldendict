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

class MediaWikiDictionary: public Dictionary::Class
{
  string name;
  QString url, icon;
  QNetworkAccessManager & netMgr;
  quint32 langId;

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

};

void MediaWikiDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  if( !icon.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), icon );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/icon32_wiki.png");
  dictionaryIconLoaded = true;
}

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

class MediaWikiSectionsParser
{
public:
  /// Since a recent Wikipedia UI redesign, the table of contents (ToC) is no longer part of an article's HTML.
  /// ToC is absent from the text node of Wikipedia's MediaWiki API reply. Quote from
  /// https://www.mediawiki.org/wiki/Reading/Web/Desktop_Improvements/Features/Table_of_contents#How_can_I_get_the_old_table_of_contents?
  /// We intentionally do not add the old table of contents to the article in addition to the new sidebar location...
  /// Users can restore the old table of contents position with the following JavaScript code:
  /// document.querySelector('mw\\3Atocplace,meta[property="mw:PageProp/toc"]').replaceWith( document.getElementById('mw-panel-toc') )
  ///
  /// This function searches for an indicator of the empty ToC in an article HTML. If the indicator is present,
  /// generates ToC HTML from the sections element and replaces the indicator with the generated ToC.
  static void generateTableOfContentsIfEmpty( QDomNode const & parseNode, QString & articleString )
  {
    QString const emptyTocIndicator = "<meta property=\"mw:PageProp/toc\" />";
    int const emptyTocPos = articleString.indexOf( emptyTocIndicator );
    if( emptyTocPos == -1 )
      return; // The ToC must be absent or nonempty => nothing to do.

    QDomElement const sectionsElement = parseNode.firstChildElement( "sections" );
    if( sectionsElement.isNull() )
    {
      gdWarning( "MediaWiki: empty table of contents and missing sections element." );
      return;
    }

    gdDebug( "MediaWiki: generating table of contents from the sections element." );
    MediaWikiSectionsParser parser;
    parser.generateTableOfContents( sectionsElement );
    articleString.replace( emptyTocPos, emptyTocIndicator.size(), parser.tableOfContents );
  }

private:
  MediaWikiSectionsParser() : previousLevel( 0 ) {}
  void generateTableOfContents( QDomElement const & sectionsElement );

  bool addListLevel( QString const & levelString );
  void closeListTags( int currentLevel );

  QString tableOfContents;
  int previousLevel;
};

void MediaWikiSectionsParser::generateTableOfContents( QDomElement const & sectionsElement )
{
  // A real example of a typical child of the <sections> element:
  // <s linkAnchor="Marginal_densities" toclevel="2" fromtitle="Probability_density_function" level="3"
  //  line="Marginal densities" byteoffset="15868" anchor="Marginal_densities" number="7.1" index="9"/>

  // Use Wiktionary's ToC style, which had also been Wikipedia's ToC style until the UI redesign.
  // Replace double quotes with single quotes to avoid escaping " within string literals.

  QString const elTagName = "s";
  QDomElement el = sectionsElement.firstChildElement( elTagName );
  if( el.isNull() )
    return;

  // Omit invisible and useless toctogglecheckbox, toctogglespan and toctogglelabel elements.
  // The values of lang (e.g. 'en') and dir (e.g. 'ltr') attributes of the toctitle element depend on
  // the article's language. These attributes have no visible effect and so are simply omitted here.
  // TODO: the "Contents" string should be translated to the article's language, but I don't know how
  // to implement this. Should "Contents" be enclosed in tr() to at least translate it to GoldenDict's
  // interface language? Is there a language-agnostic Unicode symbol that stands for "Contents"?
  tableOfContents = "<div id='toc' class='toc' role='navigation' aria-labelledby='mw-toc-heading'>"
                    "<div class='toctitle'><h2 id='mw-toc-heading'>Contents</h2></div>";

  do
  {
    if( !addListLevel( el.attribute( "toclevel" ) ) )
    {
      tableOfContents.clear();
      return;
    }

    // From https://gerrit.wikimedia.org/r/c/mediawiki/core/+/831147/
    // The anchor property ... should be used if you want to (eg) look up an element by ID using
    // document.getElementById(). The linkAnchor property ... contains additional escaping appropriate for
    // use in a URL fragment, and should be used (eg) if you are creating the href attribute of an <a> tag.
    tableOfContents += "<a href='#";
    tableOfContents += el.attribute( "linkAnchor" );
    tableOfContents += "'>";

    // Omit <span class="tocnumber"> because it has no visible effect.
    tableOfContents += el.attribute( "number" );
    tableOfContents += ' ';
    // Omit <span class="toctext"> because it has no visible effect.
    tableOfContents += el.attribute( "line" );

    tableOfContents += "</a>";

    el = el.nextSiblingElement( elTagName );
  } while( !el.isNull() );

  closeListTags( 1 );
  // Close the first-level list tag and the toc div tag.
  tableOfContents += "</ul>\n</div>";
}

bool MediaWikiSectionsParser::addListLevel( QString const & levelString )
{
  bool convertedToInt;
  int const level = levelString.toInt( &convertedToInt );

  if( !convertedToInt )
  {
    gdWarning( "MediaWiki: sections level is not an integer: %s", levelString.toUtf8().constData() );
    return false;
  }
  if( level <= 0 )
  {
    gdWarning( "MediaWiki: unsupported nonpositive sections level: %s", levelString.toUtf8().constData() );
    return false;
  }
  if( level > previousLevel + 1 )
  {
    gdWarning( "MediaWiki: unsupported sections level increase by more than one: from %d to %s",
               previousLevel, levelString.toUtf8().constData() );
    return false;
  }

  if( level == previousLevel + 1 )
  {
    // Don't close the previous list item tag to nest the current deeper level's list in it.
    tableOfContents += "\n<ul>\n";
    previousLevel = level;
  }
  else
    closeListTags( level );
  Q_ASSERT( level == previousLevel );

  // Open this list item tag.
  // Omit the (e.g.) class="toclevel-4 tocsection-9" attribute of <li> because it has no visible effect.
  tableOfContents += "<li>";

  return true;
}

void MediaWikiSectionsParser::closeListTags( int currentLevel )
{
  Q_ASSERT( currentLevel <= previousLevel );

  // Close the previous list item tag.
  tableOfContents += "</li>\n";
  // Close list and list item tags of deeper levels, if any.
  while( currentLevel < previousLevel )
  {
    tableOfContents += "</ul>\n</li>\n";
    --previousLevel;
  }
}

class MediaWikiArticleRequest: public MediaWikiDataRequestSlots
{
  typedef std::list< std::pair< QNetworkReply *, bool > > NetReplies;
  NetReplies netReplies;
  QString url;

public:

  MediaWikiArticleRequest( wstring const & word, vector< wstring > const & alts,
                           QString const & url, QNetworkAccessManager & mgr,
                           Class * dictPtr_ );

  virtual void cancel();

private:

  void addQuery( QNetworkAccessManager & mgr, wstring const & word );

  virtual void requestFinished( QNetworkReply * );

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
  Class * dictPtr;
};

void MediaWikiArticleRequest::cancel()
{
  finish();
}

MediaWikiArticleRequest::MediaWikiArticleRequest( wstring const & str,
                                                  vector< wstring > const & alts,
                                                  QString const & url_,
                                                  QNetworkAccessManager & mgr,
                                                  Class * dictPtr_ ):
  url( url_ ), dictPtr( dictPtr_ )
{
  connect( &mgr, SIGNAL( finished( QNetworkReply * ) ),
           this, SLOT( requestFinished( QNetworkReply * ) ),
           Qt::QueuedConnection );
  
  addQuery(  mgr, str );

  for( unsigned x = 0; x < alts.size(); ++x )
    addQuery( mgr, alts[ x ] );
}

void MediaWikiArticleRequest::addQuery( QNetworkAccessManager & mgr,
                                        wstring const & str )
{
  gdDebug( "MediaWiki: requesting article %s\n", gd::toQString( str ).toUtf8().data() );

  QUrl reqUrl( url + "/api.php?action=parse&prop=text|revid|sections&format=xml&redirects" );

#if IS_QT_5
  Qt4x5::Url::addQueryItem( reqUrl, "page", gd::toQString( str ).replace( '+', "%2B" ) );
#else
  reqUrl.addEncodedQueryItem( "page", QUrl::toPercentEncoding( gd::toQString( str ) ) );
#endif

  QNetworkReply * netReply = mgr.get( QNetworkRequest( reqUrl ) );
  
#ifndef QT_NO_OPENSSL

  connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
           netReply, SLOT( ignoreSslErrors() ) );

#endif

  netReplies.push_back( std::make_pair( netReply, false ) );
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

  for( ; netReplies.size() && netReplies.front().second; netReplies.pop_front() )
  {
    QNetworkReply * netReply = netReplies.front().first;
    
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
                                    + "\"><img src=\"qrc:///icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a>";
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
                                      + "\"><img src=\"qrc:///icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a>";
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
                                   QString::fromStdString( addAudioLink( string( "\"" ) + wikiUrl.scheme().toStdString() + ":\\1\"" )
                                                           + "<a href=\"" + wikiUrl.scheme().toStdString() + ":\\1\"" ) );

            // Add url scheme to image source urls
            articleString.replace( " src=\"//", " src=\"" + wikiUrl.scheme() + "://" );
            //fix src="/foo/bar/Baz.png"
            articleString.replace( "src=\"/", "src=\"" + wikiUrl.toString() );

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

            // Insert the ToC in the end to improve performance because no replacements are needed in the generated ToC.
            MediaWikiSectionsParser::generateTableOfContentsIfEmpty( parseNode, articleString );

            QByteArray articleBody = articleString.toUtf8();

            articleBody.prepend( dictPtr->isToLanguageRTL() ? "<div class=\"mwiki\" dir=\"rtl\">" :
                                                              "<div class=\"mwiki\">" );
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
      GD_DPRINTF( "done.\n" );
    }
    else
      setErrorString( netReply->errorString() );

    disconnect( netReply, 0, 0, 0 );
    netReply->deleteLater();
  }

  if ( netReplies.empty() )
    finish();
  else
  if ( updated )
    update();
}

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
    return new MediaWikiArticleRequest( word, alts, url, netMgr, this );
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
