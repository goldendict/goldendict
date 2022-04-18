/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "website.hh"
#include "wstring_qt.hh"
#include "utf8.hh"
#include <QUrl>
#include <QTextCodec>
#include <QDir>
#include <QFileInfo>
#include "gddebug.hh"

#include <QRegularExpression>

namespace WebSite {

using namespace Dictionary;

namespace {

class WebSiteDictionary: public Dictionary::Class
{
  string name;
  QByteArray urlTemplate;
  QString iconFilename;
  bool inside_iframe;
  QNetworkAccessManager & netMgr;

public:

  WebSiteDictionary( string const & id, string const & name_,
                     QString const & urlTemplate_,
                     QString const & iconFilename_,
                     bool inside_iframe_,
                     QNetworkAccessManager & netMgr_ ):
    Dictionary::Class( id, vector< string >() ),
    name( name_ ),
    urlTemplate( QUrl( urlTemplate_ ).toEncoded() ),
    iconFilename( iconFilename_ ),
    inside_iframe( inside_iframe_ ),
    netMgr( netMgr_ )
  {
    dictionaryDescription = urlTemplate_;
  }

  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const & word,
                                                 unsigned long ) ;

  virtual sptr< DataRequest > getArticle( wstring const &,
                                          vector< wstring > const & alts,
                                          wstring const & context, bool )
    ;

  virtual sptr< Dictionary::DataRequest > getResource( string const & name ) ;

  void isolateWebCSS( QString & css );

protected:

  virtual void loadIcon() throw();
};

sptr< WordSearchRequest > WebSiteDictionary::prefixMatch( wstring const & /*word*/,
                                                          unsigned long ) 
{
  sptr< WordSearchRequestInstant > sr = new WordSearchRequestInstant;

  sr->setUncertain( true );

  return sr;
}

void WebSiteDictionary::isolateWebCSS( QString & css )
{
  isolateCSS( css, ".website" );
}

class WebSiteArticleRequest: public WebSiteDataRequestSlots
{
  QNetworkReply * netReply;
  QString url;
  Class * dictPtr;
  QNetworkAccessManager & mgr;

public:

  WebSiteArticleRequest( QString const & url, QNetworkAccessManager & _mgr,
                         Class * dictPtr_ );
  ~WebSiteArticleRequest()
  {}

  virtual void cancel();

private:

  virtual void requestFinished( QNetworkReply * );
  static QTextCodec * codecForHtml( QByteArray const & ba );
};

void WebSiteArticleRequest::cancel()
{
  finish();
}

WebSiteArticleRequest::WebSiteArticleRequest( QString const & url_,
                                              QNetworkAccessManager & _mgr,
                                              Class * dictPtr_ ):
  url( url_ ), dictPtr( dictPtr_ ), mgr( _mgr )
{
  connect( &mgr, SIGNAL( finished( QNetworkReply * ) ),
           this, SLOT( requestFinished( QNetworkReply * ) ),
           Qt::QueuedConnection );

  QUrl reqUrl( url );

  auto request = QNetworkRequest( reqUrl );
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy );
  netReply = mgr.get( request );

#ifndef QT_NO_OPENSSL
  connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
           netReply, SLOT( ignoreSslErrors() ) );
#endif
}

QTextCodec * WebSiteArticleRequest::codecForHtml( QByteArray const & ba )
{
  return QTextCodec::codecForHtml( ba, 0 );
}

