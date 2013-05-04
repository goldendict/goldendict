// Wrapper for bass.dll
#include "bass.hh"
#include "dprintf.hh"

#include <QWidget>
#include <QApplication>

#include <stdio.h>
#include <memory.h>
#include <string>
#ifdef Q_OS_LINUX
#include <dlfcn.h>
#endif

using std::string;

BassAudioPlayer::BassAudioPlayer() :
  fBASS_Free( 0 ),
  currentHandle( 0 ),
  data( 0 ),
  wid( 0 ),
  spxPluginHandle( 0 )
{
  Mutex::Lock _( mt );

  string bassLibName;
  string libSuffix;

#if defined(Q_OS_WIN32)
  bassLibName = "bass";
  libSuffix = ".dll";
#else // defined(Q_OS_WIN32)
  bassLibName = "libbass";
# if defined(Q_OS_MAC)
  libSuffix = ".dylib";
# else // defined(Q_OS_MAC)
  libSuffix = ".so";
# endif // defined(Q_OS_MAC)
#endif // defined(Q_OS_WIN32)

  if ( !bassLib.load( string( bassLibName + libSuffix ).c_str() ) )
    return;

  for( ; ; )
  {
    fBASS_Init = ( pBASS_Init )bassLib.getSymbol( "BASS_Init" );
    if( fBASS_Init == 0 )
      break;

    fBASS_Free = ( pBASS_Free )bassLib.getSymbol( "BASS_Free" );
    if( fBASS_Free == 0 )
      break;

    fBASS_Stop = ( pBASS_Stop )bassLib.getSymbol( "BASS_Stop" );
    if( fBASS_Stop == 0 )
      break;

    fBASS_ErrorGetCode = ( pBASS_ErrorGetCode )bassLib.getSymbol( "BASS_ErrorGetCode" );
    if( fBASS_ErrorGetCode == 0 )
      break;

    fBASS_StreamCreateFile = ( pBASS_StreamCreateFile )bassLib.getSymbol( "BASS_StreamCreateFile" );
    if( fBASS_StreamCreateFile == 0 )
      break;

    fBASS_StreamFree = ( pBASS_StreamFree )bassLib.getSymbol( "BASS_StreamFree" );
    if( fBASS_StreamFree == 0 )
      break;

    fBASS_MusicLoad = ( pBASS_MusicLoad )bassLib.getSymbol( "BASS_MusicLoad" );
    if( fBASS_MusicLoad == 0 )
      break;

    fBASS_MusicFree = ( pBASS_MusicFree )bassLib.getSymbol( "BASS_MusicFree" );
    if( fBASS_MusicFree == 0 )
      break;

    fBASS_ChannelPlay = ( pBASS_ChannelPlay )bassLib.getSymbol( "BASS_ChannelPlay" );
    if( fBASS_ChannelPlay == 0 )
      break;

    fBASS_ChannelStop = ( pBASS_ChannelStop )bassLib.getSymbol( "BASS_ChannelStop" );
    if( fBASS_ChannelStop == 0 )
      break;

    fBASS_PluginLoad = ( pBASS_PluginLoad )bassLib.getSymbol( "BASS_PluginLoad" );
    if ( fBASS_PluginLoad == 0 )
      break;

    fBASS_PluginFree = ( pBASS_PluginFree )bassLib.getSymbol( "BASS_PluginFree" );
    if ( fBASS_PluginFree == 0 )
      break;

    string spxPluginName = bassLibName + "_spx" + libSuffix;

    #ifdef Q_OS_LINUX
    {
      // BASS_PluginLoad() loads plugin from absolute path or relative path, it won't
      // search any library path, so make a full plugin path here, ugly though.
      Dl_info dlInfo;
      SharedLibrary bassSpxLib( spxPluginName.c_str() );
      void * proc = bassSpxLib.getSymbol( "BASS_SPX_StreamCreateFile" );
      if ( proc && dladdr( proc, &dlInfo ) )
      {
        spxPluginName = dlInfo.dli_fname;
      }
    }
    #endif

    spxPluginHandle = fBASS_PluginLoad( spxPluginName.c_str(), 0 );
    if ( spxPluginHandle == 0 )
    {
      DPRINTF( "Load \"%s\" failed, error text: %s\n", spxPluginName.c_str(),
               errorText( fBASS_ErrorGetCode() ) );
    }

    return;
  }

  bassLib.unload();
}

