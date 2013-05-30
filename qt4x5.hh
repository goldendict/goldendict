/* Thin wrappers for retaining compatibility for both Qt4.x and Qt5.x */

#ifndef QT4X5_HH
#define QT4X5_HH

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
# define IS_QT_5    0
#else
# define IS_QT_5    1
#endif

#include <QString>
#include <QAtomicInt>
#include <QTextDocument>

#include <QUrl>
#if IS_QT_5
#include <QUrlQuery>
#endif

namespace Qt4x5
{

inline QString escape( QString const & plain )
{
#if IS_QT_5
  return plain.toHtmlEscaped();
#else
  return Qt::escape( plain );
#endif

}

namespace AtomicInt
{

inline int loadAcquire( QAtomicInt const & ref )
{
#if IS_QT_5
  return ref.loadAcquire();
#else
  return ( int )ref;
#endif
}

}

namespace Url
{

inline bool hasQueryItem( QUrl const & url, QString const & key )
{
#if IS_QT_5
  return QUrlQuery( url ).hasQueryItem( key );
#else
  return url.hasQueryItem( key );
#endif
}

inline QString queryItemValue( QUrl const & url, QString const & item )
{
#if IS_QT_5
  return QUrlQuery( url ).queryItemValue( item );
#else
  return url.queryItemValue( item );
#endif
}

inline void addQueryItem( QUrl & url, QString const & key, QString const & value )
{
#if IS_QT_5
  QUrlQuery urlQuery( url );
  urlQuery.addQueryItem( key, value );
  url.setQuery( urlQuery );
#else
  url.addQueryItem( key, value );
#endif
}

inline void removeQueryItem( QUrl & url, QString const & key )
{
#if IS_QT_5
  QUrlQuery urlQuery( url );
  urlQuery.removeQueryItem( key );
  url.setQuery( urlQuery );
#else
  url.removeQueryItem( key );
#endif
}

inline void setQueryItems( QUrl & url, QList< QPair< QString, QString > > const & query )
{
#if IS_QT_5
  QUrlQuery urlQuery( url );
  urlQuery.setQueryItems( query );
  url.setQuery( urlQuery );
#else
  url.setQueryItems( query );
#endif
}

}

}

#endif // QT4X5_HH
