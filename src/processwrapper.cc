#include "processwrapper.hh"

#include <QtCore>

#ifdef Q_OS_WIN32

#include <windows.h>
#include <stdio.h>
#include <psapi.h>

unsigned int ProcessWrapper::currentProcessId()
{
	return GetCurrentProcessId();
}

bool ProcessWrapper::processExists(unsigned int pid)
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return false;

    cProcesses = cbNeeded / sizeof(DWORD);
    for ( i = 0; i < cProcesses; i++ )
    {
      unsigned int processID = aProcesses[i];
      if ( processID == pid )
        return true;
    }

    return false;
}

unsigned int ProcessWrapper::findProcess(const char *name, unsigned int pid_skip)
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    QString pname(name); pname += ".exe";

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 0;

    // Calculate how many process identifiers were returned.

    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.

    for ( i = 0; i < cProcesses; i++ )
    {
        unsigned int processID = aProcesses[i];
        if( processID != 0 && processID != pid_skip )
        {
            char szProcessName[MAX_PATH] = "<unknown>";

            // Get a handle to the process.

            HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                           PROCESS_VM_READ,
                                           FALSE, processID );

            // Get the process name.

            if (NULL != hProcess )
            {
                HMODULE hMod;
                DWORD cbNeeded;

                if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
                     &cbNeeded) )
                {
                    GetModuleBaseNameA( hProcess, hMod, szProcessName,
                                       sizeof(szProcessName)/sizeof(TCHAR) );

					if (QString(szProcessName) == pname) {
	                    CloseHandle( hProcess );
                        return processID;
					}
                }

				CloseHandle( hProcess );
            }

            // Print the process name and identifier.

            //_tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, processID );

        }
    }

    return 0;
}

#else

unsigned int ProcessWrapper::currentProcessId()
{
    return getpid();
}

bool ProcessWrapper::processExists(unsigned int pid)
{
  return QFile::exists(QString("/proc/%1").arg(pid));
}

unsigned int ProcessWrapper::findProcess(const char *name, unsigned int pid_skip)
{
    QString pname("(" + QString(name) + ")");
    QDir pd("/proc");
    QFileInfoList list = pd.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QFileInfoList::iterator it, it_end = list.end();
    for (it = list.begin(); it != it_end; it++)
    {
        const QFileInfo &fi = *it;
        if (fi.baseName().at(0).isDigit()) {
            QFile f(fi.absoluteFilePath()+"/stat");
            if (f.open(QIODevice::ReadOnly)) {
                QTextStream ts(&f);
                unsigned int pid; ts >> pid;
                if (pid == pid_skip)
                    continue;
                QString pn; ts >> pn;
                if (pn == pname)
                    return pid;
            }
        }
    }

    return 0;
}

#endif


ProcessWrapper::ProcessWrapper()
{
}

