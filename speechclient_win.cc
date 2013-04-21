#include "speechclient.hh"

#include <windows.h>
#include "speechhlp.hh"

#include <QtCore>

struct SpeechClient::InternalData
{
  InternalData( QString const & engineId ):
    waitingFinish( false )
  {
    sp = speechCreate( engineId.toStdWString().c_str() );
  }

  ~InternalData()
  {
    speechDestroy( sp );
  }

  SpeechHelper sp;
  Engine engine;
  bool waitingFinish;
};

SpeechClient::SpeechClient( QString const & engineId, QObject * parent ):
  QObject( parent ),
  internalData( new InternalData( engineId ) )
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
  SpeechClient::Engine engine =
  {
    QString::fromWCharArray( id ),
    QString::fromWCharArray( name )
  };
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

bool SpeechClient::tell( QString const & text )
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  if ( internalData->waitingFinish )
    return false;

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

bool SpeechClient::say( QString const & text )
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  return speechSay( internalData->sp, text.toStdWString().c_str() );
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
    emit finished();
  }
}
