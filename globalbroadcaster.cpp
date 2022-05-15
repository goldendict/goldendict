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

void GlobalBroadcaster::addWhitelist(QString url){
  whitelist.push_back(url);
}

bool GlobalBroadcaster::existedInWhitelist(QString url){
  return std::find(whitelist.begin(), whitelist.end(), url) != whitelist.end();
}
// namespace global