void WebSiteArticleRequest::requestFinished( QNetworkReply * r )
{
  if ( isFinished() ) // Was cancelled
    return;

  if ( r != netReply )
  {
    // Well, that's not our reply, don't do anything
    return;
  }

  if ( netReply->error() == QNetworkReply::NoError )
  {
    // Check for redirect reply

    QVariant possibleRedirectUrl = netReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    QUrl redirectUrl = possibleRedirectUrl.toUrl();
    if( !redirectUrl.isEmpty() )
    {
      disconnect( netReply, 0, 0, 0 );
      netReply->deleteLater();
      netReply = mgr.get( QNetworkRequest( redirectUrl ) );
#ifndef QT_NO_OPENSSL
      connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
               netReply, SLOT( ignoreSslErrors() ) );
#endif
      return;
    }

    // Handle reply data

    QByteArray replyData = netReply->readAll();
    QString articleString;

    QTextCodec * codec = WebSiteArticleRequest::codecForHtml( replyData );
    if( codec )
      articleString = codec->toUnicode( replyData );
    else
      articleString = QString::fromUtf8( replyData );

    // Change links from relative to absolute

    QString root = netReply->url().scheme() + "://" + netReply->url().host();
    QString base = root + netReply->url().path();
    while( !base.isEmpty() && !base.endsWith( "/" ) )
      base.chop( 1 );

    QRegularExpression tags( "<\\s*(a|link|img|script)\\s+[^>]*(src|href)\\s*=\\s*['\"][^>]+>",
                             QRegularExpression::CaseInsensitiveOption );
    QRegularExpression links( "\\b(src|href)\\s*=\\s*(['\"])([^'\"]+['\"])",
                              QRegularExpression::CaseInsensitiveOption );
    int pos = 0;
    QString articleNewString;
    QRegularExpressionMatchIterator it = tags.globalMatch( articleString );
    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();
      articleNewString += articleString.mid( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QString tag = match.captured();

      QRegularExpressionMatch match_links = links.match( tag );
      if( !match_links.hasMatch() )
      {
        articleNewString += tag;
        continue;
      }

      QString url = match_links.captured( 3 );

      if( url.indexOf( ":/" ) >= 0 || url.indexOf( "data:" ) >= 0
          || url.indexOf( "mailto:" ) >= 0 || url.startsWith( "#" )
          || url.startsWith( "javascript:" ) )
      {
        // External link, anchor or base64-encoded data
        articleNewString += tag;
        continue;
      }

      QString newUrl = match_links.captured( 1 ) + "=" + match_links.captured( 2 );
      if( url.startsWith( "//" ) )
        newUrl += netReply->url().scheme() + ":";
      else
      if( url.startsWith( "/" ) )
        newUrl += root;
      else
        newUrl += base;
      newUrl += match_links.captured( 3 );

      tag.replace( match_links.capturedStart(), match_links.capturedLength(), newUrl );
      articleNewString += tag;
    }
    if( pos )
    {
      articleNewString += articleString.mid( pos );
      articleString = articleNewString;
      articleNewString.clear();
    }

    // Redirect CSS links to own handler

    QString prefix = QString( "bres://" ) + dictPtr->getId().c_str() + "/";
    QRegularExpression linkTags( "(<\\s*link\\s[^>]*rel\\s*=\\s*['\"]stylesheet['\"]\\s+[^>]*href\\s*=\\s*['\"])([^'\"]+)://([^'\"]+['\"][^>]+>)",
                                 QRegularExpression::CaseInsensitiveOption );
    pos = 0;
    it = linkTags.globalMatch( articleString );

    while( it.hasNext() )
    {
      QRegularExpressionMatch match = it.next();
      articleNewString += articleString.mid( pos, match.capturedStart() - pos );
      pos = match.capturedEnd();

      QString newTag = match.captured( 1 ) + prefix + match.captured( 2 )
                       + "/" + match.captured( 3 );
      articleNewString += newTag;
    }
    if( pos )
    {
      articleNewString += articleString.mid( pos );
      articleString = articleNewString;
      articleNewString.clear();
    }

    // See Issue #271: A mechanism to clean-up invalid HTML cards.
    articleString += "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</font>""</font>""</font>""</font>""</font>""</font>"
                     "</b></b></b></b></b></b></b></b>"
                     "</i></i></i></i></i></i></i></i>"
                     "</a></a></a></a></a></a></a></a>";

    QByteArray articleBody = articleString.toUtf8();

    QString divStr = QString( "<div class=\"website\"" );
    divStr += dictPtr->isToLanguageRTL() ? " dir=\"rtl\">" : ">";

    articleBody.prepend( divStr.toUtf8() );
    articleBody.append( "</div>" );

    articleBody.prepend( "<div class=\"website_padding\"></div>" );

    Mutex::Lock _( dataMutex );

    size_t prevSize = data.size();

    data.resize( prevSize + articleBody.size() );

    memcpy( &data.front() + prevSize, articleBody.data(), articleBody.size() );

    hasAnyData = true;

  }
  else
  {
    if( netReply->url().scheme() == "file" )
    {
      gdWarning( "WebSites: Failed loading article from \"%s\", reason: %s\n", dictPtr->getName().c_str(),
                 netReply->errorString().toUtf8().data() );
    }
    else
    {
      setErrorString( netReply->errorString() );
    }
  }

  disconnect( netReply, 0, 0, 0 );
  netReply->deleteLater();

  finish();
}

