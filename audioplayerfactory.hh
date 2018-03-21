/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef AUDIOPLAYERFACTORY_HH_INCLUDED
#define AUDIOPLAYERFACTORY_HH_INCLUDED

#include "audioplayerinterface.hh"
#include "config.hh"

class AudioPlayerFactory
{
  Q_DISABLE_COPY( AudioPlayerFactory )
public:
  explicit AudioPlayerFactory( Config::Preferences const & );
  void setPreferences( Config::Preferences const & );
  AudioPlayerPtr const & player() const { return playerPtr; }

private:
  void reset();
  bool useInternalPlayer;
  AudioPlayerPtr playerPtr;
};

#endif // AUDIOPLAYERFACTORY_HH_INCLUDED
