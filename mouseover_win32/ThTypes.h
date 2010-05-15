#ifndef _ThTypes_H_
#define _ThTypes_H_

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

typedef struct TCurrentMode {
	HWND WND;
	POINT Pt;
	size_t WordLen;
	char MatchedWord[256];
	int BeginPos;
} TCurrentMode;

typedef struct TGlobalDLLData {
	HWND ServerWND;
	HHOOK g_hHookMouse;
	UINT_PTR TimerID;
	HWND LastWND;
	POINT LastPt;
	TCurrentMode CurMod;
	char LibName[256];
} TGlobalDLLData;

extern TGlobalDLLData *GlobalData;


void ThTypes_Init();
void Thtypes_End();

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif
