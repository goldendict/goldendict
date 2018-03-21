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
  virtual void play( const char * data, int size ) = 0;
  virtual void stop() = 0;

signals:
  void error( QString message );
};

typedef QScopedPointer< AudioPlayerInterface > AudioPlayerPtr;

#endif // AUDIOPLAYERINTERFACE_HH_INCLUDED
