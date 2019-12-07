/* This file is (c) 2019 nonwill <nonwill@hotmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file
 * FMOD NON-COMMERCIAL LICENSE
 * FMOD Sound System, copyright ? Firelight Technologies Pty, Ltd., 1994-2016. */

#ifndef FMODEX_PLAYER_HH_INCLUDED
#define FMODEX_PLAYER_HH_INCLUDED

#ifdef MAKE_FMODEX_PLAYER

#include "audioplayerinterface.hh"
#include <QLibrary>

class FmodexAudioPlayer : public AudioPlayerInterface
{
  Q_OBJECT
public:
  FmodexAudioPlayer();
  ~FmodexAudioPlayer();

  virtual QString play( const char * data, int size );
  virtual void stop();
  static bool available();
private slots:

private:
  void clean();
  bool ERRCHECK(int);
private:
  static const QString fmodex_dyl_name;
  struct FMOD_EX_API;
  FMOD_EX_API *fmodex;
  QLibrary fmodex_dl;

};

#endif // MAKE_FMODEX_PLAYER

#endif // FMODEX_PLAYER_HH_INCLUDED
