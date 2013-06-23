#ifndef __FFMPEGAUDIO_HH_INCLUDED__
#define __FFMPEGAUDIO_HH_INCLUDED__

#ifndef DISABLE_INTERNAL_PLAYER

#include <QObject>
#include <QMutex>
#include <QAtomicInt>
#include <QByteArray>
#include <QThread>

namespace Ffmpeg
{

class AudioPlayer : public QObject
{
  Q_OBJECT

public:
  static AudioPlayer & instance();
  void playMemory( const void * ptr, int size );
  void stop();

signals:
  void cancelPlaying( bool waitUntilFinished );
  void error( QString const & message );

private:
  AudioPlayer();
  ~AudioPlayer();
  AudioPlayer( AudioPlayer const & );
  AudioPlayer & operator=( AudioPlayer const & );
};

class DecoderThread: public QThread
{
  Q_OBJECT

  static QMutex deviceMutex_;
  QAtomicInt isCancelled_;
  QByteArray audioData_;

public:
  DecoderThread( QByteArray const & audioData, QObject * parent );
  virtual ~DecoderThread();

public slots:
  void run();
  void cancel( bool waitUntilFinished );

signals:
  void error( QString const & message );
};

}

#endif // DISABLE_INTERNAL_PLAYER

#endif // __FFMPEGAUDIO_HH_INCLUDED__
