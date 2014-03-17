#include "mouseover.hh"
#include "utf8.hh"
#include <QCoreApplication>
#include <QDir>
#include <algorithm>

#ifdef Q_OS_WIN32
#undef WINVER
#define WINVER 0x0500
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>
#include "mouseover_win32/ThTypes.h"
#include "wordbyauto.hh"
#include "x64.hh"
#endif

MouseOver & MouseOver::instance()
{
  static MouseOver m;

  return m;
}

#ifdef Q_OS_WIN32
const UINT WM_MY_SHOW_TRANSLATION = WM_USER + 301;
static wchar_t className[] = L"GoldenDictMouseover";
typedef BOOL ( WINAPI  *ChangeWindowMessageFilterFunc )( UINT, DWORD );

#ifndef _MSC_VER
typedef struct tagCHANGEFILTERSTRUCT {
  DWORD cbSize;
  DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;
#endif

typedef BOOL ( WINAPI  *ChangeWindowMessageFilterExFunc )( HWND, UINT, DWORD, PCHANGEFILTERSTRUCT );

#endif // Q_OS_WIN32

#ifdef Q_OS_WIN32

#ifndef ConvertStringSecurityDescriptorToSecurityDescriptor

extern "C" BOOL WINAPI ConvertStringSecurityDescriptorToSecurityDescriptorW(
                        LPCWSTR StringSecurityDescriptor,
                        DWORD StringSDRevision,
                        PSECURITY_DESCRIPTOR *SecurityDescriptor,
                        PULONG SecurityDescriptorSize );

#endif

static void SetLowLabelToGDSynchroObjects()
{
// The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity
#define LOW_INTEGRITY_SDDL_SACL_W L"S:(ML;;NW;;;LW)"
//    DWORD dwErr = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;

    PACL pSacl = NULL; // not allocated
    BOOL fSaclPresent = FALSE;
    BOOL fSaclDefaulted = FALSE;
#ifdef Q_OS_WIN64
    LPCWSTR pwszMapFileName64 = L"GoldenDictTextOutHookSharedMem64";
#endif
    LPCWSTR pwszMapFileName = L"GoldenDictTextOutHookSharedMem";
    LPCWSTR pwszSpyMutexName = L"GoldenDictTextOutSpyMutex";

    if( ConvertStringSecurityDescriptorToSecurityDescriptorW( LOW_INTEGRITY_SDDL_SACL_W, 1 /* SDDL_REVISION_1 */, &pSD, NULL ) )
    {
        if( GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
        {
// Note that psidOwner, psidGroup, and pDacl are
// all NULL and set the new LABEL_SECURITY_INFORMATION

#ifdef Q_OS_WIN64
           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszMapFileName64,
                     SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);
#endif
           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszMapFileName,
                    SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);

           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszSpyMutexName,
                    SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);

        }
        LocalFree(pSD);
    }
}

#endif // Q_OS_WIN32

MouseOver::MouseOver() :
  pPref(NULL)
{
#ifdef Q_OS_WIN32
HMODULE hm;
ChangeWindowMessageFilterFunc changeWindowMessageFilterFunc = NULL;
ChangeWindowMessageFilterExFunc changeWindowMessageFilterExFunc = NULL;
  mouseOverEnabled = false;
  
  ThTypes_Init();
  memset( GlobalData, 0, sizeof( TGlobalDLLData ) );
#ifdef Q_OS_WIN64
  memset( GlobalData32, 0, sizeof( TGlobalDLLData32 ) );
#endif
//  strcpy( GlobalData->LibName,
//    QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() ).filePath( "GdTextOutHook.dll" ) ).toLocal8Bit().data() );
#ifdef Q_OS_WIN64
  QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() ).filePath( "GdTextOutHook64.dll" ) ).toWCharArray( GlobalData->LibName );
  QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() + "/x86" ).filePath( "GdTextOutHook.dll" ) ).toWCharArray( GlobalData32->LibName );
#else
  QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() ).filePath( "GdTextOutHook.dll" ) ).toWCharArray( GlobalData->LibName );
