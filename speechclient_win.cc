#include "speechclient.hh"

#include <windows.h>
#include "speechhlp.hh"

#include <QtCore>

struct SpeechClient::InternalData
{
  InternalData( Config::VoiceEngine const & e ):
    waitingFinish( false )
    , engine( e )
    , oldVolume( -1 )
    , oldRate( -1 )
  {
    sp = speechCreate( e.id.toStdWString().c_str() );
  }

  ~InternalData()
  {
    speechDestroy( sp );
  }

  SpeechHelper sp;
  bool waitingFinish;
  Engine engine;
  int oldVolume;
  int oldRate;
};

SpeechClient::SpeechClient( Config::VoiceEngine const & e, QObject * parent ):
  QObject( parent ),
  internalData( new InternalData( e ) )
{
}

SpeechClient::~SpeechClient()
{
  delete internalData;
}

static bool enumEngines( void * /* token */,
                         const wchar_t * id,
                         const wchar_t * name,
                         void * userData )
{
  SpeechClient::Engines * pEngines = ( SpeechClient::Engines * )userData;
  SpeechClient::Engine engine( Config::VoiceEngine(
    QString::fromWCharArray( id ),
    QString::fromWCharArray( name ),
    50, 50 ) );
  pEngines->push_back( engine );
  return true;
}

SpeechClient::Engines SpeechClient::availableEngines()
{
  Engines engines;
  speechEnumerateAvailableEngines( enumEngines, &engines );
  return engines;
}

const SpeechClient::Engine & SpeechClient::engine() const
{
  return internalData->engine;
}

bool SpeechClient::tell( QString const & text, int volume, int rate )
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  if ( internalData->waitingFinish )
    return false;

  if( volume < 0 )
    volume = engine().volume;
  if( rate < 0 )
    rate = engine().rate;

  internalData->oldVolume = setSpeechVolume( internalData->sp, volume );
  internalData->oldRate = setSpeechRate( internalData->sp, rate );

  bool ok = speechTell( internalData->sp, text.toStdWString().c_str() );

  emit started( ok );

  if ( ok )
  {
    internalData->waitingFinish = true;
    startTimer( 50 );
  }
  else
  {
    emit finished();
  }
  return ok;
}

bool SpeechClient::say( QString const & text, int volume, int rate )
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  if( volume < 0 )
    volume = engine().volume;
  if( rate < 0 )
    rate = engine().rate;

  int oldVolume = setSpeechVolume( internalData->sp, volume );
  int oldRate = setSpeechRate( internalData->sp, rate );

  bool ok = speechSay( internalData->sp, text.toStdWString().c_str() );

  if( oldVolume >=0 )
    setSpeechVolume( internalData->sp, oldVolume );
  if( oldRate >=0 )
    setSpeechRate( internalData->sp, oldRate );

  return ok;
}

void SpeechClient::timerEvent( QTimerEvent * evt )
{
  QObject::timerEvent( evt );

  if ( !internalData->waitingFinish )
    return;

  if ( speechTellFinished( internalData->sp ) )
  {
    killTimer( evt->timerId() ) ;
    internalData->waitingFinish = false;

    if( internalData->oldVolume >=0 )
      setSpeechVolume( internalData->sp, internalData->oldVolume );
    if( internalData->oldRate >=0 )
      setSpeechRate( internalData->sp, internalData->oldRate );
    internalData->oldVolume = -1;
    internalData->oldRate = -1;

    emit finished();
  }
}
