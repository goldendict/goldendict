/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QDir>
#include "externalviewer.hh"

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
}

void ExternalViewer::start() throw( exCantRunViewer )
{
  connect( &viewer, SIGNAL( finished( int, QProcess::ExitStatus ) ),
           this, SLOT( deleteLater() ) );
  connect( &viewer, SIGNAL( error( QProcess::ProcessError ) ),
           this, SLOT( deleteLater() ) );

  viewer.start( viewerProgram, QStringList( tempFileName ), QIODevice::NotOpen );

  if ( !viewer.waitForStarted() )
    throw exCantRunViewer( viewerProgram.toStdString() );
}
