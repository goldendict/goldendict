#ifndef __GDDATATRANSFER_H_
#define __GDDATATRANSFER_H_

/*
Header for other program to interact with GoldenDict

When GD needs to retrieve word under mouse cursor it can ask for this a target program
by sending a message.

GD_MESSAGE_NAME - name of the this message
Message number must be retrieved through RegisterWindowMessage function

Message parameters:
WPARAM - 0, not used
LPARAM - pointer to GDDataStruct structure

GDDataStruct fields:
dwSize		- Structure size
hWnd		- Window with requested word
Pt		- Request coordinates (in screen coordinates, device units)
dwMaxLength	- Maximum word length to transfer (buffer size in unicode symbols)
cwData		- Buffer for requested word in UNICODE

If program process this message it must fill cwData and return TRUE
Otherwise GD will work by old technique
*/

#ifdef UNICODE
#define GD_MESSAGE_NAME L"GOLDENDICT_GET_WORD_IN_COORDINATES"
#else
#define GD_MESSAGE_NAME "GOLDENDICT_GET_WORD_IN_COORDINATES"
#endif

#pragma pack(push,1)

typedef struct {
	DWORD	dwSize;
	HWND	hWnd;
	POINT	Pt;
	DWORD	dwMaxLength;
	WCHAR	*cwData;
} GDDataStruct, *LPGDDataStruct;

#pragma pack(pop)

#endif
