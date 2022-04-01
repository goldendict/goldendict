/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __EXTERNALVIEWER_HH_INCLUDED__
#define __EXTERNALVIEWER_HH_INCLUDED__

#include <QObject>
#include <QTemporaryFile>
#include <QProcess>
#include "ex.hh"

/// An external viewer, opens resources in other programs
class ExternalViewer: public QObject
{
  Q_OBJECT

  QTemporaryFile tempFile;
  QProcess viewer;
  QString viewerCmdLine;
  QString tempFileName;

public:

  DEF_EX( Ex, "External viewer exception", std::exception )
  DEF_EX( exCantCreateTempFile, "Couldn't create temporary file.", Ex )
  DEF_EX_STR( exCantRunViewer, "Couldn't run external viewer:", Ex )

  ExternalViewer( const char * data, int size,
                  QString const & extension, QString const & viewerCmdLine,
                  QObject * parent = 0 )
    ;

  // Once this is called, the object will be deleted when it's done, even if
  // the function throws.
  void start() ;

  /// If the external process is running, requests its termination and returns
  /// false - expect the QObject::destroyed() signal to be emitted soon.
  /// If the external process is not running, returns true, the object
  /// destruction is not necessarily scheduled in this case.
  bool stop();
  /// Kills the process if it is running and waits for it to finish.
  void stopSynchronously();
};

/// Call this function instead of simply deleting viewer to prevent the
/// "QProcess: Destroyed while process X is still running." warning in log.
void stopAndDestroySynchronously( ExternalViewer * viewer );

#endif
