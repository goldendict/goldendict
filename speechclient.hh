#ifndef __SPEECHCLIENT_HH_INCLUDED__
#define __SPEECHCLIENT_HH_INCLUDED__

#include <QObject>

class SpeechClient: public QObject
{
Q_OBJECT

public:

  struct Engine
  {
    QString id;
    QString name;
  };

  typedef QList<Engine> Engines;

  SpeechClient( QString const & engineId, QObject * parent = 0L );
  virtual ~SpeechClient();

  static Engines availableEngines();

  const Engine & engine() const;
  bool tell( QString const & text );
  bool say( QString const & text );

signals:
  void started( bool ok );
  void finished();

protected:
  virtual void timerEvent( QTimerEvent * evt );

private:
  struct InternalData;
  InternalData * internalData;

};

#endif // __SPEECHCLIENT_HH_INCLUDED__
