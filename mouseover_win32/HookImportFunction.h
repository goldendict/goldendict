#ifndef _HookImportFunction_H_
#define _HookImportFunction_H_

#include <windows.h>


BOOL HookAPI(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs);

#endif
