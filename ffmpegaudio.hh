#ifndef __FFMPEGAUDIO_HH_INCLUDED__
#define __FFMPEGAUDIO_HH_INCLUDED__

#ifdef MAKE_FFMPEG_PLAYER

#include <QObject>
#include <QMutex>
#include <QAtomicInt>
#include <QByteArray>
#include <QThread>

namespace Ffmpeg
{

class AudioService : public QObject
{
  Q_OBJECT

public:
  static AudioService & instance();
  void playMemory( const char * ptr, int size );
  void stop();

signals:
  void cancelPlaying( bool waitUntilFinished );
  void error( QString const & message );
  /// Is emitted when playback actually ends (not when stop is requested).
  void playbackStopped();

private slots:
  void onThreadDestroyed();

private:
  AudioService();
  ~AudioService();

  int runningThreadCount;
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

#endif // MAKE_FFMPEG_PLAYER

#endif // __FFMPEGAUDIO_HH_INCLUDED__
