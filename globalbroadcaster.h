#ifndef GLOBAL_GLOBALBROADCASTER_H
#define GLOBAL_GLOBALBROADCASTER_H

#include <QObject>

struct ActiveDictIds
{
  QString word;
  QStringList dictIds;
};

class GlobalBroadcaster : public QObject
{
  Q_OBJECT
public:
  GlobalBroadcaster( QObject * parent = nullptr );
  static GlobalBroadcaster * instance();
signals:
  void emitDictIds( ActiveDictIds ad );
};

#endif // GLOBAL_GLOBALBROADCASTER_H
