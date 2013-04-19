#include "speechclient.hh"
#include <QtCore>

#include <windows.h>
#include "speechhlp.hh"

struct SpeechClient::InternalData
{
  InternalData( QString const & engineId ):
    waitingFinish( false )
  {
    sp = speechCreate( reinterpret_cast<const wchar_t*>( engineId.utf16() ) );
  }

  ~InternalData()
  {
    speechDestroy( sp );
  }

  SpeechHelper sp;
  Engine engine;
  typedef QPointer<SpeechClient> Ptr;
  static QList<Ptr> ptrs;
  QPointer<QObject> onFinishObj;
  bool waitingFinish;
};

QList<SpeechClient::InternalData::Ptr> SpeechClient::InternalData::ptrs =
    QList<SpeechClient::InternalData::Ptr>();

SpeechClient::SpeechClient( QString const & engineId, QObject * parent ):
  QObject( parent ),
  internalData( new InternalData( engineId ) )
{
  Engine engine;
  engine.id = QString::fromWCharArray( speechEngineId( internalData->sp ) );
  engine.name = QString::fromWCharArray( speechEngineName( internalData->sp ) );
  internalData->ptrs.push_back( this );
}

SpeechClient::~SpeechClient()
{
  internalData->ptrs.removeAll( this );
  delete internalData;
}

static bool enumEngines( void * /* token */,
                         const wchar_t * id,
                         const wchar_t * name,
                         void * userData )
{
  SpeechClient::Engines *pEngines = (SpeechClient::Engines *)userData;
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

bool SpeechClient::tell( QString const & text, QObject * obj, const char * slot ) const
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  if ( internalData->waitingFinish )
    return false;

  if ( obj && slot )
  {
    internalData->onFinishObj = obj;
    connect( const_cast<SpeechClient *>( this ), SIGNAL( finished() ),
             obj, slot );
    connect( const_cast<SpeechClient *>( this ), SIGNAL( finished( SpeechClient * ) ),
             obj, slot );
  }

  internalData->waitingFinish = true;
  const_cast<SpeechClient *>( this )->startTimer( 50 );
  return speechTell( internalData->sp, reinterpret_cast<const wchar_t*>( text.utf16() ) );
}

bool SpeechClient::say( QString const & text ) const
{
  if ( !speechAvailable( internalData->sp ) )
    return false;

  return speechSay( internalData->sp, reinterpret_cast<const wchar_t*>( text.utf16() ) );
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
      finished();
      finished( this );
//      internalData->onFinishObj = NULL;
  }
}
