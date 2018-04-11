/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef EXTERNALAUDIOPLAYER_HH_INCLUDED
#define EXTERNALAUDIOPLAYER_HH_INCLUDED

#include <QScopedPointer>
#include <QString>
#include "audioplayerinterface.hh"

class ExternalViewer;

class ExternalAudioPlayer : public AudioPlayerInterface
{
  Q_OBJECT
public:
  ExternalAudioPlayer();
  ~ExternalAudioPlayer();
  /// \param playerCommandLine_ Will be used in future play() calls.
  void setPlayerCommandLine( QString const & playerCommandLine_ );

  virtual QString play( const char * data, int size );
  virtual void stop();

private slots:
  void onViewerDestroyed( QObject * destroyedViewer );

private:
  QString startViewer();

  QString playerCommandLine;
  ExternalViewer * exitingViewer; ///< If not null: points to the previous viewer,
                                  ///< the current viewer (if any) is not started yet
                                  ///< and waits for exitingViewer to be destroyed first.

  struct ScopedPointerDeleteLater
  {
    static void cleanup( QObject * p ) { if( p ) p->deleteLater(); }
  };
  // deleteLater() is safer because viewer actively participates in the QEventLoop.
  QScopedPointer< ExternalViewer, ScopedPointerDeleteLater > viewer;
};

#endif // EXTERNALAUDIOPLAYER_HH_INCLUDED
