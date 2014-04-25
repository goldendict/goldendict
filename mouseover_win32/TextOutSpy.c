#include <tchar.h>
#include <windowsx.h>
#include "TextOutSpy.h"
#include "ThTypes.h"
#include "GDDataTranfer.h"
#include "GetWordByIAccEx.h"

const int MOUSEOVER_INTERVAL = 300;
const int REQUEST_MESSAGE_INTERVAL = 500;
const int WM_MY_SHOW_TRANSLATION = WM_USER + 301;

HINSTANCE g_hInstance = NULL;
HANDLE hSynhroMutex = 0;
HINSTANCE hGetWordLib = 0;
UINT_PTR TimerID = 0;
typedef DWORD (*GetWordProc_t)(TCurrentMode *);
GetWordProc_t GetWordProc = NULL;
GDDataStruct gds;
UINT uGdAskMessage;
WCHAR Buffer[256];
DWORD ourProcessID, gdProcessID, winProcessID;

static HWND GetWindowFromPoint(POINT pt)
{
HWND WndParent,WndChild;
	WndParent = WindowFromPoint(pt);
	if(WndParent == NULL) return WndParent;
	ScreenToClient(WndParent, &pt);
	WndChild = RealChildWindowFromPoint(WndParent, pt);
	if(WndChild == NULL) return WndParent;
	return WndChild;
};

static void SendWordToServer()
{
DWORD_PTR SendMsgAnswer;
DWORD flags;
LRESULT lr;

	if( !IsWindow( GlobalData->ServerWND ) )
		return;

	// Ask for needing to retrieve word - WPARAM = 1
	lr = SendMessageTimeout(GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 1, 0, SMTO_ABORTIFHUNG, MOUSEOVER_INTERVAL, &SendMsgAnswer);
	if( lr == 0 || SendMsgAnswer == 0)	//No answer or no needing
		return;

	flags = SendMsgAnswer;

	if (hGetWordLib == 0 && ( flags & GD_FLAG_METHOD_STANDARD ) ) {
		hGetWordLib = LoadLibraryW(GlobalData->LibName);
		if (hGetWordLib) {
			GetWordProc = (GetWordProc_t)GetProcAddress(hGetWordLib, "__gdGetWord");
		}
		else {
			hGetWordLib = INVALID_HANDLE_VALUE;
		}
	}

	GlobalData->CurMod.MatchedWord[0] = 0;
	GlobalData->CurMod.WordLen = 0;
	GlobalData->CurMod.BeginPos = 0;

	if( ( flags & GD_FLAG_METHOD_GD_MESSAGE ) != 0 && uGdAskMessage != 0 ) {
		int n;
		gds.dwSize = sizeof(gds);
		gds.cwData = Buffer;
		gds.dwMaxLength = sizeof(Buffer) / sizeof(Buffer[0]);
		Buffer[0] = 0;
		gds.hWnd = GlobalData->LastWND;
		gds.Pt = GlobalData->LastPt;
		lr = SendMessageTimeout(gds.hWnd, uGdAskMessage, 0, (LPARAM)&gds, SMTO_ABORTIFHUNG, REQUEST_MESSAGE_INTERVAL, &SendMsgAnswer);
		if(lr != 0 && SendMsgAnswer != 0) {
			n = WideCharToMultiByte(CP_UTF8, 0, gds.cwData, lstrlenW(gds.cwData), GlobalData->CurMod.MatchedWord, sizeof(GlobalData->CurMod.MatchedWord) - 1, 0, 0);
			GlobalData->CurMod.MatchedWord[n] = 0;
			GlobalData->CurMod.WordLen = n;
			GlobalData->CurMod.BeginPos = 0;
			if(n > 0) {
				if( IsWindow( GlobalData->ServerWND ) ) {
#ifdef __WIN64
					GlobalData32->LastWND = HandleToLong( GlobalData->LastWND );
					GlobalData32->CurMod.WordLen = n;
					GlobalData32->CurMod.BeginPos = 0;
					lstrcpyn( GlobalData32->CurMod.MatchedWord, GlobalData->CurMod.MatchedWord, sizeof( GlobalData32->CurMod.MatchedWord ) );
#endif
					SendMessageTimeout(GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 0, 1, SMTO_ABORTIFHUNG, MOUSEOVER_INTERVAL, &SendMsgAnswer);
				}
			}
			return;
		}
	}

	// Don't use other methods for GD own windows
	if( winProcessID == gdProcessID ) 
		return;

	if( ( flags & GD_FLAG_METHOD_STANDARD ) != 0 && GetWordProc != 0 ) {
		GlobalData->CurMod.WND = GlobalData->LastWND;
		GlobalData->CurMod.Pt = GlobalData->LastPt;

		LPARAM lparam = GetWordProc(&(GlobalData->CurMod));
		// lparam == 0 - need to reverse RTL text, else don't reverse

		if (GlobalData->CurMod.WordLen > 0) {
			if( IsWindow( GlobalData->ServerWND ) ) {
#ifdef __WIN64
				GlobalData32->LastWND = HandleToLong( GlobalData->LastWND );
				GlobalData32->CurMod.WordLen = GlobalData->CurMod.WordLen;
				GlobalData32->CurMod.BeginPos = GlobalData->CurMod.BeginPos;
				lstrcpyn( GlobalData32->CurMod.MatchedWord, GlobalData->CurMod.MatchedWord, sizeof( GlobalData32->CurMod.MatchedWord ) );
#endif
				SendMessageTimeout(GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 0, lparam, SMTO_ABORTIFHUNG, MOUSEOVER_INTERVAL, &SendMsgAnswer);
			}
			return;
		}
	}

	if( ( flags & GD_FLAG_METHOD_IACCESSIBLEEX ) != 0 ) {
		getWordByAccEx( GlobalData->LastPt );
		if (GlobalData->CurMod.WordLen > 0 ) {
			if( IsWindow( GlobalData->ServerWND ) ) {
#ifdef __WIN64
				GlobalData32->LastWND = HandleToLong( GlobalData->LastWND );
				GlobalData32->CurMod.WordLen = GlobalData->CurMod.WordLen;
				GlobalData32->CurMod.BeginPos = GlobalData->CurMod.BeginPos;
				lstrcpyn( GlobalData32->CurMod.MatchedWord, GlobalData->CurMod.MatchedWord, sizeof( GlobalData32->CurMod.MatchedWord ) );
#endif
				SendMessageTimeout(GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 0, 1, SMTO_ABORTIFHUNG, MOUSEOVER_INTERVAL, &SendMsgAnswer);
			}
			return;
		}
	}

	if( ( flags & GD_FLAG_METHOD_UI_AUTOMATION ) != 0 && IsWindow( GlobalData->ServerWND ) ) {
#ifdef __WIN64
		GlobalData32->CurMod.MatchedWord[0] = 0;
		GlobalData32->CurMod.WordLen = 0;
		GlobalData32->CurMod.BeginPos = 0;
		GlobalData32->LastPt = GlobalData->LastPt;
#endif
		PostMessage( GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 0, 1 );
	}		
}

