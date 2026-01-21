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
  enum State { StoppedState, PlayingState };

  /// Stops current playback if any, copies the audio buffer at [data, data + size),
  /// then plays this buffer. It is safe to invalidate \p data after this function call.
  /// Always emits at least one stateChanged() signal.
  /// Returns an error message in case of immediate failure; an empty string
  /// in case of success.
  virtual QString play( const char * data, int size ) = 0;
  /// Stops current playback if any. Always emits stateChanged( StoppedState ).
  virtual void stop() = 0;

signals:
  /// Notifies of asynchronous errors.
  void error( QString message );

  /// Is emitted whenever playback is started or stopped.
  /// Note: this signal may be emitted more than once in a row with the same or
  /// different state values. Prohibiting such redundant signals would
  /// complicate implementations significantly.
  /// Note: fully qualified AudioPlayerInterface::State must be used below.
  /// Unqualified State breaks connections from other classes at runtime.
  void stateChanged( AudioPlayerInterface::State state );
};

typedef QScopedPointer< AudioPlayerInterface > AudioPlayerPtr;

#endif // AUDIOPLAYERINTERFACE_HH_INCLUDED
