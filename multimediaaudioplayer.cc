/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_QTMULTIMEDIA_PLAYER

#include <QByteArray>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QMediaContent>
#endif
#include "multimediaaudioplayer.hh"

MultimediaAudioPlayer::MultimediaAudioPlayer()
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  : player( 0, QMediaPlayer::StreamPlayback )
#endif
{
  typedef void( QMediaPlayer::* ErrorSignal )( QMediaPlayer::Error );
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  connect( &player, static_cast< ErrorSignal >( &QMediaPlayer::error ),
           this, &MultimediaAudioPlayer::onMediaPlayerError );
#else
  player.setAudioOutput(&audioOutput);

  connect( &player,  &QMediaPlayer::errorChanged ,
           this, &MultimediaAudioPlayer::onMediaPlayerError );
#endif
}

QString MultimediaAudioPlayer::play( const char * data, int size )
{
  stop();

  audioBuffer.setData( data, size );
  if( !audioBuffer.open( QIODevice::ReadOnly ) )
    return tr( "Couldn't open audio buffer for reading." );
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
  player.setSourceDevice (&audioBuffer );
#else
  player.setMedia( QMediaContent(), &audioBuffer );
#endif
  player.play();
  return QString();
}

void MultimediaAudioPlayer::stop()
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  player.setMedia( QMediaContent() ); // Forget about audioBuffer.
#endif
  audioBuffer.close();
  audioBuffer.setData( QByteArray() ); // Free memory.
}

void MultimediaAudioPlayer::onMediaPlayerError()
{
  emit error( player.errorString() );
}

#endif // MAKE_QTMULTIMEDIA_PLAYER
