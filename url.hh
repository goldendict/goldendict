#ifndef URL_HH
#define URL_HH

#include <QUrl>
#include "qt4x5.hh"

namespace Url
{

#if IS_QT_5

/// This class is created due to behavior change of the setPath() method
/// See: https://bugreports.qt-project.org/browse/QTBUG-27728
//       https://codereview.qt-project.org/#change,38257
class Class : public QUrl
{
public:
  Class() : QUrl() {}
  Class( QString const & url ) : QUrl( url ) {}
  Class( QUrl const & other ) : QUrl( other ) {}
  Class( QString const & url, ParsingMode parsingMode ) : QUrl( url, parsingMode ) {}

  void setPath( QString const & path );
};
#else
typedef QUrl Class;
#endif

}

#endif // URL_HH
