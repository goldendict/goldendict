/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef MULTIMEDIAAUDIOPLAYER_HH_INCLUDED
#define MULTIMEDIAAUDIOPLAYER_HH_INCLUDED

#ifdef MAKE_QTMULTIMEDIA_PLAYER

#include <QBuffer>
#include <QMediaPlayer>
#include "audioplayerinterface.hh"

class MultimediaAudioPlayer : public AudioPlayerInterface
{
  Q_OBJECT
public:
  MultimediaAudioPlayer();

  virtual QString play( const char * data, int size );
  virtual void stop();

private slots:
  void onMediaPlayerError();

private:
  QBuffer audioBuffer;
  QMediaPlayer player; ///< Depends on audioBuffer.
};

#endif // MAKE_QTMULTIMEDIA_PLAYER

#endif // MULTIMEDIAAUDIOPLAYER_HH_INCLUDED
