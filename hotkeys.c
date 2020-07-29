#include <windows.h>
#include "hotkeys.h"
#include "stdio.h"

static LRESULT CALLBACK lowLevelKeyboardProc(int, WPARAM, LPARAM);

// Max number of hotkeys
#define MAX_HOTKEYS 5

// Max time interval between first and second part of hotkey (ms)
#define MAX_KEYS_TIME_INTERVAL 500

static HINSTANCE hInstance;
static HWND hGDWindow;
static HHOOK hKbdHook;

typedef struct HotkeyStruct
{
  DWORD key1;
  DWORD key2;
  DWORD mods;
  DWORD lasttime;
} HotkeyStruct;

static HotkeyStruct hotkeys[ MAX_HOTKEYS ];

__declspec (dllexport) void removeHook()
{
  if( hKbdHook )
  {
    UnhookWindowsHookEx( hKbdHook );
    hKbdHook = 0;
  }
}

__declspec (dllexport) BOOL setHook( HWND hwnd )
{
  hGDWindow = hwnd;
  removeHook();
  hKbdHook = SetWindowsHookEx( WH_KEYBOARD_LL, lowLevelKeyboardProc, hInstance, 0 );
  return hKbdHook != 0;
}

__declspec (dllexport) BOOL setHotkeys( DWORD key1, DWORD key2, DWORD modifiers, int nom )
{
  if( nom < 0 || nom >= MAX_HOTKEYS )
    return FALSE;
  hotkeys[ nom ].key1 = key1;
  hotkeys[ nom ].key2 = key2;
  hotkeys[ nom ].mods = modifiers;
  hotkeys[ nom ].lasttime = 0;
  return TRUE;
}

__declspec (dllexport) void clearHotkeys()
{
  int i;
  for( i = 0; i < MAX_HOTKEYS; i++ )
    memset( hotkeys + i, 0, sizeof( HotkeyStruct ) );
}

static BOOL isModifiersPressed( DWORD modifiers )
{
  int n = GetAsyncKeyState( VK_MENU ) & 0x8000;
  if( ( ( modifiers & MOD_ALT ) && n == 0 )
      || ( ( modifiers & MOD_ALT ) == 0 && n ) )
    return FALSE;

  n = GetAsyncKeyState( VK_SHIFT ) & 0x8000;
  if( ( ( modifiers & MOD_SHIFT ) && n == 0 )
      || ( ( modifiers & MOD_SHIFT ) == 0 && n ) )
    return FALSE;

  n = GetAsyncKeyState( VK_CONTROL ) & 0x8000;
  if( ( ( modifiers & MOD_CONTROL ) && n == 0 )
      || ( ( modifiers & MOD_CONTROL ) == 0 && n ) )
    return FALSE;

  n = ( GetAsyncKeyState( VK_LWIN ) & 0x8000 ) | ( GetAsyncKeyState( VK_RWIN ) & 0x8000 );
  if( ( ( modifiers & MOD_WIN ) && n == 0 )
      || ( ( modifiers & MOD_WIN ) == 0 && n ) )
    return FALSE;

  return TRUE;
}

static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
int i;
PKBDLLHOOKSTRUCT p;
BOOL stop = FALSE;

  if ( nCode < 0 ) return CallNextHookEx( hKbdHook, nCode, wParam, lParam );

  if( nCode == HC_ACTION )
  {
    p = (PKBDLLHOOKSTRUCT)lParam;

    for( ; ; )
    {
      // Check hotkeys

      if( wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN )
      {
        DWORD new_time = GetTickCount();

        // Check if key is second part of hotkey

        for( i = 0; i < MAX_HOTKEYS; i++ )
        {
          if( hotkeys[ i ].key1 == 0 )
            break;
          if( hotkeys[ i ].key2 == 0 || hotkeys[ i ].lasttime == 0 )
            continue;
          if( hotkeys[ i ].key2 == p->vkCode && isModifiersPressed( hotkeys[ i ].mods ) )
          {
            if( new_time - hotkeys[ i ].lasttime <= MAX_KEYS_TIME_INTERVAL )
            {
              // Hotkey completed
              // Clear all flags for first part

              int j;
              for( j = 0; j < MAX_HOTKEYS; j++ )
                hotkeys[ j ].lasttime = 0;

              PostMessage( hGDWindow, GD_HOTKEY_MESSAGE, (WPARAM)i, 0 );

              stop = TRUE;
              break;
            }
            else
            {
              // Interval exceeded, reset time
              hotkeys[ i ].lasttime = new_time;
              continue;
            }
          }
          else
          {
            // Key is not second part, clear flag
            hotkeys[ i ].lasttime = 0;
          }
        }
        if( stop )
          break;

        // Check if key is first part of hotkey

        for( i = 0; i < MAX_HOTKEYS; i++ )
        {
          if( hotkeys[ i ].key1 == 0 )
            break;
          if( hotkeys[ i ].key1 == p->vkCode && isModifiersPressed( hotkeys[ i ].mods ) )
          {
            // Match found
            if( hotkeys[ i ].key2 == 0 )
            {
              // No second part, hotkey completed

              // Clear all flags for first part
              int j;
              for( j = 0; j < MAX_HOTKEYS; j++ )
                hotkeys[ j ].lasttime = 0;

              PostMessage( hGDWindow, GD_HOTKEY_MESSAGE, (WPARAM)i, 0 );

              stop = TRUE;
              break;
            }
            else
            {
              // First part detected, need wait for second part
              hotkeys[ i ].lasttime = new_time;
              break;
            }
          }
        }
      }
      break;
    }
  }

  LRESULT result = CallNextHookEx(hKbdHook, nCode, wParam, lParam);
  return ( stop ? 1 : result );
}

BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
(void) reserved;
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        hInstance = hInst;
        break;

      case DLL_PROCESS_DETACH:
        removeHook();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
