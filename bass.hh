#ifndef __BASS_HH_INCLUDED__
#define __BASS_HH_INCLUDED__

#ifdef __WIN32

#include <windows.h>
#include "bass.h"
#include "mutex.hh"

// Used bass.dll functions

class BassAudioPlayer
{
public:
  bool canBeUsed()
  { return hBass != 0; }

  BOOL playMemory( const void * ptr, size_t size );

  static BassAudioPlayer & instance();

private:

  BassAudioPlayer();
  ~BassAudioPlayer();

  HMODULE hBass;

  // Some bass.dll functions

  typedef  BOOL BASSDEF( ( *pBASS_Init ) )( int , DWORD, DWORD, HWND, GUID * );
  typedef BOOL BASSDEF( ( *pBASS_Free ) )();
  typedef BOOL BASSDEF( ( *pBASS_Stop ) )();
  typedef int BASSDEF( ( *pBASS_ErrorGetCode ) )();
  typedef HSTREAM BASSDEF( ( *pBASS_StreamCreateFile ) )( BOOL, void *, QWORD, QWORD, DWORD );
  typedef BOOL BASSDEF( ( *pBASS_StreamFree ) )( HSTREAM );
  typedef HMUSIC BASSDEF( ( *pBASS_MusicLoad ) )( BOOL, void *, QWORD, DWORD, DWORD, DWORD );
  typedef BOOL BASSDEF( ( *pBASS_MusicFree ) )( HMUSIC );
  typedef BOOL BASSDEF( ( *pBASS_ChannelPlay ) )( DWORD, BOOL );
  typedef BOOL BASSDEF( ( *pBASS_ChannelStop ) )( DWORD );

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

  DWORD currentHandle;
  void * data;
  HWND hwnd;

  Mutex mt;

  enum SoundType { STREAM, MUSIC } currentType;
};

#endif

#endif // __BASS_HH_INCLUDED__
