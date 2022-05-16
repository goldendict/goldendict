/* Thin wrappers for retaining compatibility for both Qt6.x and Qt5.x */

#ifndef UTILS_HH
#define UTILS_HH

#include <QString>
#include <QAtomicInt>
#include <QTextDocument>
#include <QKeyEvent>
#include <QUrl>
#include <QUrlQuery>

namespace Utils
{

/**
 * remove right end space
 */
inline QString rstrip(const QString &str) {
  int n = str.size() - 1;
  for (; n >= 0; --n) {
    if (!str.at(n).isSpace()) {
      return str.left(n + 1);
    }
  }
  return "";
}

/**
 * str="abc\r\n\u0000" should be returned as "abc"
 * @brief rstripnull
 * @param str
 * @return
 */
inline QString rstripnull(const QString &str) {
  int n = str.size() - 1;
  for (; n >= 0; --n) {
    if (!str.at(n).isSpace()&&!str.at(n).isNull()) {
      return str.left(n + 1);
    }
  }
  return "";
}

inline QString unescapeHtml(const QString &str) {
  QTextDocument text;
  text.setHtml(str);
  return text.toPlainText();
}


inline bool isExternalLink(QUrl const &url) {
  return url.scheme() == "http" || url.scheme() == "https" ||
         url.scheme() == "ftp" || url.scheme() == "mailto" ||
         url.scheme() == "file";
}

inline bool isCssFontImage(QUrl const &url) {
  auto fileName = url.fileName();
  auto ext=fileName.mid(fileName.lastIndexOf("."));
  QStringList extensions{".css",".woff",".woff2",".bmp" ,".jpg", ".png", ".tif",".wav", ".ogg", ".oga", ".mp3", ".mp4", ".aac", ".flac",".mid", ".wv ",".ape"} ;
  return extensions.indexOf(ext)>-1;
}

inline QString escape( QString const & plain )
{
  return plain.toHtmlEscaped();
}

// should ignore key event.
inline bool ignoreKeyEvent(QKeyEvent *keyEvent) {
  if ( keyEvent->key() == Qt::Key_Space ||
      keyEvent->key() == Qt::Key_Backspace ||
      keyEvent->key() == Qt::Key_Tab ||
      keyEvent->key() == Qt::Key_Backtab ||
      keyEvent->key() == Qt::Key_Escape)
    return true;
  return false;
}

namespace AtomicInt
{

inline int loadAcquire( QAtomicInt const & ref )
{
  return ref.loadAcquire();
}

}

namespace Url
{
// This wrapper is created due to behavior change of the setPath() method
// See: https://bugreports.qt-project.org/browse/QTBUG-27728
//       https://codereview.qt-project.org/#change,38257
inline QString ensureLeadingSlash( const QString & path )
{
  QLatin1Char slash( '/' );
  if ( path.startsWith( slash ) )
    return path;
  return slash + path;
}

inline bool hasQueryItem( QUrl const & url, QString const & key )
{
  return QUrlQuery( url ).hasQueryItem( key );
}

inline QString queryItemValue( QUrl const & url, QString const & item )
{
  return QUrlQuery( url ).queryItemValue( item, QUrl::FullyDecoded );
}

inline QByteArray encodedQueryItemValue( QUrl const & url, QString const & item )
{
  return QUrlQuery( url ).queryItemValue( item, QUrl::FullyEncoded ).toLatin1();
}

inline void addQueryItem( QUrl & url, QString const & key, QString const & value )
{
  QUrlQuery urlQuery( url );
  urlQuery.addQueryItem( key, value );
  url.setQuery( urlQuery );
}

inline void removeQueryItem( QUrl & url, QString const & key )
{
  QUrlQuery urlQuery( url );
  urlQuery.removeQueryItem( key );
  url.setQuery( urlQuery );
}

inline void setQueryItems( QUrl & url, QList< QPair< QString, QString > > const & query )
{
  QUrlQuery urlQuery( url );
  urlQuery.setQueryItems( query );
  url.setQuery( urlQuery );
}

inline QString path( QUrl const & url )
{
  return url.path( QUrl::FullyDecoded );
}

inline void setFragment( QUrl & url, const QString & fragment )
{
  url.setFragment( fragment, QUrl::DecodedMode );
}

inline QString fragment( const QUrl & url )
{
  return url.fragment( QUrl::FullyDecoded );
}

// extract query word from url
inline QString getWordFromUrl( const QUrl & url )
{
  QString word;
  if( url.scheme().compare( "bword" ) == 0 )
  {
    word = url.path();
  }
  else if( url.scheme() == "gdlookup" ) // Plain html links inherit gdlookup scheme
  {
    if( hasQueryItem( url, "word" ) )
    {
      word = queryItemValue( url, "word" );
    }
    else
    {
      word = url.path().mid( 1 );
    }
  }

  return word;
}
}

}

namespace
{
/// Uses some heuristics to chop off the first domain name from the host name,
/// but only if it's not too base. Returns the resulting host name.
inline QString getHostBase( QUrl const & url )
{
  QString host = url.host();

  return getHostBase(host);
}

inline QString getHostBase( QString const & host )
{
  QStringList domains = host.split( '.' );

  int left = domains.size();

       // Skip last <=3-letter domain name
  if ( left && domains[ left - 1 ].size() <= 3 )
    --left;

       // Skip another <=3-letter domain name
  if ( left && domains[ left - 1 ].size() <= 3 )
    --left;

  if ( left > 1 )
  {
    // We've got something like www.foobar.co.uk -- we can chop off the first
    // domain

    return host.mid( domains[ 0 ].size() + 1 );
  }
  else
    return host;
}
}

#endif // UTILS_HH
