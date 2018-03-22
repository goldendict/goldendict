/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QScopedPointer>
#include <QObject>
#include "audioplayerfactory.hh"
#include "ffmpegaudioplayer.hh"
#include "externalaudioplayer.hh"
#include "gddebug.hh"

AudioPlayerFactory::AudioPlayerFactory( Config::Preferences const & p ) :
  useInternalPlayer( p.useInternalPlayer ),
  audioPlaybackProgram( p.audioPlaybackProgram )
{
  reset();
}

void AudioPlayerFactory::setPreferences( Config::Preferences const & p )
{
  if( p.useInternalPlayer != useInternalPlayer )
  {
    useInternalPlayer = p.useInternalPlayer;
    audioPlaybackProgram = p.audioPlaybackProgram;
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
#ifndef DISABLE_INTERNAL_PLAYER
  if( useInternalPlayer )
    playerPtr.reset( new Ffmpeg::AudioPlayer );
  else
#endif
  {
    QScopedPointer< ExternalAudioPlayer > externalPlayer( new ExternalAudioPlayer );
    setAudioPlaybackProgram( *externalPlayer );
    playerPtr.reset( externalPlayer.take() );
  }
}

void AudioPlayerFactory::setAudioPlaybackProgram( ExternalAudioPlayer & externalPlayer )
{
  externalPlayer.setPlayerCommandLine( audioPlaybackProgram.trimmed() );
}
