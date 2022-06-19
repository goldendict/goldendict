#ifndef GLOBAL_GLOBALBROADCASTER_H
#define GLOBAL_GLOBALBROADCASTER_H

#include <QObject>
#include <vector>
#include "config.hh"

struct ActiveDictIds
{
  QString word;
  QStringList dictIds;
};

class GlobalBroadcaster : public QObject
{
  Q_OBJECT
private:
  Config::Preferences * preference;
  std::vector<QString> whitelist;

public:
  void setPreference( Config::Preferences * _pre );
  Config::Preferences * getPreference();
  GlobalBroadcaster( QObject * parent = nullptr );
  void addWhitelist(QString host);
  bool existedInWhitelist(QString host);
  static GlobalBroadcaster * instance();

signals:
  void dictionaryChanges( ActiveDictIds ad );
};

#endif // GLOBAL_GLOBALBROADCASTER_H
