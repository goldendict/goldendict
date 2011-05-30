#include "TextOutSpy.h"
#include "ThTypes.h"


const int MOUSEOVER_INTERVAL = 300;
const int WM_MY_SHOW_TRANSLATION = WM_USER + 301;

HINSTANCE g_hInstance = NULL;
HANDLE hSynhroMutex = 0;
HINSTANCE hGetWordLib = 0;
UINT_PTR TimerID = 0;
typedef void (*GetWordProc_t)(TCurrentMode *);
GetWordProc_t GetWordProc = NULL;

static HWND GetWindowFromPoint(POINT pt) {
HWND WndParent,WndChild;
	WndParent = WindowFromPoint(pt);
	if(WndParent == NULL) return WndParent;
	ScreenToClient(WndParent, &pt);
	WndChild=RealChildWindowFromPoint(WndParent, pt);
	if(WndChild == NULL) return WndParent;
	return WndChild;
};

static void SendWordToServer()
{
	if (hGetWordLib == 0) {
		hGetWordLib = LoadLibrary(GlobalData->LibName);
		if (hGetWordLib) {
			GetWordProc = (GetWordProc_t)GetProcAddress(hGetWordLib, "__gdGetWord");
		}
		else {
			hGetWordLib = (HINSTANCE)-1;
		}
	}
	if (GetWordProc) {
		GlobalData->CurMod.WND = GlobalData->LastWND;
		GlobalData->CurMod.Pt = GlobalData->LastPt;
		GetWordProc(&(GlobalData->CurMod));
		if (GlobalData->CurMod.WordLen > 0) {
			DWORD SendMsgAnswer;
			SendMessageTimeout(GlobalData->ServerWND, WM_MY_SHOW_TRANSLATION, 0, 0, SMTO_ABORTIFHUNG, MOUSEOVER_INTERVAL, &SendMsgAnswer);
		}
	}
}

void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
DWORD wso;
	wso = WaitForSingleObject(hSynhroMutex, 0);
	if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) {
		KillTimer(0, nTimerid);
		if ((GlobalData->LastWND!=0)&&(GlobalData->LastWND == GetWindowFromPoint(GlobalData->LastPt))) {
			SendWordToServer();
		}
		ReleaseMutex(hSynhroMutex);
	}
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
DWORD wso;
	if ((nCode == HC_ACTION) && ((wParam == WM_MOUSEMOVE) || (wParam == WM_NCMOUSEMOVE)) && (GlobalData != NULL)) {
		wso = WaitForSingleObject(hSynhroMutex, 0);
		if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) {
			HWND WND;
			TCHAR wClassName[64];

			WND = GetWindowFromPoint(((PMOUSEHOOKSTRUCT)lParam)->pt);

			if(WND == NULL) {
				ReleaseMutex(hSynhroMutex);
				return CallNextHookEx(GlobalData->g_hHookMouse, nCode, wParam, lParam);
			}

			if (GetClassName(WND, wClassName, sizeof(wClassName) / sizeof(TCHAR))) {
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
					if (i<4) {
						ReleaseMutex(hSynhroMutex);
						return CallNextHookEx(GlobalData->g_hHookMouse, nCode, wParam, lParam);
					}
			}

			if(GlobalData->LastPt.x!=((PMOUSEHOOKSTRUCT)lParam)->pt.x || GlobalData->LastPt.y!=((PMOUSEHOOKSTRUCT)lParam)->pt.y || GlobalData->LastWND != WND) {
				TimerID = SetTimer(0, TimerID, MOUSEOVER_INTERVAL, TimerFunc);
				GlobalData->LastWND = WND;
				GlobalData->LastPt = ((PMOUSEHOOKSTRUCT)lParam)->pt;
			}
			ReleaseMutex(hSynhroMutex);
		}
	}
	return CallNextHookEx(GlobalData->g_hHookMouse, nCode, wParam, lParam);
}

DLLIMPORT void ActivateTextOutSpying (int Activate)
{
	// After call SetWindowsHookEx(), when you move mouse to a application's window, 
	// this dll will load into this application automatically. And it is unloaded 
	// after call UnhookWindowsHookEx().
DWORD wso;
	if(GlobalData == NULL) return;
	if (Activate) {
		if (GlobalData->g_hHookMouse != NULL) return;
		GlobalData->g_hHookMouse = SetWindowsHookEx(WH_MOUSE, MouseHookProc, g_hInstance, 0);
	}
	else {
		if (GlobalData->g_hHookMouse == NULL) return;
		UnhookWindowsHookEx(GlobalData->g_hHookMouse);
		wso = WaitForSingleObject(hSynhroMutex, 4*MOUSEOVER_INTERVAL);
		if (wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED) {
			if (TimerID) {
				if (KillTimer(0, TimerID))
					TimerID=0;
			}
			ReleaseMutex(hSynhroMutex);
		}
		GlobalData->g_hHookMouse = NULL;
	}
}


BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
			g_hInstance = hInst;
			if(hSynhroMutex==0) {
				hSynhroMutex = CreateMutex(NULL, FALSE, "GoldenDictTextOutSpyMutex");
				if(hSynhroMutex==0) return(FALSE);
			}
			ThTypes_Init();
        break;

      case DLL_PROCESS_DETACH:
//			if(hSynhroMutex) WaitForSingleObject(hSynhroMutex, INFINITE);
			if(hSynhroMutex) WaitForSingleObject(hSynhroMutex, 2000);
			if (TimerID) {
				if (KillTimer(0, TimerID))
					TimerID=0;
			}
			if(hSynhroMutex) {
				ReleaseMutex(hSynhroMutex);
				CloseHandle(hSynhroMutex);
				hSynhroMutex=0;
			}
			{
			MSG msg ;
			while (PeekMessage (&msg, 0, WM_TIMER, WM_TIMER, PM_REMOVE)) {}
			}
			if ((hGetWordLib != 0)&&(hGetWordLib != (HINSTANCE)(-1))) {
				FreeLibrary(hGetWordLib);
			}
			Thtypes_End();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
