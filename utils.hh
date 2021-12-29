/* Thin wrappers for retaining compatibility for both Qt4.x and Qt5.x */

#ifndef UTILS_HH
#define UTILS_HH

#include <QString>
#include <QAtomicInt>
#include <QTextDocument>

#include <QUrl>
#include <QUrlQuery>

namespace Utils
{

  /**
   * remove right end space
   */
inline  QString rstrip(const QString& str) {
    int n = str.size() - 1;
    for (; n >= 0; --n) {
      if (!str.at(n).isSpace()) {
        return str.left(n + 1);
      }
    }
    return "";
  }

  inline bool isExternalLink( QUrl const & url )
  {
    return url.scheme() == "http" || url.scheme() == "https" ||
           url.scheme() == "ftp" || url.scheme() == "mailto" ||
           url.scheme() == "file";
  }

inline QString escape( QString const & plain )
{
  return plain.toHtmlEscaped();
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

}

namespace Dom
{
  typedef int size_type;
}

}

#endif // UTILS_HH