void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT_PTR nTimerid,DWORD dwTime)
{
(void) hWnd;
(void) nMsg;
(void) dwTime;
DWORD wso;
	wso = WaitForSingleObject(hSynhroMutex, 0);
	if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) {
		KillTimer(0, nTimerid);
		TimerID = 0;
		while( 1 ) {
			POINT curPt;
			HWND targetWnd;
			winProcessID = 0;

			if( !GetCursorPos( &curPt ) ) 
				break;

			if( GlobalData == NULL || GlobalData->LastPt.x != curPt.x || GlobalData->LastPt.y != curPt.y) 
				break;

			if( ( targetWnd = GetWindowFromPoint( curPt ) ) == NULL )
				break;

			if( GlobalData->LastWND != targetWnd ) 
				break;

			GetWindowThreadProcessId( targetWnd, &winProcessID );
			if( winProcessID != ourProcessID ) {
				char className[64];
				if( !GetClassName( targetWnd, className, sizeof(className) ) )
					break;
				if( lstrcmpi( className, "ConsoleWindowClass" ) != 0 )
					break;
			}

			SendWordToServer();

			break;
		}
		ReleaseMutex(hSynhroMutex);
	}
}

void HookProc( POINT *ppt )
{
HWND WND;
TCHAR wClassName[64];
DWORD winProcessID;
	WND = GetWindowFromPoint( *ppt );
	if(WND == NULL) return;

	if ( !GetClassName(WND, wClassName, sizeof(wClassName) / sizeof(TCHAR)) ) 
		return;

	GetWindowThreadProcessId( WND, &winProcessID );

	if( winProcessID != ourProcessID && lstrcmpi( wClassName, _T("ConsoleWindowClass") ) != 0 )
		return;

	if(TimerID && ( GlobalData->LastPt.x != ppt->x || GlobalData->LastPt.y != ppt->y ) ) 
	{
		KillTimer(0, TimerID);
		TimerID = 0;
	}

	const char* DisableClasses[] = {
				"gdkWindowChild",
				"gdkWindowTemp",
				"Progman",
				"WorkerW",
				};
	int i;
	for (i=0; i<4; i++) {
		if (lstrcmp(wClassName, DisableClasses[i])==0)
			break;
	}
	if (i<4) return;

	if(GlobalData->LastPt.x != ppt->x || GlobalData->LastPt.y != ppt->y || GlobalData->LastWND != WND ) 
	{
		GlobalData->LastWND = WND;
		GlobalData->LastPt = *ppt;
		TimerID = SetTimer(0, TimerID, MOUSEOVER_INTERVAL, TimerFunc);
	}
}

