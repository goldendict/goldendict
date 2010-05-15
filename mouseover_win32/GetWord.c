#include "GetWord.h"
#include "TextOutHook.h"

TKnownWndClass GetWindowType(HWND WND, const char* WNDClass)
{
	const char* StrKnownClasses[] = {
		"RICHEDIT20A",
		"RICHEDIT20W",
		"RICHEDIT",
		"EDIT",
		"INTERNET EXPLORER_SERVER",
		"CONSOLEWINDOWCLASS", // NT
		"TTYGRAB", // 9x
		};
	TKnownWndClass KnownClasses[] = {
		kwcRichEdit,
		kwcRichEdit,
		kwcRichEdit,
		kwcMultiLineEdit,
		kwcInternetExplorer_Server,
		kwcConsole,
		kwcConsole,
	};
	int i;
	for (i=0; i<7; i++) {
		if (_stricmp(WNDClass, StrKnownClasses[i])==0)
			break;
	}
	if (i<7) {
		if (KnownClasses[i] == kwcMultiLineEdit) {
			if ((GetWindowLong(WND, GWL_STYLE) & ES_MULTILINE) == 0)
				return kwcSingleLineEdit;
		}
		return KnownClasses[i];
	} else
		return kwcUnknown;
}

static char* ExtractWordFromRichEditPos(HWND WND, POINT Pt, int *BeginPos)
{
	return ExtractFromEverything(WND, Pt, BeginPos);
}
/*
typedef struct TEditParams {
	HWND WND;
	POINT Pt;
	char Buffer[256];
} TEditParams;

static int ExtractWordFromEditPosPack(TEditParams *params)
{
	int Result = 0;
	int BegPos;
	BegPos = SendMessage(params->WND, EM_CHARFROMPOS, 0, params->Pt.x | params->Pt.y << 16);
	if (BegPos == -1)
		return Result;
	int MaxLength;
	MaxLength = SendMessage(params->WND, EM_LINELENGTH, BegPos & 0xFFFF, 0);
	if (MaxLength <= 0)
		return Result;
	char *Buf;
	Buf = GlobalAlloc(GMEM_FIXED, MaxLength + 1);
	if (Buf) {
		*Buf = MaxLength;
		MaxLength = SendMessage(params->WND, EM_GETLINE, BegPos >> 16, (int)Buf);
		Buf[MaxLength] = '\0';
		BegPos = (BegPos & 0xFFFF) - SendMessage(params->WND, EM_LINEINDEX, BegPos >> 16, 0) - 1;
		int EndPos;
		EndPos = BegPos;
		while ((BegPos >= 0) && IsCharAlpha(Buf[BegPos]))
			BegPos--;
		while ((EndPos < MaxLength) && IsCharAlpha(Buf[EndPos]))
			EndPos++;
		MaxLength = EndPos - BegPos - 1;
		if (MaxLength >= 0) {
			if (255 >= MaxLength) {
				Buf[EndPos] = '\0';
				lstrcpy(params->Buffer, Buf + BegPos + 1);
				Result = MaxLength;
			}
		}
		GlobalFree(Buf);
	}
	return Result;
}
*/
static char* ExtractWordFromEditPos(HWND hEdit, POINT Pt, int *BeginPos)
{
	return ExtractFromEverything(hEdit, Pt, BeginPos);
/*	TEditParams *TP;
	TP = malloc(sizeof(TEditParams));
	TP->WND = hEdit;
	TP->Pt = Pt;
	TP->Buffer[0] = '\0';
	ScreenToClient(hEdit, &(TP->Pt));
	int MaxLength;
	MaxLength = ExtractWordFromEditPosPack(TP);
	char *Result;
	if (MaxLength>0) {
		Result = strdup(TP->Buffer);
	} else {
		Result = NULL;
	}
	free(TP);
	return Result;
*/
}

static char* ExtractWordFromIE(HWND WND, POINT Pt, int *BeginPos)
{	
	return ExtractFromEverything(WND, Pt, BeginPos);
}

typedef struct TConsoleParams {
	HWND WND;
	POINT Pt;
	RECT ClientRect;
	char Buffer[256];
} TConsoleParams;

