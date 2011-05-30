/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QDir>
#include "externalviewer.hh"
#include "parsecmdline.hh"

using std::vector;

ExternalViewer::ExternalViewer( QObject * parent, vector< char > const & data,
                                QString const & extension,
                                QString const & viewerCmdLine_ )
    throw( exCantCreateTempFile ):
  QObject( parent ),
  tempFile( QDir::temp().filePath( QString( "gd-XXXXXXXX." ) + extension ) ),
  viewer( this ),
  viewerCmdLine( viewerCmdLine_ )
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

  QStringList args = parseCommandLine( viewerCmdLine );

  if ( !args.isEmpty() )
  {
    QString program = args.first();
    args.pop_front();
    args.push_back( tempFileName );
    viewer.start( program, args, QIODevice::NotOpen );
    if ( !viewer.waitForStarted() )
      throw exCantRunViewer( viewerCmdLine.toUtf8().data() );
  }
  else
    throw exCantRunViewer( tr( "the viewer program name is empty" ).toUtf8().data() );
}
