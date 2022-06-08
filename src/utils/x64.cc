#include <QCoreApplication>
#include <QDir>

#ifndef _UNICODE
#define _UNICODE
#endif

#include "x64.hh"
#include <windows.h>
#include <tchar.h>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
PROCESS_INFORMATION pInfo;

#ifndef Q_OS_WIN64
bool isWow64()
{
    static LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;

    if( NULL == fnIsWow64Process )
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle( _T("kernel32") ), "IsWow64Process" );
    if( NULL != fnIsWow64Process )
    {
        if ( !fnIsWow64Process( GetCurrentProcess(), &bIsWow64 ) )
            return false;
    }
    return bIsWow64;
}
#endif

bool installx64Hooks()
{
STARTUPINFO startup;
#ifndef Q_OS_WIN64
    if( !isWow64() )
        return false;
#endif
    if( pInfo.hProcess != NULL )
        removex64Hooks();
    QDir dir =  QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN64
    if( !dir.cd("x86") )
        return false;
    QString starterProc = QDir::toNativeSeparators( dir.filePath( "x86helper.exe" ) );
#else
    if( !dir.cd("x64") )
        return false;
    QString starterProc = QDir::toNativeSeparators( dir.filePath( "x64helper.exe" ) );
#endif

    memset( &startup, 0, sizeof(startup) );
    startup.cb = sizeof(startup);

    BOOL b = CreateProcess( starterProc.toStdWString().c_str(), NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &startup, &pInfo );
    if( !b )
        pInfo.hProcess = NULL;

    return b;
}

void removex64Hooks()
{
    if( pInfo.hProcess == NULL )
        return;
    PostThreadMessage( pInfo.dwThreadId, WM_QUIT, 0, 0 );
    DWORD res = WaitForSingleObject( pInfo.hProcess, 3000 );
    if( res == WAIT_TIMEOUT )
        TerminateProcess( pInfo.hProcess, 1 );
    CloseHandle( pInfo.hProcess );
    CloseHandle( pInfo.hThread );
    pInfo.hProcess = NULL;
}
