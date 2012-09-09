/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "website.hh"
#include "wstring_qt.hh"
#include "utf8.hh"
#include <QUrl>
#include <QTextCodec>

namespace WebSite {

using namespace Dictionary;

namespace {

class WebSiteDictionary: public Dictionary::Class
{
  string name;
  QByteArray urlTemplate;

public:

  WebSiteDictionary( string const & id, string const & name_,
                     QString const & urlTemplate_ ):
    Dictionary::Class( id, vector< string >() ),
    name( name_ ),
    urlTemplate( QUrl( urlTemplate_ ).toEncoded() )
  {
  }

  virtual bool needNetwork() throw()
  { return true; }
  
  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual QIcon getIcon() throw()
  { return QIcon(":/icons/internet.png"); }

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
  sptr< DataRequestInstant > dr = new DataRequestInstant( true );

  QByteArray urlString;

  // Context contains the right url to go to
  if ( context.size() )
    urlString = Utf8::encode( context ).c_str();
  else
  {
    urlString = urlTemplate;

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
                                               ws[ x ].url ) );
  }

  return result;
}

}
