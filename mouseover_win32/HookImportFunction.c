#include "HookImportFunction.h"
#include <tlhelp32.h>


// These code come from: http://dev.csdn.net/article/2/2786.shtm
// I fixed a bug in it and improved it to hook all the modules of a program.

#define MakePtr(cast, ptr, AddValue) (cast)((size_t)(ptr)+(size_t)(AddValue))

static PIMAGE_IMPORT_DESCRIPTOR GetNamedImportDescriptor(HMODULE hModule, LPCSTR szImportModule)
{
	PIMAGE_DOS_HEADER pDOSHeader;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;

	if ((szImportModule == NULL) || (hModule == NULL))
		return NULL;
	pDOSHeader = (PIMAGE_DOS_HEADER) hModule;
	if (IsBadReadPtr(pDOSHeader, sizeof(IMAGE_DOS_HEADER)) || (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)) {
		return NULL;
	}
	pNTHeader = MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
	if (IsBadReadPtr(pNTHeader, sizeof(IMAGE_NT_HEADERS)) || (pNTHeader->Signature != IMAGE_NT_SIGNATURE))
		return NULL;
	if (pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0)
		return NULL;
	pImportDesc = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	while (pImportDesc->Name) {
		PSTR szCurrMod = MakePtr(PSTR, pDOSHeader, pImportDesc->Name);
//		if (_stricmp(szCurrMod, szImportModule) == 0)
		if (lstrcmpi(szCurrMod, szImportModule) == 0)
			break;
		pImportDesc++;
	}
	if (pImportDesc->Name == (DWORD)0)
		return NULL;
	return pImportDesc;
}

static BOOL IsNT()
{
	OSVERSIONINFO stOSVI;
	BOOL bRet;

	memset(&stOSVI, 0, sizeof(OSVERSIONINFO));
	stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bRet = GetVersionEx(&stOSVI);
	if (FALSE == bRet) return FALSE;
	return (VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId);
}

static BOOL HookImportFunction(HMODULE hModule, LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs)
{
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_THUNK_DATA pOrigThunk;
	PIMAGE_THUNK_DATA pRealThunk;

	if (!IsNT() && ((size_t)hModule >= 0x80000000))
		return FALSE;
	pImportDesc = GetNamedImportDescriptor(hModule, szImportModule);
	if (pImportDesc == NULL)
		return FALSE;
	pOrigThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->OriginalFirstThunk);
	pRealThunk = MakePtr(PIMAGE_THUNK_DATA, hModule, pImportDesc->FirstThunk);
	while (pOrigThunk->u1.Function) {
		if (IMAGE_ORDINAL_FLAG != (pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)) {
			PIMAGE_IMPORT_BY_NAME pByName = MakePtr(PIMAGE_IMPORT_BY_NAME, hModule, pOrigThunk->u1.AddressOfData);
			BOOL bDoHook;
			// When hook EditPlus, read pByName->Name[0] will case this dll terminate, so call IsBadReadPtr() here.
			if (IsBadReadPtr(pByName, sizeof(IMAGE_IMPORT_BY_NAME))) {
				pOrigThunk++;
				pRealThunk++;
				continue;				
			}
			if ('\0' == pByName->Name[0]) {
				pOrigThunk++;
				pRealThunk++;
				continue;
			}
			bDoHook = FALSE;
//			if ((szFunc[0] == pByName->Name[0]) && (_strcmpi(szFunc, (char*)pByName->Name) == 0)) {
			if ((szFunc[0] == pByName->Name[0]) && (lstrcmpi(szFunc, (char*)pByName->Name) == 0)) {
				if (paHookFuncs)
					bDoHook = TRUE;
			}
			if (bDoHook) {
				MEMORY_BASIC_INFORMATION mbi_thunk;
				DWORD dwOldProtect;
				if( VirtualQuery(pRealThunk, &mbi_thunk, sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION))
					return FALSE;
				if( !VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, PAGE_READWRITE, &mbi_thunk.Protect) )
					return FALSE;
				if (paOrigFuncs)
					*paOrigFuncs = (PROC)InterlockedExchangePointer(&(pRealThunk->u1.Function), paHookFuncs);
				else
					InterlockedExchangePointer(&(pRealThunk->u1.Function), paHookFuncs);

				VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize, mbi_thunk.Protect, &dwOldProtect);
				return TRUE;
			}
		}
		pOrigThunk++;
		pRealThunk++;
	}
	return FALSE;
}

BOOL HookAPI(LPCSTR szImportModule, LPCSTR szFunc, PROC paHookFuncs, PROC* paOrigFuncs)
{
	HANDLE hSnapshot;
	MODULEENTRY32 me;
	BOOL bOk;
	DWORD err;

	ZeroMemory(&me,sizeof(me));
	me.dwSize=sizeof(me);

	if ((szImportModule == NULL) || (szFunc == NULL)) {
		return FALSE;
	}

	do {	
		hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0);
		if(hSnapshot != INVALID_HANDLE_VALUE) break;
		err = GetLastError();
		if(err == ERROR_BAD_LENGTH) Sleep(100);
	} while(err == ERROR_BAD_LENGTH);

	if(hSnapshot == INVALID_HANDLE_VALUE) return FALSE;

	bOk = Module32First(hSnapshot, &me);
	while (bOk) {
		HookImportFunction(me.hModule, szImportModule, szFunc, paHookFuncs, paOrigFuncs);
		bOk = Module32Next(hSnapshot, &me);
	}
	CloseHandle(hSnapshot);
	return TRUE;
}
