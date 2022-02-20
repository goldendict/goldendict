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

// namespace global
