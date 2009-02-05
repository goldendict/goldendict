/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "externalviewer.hh"
#include <QDir>

using std::vector;

ExternalViewer::ExternalViewer( QObject * parent, vector< char > const & data,
                                QString const & extension,
                                QString const & viewerProgram_ )
    throw( exCantCreateTempFile ):
  QObject( parent ),
  tempFile( QDir::temp().filePath( QString( "gd-XXXXXXXX." ) + extension ) ),
  viewer( this ),
  viewerProgram( viewerProgram_ )
{
  if ( !tempFile.open() || tempFile.write( &data.front(), data.size() ) != data.size() )
    throw exCantCreateTempFile();

  tempFileName = tempFile.fileName(); // For some reason it loses it after it was closed()

  tempFile.close();

  printf( "%s\n", tempFile.fileName().toLocal8Bit().data() );

  connect( &viewer, SIGNAL( finished( int, QProcess::ExitStatus ) ),
           this, SLOT( viewerFinished( int, QProcess::ExitStatus ) ) );

  connect( this, SIGNAL( finished( ExternalViewer * ) ),
           &ExternalViewerDeleter::instance(), SLOT( deleteExternalViewer( ExternalViewer * ) ),
           Qt::QueuedConnection );
}

void ExternalViewer::start() throw( exCantRunViewer )
{
  viewer.start( viewerProgram, QStringList( tempFileName ), QIODevice::NotOpen );

  if ( !viewer.waitForStarted() )
    throw exCantRunViewer( viewerProgram.toStdString() );
}

void ExternalViewer::viewerFinished( int, QProcess::ExitStatus )
{
  emit finished( this );
}

ExternalViewerDeleter & ExternalViewerDeleter::instance()
{
  static ExternalViewerDeleter evd( 0 );

  return evd;
}

void ExternalViewerDeleter::deleteExternalViewer( ExternalViewer * e )
{
  printf( "Deleting external viewer\n" );

  delete e;
}