static int GetWordFromConsolePack(TConsoleParams *params)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut != INVALID_HANDLE_VALUE) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
			COORD CurPos;
			CurPos.X = csbi.srWindow.Left + (SHORT)(params->Pt.x * (csbi.srWindow.Right - csbi.srWindow.Left + 1) / params->ClientRect.right);
			CurPos.Y = csbi.srWindow.Top + (SHORT)(params->Pt.y * (csbi.srWindow.Bottom - csbi.srWindow.Top + 1) / params->ClientRect.bottom);
			if ((CurPos.X >= 0) && (CurPos.X <= csbi.dwSize.X - 1) && (CurPos.Y >= 0) && (CurPos.Y <= csbi.dwSize.Y - 1)) {
				int BegPos;
				char *Buf;

				BegPos = CurPos.X;
				CurPos.X = 0;
				Buf = GlobalAlloc(GMEM_FIXED, csbi.dwSize.X + 1);
				if (Buf) {
					DWORD ActualRead;
					if ((ReadConsoleOutputCharacter(hStdOut, Buf, csbi.dwSize.X, CurPos, &ActualRead)) && (ActualRead == csbi.dwSize.X)) {
						int WordLen;

						OemToCharBuff(Buf, Buf, csbi.dwSize.X);
						if (csbi.dwSize.X > 255)
							WordLen = 255;
						else
							WordLen = csbi.dwSize.X;
						strncpy(params->Buffer, Buf, WordLen);
						GlobalFree(Buf);
						return WordLen;
					}
				}
			}
		}
	}
	return 0;
}
static void GetWordFromConsolePackEnd() {}

static BOOL RemoteExecute(HANDLE hProcess, void *RemoteThread, size_t RemoteSize, void *Data, int DataSize, DWORD *dwReturn)
{
	void *pRemoteThread = VirtualAllocEx(hProcess, NULL, RemoteSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	void *pData;
	HANDLE hThread;

	if (!pRemoteThread)
		return FALSE;
	if (!WriteProcessMemory(hProcess, pRemoteThread, RemoteThread, RemoteSize, 0)) {
		VirtualFreeEx(hProcess, pRemoteThread, RemoteSize, MEM_RELEASE);
		return FALSE;
	}
	pData = VirtualAllocEx(hProcess, NULL, DataSize, MEM_COMMIT, PAGE_READWRITE);
	if (!pData) {
		VirtualFreeEx(hProcess, pRemoteThread, RemoteSize, MEM_RELEASE);
		return FALSE;
	}
	if (!WriteProcessMemory(hProcess, pData, Data, DataSize, 0)) {
		VirtualFreeEx(hProcess, pRemoteThread, RemoteSize, MEM_RELEASE);
		VirtualFreeEx(hProcess, pData, DataSize, MEM_RELEASE);
		return FALSE;
	}
	// Bug: I don't know why the next line will fail in Windows XP, so get word from cmd.exe can't work presently.
	hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pRemoteThread, pData, 0, 0);
	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, dwReturn);
	ReadProcessMemory(hProcess, pData, Data, DataSize, 0);
	VirtualFreeEx(hProcess, pRemoteThread, RemoteSize, MEM_RELEASE);
	VirtualFreeEx(hProcess, pData, DataSize, MEM_RELEASE);
	if (hThread) {
		CloseHandle(hThread);
		return TRUE;
	} else {
		return FALSE;
	}
}

static char* GetWordFromConsole(HWND WND, POINT Pt, int *BeginPos)
{
	TConsoleParams *TP;
	DWORD pid;
	DWORD MaxWordSize;
	char *Result;

	TP = malloc(sizeof(TConsoleParams));
	TP->WND = WND;
	TP->Pt = Pt;
	ScreenToClient(WND, &(TP->Pt));
	GetClientRect(WND, &(TP->ClientRect));
	
	GetWindowThreadProcessId(GetParent(WND), &pid);

	if (pid != GetCurrentProcessId()) {
		// The next line will fail in Win2k, but OK in Windows XP.
		HANDLE ph = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
		if (ph) {
			if (!RemoteExecute(ph, GetWordFromConsolePack, (size_t)GetWordFromConsolePackEnd - (size_t)GetWordFromConsolePack, TP, sizeof(TConsoleParams), &MaxWordSize))
				MaxWordSize = 0;
			CloseHandle(ph);
		}
	} else {
		MaxWordSize = GetWordFromConsolePack(TP);
	}

	if (MaxWordSize > 0) {
		Result = _strdup(TP->Buffer);
	} else {
		Result = NULL;
	}
	free(TP);
	return Result;
}

char* TryGetWordFromAnyWindow(TKnownWndClass WndType, HWND WND, POINT Pt, int *BeginPos)
{
	typedef char* (*GetWordFunction_t)(HWND, POINT, int*);
	const GetWordFunction_t GetWordFunction[]= {
		ExtractFromEverything,
		ExtractWordFromRichEditPos,
		ExtractWordFromEditPos,
		ExtractWordFromEditPos,
		ExtractWordFromIE,
		GetWordFromConsole,
	};
	return GetWordFunction[WndType](WND, Pt, BeginPos);
}
