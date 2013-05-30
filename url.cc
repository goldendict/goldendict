#include "url.hh"

namespace Url
{

void Class::setPath( const QString & path )
{
  QLatin1Char slash( '/' );
  if ( path.startsWith( slash ) )
    QUrl::setPath( path );
  else
    QUrl::setPath( slash + path );
}

}
