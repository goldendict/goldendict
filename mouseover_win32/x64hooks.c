#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <Tlhelp32.h>
#include <tchar.h>
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>
#include <basetsd.h>
#include "thtypes.h"

typedef void ( *ActivateSpyFn )( BOOL );

BOOL parentIsGD()
{
HANDLE hSnapshot, hModuleSnapshot;
PROCESSENTRY32 pe;
MODULEENTRY32 me;
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
			break;
		}
		b = Process32Next( hSnapshot, &pe );
	}
	CloseHandle( hSnapshot );
	return b;
}

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
HWND hServerWnd;
ActivateSpyFn activateSpyFn = NULL;
HINSTANCE spyDll = NULL;
int ret = -1;
MSG msg;

	if( !parentIsGD() )
		return -1;

	while( 1 ) {
		ThTypes_Init();
		if( GlobalData == NULL || GlobalData32 == NULL) 
			break;
		hServerWnd = LongToHandle( GlobalData32->ServerWND );

		GetModuleFileName( NULL, dir, MAX_PATH );
		pch = dir + lstrlen( dir );
		while( pch != dir && *pch != '\\' ) pch--;
		*(pch + 1) = 0;
		lstrcpy( libName, dir );
		lstrcat( libName, _T("GdTextOutSpy64.dll") );

		SetLowLabelToGDSynchroObjects();

		memset( GlobalData, 0, sizeof( TGlobalDLLData ) );
		lstrcpy( GlobalData->LibName, dir );
		lstrcat( GlobalData->LibName, _T("GdTextOutHook64.dll") );
		GlobalData->ServerWND = hServerWnd;

		spyDll = LoadLibrary( libName );
		if ( spyDll )
			activateSpyFn = ( ActivateSpyFn ) GetProcAddress( spyDll, "ActivateTextOutSpying" );
		if( !activateSpyFn ) {
			ret = -2;
			break;
		}

		activateSpyFn( TRUE );

		while( GetMessage( &msg, 0, 0, 0 ) )
			DispatchMessage( &msg );

		activateSpyFn( FALSE );

		ret = 0;
		break;
	}

	if( spyDll )
		FreeLibrary( spyDll );
	ThTypes_End();
	return ret;
}
