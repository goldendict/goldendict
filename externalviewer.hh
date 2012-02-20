/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __EXTERNALVIEWER_HH_INCLUDED__
#define __EXTERNALVIEWER_HH_INCLUDED__

#include <QObject>
#include <QTemporaryFile>
#include <QProcess>
#include <vector>
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

  ExternalViewer( QObject * parent, std::vector< char > const & data,
                  QString const & extension, QString const & viewerCmdLine )
    throw( exCantCreateTempFile );

  // Once this is called, the object will be deleted when it's done, even if
  // the function throws.
  void start() throw( exCantRunViewer );
};

#endif

