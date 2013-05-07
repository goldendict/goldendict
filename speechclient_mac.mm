#include "speechclient.hh"

#include <QtCore>
#include <AppKit/NSSpeechSynthesizer.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSError.h>
#include <Foundation/NSString.h>
#include <Foundation/NSAutoreleasePool.h>

static QString NSStringToQString(const NSString * nsstr )
{
  return QString::fromUtf8( [ nsstr UTF8String ] );
}

static NSString * QStringToNSString( QString const & qstr, bool needAlloc = false )
{
  if( needAlloc )
    return [ [ NSString alloc ] initWithUTF8String : qstr.toUtf8().data() ];
  return [ NSString stringWithUTF8String : qstr.toUtf8().data() ];
}

struct SpeechClient::InternalData
{
  InternalData( Config::VoiceEngine const & e ):
    waitingFinish( false )
    , engine( e )
    , oldVolume( -1 )
    , oldRate( -1 )
    , stringToPlay( nil )
  {
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    sp = [ [ NSSpeechSynthesizer alloc ] initWithVoice : QStringToNSString( e.id ) ];
    [ pool drain ];
  }

  ~InternalData()
  {
    [ sp release ];
    if( stringToPlay != nil )
      [ stringToPlay release ];
  }

  NSSpeechSynthesizer * sp;
  bool waitingFinish;
  SpeechClient::Engine engine;
  float oldVolume;
  float oldRate;
  QString oldMode;
  NSString * stringToPlay;
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

SpeechClient::Engines SpeechClient::availableEngines()
{
  Engines engines;
  NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];

  NSArray * voices = [ NSSpeechSynthesizer availableVoices ];
  int voicesNum = [ voices count ];
  for( int i = 0; i < voicesNum; i++ )
  {
    QString id = NSStringToQString( [ voices objectAtIndex : i ] );
    QString name;
    int n = id.lastIndexOf( '.' );
    if( n >= 0 )
      name = id.right( id.size() - n - 1 );
    else
      name = id;
    engines.push_back( SpeechClient::Engine( Config::VoiceEngine(
                                             id, name, 50, 50 ) ) );
  }

  [ pool drain ];
  return engines;
}

const SpeechClient::Engine & SpeechClient::engine() const
{
  return internalData->engine;
}

bool SpeechClient::tell( QString const & text, int volume, int rate )
{
  if( !internalData->sp || [ NSSpeechSynthesizer isAnyApplicationSpeaking ] )
    return false;

  if ( internalData->waitingFinish )
    return false;

  if( volume < 0 )
    volume = engine().volume;
  if( rate < 0 )
    rate = engine().rate;

  NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];

  internalData->oldVolume = [ internalData->sp volume ];
  [ internalData->sp setVolume : ( volume / 100.0 ) ];

  internalData->oldRate = [ internalData->sp rate ];
  [ internalData->sp setRate : ( rate * 2.0 + 100 ) ];

  NSError * err = nil;
  NSString * oldMode = [ internalData->sp objectForProperty : NSSpeechInputModeProperty error : &err ];
  if( err == nil || [ err code ] == 0 )
  {
    internalData->oldMode = NSStringToQString( oldMode );
    [ internalData->sp setObject : NSSpeechModeText forProperty : NSSpeechInputModeProperty error : &err ];
  }
  else
    internalData->oldMode.clear();

  internalData->stringToPlay = QStringToNSString( text, true );

  bool ok = [ internalData->sp startSpeakingString : internalData->stringToPlay ];

  emit started( ok );

  if ( ok )
  {
    internalData->waitingFinish = true;
    startTimer( 50 );
  }
  else
  {
    if( internalData->stringToPlay != nil )
      [ internalData->stringToPlay release ];
    internalData->stringToPlay = nil;
    emit finished();
  }

  [ pool drain ];

  return ok;
}

bool SpeechClient::say( QString const & text, int volume, int rate )
{
(void) text;
(void) volume;
(void) rate;
  return false;
}

void SpeechClient::timerEvent( QTimerEvent * evt )
{
  QObject::timerEvent( evt );

  if ( !internalData->waitingFinish )
    return;

  if ( ![ internalData->sp isSpeaking ] )
  {
    killTimer( evt->timerId() ) ;
    internalData->waitingFinish = false;

    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];

    if( internalData->oldVolume >=0 )
      [ internalData->sp setVolume : internalData->oldVolume ];
    if( internalData->oldRate >=0 )
      [ internalData->sp setRate : internalData->oldRate ];
    internalData->oldVolume = -1;
    internalData->oldRate = -1;

    NSError * err;
    if( !internalData->oldMode.isEmpty() )
      [ internalData->sp setObject : QStringToNSString( internalData->oldMode )
                         forProperty : NSSpeechInputModeProperty error : &err ];

    internalData->oldMode.clear();

    if( internalData->stringToPlay != nil )
      [ internalData->stringToPlay release ];
    internalData->stringToPlay = nil;

    [ pool drain ];

    emit finished();
  }
}