#endif

  // Create the window to recive spying results to

  WNDCLASSEX wcex;

  wcex.cbSize = sizeof( WNDCLASSEX );

  wcex.style            = 0;
  wcex.lpfnWndProc      = ( WNDPROC ) eventHandler;
  wcex.cbClsExtra       = 0;
  wcex.cbWndExtra       = 0;
  wcex.hInstance        = GetModuleHandle( 0 );
  wcex.hIcon            = NULL;
  wcex.hCursor          = NULL,
  wcex.hbrBackground    = NULL;
  wcex.lpszMenuName     = NULL;
  wcex.lpszClassName    = className;
  wcex.hIconSm          = NULL;

  RegisterClassEx( &wcex );

  GlobalData->ServerWND = CreateWindow( className, L"", 0, 0, 0, 0, 0, GetDesktopWindow(), NULL, GetModuleHandle( 0 ), 0 );
#ifdef Q_OS_WIN64
  GlobalData32->ServerWND = HandleToLong( GlobalData->ServerWND );
#endif

#ifdef Q_OS_WIN64
  spyDll = LoadLibrary( QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() ).filePath( "GdTextOutSpy64.dll" ) ).toStdWString().c_str() );
#else
  spyDll = LoadLibrary( QDir::toNativeSeparators( QDir( QCoreApplication::applicationDirPath() ).filePath( "GdTextOutSpy.dll" ) ).toStdWString().c_str() );
#endif

  if ( spyDll )
    activateSpyFn = ( ActivateSpyFn ) GetProcAddress( spyDll, "ActivateTextOutSpying" );

// Allow messages from low intehrity process - for Vista and Win7
  hm = GetModuleHandle( __TEXT("user32.dll"));
  if ( hm != NULL ) {
      changeWindowMessageFilterExFunc = (ChangeWindowMessageFilterExFunc)GetProcAddress( hm, "ChangeWindowMessageFilterEx" );
      if( changeWindowMessageFilterExFunc ) {
          CHANGEFILTERSTRUCT cfs = { sizeof( CHANGEFILTERSTRUCT ), 0 };
          changeWindowMessageFilterExFunc( GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 1 /* MSGFLT_ALLOW */, &cfs );
      } else {
          changeWindowMessageFilterFunc = (ChangeWindowMessageFilterFunc)GetProcAddress( hm, "ChangeWindowMessageFilter" );
          if( changeWindowMessageFilterFunc )
              changeWindowMessageFilterFunc( WM_MY_SHOW_TRANSLATION, 1 /* MSGFLT_ADD */ );
      }
  }

//Allow object access from low intehrity process - for Vista and Win7
  SetLowLabelToGDSynchroObjects();

#endif
}

void MouseOver::enableMouseOver()
{
#ifdef Q_OS_WIN32
  if ( !mouseOverEnabled && activateSpyFn )
  {
    activateSpyFn( true );
    installx64Hooks();
    mouseOverEnabled = true;
  }
#endif
}

void MouseOver::disableMouseOver()
{
#ifdef Q_OS_WIN32
  if ( mouseOverEnabled && activateSpyFn )
  {
    activateSpyFn( false );
    removex64Hooks();
    mouseOverEnabled = false;
  }
#endif
}

#ifdef Q_OS_WIN32

LRESULT MouseOver::makeScanBitMask()
{
LRESULT res = 0;
    if( pPref == NULL )
        return 0;
    if( !pPref->enableScanPopupModifiers || checkModifiersPressed( pPref->scanPopupModifiers ) ) {
        res = GD_FLAG_METHOD_STANDARD;
        if( pPref->scanPopupUseUIAutomation !=0 )
            res |= GD_FLAG_METHOD_UI_AUTOMATION;
        if( pPref->scanPopupUseIAccessibleEx !=0 )
            res |= GD_FLAG_METHOD_IACCESSIBLEEX;
        if( pPref->scanPopupUseGDMessage !=0 )
            res |= GD_FLAG_METHOD_GD_MESSAGE;
    }
    return res;
}

#ifdef Q_OS_WIN64
#define Global_Data GlobalData32
#else
#define Global_Data GlobalData
#endif

