/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "audioplayerfactory.hh"
#include "ffmpegaudioplayer.hh"

AudioPlayerFactory::AudioPlayerFactory( Config::Preferences const & p ) :
  useInternalPlayer( p.useInternalPlayer )
{
  reset();
}

void AudioPlayerFactory::setPreferences( Config::Preferences const & p )
{
  if( p.useInternalPlayer != useInternalPlayer )
  {
    useInternalPlayer = p.useInternalPlayer;
    reset();
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
    playerPtr.reset();
  }
}
