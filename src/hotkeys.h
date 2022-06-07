#ifndef __HOTKEYS_H_INCLUDED__
#define __HOTKEYS_H_INCLUDED__

#include <windows.h>

// Message if hotkey completed; WPARAM - hotkey number
#define GD_HOTKEY_MESSAGE ( WM_APP + 1 )

typedef BOOL ( *setHookProc )( HWND hwnd );
typedef void ( *removeHookProc )();
typedef BOOL ( *setHotkeysProc )( DWORD, DWORD, DWORD, int );
typedef void ( *clearHotkeysProc )();

#endif // __HOTKEYS_H_INCLUDED__