#ifdef __WIN64

LRESULT CALLBACK GetMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
PMSG pMsg;
DWORD wso;
	if( nCode == HC_ACTION && wParam == PM_REMOVE ) 
	{
		pMsg = (PMSG)lParam;
		if( pMsg && ( pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE ) ) 
		{
			wso = WaitForSingleObject(hSynhroMutex, 0);
			if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) 
			{
				POINT pt;
				pt.x = GET_X_LPARAM( pMsg->lParam );
				pt.y = GET_Y_LPARAM( pMsg->lParam );
				if( pMsg->message == WM_MOUSEMOVE && pMsg->hwnd != NULL )
					ClientToScreen( pMsg->hwnd, &pt );
				HookProc( &pt );
				ReleaseMutex(hSynhroMutex);
			}
		}
	}
	return CallNextHookEx(GlobalData->g_hHook, nCode, wParam, lParam);
}

#else

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
DWORD wso;
	if ( (nCode == HC_ACTION) && ((wParam == WM_MOUSEMOVE) || (wParam == WM_NCMOUSEMOVE)) ) {
		wso = WaitForSingleObject(hSynhroMutex, 0);
		if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) {
			HookProc( &(((PMOUSEHOOKSTRUCT)lParam)->pt) );
			ReleaseMutex(hSynhroMutex);
		}
	}
	return CallNextHookEx(GlobalData->g_hHook, nCode, wParam, lParam);
}

#endif

DLLIMPORT void ActivateTextOutSpying (int Activate)
{
	// After call SetWindowsHookEx(), when you move mouse to a application's window, 
	// this dll will load into this application automatically. And it is unloaded 
	// after call UnhookWindowsHookEx().
	if(GlobalData == NULL) return;
	if (Activate) {
		if (GlobalData->g_hHook != NULL) return;
#ifdef __WIN64
		GlobalData->g_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMessageHookProc, g_hInstance, 0);
#else
		GlobalData->g_hHook = SetWindowsHookEx(WH_MOUSE, MouseHookProc, g_hInstance, 0);
#endif
	}
	else {
		if (GlobalData->g_hHook == NULL) return;
		WaitForSingleObject(hSynhroMutex, 2000);
		UnhookWindowsHookEx(GlobalData->g_hHook);
		if (TimerID) {
			KillTimer(0, TimerID);
			TimerID=0;
		}
		ReleaseMutex(hSynhroMutex);
		GlobalData->g_hHook = NULL;
	}
}


BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
(void) reserved;
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
			g_hInstance = hInst;
			ThTypes_Init();
#ifdef __WIN64
			if( GlobalData == NULL || GlobalData32 == NULL ) {
#else
			if( GlobalData == NULL ) {
#endif
				ThTypes_End();
				return FALSE;
			}
			ourProcessID = GetCurrentProcessId();
			GetWindowThreadProcessId( GlobalData->ServerWND, &gdProcessID );
			if(hSynhroMutex==0) {
				hSynhroMutex = CreateMutex(NULL, FALSE, "GoldenDictTextOutSpyMutex");
				if(hSynhroMutex==0) {
					return(FALSE);
				}
			}
			uGdAskMessage = RegisterWindowMessage(GD_MESSAGE_NAME);
			FindGetPhysicalCursorPos();
        break;

      case DLL_PROCESS_DETACH:
//			if(hSynhroMutex) WaitForSingleObject(hSynhroMutex, INFINITE);
			if(hSynhroMutex) WaitForSingleObject(hSynhroMutex, 2000);
			if (TimerID) {
				KillTimer(0, TimerID);
				TimerID=0;
			}
			if(hSynhroMutex) {
				ReleaseMutex(hSynhroMutex);
				CloseHandle(hSynhroMutex);
				hSynhroMutex=0;
			}
			{
				MSG msg ;
				while (PeekMessage (&msg, 0, WM_TIMER, WM_TIMER, PM_REMOVE));
			}
			if ( (hGetWordLib != 0) && (hGetWordLib != INVALID_HANDLE_VALUE) ) {
				FreeLibrary(hGetWordLib);
			}
			ThTypes_End();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