sptr< DataRequest > WebSiteDictionary::getArticle( wstring const & str,
                                                   vector< wstring > const &,
                                                   wstring const & context, bool )
  
{
  QByteArray urlString;

  // Context contains the right url to go to
  if ( context.size() )
    urlString = Utf8::encode( context ).c_str();
  else
  {
    urlString = urlTemplate;

    QString inputWord = gd::toQString( str );

    urlString.replace( "%25GDWORD%25", inputWord.toUtf8().toPercentEncoding() );

    QTextCodec *codec = QTextCodec::codecForName( "Windows-1251" );
    if( codec )
      urlString.replace( "%25GD1251%25", codec->fromUnicode( inputWord ).toPercentEncoding() );

    codec = QTextCodec::codecForName( "Big-5" );
    if( codec )
      urlString.replace( "%25GDBIG5%25", codec->fromUnicode( inputWord ).toPercentEncoding() );

    codec = QTextCodec::codecForName( "Big5-HKSCS" );
    if( codec )
      urlString.replace( "%25GDBIG5HKSCS%25", codec->fromUnicode( inputWord ).toPercentEncoding() );

    codec = QTextCodec::codecForName( "Shift-JIS" );
    if( codec )
      urlString.replace( "%25GDSHIFTJIS%25", codec->fromUnicode( inputWord ).toPercentEncoding() );

    codec = QTextCodec::codecForName( "GB18030" );
    if( codec )
      urlString.replace( "%25GDGBK%25", codec->fromUnicode( inputWord ).toPercentEncoding() );


    // Handle all ISO-8859 encodings
    for( int x = 1; x <= 16; ++x )
    {
      codec = QTextCodec::codecForName( QString( "ISO 8859-%1" ).arg( x ).toLatin1() );
      if( codec )
        urlString.replace( QString( "%25GDISO%1%25" ).arg( x ).toUtf8(), codec->fromUnicode( inputWord ).toPercentEncoding() );

      if ( x == 10 )
        x = 12; // Skip encodings 11..12, they don't exist
    }
  }

  if( inside_iframe )
  {
    // Just insert link in <iframe> tag

    sptr< DataRequestInstant > dr = new DataRequestInstant( true );

    string result = "<div class=\"website_padding\"></div>";

    result += string( "<iframe id=\"gdexpandframe-" ) + getId() +
                      "\" src=\"" + urlString.data() +
                      "\" onmouseover=\"processIframeMouseOver('gdexpandframe-" + getId() + "');\" "
                      "onmouseout=\"processIframeMouseOut();\" "
                      "scrolling=\"no\" marginwidth=\"0\" marginheight=\"0\" "
                      "frameborder=\"0\" vspace=\"0\" hspace=\"0\""
                      "style=\"overflow:visible; width:100%; display:block;\">"
                      "</iframe>";

    dr->getData().resize( result.size() );

    memcpy( &( dr->getData().front() ), result.data(), result.size() );

    return dr;
  }

  // To load data from site

  return new WebSiteArticleRequest( urlString, netMgr, this );
}