LRESULT CALLBACK MouseOver::eventHandler( HWND hwnd_, UINT msg,
                                          WPARAM wparam, LPARAM lparam )
{
  if ( msg == WM_MY_SHOW_TRANSLATION )
  {
    LRESULT res = instance().makeScanBitMask();

    if( res == 0 )
        return 0;  // Don't handle word without necessity

    if( wparam != 0) //Ask for methods of word retrieving
        return res;

    int wordSeqPos = 0;
    QString wordSeq;

#ifdef Q_OS_WIN64
    HWND hwnd = ( HWND )LongToHandle( Global_Data->LastWND );
#else
    HWND hwnd = Global_Data->LastWND;
#endif

    if( Global_Data->CurMod.WordLen == 0)
    {
        if( ( res & GD_FLAG_METHOD_UI_AUTOMATION ) == 0 )
            return 0;
        POINT pt = Global_Data->LastPt;
        WCHAR *pwstr = gdGetWordAtPointByAutomation( pt );
        if( pwstr == NULL ) return 0;
        wordSeq = QString::fromWCharArray( pwstr );
    }
    else
    {

        // Is the string in utf8 or in locale encoding?

        gd::wchar testBuf[ 256 ];

        long result = Utf8::decode( Global_Data->CurMod.MatchedWord,
                                    strlen( Global_Data->CurMod.MatchedWord ),
                                    testBuf );

        if ( result >= 0 )
        {
          // It seems to be
          QString begin = QString::fromUtf8( Global_Data->CurMod.MatchedWord,
                                             Global_Data->CurMod.BeginPos ).normalized( QString::NormalizationForm_C );

          QString end = QString::fromUtf8( Global_Data->CurMod.MatchedWord +
                                           Global_Data->CurMod.BeginPos ).normalized( QString::NormalizationForm_C );

          wordSeq = begin + end;
          wordSeqPos = begin.size();
        }
        else
        {
        // It's not, so interpret it as in local encoding
            QString begin = QString::fromLocal8Bit( Global_Data->CurMod.MatchedWord,
                                                    Global_Data->CurMod.BeginPos ).normalized( QString::NormalizationForm_C );

            QString end = QString::fromLocal8Bit( Global_Data->CurMod.MatchedWord +
                                                  Global_Data->CurMod.BeginPos ).normalized( QString::NormalizationForm_C );

            wordSeq = begin + end;
            wordSeqPos = begin.size();
        }
    }

    // Now locate the word inside the sequence

    QString word;

    if ( wordSeq[ wordSeqPos ].isSpace() )
    {
      // Currently we ignore such cases
      return 0;
    }
    else
    if ( !wordSeq[ wordSeqPos ].isLetterOrNumber() )
    {
      // Special case: the cursor points to something which doesn't look like a
      // middle of the word -- assume that it's something that joins two words
      // together.

      int begin = wordSeqPos;

      for( ; begin; --begin )
        if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
          break;

      int end = wordSeqPos;

      while( ++end < wordSeq.size() )
        if ( !wordSeq[ end ].isLetterOrNumber() )
          break;

      if ( end - begin == 1 )
      {
        // Well, turns out it was just a single non-letter char, discard it
        return 0;
      }

      word = wordSeq.mid( begin, end - begin );
    }
    else
    {
      // Cursor points to a letter -- cut the word it points to

      int begin = wordSeqPos;

      for( ; begin; --begin )
        if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
          break;

      int end = wordSeqPos;

      while( ++end < wordSeq.size() )
      {
        if ( !wordSeq[ end ].isLetterOrNumber() )
          break;
      }
      word = wordSeq.mid( begin, end - begin );
    }

    // See if we have an RTL char. Reverse the whole string if we do.

    if( lparam == 0 )
    {
      for( int x = 0; x < word.size(); ++x )
      {
        QChar::Direction d = word[ x ].direction();

        if ( d == QChar::DirR || d == QChar::DirAL ||
             d == QChar::DirRLE || d == QChar::DirRLO )
        {
          std::reverse( word.begin(), word.end() );
          break;
        }
      }
    }

    bool forcePopup = false;
    forcePopup = emit instance().isGoldenDictWindow( hwnd );
    emit instance().hovered( word, forcePopup );
    return 0;
  }

  return DefWindowProc( hwnd_, msg, wparam, lparam );
}

#endif

MouseOver::~MouseOver()
{
#ifdef Q_OS_WIN32

  disableMouseOver();

  FreeLibrary( spyDll );

  DestroyWindow( GlobalData->ServerWND );

  UnregisterClass( className, GetModuleHandle( 0 ) );

  ThTypes_End();

#endif
}

