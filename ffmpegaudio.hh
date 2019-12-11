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
  AudioService();
  ~AudioService();
  void playMemory( const char * ptr, int size );
  void stop();

  void init();
  void clean();

signals:
  void cancelPlaying( bool waitUntilFinished );
  void error( QString const & message );

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
