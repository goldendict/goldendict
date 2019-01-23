/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef AUDIOPLAYERFACTORY_HH_INCLUDED
#define AUDIOPLAYERFACTORY_HH_INCLUDED

#include "audioplayerinterface.hh"
#include "config.hh"

class ExternalAudioPlayer;

class AudioPlayerFactory
{
  Q_DISABLE_COPY( AudioPlayerFactory )
public:
  explicit AudioPlayerFactory( Config::Preferences const & );
  void setPreferences( Config::Preferences const & );
  /// The returned reference to a smart pointer is valid as long as this object
  /// exists. The pointer to the owned AudioPlayerInterface may change after the
  /// call to setPreferences(), but it is guaranteed to never be null.
  AudioPlayerPtr const & player() const { return playerPtr; }

private:
  void reset();
  void setAudioPlaybackProgram( ExternalAudioPlayer & externalPlayer );

  bool useInternalPlayer;
  Config::InternalPlayerBackend internalPlayerBackend;
  QString audioPlaybackProgram;
  AudioPlayerPtr playerPtr;
};

#endif // AUDIOPLAYERFACTORY_HH_INCLUDED
