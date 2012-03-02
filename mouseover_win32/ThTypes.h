#ifndef _ThTypes_H_
#define _ThTypes_H_

#include <windows.h>

#define GD_FLAG_METHOD_STANDARD		0x00000001
#define GD_FLAG_METHOD_GD_MESSAGE	0x00000002
#define GD_FLAG_METHOD_IACCESSIBLEEX	0x00000004
#define GD_FLAG_METHOD_UI_AUTOMATION	0x00000008

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

#pragma pack(push,4)

typedef struct TCurrentMode {
	HWND WND;
	POINT Pt;
	DWORD WordLen;
	char MatchedWord[256];
	DWORD BeginPos;
} TCurrentMode;

typedef struct TGlobalDLLData {
	HWND ServerWND;
	HHOOK g_hHook;
	HWND LastWND;
	POINT LastPt;
	TCurrentMode CurMod;
	WCHAR LibName[256];
} TGlobalDLLData;

#pragma pack(pop)

extern TGlobalDLLData *GlobalData;

#ifdef __WIN64

#pragma pack(push,4)

typedef struct TCurrentMode32 {
	DWORD WND;
	POINT Pt;
	DWORD WordLen;
	char MatchedWord[256];
	DWORD BeginPos;
} TCurrentMode32;

typedef struct TGlobalDLLData32 {
	DWORD ServerWND;
	DWORD g_hHook;
	DWORD LastWND;
	POINT LastPt;
	TCurrentMode32 CurMod;
	WCHAR LibName[256];
} TGlobalDLLData32;

#pragma pack(pop)

extern TGlobalDLLData32 *GlobalData32;

#endif

void ThTypes_Init();
void ThTypes_End();

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif
