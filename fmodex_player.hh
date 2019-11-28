/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef FMODEX_PLAYER_HH_INCLUDED
#define FMODEX_PLAYER_HH_INCLUDED

#ifdef MAKE_FMODEX_PLAYER

#include <QBuffer>
#include "audioplayerinterface.hh"
#include "fmod.h"
#include "fmod_errors.h"

class FmodexAudioPlayer : public AudioPlayerInterface
{
  Q_OBJECT
public:
  FmodexAudioPlayer();
  ~FmodexAudioPlayer();

  virtual QString play( const char * data, int size );
  virtual void stop();

private slots:

private:
  void clean();
  bool ERRCHECK(FMOD_RESULT);
private:
  FMOD_SYSTEM     *system;
  FMOD_SOUND       *sound;
  FMOD_CHANNEL    *channel;
};

#endif // MAKE_FMODEX_PLAYER

#endif // FMODEX_PLAYER_HH_INCLUDED
