#include "globalbroadcaster.h"
#include <QGlobalStatic>

Q_GLOBAL_STATIC( GlobalBroadcaster, bdcaster )
GlobalBroadcaster::GlobalBroadcaster( QObject * parent ) : QObject( parent )
{
}

GlobalBroadcaster * GlobalBroadcaster::instance()
{
  return bdcaster;
}
void GlobalBroadcaster::setPreference( Config::Preferences * p )
{
  preference = p;
}
Config::Preferences * GlobalBroadcaster::getPreference()
{
  return preference;
}

// namespace global
