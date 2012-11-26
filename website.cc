/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "website.hh"
#include "wstring_qt.hh"
#include "utf8.hh"
#include <QUrl>
#include <QTextCodec>
#include <QFileInfo>
#include "langcoder.hh"
#include <QWebFrame>
#include <QWebElement>
#include "article_netmgr.hh"
#include "htmlescape.hh"
#include "filetype.hh"
namespace WebSite {

using namespace Dictionary;

namespace {

class WebSiteDictionary: public Dictionary::Class
{
  string name;
  QByteArray urlTemplate;
  QString resultselectors, noresulttext, filter, icon, customcss, fromlang, tolang;
  bool usePost;
public:

  WebSiteDictionary( string const & id, string const & name_,
                     QString const & urlTemplate_ , QString const & resultselectors_,
                     QString const & noresulttext_, QString const & filter_,
                     QString const & icon_, QString const & customcss_,
                     QString const & fromlang_,
                     QString const & tolang_, bool usePost_):
    Dictionary::Class( id, vector< string >() ),
      name( name_ ), resultselectors( resultselectors_ ),
      noresulttext( noresulttext_ ), filter( filter_ ),
      icon( icon_ ), customcss( customcss_ ),
      fromlang( fromlang_ ),
      tolang( tolang_ ), usePost( usePost_ ),
    urlTemplate( QUrl( urlTemplate_ ).toEncoded())
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
  {
      if( !icon.isNull() && !icon.isEmpty() )
      {
          QFileInfo fInfo(  QDir( Config::getConfigDir() ), icon );
          if( fInfo.isFile() )
              return QIcon( fInfo.absoluteFilePath() );
      }
      return QIcon(":/icons/internet.png");
  }
  virtual quint32 getLangFrom() const
  { return LangCoder::code3toInt( fromlang.toStdString() ); }

  /// Returns the dictionary's target language.
  virtual quint32 getLangTo() const
  { return LangCoder::code3toInt( tolang.toStdString() ); }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const & word,
                                                 unsigned long ) throw( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &,
                                          vector< wstring > const & alts,
                                          wstring const & context )
    throw( std::exception );
};

sptr< WordSearchRequest > WebSiteDictionary::prefixMatch( wstring const & /*word*/,
                                                          unsigned long ) throw( std::exception )
{
  sptr< WordSearchRequestInstant > sr = new WordSearchRequestInstant;

  sr->setUncertain( true );

  return sr;
}

sptr< DataRequest > WebSiteDictionary::getArticle( wstring const & str,
                                                   vector< wstring > const &,
                                                   wstring const & context )
  throw( std::exception )
{

  QByteArray urlString;

  // Context contains the right url to go to
  if ( context.size() )
    urlString = Utf8::encode( context ).c_str();
  else
  {
    urlString = urlTemplate;

    if( usePost && resultselectors.isEmpty() )
    {
        QUrl url = QUrl::fromEncoded( urlString );
        if( !url.hasQueryItem( POSTQUERYITEM ) )
        {
            url.addQueryItem( POSTQUERYITEM, "true" );
            urlString = url.toEncoded();
        }

    }
    QString inputWord = gd::toQString( str );

    urlString.replace( "%25GDWORD%25", inputWord.toUtf8().toPercentEncoding() );
    urlString.replace( "%25GD1251%25",
                       QTextCodec::codecForName( "Windows-1251" )->fromUnicode( inputWord ).toPercentEncoding() );

    urlString.replace( "%25GDBIG5%25",
                       QTextCodec::codecForName( "Big-5" )->fromUnicode( inputWord ).toPercentEncoding() );

    // Handle all ISO-8859 encodings
    for( int x = 1; x <= 16; ++x )
    {
      urlString.replace( QString( "%25GDISO%1%25" ).arg( x ),
                         QTextCodec::codecForName( QString( "ISO 8859-%1" ).arg( x ).toAscii() )->fromUnicode( inputWord ).toPercentEncoding() );

      if ( x == 10 )
        x = 12; // Skip encodings 11..12, they don't exist
    }
  }

  if( resultselectors.isEmpty() )
  {

      sptr< DataRequestInstant > dr = new DataRequestInstant( true );

      string result = "<div class=\"website_padding\"></div>";
      result += string( "<iframe id=\"gdexpandframe-" ) + getId() +
                        "\" src=\"" + urlString.data() +
                        "\" onmouseover=\"processIframeMouseOver('gdexpandframe-" + getId() + "');\" "
                        "onmouseout=\"processIframeMouseOut();\" "
                        "scrolling=\"no\" marginwidth=\"0\" marginheight=\"0\" "
                        "frameborder=\"0\" vspace=\"0\" hspace=\"0\" "
                        "style=\"overflow:visible; width:100%; display:none;\">"
                        "</iframe>";

      dr->getData().resize( result.size() );

      memcpy( &( dr->getData().front() ), result.data(), result.size() );

      return dr;
  }
  return new WebDictHttpRequest(  QUrl::fromEncoded( urlString ),
                                   gd::toQString( str ),
                                  resultselectors,
                                  noresulttext,
                                  filter,
                                  customcss,
                                  usePost );
}

}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::WebSites const & ws )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  for( unsigned x = 0; x < ws.size(); ++x )
  {
    if ( ws[ x ].enabled )
      result.push_back( new WebSiteDictionary( ws[ x ].id.toUtf8().data(),
                                               ws[ x ].name.toUtf8().data(),
                                               ws[ x ].url,
                                               ws[ x ].resultselectors,
                                               ws[ x ].noresulttext,
                                               ws[ x ].filter,
                                               ws[ x ].icon,
                                               ws[ x ].customcss,
                                               ws[ x ].fromlang,
                                               ws[ x ].tolang,
                                               ws[ x ].usepost ) );
  }

  return result;
}

