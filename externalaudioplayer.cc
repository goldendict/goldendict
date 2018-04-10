/* This file is (c) 2018 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "externalaudioplayer.hh"
#include "externalviewer.hh"

ExternalAudioPlayer::ExternalAudioPlayer() : exitingViewer( 0 )
{
}

ExternalAudioPlayer::~ExternalAudioPlayer()
{
  // Destroy viewers immediately to prevent memory and temporary file leaks
  // at application exit.

  // Set viewer to null first and foremost to make sure that onViewerDestroyed()
  // doesn't attempt to start viewer or mess the smart pointer up.
  stopAndDestroySynchronously( viewer.take() );

  stopAndDestroySynchronously( exitingViewer );
}

void ExternalAudioPlayer::setPlayerCommandLine( QString const & playerCommandLine_ )
{
  playerCommandLine = playerCommandLine_;
}

QString ExternalAudioPlayer::play( const char * data, int size )
{
  stop();

  Q_ASSERT( !viewer && "viewer must be null at this point for exception safety." );
  try
  {
    // Our destructor properly destroys viewers we remember about.
    // In the unlikely case that we call viewer.reset() during the application
    // exit, ~QObject() prevents leaks as this class is a parent of all viewers.
    viewer.reset( new ExternalViewer( data, size, "wav", playerCommandLine, this ) );
  }
  catch( const ExternalViewer::Ex & e )
  {
    return e.what();
  }

  if( exitingViewer )
    return QString(); // Will start later.
  return startViewer();
}

void ExternalAudioPlayer::stop()
{
  if( !exitingViewer && viewer && !viewer->stop() )
  {
    // Give the previous viewer a chance to stop before starting a new one.
    // This prevents overlapping audio and possible conflicts between
    // external program instances.
    // Graceful stopping is better than calling viewer.reset() because:
    //   1) the process gets a chance to clean up and save its state;
    //   2) there is no event loop blocking and consequently no (short) UI freeze
    //      while synchronously waiting for the external process to exit.
    exitingViewer = viewer.take();
  }
  else // viewer is either not started or already stopped -> simply destroy it.
    viewer.reset();
}

void ExternalAudioPlayer::onViewerDestroyed( QObject * destroyedViewer )
{
  if( exitingViewer == destroyedViewer )
  {
    exitingViewer = 0;
    if( viewer )
    {
      QString errorMessage = startViewer();
      if( !errorMessage.isEmpty() )
        emit error( errorMessage );
    }
  }
  else
  if( viewer.data() == destroyedViewer )
    viewer.take(); // viewer finished and died -> release ownership.
}

QString ExternalAudioPlayer::startViewer()
{
  Q_ASSERT( !exitingViewer && viewer );
  connect( viewer.data(), SIGNAL( destroyed( QObject * ) ),
           this, SLOT( onViewerDestroyed( QObject * ) ) );
  try
  {
    viewer->start();
  }
  catch( const ExternalViewer::Ex & e )
  {
    viewer.reset();
    return e.what();
  }
  return QString();
}
