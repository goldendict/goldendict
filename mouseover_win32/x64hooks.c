#define UNICODE
#define _UNICODE

#ifndef __WIN64
#define _WIN32_WINNT 0x0600
#endif

#include <Windows.h>
#include <Tlhelp32.h>
#include <tchar.h>
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>
#include <basetsd.h>

#include "thtypes.h"

typedef void ( *ActivateSpyFn )( BOOL );

HANDLE hGDProcess;
UINT_PTR timerID;

typedef BOOL WINAPI ( *QueryFullProcessImageNameWFunc)( HANDLE, DWORD, LPWSTR, PDWORD );

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000
#endif

void CALLBACK TimerFunc(HWND hWnd,UINT nMsg,UINT_PTR nTimerID,DWORD dwTime)
{
(void) hWnd;
(void) nMsg;
(void) dwTime;
	if( nTimerID == timerID )
	{
		DWORD wso = WaitForSingleObject( hGDProcess, 0 );
		if( wso == WAIT_OBJECT_0 || wso == WAIT_ABANDONED )
			PostThreadMessage( GetCurrentThreadId(), WM_QUIT, 0, 0 );
	}
}


BOOL parentIsGD()
{
HANDLE hSnapshot;
PROCESSENTRY32 pe;
#ifdef __WIN64
MODULEENTRY32 me;
HANDLE hModuleSnapshot;
#else
HMODULE hm;
#endif
DWORD procID;
BOOL b;
	procID = GetCurrentProcessId();
	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if( hSnapshot == INVALID_HANDLE_VALUE )
		return FALSE;
	ZeroMemory( &pe, sizeof(pe) );
	pe.dwSize = sizeof(pe);
	b = Process32First( hSnapshot, &pe );
	while(b) {
		if( pe.th32ProcessID == procID ) {
#ifdef __WIN64
			hModuleSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pe.th32ParentProcessID );
			if( hModuleSnapshot == INVALID_HANDLE_VALUE ) {
				b = FALSE;
				break;
			}
			ZeroMemory( &me, sizeof(me) );
			me.dwSize = sizeof(me);
			b = Module32First( hModuleSnapshot, &me );
			if( b ) {
				int n = lstrlen( me.szExePath );
				b = n >= 14 && lstrcmpi( me.szExePath + n - 14, _T("GoldenDict.exe") ) == 0;
			}
			CloseHandle( hModuleSnapshot );

			if( b )
				hGDProcess = OpenProcess( SYNCHRONIZE, FALSE, pe.th32ParentProcessID );
#else
			WCHAR name[4096];
			DWORD dwSize = 4096;
			QueryFullProcessImageNameWFunc queryFullProcessImageNameWFunc = NULL;
			hm = GetModuleHandle( __TEXT("kernel32.dll"));
			if ( hm != NULL ) 
			 	queryFullProcessImageNameWFunc = (QueryFullProcessImageNameWFunc)GetProcAddress( hm, "QueryFullProcessImageNameW" );
			if( queryFullProcessImageNameWFunc == NULL ) {
				b = FALSE;
				break;
			}
			hGDProcess = OpenProcess( SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ParentProcessID );
			b = hGDProcess != NULL;
			if( b )	{
				b = queryFullProcessImageNameWFunc( hGDProcess, 0, name, &dwSize );
				if( b ) {
					b = dwSize >= 14 && lstrcmpiW( name + dwSize - 14, L"GoldenDict.exe" ) == 0;
				}
			}
			if( !b && hGDProcess != NULL )
			{
				CloseHandle( hGDProcess );
				hGDProcess = NULL;
			}
#endif
			break;
		}
		b = Process32Next( hSnapshot, &pe );
	}
	CloseHandle( hSnapshot );
	return b;
}

#ifdef __WIN64
void SetLowLabelToGDSynchroObjects()
{
// The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity
#define LOW_INTEGRITY_SDDL_SACL_W L"S:(ML;;NW;;;LW)"
    PSECURITY_DESCRIPTOR pSD = NULL;

    PACL pSacl = NULL; // not allocated
    BOOL fSaclPresent = FALSE;
    BOOL fSaclDefaulted = FALSE;
    LPCWSTR pwszMapFileName = L"GoldenDictTextOutHookSharedMem64";

    if( ConvertStringSecurityDescriptorToSecurityDescriptorW( LOW_INTEGRITY_SDDL_SACL_W, 1 /* SDDL_REVISION_1 */, &pSD, NULL ) )
    {
        if( GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
        {
// Note that psidOwner, psidGroup, and pDacl are
// all NULL and set the new LABEL_SECURITY_INFORMATION

            SetNamedSecurityInfoW( (LPWSTR)pwszMapFileName, SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);

        }
        LocalFree(pSD);
    }
}
#endif

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
(void) hInstance;
(void) hPrevInstance;
(void) lpCmdLine;
(void) nCmdShow;

TCHAR dir[MAX_PATH], libName[MAX_PATH], *pch;
#ifdef __WIN64
HWND hServerWnd;
#endif
ActivateSpyFn activateSpyFn = NULL;
HINSTANCE spyDll = NULL;
int ret = -1;
MSG msg;

	if( !parentIsGD() )
		return -1;
	if( hGDProcess == NULL )
		return -1;

	while( 1 ) {
		ThTypes_Init();
#ifdef __WIN64
		if( GlobalData == NULL || GlobalData32 == NULL) 
			break;
		hServerWnd = LongToHandle( GlobalData32->ServerWND );
#else
		if( GlobalData == NULL ) 
			break;
#endif
		              
		GetModuleFileName( NULL, dir, MAX_PATH );
		pch = dir + lstrlen( dir );
		while( pch != dir && *pch != '\\' ) pch--;
		*(pch + 1) = 0;
		lstrcpy( libName, dir );
#ifdef __WIN64
		lstrcat( libName, _T("GdTextOutSpy64.dll") );
		SetLowLabelToGDSynchroObjects();

		memset( GlobalData, 0, sizeof( TGlobalDLLData ) );
		lstrcpy( GlobalData->LibName, dir );
		lstrcat( GlobalData->LibName, _T("GdTextOutHook64.dll") );

		GlobalData->ServerWND = hServerWnd;
#else
		lstrcat( libName, _T("GdTextOutSpy.dll") );
#endif

		spyDll = LoadLibrary( libName );
		if ( spyDll )
			activateSpyFn = ( ActivateSpyFn ) GetProcAddress( spyDll, "ActivateTextOutSpying" );
		if( !activateSpyFn ) {
			ret = -2;
			break;
		}

		timerID = SetTimer( 0, 0, 1000, TimerFunc );

		activateSpyFn( TRUE );

		while( GetMessage( &msg, 0, 0, 0 ) )
			DispatchMessage( &msg );

		if( timerID )
			KillTimer( NULL, timerID);

		activateSpyFn( FALSE );

		ret = 0;
		break;
	}

	CloseHandle( hGDProcess );
	if( spyDll )
		FreeLibrary( spyDll );
	ThTypes_End();
	return ret;
}
