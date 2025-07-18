#include "ThTypes.h"

HANDLE MMFHandle = 0;
TGlobalDLLData *GlobalData = NULL;

#ifdef __WIN64
HANDLE MMFHandle32 = 0;
TGlobalDLLData32 *GlobalData32 = NULL;
LPCSTR SHARE_NAME32 = "GoldenDictTextOutHookSharedMem";
LPCSTR SHARE_NAME = "GoldenDictTextOutHookSharedMem64";
#else
LPCSTR SHARE_NAME = "GoldenDictTextOutHookSharedMem";
#endif

void ThTypes_Init()
{
	if (!MMFHandle) {
		MMFHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGlobalDLLData), SHARE_NAME);
	}
	if (!MMFHandle) {
		MMFHandle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, 0, SHARE_NAME);
	}
	if (!GlobalData && MMFHandle != NULL)
		GlobalData = MapViewOfFile(MMFHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#ifdef __WIN64
	if (!MMFHandle32) {
		MMFHandle32 = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGlobalDLLData32), SHARE_NAME32);
	}
	if (!MMFHandle32) {
		MMFHandle32 = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, 0, SHARE_NAME32);
	}
	if (!GlobalData32 && MMFHandle32 != NULL)
		GlobalData32 = MapViewOfFile(MMFHandle32, FILE_MAP_ALL_ACCESS, 0, 0, 0);
#endif
}

void ThTypes_End()
{
	if (GlobalData) {
		UnmapViewOfFile(GlobalData);
		GlobalData = NULL;
	}
	if (MMFHandle) {
		CloseHandle(MMFHandle);
		MMFHandle = 0;
	}
#ifdef __WIN64
	if (GlobalData32) {
		UnmapViewOfFile(GlobalData32);
		GlobalData32 = NULL;
	}
	if (MMFHandle32) {
		CloseHandle(MMFHandle32);
		MMFHandle32 = 0;
	}
#endif
}
