#ifndef __SPEECHCLIENT_HH_INCLUDED__
#define __SPEECHCLIENT_HH_INCLUDED__

#include <QObject>
#include "config.hh"

class SpeechClient: public QObject
{
Q_OBJECT

public:

  struct Engine
  {
    QString id;
    QString name;
    // Volume and rate may vary from 0 to 100
    int volume;
    int rate;
    Engine( Config::VoiceEngine const & e ) :
      id( e.id )
      , name( e.name )
      , volume( e.volume )
      , rate( e.rate )
    {}
  };

  typedef QList<Engine> Engines;

  SpeechClient( Config::VoiceEngine const & e, QObject * parent = 0L );
  virtual ~SpeechClient();

  static Engines availableEngines();

  const Engine & engine() const;

  bool tell( QString const & text, int volume = -1, int rate = -1 );
  bool say( QString const & text, int volume = -1, int rate = -1 );

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
