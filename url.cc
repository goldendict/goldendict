#include "url.hh"

namespace Url
{

#if IS_QT_5
void Class::setPath( const QString & path )
{
  QLatin1Char slash( '/' );
  if ( path.startsWith( slash ) )
    QUrl::setPath( path );
  else
    QUrl::setPath( slash + path );
}
#endif

}
