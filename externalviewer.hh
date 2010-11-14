/* This file is (c) 2008-2011 Konstantin Isakov <ikm@users.berlios.de>
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
  QString viewerProgram;
  QString tempFileName;

public:

  DEF_EX( Ex, "External viewer exception", std::exception )
  DEF_EX( exCantCreateTempFile, "Couldn't create temporary file.", Ex )
  DEF_EX_STR( exCantRunViewer, "Couldn't run external viewer:", Ex )

  ExternalViewer( QObject * parent, std::vector< char > const & data,
                  QString const & extension, QString const & viewerProgram )
    throw( exCantCreateTempFile );

  ~ExternalViewer();

  void start() throw( exCantRunViewer );

private slots:

  void viewerFinished( int, QProcess::ExitStatus );

signals:

  void finished( ExternalViewer * );
};

class ExternalViewerDeleter: public QObject
{
  Q_OBJECT

public:

  static ExternalViewerDeleter & instance();

public slots:

  void deleteExternalViewer( ExternalViewer * e );

private:

  ExternalViewerDeleter( QObject * parent ): QObject( parent )
  {}
};

#endif

