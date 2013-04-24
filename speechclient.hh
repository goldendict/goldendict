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
  bool tell( QString const & text ) const;
  bool say( QString const & text ) const;

signals:
  void finished();
  void finished(SpeechClient *client);

protected:
  virtual void timerEvent( QTimerEvent * evt );

private:
  struct InternalData;
  InternalData * internalData;

};

#endif // __SPEECHCLIENT_HH_INCLUDED__
