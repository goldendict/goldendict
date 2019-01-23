/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QScopedPointer>
#include <QObject>
#include "audioplayerfactory.hh"
#include "ffmpegaudioplayer.hh"
#include "multimediaaudioplayer.hh"
#include "externalaudioplayer.hh"
#include "gddebug.hh"

AudioPlayerFactory::AudioPlayerFactory( Config::Preferences const & p ) :
  useInternalPlayer( p.useInternalPlayer ),
  internalPlayerBackend( p.internalPlayerBackend ),
  audioPlaybackProgram( p.audioPlaybackProgram )
{
  reset();
}

void AudioPlayerFactory::setPreferences( Config::Preferences const & p )
{
  if( p.useInternalPlayer != useInternalPlayer )
  {
    useInternalPlayer = p.useInternalPlayer;
    internalPlayerBackend = p.internalPlayerBackend;
    audioPlaybackProgram = p.audioPlaybackProgram;
    reset();
  }
  else
  if( useInternalPlayer && p.internalPlayerBackend != internalPlayerBackend )
  {
    internalPlayerBackend = p.internalPlayerBackend;
    reset();
  }
  else
  if( !useInternalPlayer && p.audioPlaybackProgram != audioPlaybackProgram )
  {
    audioPlaybackProgram = p.audioPlaybackProgram;
    ExternalAudioPlayer * const externalPlayer =
        qobject_cast< ExternalAudioPlayer * >( playerPtr.data() );
    if( externalPlayer )
      setAudioPlaybackProgram( *externalPlayer );
    else
      gdWarning( "External player was expected, but it does not exist.\n" );
  }
}

void AudioPlayerFactory::reset()
{
  if( useInternalPlayer )
  {
    // qobject_cast checks below account for the case when an unsupported backend
    // is stored in config. After this backend is replaced with the default one
    // upon preferences saving, the code below does not reset playerPtr with
    // another object of the same type.

#ifdef MAKE_FFMPEG_PLAYER
    Q_ASSERT( Config::InternalPlayerBackend::defaultBackend().isFfmpeg()
              && "Adjust the code below after changing the default backend." );

    if( !internalPlayerBackend.isQtmultimedia() )
    {
      if( !playerPtr || !qobject_cast< Ffmpeg::AudioPlayer * >( playerPtr.data() ) )
        playerPtr.reset( new Ffmpeg::AudioPlayer );
      return;
    }
#endif

#ifdef MAKE_QTMULTIMEDIA_PLAYER
    if( !playerPtr || !qobject_cast< MultimediaAudioPlayer * >( playerPtr.data() ) )
      playerPtr.reset( new MultimediaAudioPlayer );
    return;
#endif
  }

  QScopedPointer< ExternalAudioPlayer > externalPlayer( new ExternalAudioPlayer );
  setAudioPlaybackProgram( *externalPlayer );
  playerPtr.reset( externalPlayer.take() );
}

void AudioPlayerFactory::setAudioPlaybackProgram( ExternalAudioPlayer & externalPlayer )
{
  externalPlayer.setPlayerCommandLine( audioPlaybackProgram.trimmed() );
}
