#include "ThTypes.h"

HANDLE MMFHandle = 0;
TGlobalDLLData *GlobalData = NULL;

void ThTypes_Init()
{
	if (!MMFHandle) {
		MMFHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TGlobalDLLData), "GoldenDictTextOutHookSharedMem");
	}
	if (!MMFHandle) {
		MMFHandle = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, 0, "GoldenDictTextOutHookSharedMem");
	}
	if (!GlobalData && MMFHandle != NULL)
		GlobalData = MapViewOfFile(MMFHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
}

void Thtypes_End()
{
	if (GlobalData) {
		UnmapViewOfFile(GlobalData);
		GlobalData = NULL;
	}
	if (MMFHandle) {
		CloseHandle(MMFHandle);
		MMFHandle = 0;
	}
}
