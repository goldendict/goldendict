/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_QTMULTIMEDIA_PLAYER

#include <QByteArray>
#include <QMediaContent>
#include "multimediaaudioplayer.hh"

MultimediaAudioPlayer::MultimediaAudioPlayer() :
  player( 0, QMediaPlayer::StreamPlayback )
{
  typedef void( QMediaPlayer::* ErrorSignal )( QMediaPlayer::Error );
  connect( &player, static_cast< ErrorSignal >( &QMediaPlayer::error ),
           this, &MultimediaAudioPlayer::onMediaPlayerError );

  connect( &player, &QMediaPlayer::stateChanged,
           this, &MultimediaAudioPlayer::onMediaPlayerStateChanged );
}

QString MultimediaAudioPlayer::play( const char * data, int size )
{
  stop();

  audioBuffer.setData( data, size );
  if( !audioBuffer.open( QIODevice::ReadOnly ) )
    return tr( "Couldn't open audio buffer for reading." );

  player.setMedia( QMediaContent(), &audioBuffer );
  player.play();
  return QString();
}

void MultimediaAudioPlayer::stop()
{
  player.setMedia( QMediaContent() ); // Forget about audioBuffer.
  audioBuffer.close();
  audioBuffer.setData( QByteArray() ); // Free memory.

  emit stateChanged( StoppedState ); // Always emit the signal as promised by our API.
}

void MultimediaAudioPlayer::onMediaPlayerError()
{
  emit error( player.errorString() );
}

void MultimediaAudioPlayer::onMediaPlayerStateChanged( QMediaPlayer::State state )
{
  emit stateChanged( state == QMediaPlayer::PlayingState ? PlayingState : StoppedState );
}

#endif // MAKE_QTMULTIMEDIA_PLAYER
