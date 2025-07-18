/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef AUDIOPLAYERINTERFACE_HH_INCLUDED
#define AUDIOPLAYERINTERFACE_HH_INCLUDED

#include <QScopedPointer>
#include <QString>
#include <QObject>

class AudioPlayerInterface : public QObject
{
  Q_OBJECT
public:
  /// Stops current playback if any, copies the audio buffer at [data, data + size),
  /// then plays this buffer. It is safe to invalidate \p data after this function call.
  /// Returns an error message in case of immediate failure; an empty string
  /// in case of success.
  virtual QString play( const char * data, int size ) = 0;
  /// Stops current playback if any.
  virtual void stop() = 0;

signals:
  /// Notifies of asynchronous errors.
  void error( QString message );
};

typedef QScopedPointer< AudioPlayerInterface > AudioPlayerPtr;

#endif // AUDIOPLAYERINTERFACE_HH_INCLUDED
