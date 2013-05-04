#ifndef __BASS_HH_INCLUDED__
#define __BASS_HH_INCLUDED__

#include "bass.h"
#include "mutex.hh"
#include "sharedlibrary.hh"

#include <QWidget>

#ifdef Q_OS_WIN32
#include <wtypes.h>
#endif

// Used bass.dll functions

class BassAudioPlayer
{
public:
  bool canBeUsed()
  { return bassLib.loaded(); }

  BOOL playMemory( const void * ptr, size_t size, int *errorCodePtr = 0 );
  const char * errorText( int errorCode );
  void setMainWindow( WId wid_ )
  {
#ifdef Q_OS_WIN32
    wid = ( HWND )wid_;
#else
    wid = ( void * )wid_;
#endif
  }

  static BassAudioPlayer & instance();

private:

  BassAudioPlayer();
  ~BassAudioPlayer();

  SharedLibrary bassLib;

  // Some bass.dll functions

#ifdef Q_OS_WIN32
  typedef BOOL BASSDEF( ( *pBASS_Init ) )( int , DWORD, DWORD, HWND, GUID * );
#else
  typedef BOOL BASSDEF( ( *pBASS_Init ) )( int , DWORD, DWORD, void *, void * );
#endif

  typedef BOOL BASSDEF( ( *pBASS_Free ) )();
  typedef BOOL BASSDEF( ( *pBASS_Stop ) )();
  typedef int BASSDEF( ( *pBASS_ErrorGetCode ) )();
  typedef HSTREAM BASSDEF( ( *pBASS_StreamCreateFile ) )( BOOL, void *, QWORD, QWORD, DWORD );
  typedef BOOL BASSDEF( ( *pBASS_StreamFree ) )( HSTREAM );
  typedef HMUSIC BASSDEF( ( *pBASS_MusicLoad ) )( BOOL, void *, QWORD, DWORD, DWORD, DWORD );
  typedef BOOL BASSDEF( ( *pBASS_MusicFree ) )( HMUSIC );
  typedef BOOL BASSDEF( ( *pBASS_ChannelPlay ) )( DWORD, BOOL );
  typedef BOOL BASSDEF( ( *pBASS_ChannelStop ) )( DWORD );
  typedef BOOL BASSDEF( ( *pBASS_PluginLoad ) )( const char *, DWORD );
  typedef BOOL BASSDEF( ( *pBASS_PluginFree ) )( HPLUGIN );

  pBASS_Init fBASS_Init;
  pBASS_Free fBASS_Free;
  pBASS_Stop fBASS_Stop;
  pBASS_StreamCreateFile fBASS_StreamCreateFile;
  pBASS_ErrorGetCode fBASS_ErrorGetCode;
  pBASS_StreamFree fBASS_StreamFree;
  pBASS_MusicLoad fBASS_MusicLoad;
  pBASS_MusicFree fBASS_MusicFree;
  pBASS_ChannelPlay fBASS_ChannelPlay;
  pBASS_ChannelStop fBASS_ChannelStop;
  pBASS_PluginLoad fBASS_PluginLoad;
  pBASS_PluginFree fBASS_PluginFree;

  DWORD currentHandle;
  void * data;
#ifdef Q_OS_WIN32
  HWND wid;
#else
  void * wid;
#endif
  HPLUGIN spxPluginHandle;

  Mutex mt;

  enum SoundType { STREAM, MUSIC } currentType;
};

#endif // __BASS_HH_INCLUDED__