BassAudioPlayer::~BassAudioPlayer()
{
  if( bassLib.loaded() )
  {
    if( currentHandle )
      fBASS_Stop();
    if ( spxPluginHandle )
      fBASS_PluginFree( spxPluginHandle );
    if( fBASS_Free )
      fBASS_Free();
    bassLib.unload();
    if( data )
      free( data );
  }
}

BOOL BassAudioPlayer::playMemory( const void * ptr, size_t size, int * errorCodePtr )
{
  Mutex::Lock _( mt );
  if( errorCodePtr )
    *errorCodePtr = -2;

  if( ptr == 0 || size == 0 )
    return( false );
  if( !canBeUsed() )
    return false;

  if( currentHandle )
  {
    fBASS_Stop();
    currentHandle = 0;
  }

  fBASS_Free();

  if( !fBASS_Init( -1, 44100, 0, wid, 0 ) )
  {
    if( errorCodePtr )
      *errorCodePtr = fBASS_ErrorGetCode();
    return false;
  }

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
  {
    bool res = fBASS_ChannelPlay( currentHandle, TRUE );
    if( !res && errorCodePtr != 0 )
      *errorCodePtr = fBASS_ErrorGetCode();
    return res;
  }

  if( errorCodePtr )
    *errorCodePtr = fBASS_ErrorGetCode();

  return false ;
}

BassAudioPlayer & BassAudioPlayer::instance()
{
  static BassAudioPlayer a;
  return a;
}

const char * BassAudioPlayer::errorText( int errorCode )
{
  switch( errorCode )
  {
    case 0:  return( "BASS_OK" );
    case 1:  return( "BASS_ERROR_MEM" );
    case 2:  return( "BASS_ERROR_FILEOPEN" );
    case 3:  return( "BASS_ERROR_DRIVER" );
    case 4:  return( "BASS_ERROR_BUFLOST" );
    case 5:  return( "BASS_ERROR_HANDLE" );
    case 6:  return( "BASS_ERROR_FORMAT" );
    case 7:  return( "BASS_ERROR_POSITION" );
    case 8:  return( "BASS_ERROR_INIT" );
    case 9:  return( "BASS_ERROR_START" );
    case 14: return( "BASS_ERROR_ALREADY" );
    case 18: return( "BASS_ERROR_NOCHAN" );
    case 19: return( "BASS_ERROR_ILLTYPE" );
    case 20: return( "BASS_ERROR_ILLPARAM" );
    case 21: return( "BASS_ERROR_NO3D" );
    case 22: return( "BASS_ERROR_NOEAX" );
    case 23: return( "BASS_ERROR_DEVICE" );
    case 24: return( "BASS_ERROR_NOPLAY" );
    case 25: return( "BASS_ERROR_FREQ" );
    case 27: return( "BASS_ERROR_NOTFILE" );
    case 29: return( "BASS_ERROR_NOHW" );
    case 31: return( "BASS_ERROR_EMPTY" );
    case 32: return( "BASS_ERROR_NONET" );
    case 33: return( "BASS_ERROR_CREATE" );
    case 34: return( "BASS_ERROR_NOFX" );
    case 37: return( "BASS_ERROR_NOTAVAIL");
    case 38: return( "BASS_ERROR_DECODE" );
    case 39: return( "BASS_ERROR_DX" );
    case 40: return( "BASS_ERROR_TIMEOUT" );
    case 41: return( "BASS_ERROR_FILEFORM" );
    case 42: return( "BASS_ERROR_SPEAKER" );
    case 43: return( "BASS_ERROR_VERSION" );
    case 44: return( "BASS_ERROR_CODEC" );
    case 45: return( "BASS_ERROR_ENDED" );
    case 46: return( "BASS_ERROR_BUSY" );
    case -1: return( "BASS_ERROR_UNKNOWN" );
  }
  return "Unknown error";
}
