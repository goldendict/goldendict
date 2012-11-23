// Wrapper for bass.dll

#ifdef __WIN32

#include <QWidget>
#include <QApplication>

#include <memory.h>
#include "bass.hh"

BassAudioPlayer::BassAudioPlayer() :
  fBASS_Free( 0 ),
  currentHandle( 0 ),
  data( 0 ),
  hwnd( 0 )
{
  Mutex::Lock _( mt );

  if( ( hBass = LoadLibraryW( L"bass.dll" ) ) == 0 )
    return;
  for( ; ; )
  {
    fBASS_Init = ( pBASS_Init )GetProcAddress( hBass, "BASS_Init" );
    if( fBASS_Init == 0 )
      break;

    fBASS_Free = ( pBASS_Free )GetProcAddress( hBass, "BASS_Free" );
    if( fBASS_Free == 0 )
      break;

    fBASS_Stop = ( pBASS_Stop )GetProcAddress( hBass, "BASS_Stop" );
    if( fBASS_Stop == 0 )
      break;

    fBASS_ErrorGetCode = ( pBASS_ErrorGetCode )GetProcAddress( hBass, "BASS_ErrorGetCode" );
    if( fBASS_ErrorGetCode == 0 )
      break;

    fBASS_StreamCreateFile = ( pBASS_StreamCreateFile )GetProcAddress( hBass, "BASS_StreamCreateFile" );
    if( fBASS_StreamCreateFile == 0 )
      break;

    fBASS_StreamFree = ( pBASS_StreamFree )GetProcAddress( hBass, "BASS_StreamFree" );
    if( fBASS_StreamFree == 0 )
      break;

    fBASS_MusicLoad = ( pBASS_MusicLoad )GetProcAddress( hBass, "BASS_MusicLoad" );
    if( fBASS_MusicLoad == 0 )
      break;

    fBASS_MusicFree = ( pBASS_MusicFree )GetProcAddress( hBass, "BASS_MusicFree" );
    if( fBASS_MusicFree == 0 )
      break;

    fBASS_ChannelPlay = ( pBASS_ChannelPlay )GetProcAddress( hBass, "BASS_ChannelPlay" );
    if( fBASS_ChannelPlay == 0 )
      break;

    fBASS_ChannelStop = ( pBASS_ChannelStop )GetProcAddress( hBass, "BASS_ChannelStop" );
    if( fBASS_ChannelStop == 0 )
      break;

    QWidget *root = qApp->topLevelWidgets().value(0);
    hwnd = (HWND)root->winId();

    return;
  }
  FreeLibrary( hBass );
  hBass = 0;
}

BassAudioPlayer::~BassAudioPlayer()
{
  if( hBass )
  {
    if( currentHandle )
      fBASS_Stop();
    if( fBASS_Free )
      fBASS_Free();
    FreeLibrary( hBass );
    if( data )
      free( data );
  }
}

BOOL BassAudioPlayer::playMemory( const void * ptr, size_t size )
{
  Mutex::Lock _( mt );

  if( ptr == 0 || size == 0 )
    return( false );
  if( !canBeUsed() )
    return false;

  if( currentHandle )
  {
    fBASS_Stop();
    fBASS_Free();
    currentHandle = 0;
  }

  if( !fBASS_Init( -1, 44100, 0, hwnd, 0 ) )
    return false;

  if( data )
    free( data );
  if( ( data = malloc( size ) ) == 0 )
    return false;
  memcpy( data, ptr, size );

  currentType = STREAM;
  currentHandle = fBASS_StreamCreateFile( TRUE, data, 0, size, BASS_STREAM_PRESCAN );
  if( currentHandle == 0 )
  {
    currentHandle = fBASS_MusicLoad( TRUE, data, 0, size, BASS_STREAM_PRESCAN, 0 );
    currentType = MUSIC;
  }

  if( currentHandle )
    return( fBASS_ChannelPlay( currentHandle, TRUE ) );

  return false ;
}

BassAudioPlayer & BassAudioPlayer::instance()
{
  static BassAudioPlayer a;
  return a;
}

#endif