void WebDictHttpRequest::cancel()
{
    if( isFinished()) return;
    m_webpage->action( QWebPage::Stop );
    finish();
}

WebDictHttpRequest::WebDictHttpRequest( QUrl const &url_,
                                        QString const &word_,
                                        QString const & resultselectors_,
                                        QString const & noresulttext_,
                                        QString const & filter_,
                                        QString const & customcss_,
                                        bool usePost_ ):
    word( word_ ),
    resultselectors( resultselectors_ ),
    noresulttext( noresulttext_ ),
    filter( filter_ ),
    customcss( customcss_ )
{
    m_webpage = new QWebPage();
    connect( m_webpage, SIGNAL(loadFinished(bool)),
             this, SLOT( loaded(bool)) );
    if( usePost_ )
    {
        QNetworkRequest req( url_ );
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
        m_webpage->currentFrame()->load( req,
                                         QNetworkAccessManager::PostOperation,
                                         url_.encodedQuery() );
    }
    else
    {
        m_webpage->currentFrame()->load( url_ );
    }
}

void WebDictHttpRequest::loaded( bool ok )
{
    if( ok )
    {

        QWebElement doc = m_webpage->currentFrame()->documentElement();
        if( !noresulttext.isEmpty() && doc.toOuterXml().indexOf( noresulttext ) !=-1 )
        {
            finish();
            return;
        }
        if( !filter.isEmpty() )
        {
            QWebElement rel = doc.findFirst(filter);
            while(!rel.isNull() )
            {
                rel.removeFromDocument();
                rel = doc.findFirst(filter);
            }
        }
        QUrl baseUrl = m_webpage->currentFrame()->baseUrl();
        QString div;
        QWebElement paraElement = doc.findFirst( resultselectors );
        while (!paraElement.isNull()) {
             //...
            //if( paraElement.tagName().toLower() == "a" )
          //  qDebug() <<"\n-------------\nelement:"+ paraElement.toOuterXml().toUtf8();
            QUrl url;
            QString attName;
            if( paraElement.hasAttribute("href") )
            {
                url =QUrl::fromEncoded(paraElement.attribute("href").toUtf8());
                attName = "href";
            }else if( paraElement.hasAttribute("src")) {
                url =QUrl(paraElement.attribute("src"));
                 attName = "src";
            }
            if(! url.isEmpty() )
            {
              //  qDebug() << "url:" + url.toString();
                if(ArticleNetworkAccessManager::RelativeUrl2Absolute( url, baseUrl ))
                {
                    paraElement.setAttribute(attName, QString::fromUtf8( url.toEncoded() ));
                    //qDebug() << "real url:" + url.toString();
                }
            }
            //fix inner links
            //QWebElement child = paraElement.findFirst()
            foreach (QWebElement child, paraElement.findAll("[href],[src]"))
            {
                if( child.hasAttribute( "href" ) )
                {
                    QUrl cUrl = QUrl::fromEncoded( child.attribute("href").toUtf8() );
                    bool isDefineLink = false;
                    if(child.tagName().toLower() =="a"
                            &&( cUrl.host().isEmpty()  || cUrl.host() == baseUrl.host()) )
                    {
                        std::string path = cUrl.path().toUtf8().data();
                        if( !Filetype::isNameOfSound( path )
                                &&!Filetype::isNameOfPicture( path )
                                &&!Filetype::isNameOfTiff( path ) )
                            //TODO check for others file types
                        {
                            QString define = child.toPlainText().trimmed();
                            if( !define.isEmpty() )
                            {
                                cUrl.setUrl( define );
                                child.setAttribute("href", QString::fromUtf8( cUrl.toEncoded() ));
                                isDefineLink = true;
                            }
                        }
                    }
                    //qDebug() << "url:" + cUrl.toString();
                    if(!isDefineLink && ArticleNetworkAccessManager::RelativeUrl2Absolute( cUrl, baseUrl ))
                    {
                        child.setAttribute("href", QString::fromUtf8( cUrl.toEncoded() ));
                       // qDebug() << "real url:" + cUrl.toString();
                    }
                }else
                {
                    QUrl cUrl = QUrl::fromEncoded( child.attribute("src").toUtf8() );
                    //qDebug() << "url:" + url.toString();
                    if( ArticleNetworkAccessManager::RelativeUrl2Absolute( cUrl, baseUrl ))
                    {
                        child.setAttribute("src", QString::fromUtf8( cUrl.toEncoded() ));
                        //qDebug() << "real url:" + cUrl.toString();
                    }
                }

            }
            if(paraElement.tagName().toLower() =="body" )
                div +=paraElement.toInnerXml();
            else
                div +=paraElement.toOuterXml();
            paraElement.removeFromDocument();
            paraElement = doc.findFirst( resultselectors );
            //newEle.appendInside(paraElement);
        }
        if (!div.isEmpty())
        {
            QByteArray contentdata = "<div class=\"website_article\">";
            //contentdata.append( dictid.c_str() );
            if( !customcss.isEmpty())
            {
                QFileInfo fInfo(  QDir( Config::getConfigDir() ), customcss );
                if( fInfo.isFile() )
                {
                    QFile cssFile( fInfo.absoluteFilePath()  );
                    cssFile.open( QIODevice::ReadOnly );
                    contentdata.append( "<style type=\"text/css\">" );
                    contentdata.append( cssFile.readAll() );
                    contentdata.append( "</style>" );
                    cssFile.close();
                }
            }
            contentdata.append( "<h3 class=\"website_term\">" );
            contentdata.append( Html::escape( word.toUtf8().data()).c_str());
            contentdata.append("</h3>" );
            contentdata.append( div.toUtf8() );
            contentdata.append( "</div>");
            //qDebug() << contentdata;
            Mutex::Lock _( dataMutex );

            //size_t prevSize = data.size();

            data.resize( contentdata.size() );

            memcpy( &data.front() , contentdata.data(), contentdata.size() );

            hasAnyData = true;
        }
        //this->isFinishedFlag = true;
       // bool isEnd = isFinished();
       // var i = isFinishedFlag;
        //isFinished();
       // qDebug() << (isFinished()?"Finished":"not finished");
        //qDebug() << div.toUtf8();
    }
    finish();
    //qDebug() << (isFinished()?"Finished":"not finished");
}

}
