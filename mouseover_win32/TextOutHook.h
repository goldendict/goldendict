#ifndef _TextOutHook_H_
#define _TextOutHook_H_

#if BUILDING_DLL
# define DLLIMPORT __declspec (dllexport)
#else /* Not BUILDING_DLL */
# define DLLIMPORT __declspec (dllimport)
#endif /* Not BUILDING_DLL */

#include "ThTypes.h"

typedef struct TEverythingParams {
	HWND WND;
	POINT Pt;
	int Active;
	int WordLen;
	int Unicode;
	int BeginPos;
	char MatchedWordA[256];
	wchar_t MatchedWordW[256];
} TEverythingParams;

char* ExtractFromEverything(HWND WND, POINT Pt, DWORD *BeginPos);

DLLIMPORT void GetWord (TCurrentMode *P);

void ConvertToMatchedWordA(TEverythingParams *TP);

#endif /* _TextOutHook_H_ */