class WebSiteResourceRequest: public WebSiteDataRequestSlots
{
  QNetworkReply * netReply;
  QString url;
  WebSiteDictionary * dictPtr;
  QNetworkAccessManager & mgr;

public:

  WebSiteResourceRequest( QString const & url_, QNetworkAccessManager & _mgr,
                          WebSiteDictionary * dictPtr_ );
  ~WebSiteResourceRequest()
  {}

  virtual void cancel();

private:

  virtual void requestFinished( QNetworkReply * );
};

WebSiteResourceRequest::WebSiteResourceRequest( QString const & url_,
                                                QNetworkAccessManager & _mgr,
                                                WebSiteDictionary * dictPtr_ ):
  url( url_ ), dictPtr( dictPtr_ ), mgr( _mgr )
{
  connect( &mgr, SIGNAL( finished( QNetworkReply * ) ),
           this, SLOT( requestFinished( QNetworkReply * ) ),
           Qt::QueuedConnection );

  QUrl reqUrl( url );

  netReply = mgr.get( QNetworkRequest( reqUrl ) );

#ifndef QT_NO_OPENSSL
  connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
           netReply, SLOT( ignoreSslErrors() ) );
#endif
}

void WebSiteResourceRequest::cancel()
{
  finish();
}

void WebSiteResourceRequest::requestFinished( QNetworkReply * r )
{
  if ( isFinished() ) // Was cancelled
    return;

  if ( r != netReply )
  {
    // Well, that's not our reply, don't do anything
    return;
  }

  if ( netReply->error() == QNetworkReply::NoError )
  {
    // Check for redirect reply

    QVariant possibleRedirectUrl = netReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    QUrl redirectUrl = possibleRedirectUrl.toUrl();
    if( !redirectUrl.isEmpty() )
    {
      disconnect( netReply, 0, 0, 0 );
      netReply->deleteLater();
      netReply = mgr.get( QNetworkRequest( redirectUrl ) );
#ifndef QT_NO_OPENSSL
      connect( netReply, SIGNAL( sslErrors( QList< QSslError > ) ),
               netReply, SLOT( ignoreSslErrors() ) );
#endif
      return;
    }

    // Handle reply data

    QByteArray replyData = netReply->readAll();
    QString cssString = QString::fromUtf8( replyData );

    dictPtr->isolateWebCSS( cssString );

    QByteArray cssData = cssString.toUtf8();

    Mutex::Lock _( dataMutex );

    size_t prevSize = data.size();

    data.resize( prevSize + cssData.size() );

    memcpy( &data.front() + prevSize, cssData.data(), cssData.size() );

    hasAnyData = true;
  }
  else
    setErrorString( netReply->errorString() );

  disconnect( netReply, 0, 0, 0 );
  netReply->deleteLater();

  finish();
}

sptr< Dictionary::DataRequest > WebSiteDictionary::getResource( string const & name ) 
{
  QString link = QString::fromUtf8( name.c_str() );
  int pos = link.indexOf( '/' );
  if( pos > 0 )
    link.replace( pos, 1, "://" );
  return new WebSiteResourceRequest( link, netMgr, this );
}

void WebSiteDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  if( !iconFilename.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), iconFilename );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/webdict.svg");
  dictionaryIconLoaded = true;
}

}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::WebSites const & ws,
                                                      QNetworkAccessManager & mgr )
  
{
  vector< sptr< Dictionary::Class > > result;

  for( int x = 0; x < ws.size(); ++x )
  {
    if ( ws[ x ].enabled )
      result.push_back( new WebSiteDictionary( ws[ x ].id.toUtf8().data(),
                                               ws[ x ].name.toUtf8().data(),
                                               ws[ x ].url,
                                               ws[ x ].iconFilename,
                                               ws[ x ].inside_iframe,
                                               mgr )
                      );
  }

  return result;
}

}
