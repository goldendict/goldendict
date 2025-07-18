#define  _WIN32_WINNT 0x0501

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
		"VIRTUALCONSOLECLASS", // ConEmu
		};
	TKnownWndClass KnownClasses[] = {
		kwcRichEdit,
		kwcRichEdit,
		kwcRichEdit,
		kwcMultiLineEdit,
		kwcInternetExplorer_Server,
		kwcConsole,
		kwcConsole,
		kwcConEmu,
	};
	int i;
	for (i=0; i<8; i++) {
		if (_stricmp(WNDClass, StrKnownClasses[i])==0)
			break;
	}
	if (i<8) {
		if (KnownClasses[i] == kwcMultiLineEdit) {
			if ((GetWindowLong(WND, GWL_STYLE) & ES_MULTILINE) == 0)
				return kwcSingleLineEdit;
		}
		else if (KnownClasses[i] == kwcConEmu) {
			HWND hConsole = (HWND)(DWORD_PTR)GetWindowLongPtr(WND, 0);
			if (!hConsole || !IsWindow(hConsole))
				return kwcUnknown;
		}
		return KnownClasses[i];
	} else
		return kwcUnknown;
}

static BOOL Is_XP_And_Later()
{
	OSVERSIONINFO stOSVI;
	BOOL bRet;

	memset(&stOSVI, 0, sizeof(OSVERSIONINFO));
	stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bRet = GetVersionEx(&stOSVI);
	if (FALSE == bRet) return FALSE;
	return (VER_PLATFORM_WIN32_NT == stOSVI.dwPlatformId && (5 < stOSVI.dwMajorVersion || (5 == stOSVI.dwMajorVersion && 1 <= stOSVI.dwMinorVersion)));
}

static char* ExtractWordFromRichEditPos(HWND WND, POINT Pt, DWORD *BeginPos)
{
	return ExtractFromEverything(WND, Pt, BeginPos);
}

static char* ExtractWordFromEditPos(HWND hEdit, POINT Pt, DWORD *BeginPos)
{
	return ExtractFromEverything(hEdit, Pt, BeginPos);
}

static char* ExtractWordFromIE(HWND WND, POINT Pt, DWORD *BeginPos)
{	
	return ExtractFromEverything(WND, Pt, BeginPos);
}

typedef struct TConsoleParams {
	HWND WND;
	POINT Pt;
	RECT ClientRect;
	WCHAR Buffer[256];
	int BeginPos;
} TConsoleParams;

static int GetWordFromConsolePack(TConsoleParams *params, BOOL *pInvalidConsole)
{
	int WordLen=0;

	*pInvalidConsole = TRUE;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut != INVALID_HANDLE_VALUE && hStdOut != 0) {
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {

			*pInvalidConsole = FALSE;

			COORD CurPos;
			CurPos.X = csbi.srWindow.Left + (SHORT)(params->Pt.x * (csbi.srWindow.Right - csbi.srWindow.Left + 1) / params->ClientRect.right);
			CurPos.Y = csbi.srWindow.Top + (SHORT)(params->Pt.y * (csbi.srWindow.Bottom - csbi.srWindow.Top + 1) / params->ClientRect.bottom);
			if ((CurPos.X >= 0) && (CurPos.X <= csbi.dwSize.X - 1) && (CurPos.Y >= 0) && (CurPos.Y <= csbi.dwSize.Y - 1)) {
				int BegPos;
				WCHAR *Buf;

				params->BeginPos = CurPos.X;
				CurPos.X = 0;
				Buf = GlobalAlloc(GMEM_FIXED, (csbi.dwSize.X + 1)*sizeof(WCHAR));
				if (Buf) {
					DWORD ActualRead;
					if ((ReadConsoleOutputCharacterW(hStdOut, Buf, csbi.dwSize.X, CurPos, &ActualRead)) && (ActualRead == (DWORD)csbi.dwSize.X)) {
						BegPos=0;
						WordLen=ActualRead;
						if(WordLen>85) {
							while(params->BeginPos-BegPos>43 && WordLen>85) {
								BegPos++; WordLen--;
							}
							if(WordLen>85) WordLen=85;
							params->BeginPos -= BegPos;
						}
						if(WordLen) {
							memset(params->Buffer, 0, sizeof(params->Buffer));
							lstrcpynW(params->Buffer, Buf + BegPos, WordLen+1);
						}
					}
					GlobalFree(Buf);
				}
			}
		}
	}
	return WordLen;
}

static char* GetWordFromConsole(HWND WND, POINT Pt, DWORD *BeginPos)
{
	TConsoleParams *TP;
	DWORD pid;
	DWORD WordSize;
	char *Result;
	BOOL invalidConsole;

	*BeginPos=0;
	if((TP = malloc(sizeof(TConsoleParams))) == NULL)
		return(NULL);
	ZeroMemory(TP,sizeof(TConsoleParams));
	TP->WND = WND;
	TP->Pt = Pt;
	ScreenToClient(WND, &(TP->Pt));
	GetClientRect(WND, &(TP->ClientRect));

//	GetWindowThreadProcessId(GetParent(WND), &pid);
	GetWindowThreadProcessId(WND, &pid);

	if (pid != GetCurrentProcessId()) {
	        if(Is_XP_And_Later()) {
			if(AttachConsole(pid)) {
				WordSize = GetWordFromConsolePack(TP, &invalidConsole);
				FreeConsole();
			} else {
				WordSize = 0;
			}
		} else {
			WordSize = 0;
		}
	} else {
		WordSize = GetWordFromConsolePack(TP, &invalidConsole);
		if( invalidConsole ) {
			/*
			 Under Win 8.1 GetWindowThreadProcessId return current "conhost" process ID
			 instead of target window process ID.
			 We try to attach console to parent process.
			*/
			
		        if(Is_XP_And_Later()) {
				if(AttachConsole( (DWORD)-1 )) {
					WordSize = GetWordFromConsolePack(TP, &invalidConsole);
					FreeConsole();
				} else {
					WordSize = 0;
				}
			} else {
				WordSize = 0;
			}
		}	
	}

	if (WordSize > 0 && WordSize <= 255) {
		TEverythingParams CParams;

		ZeroMemory(&CParams, sizeof(CParams));
		CParams.Unicode=1;
		CParams.BeginPos=TP->BeginPos;
		CParams.WordLen=WordSize;
		CopyMemory(CParams.MatchedWordW, TP->Buffer, WordSize * sizeof(wchar_t));
		ConvertToMatchedWordA(&CParams);
		*BeginPos = CParams.BeginPos;
		Result = _strdup(CParams.MatchedWordA);

	} else {
		Result = NULL;
	}
	free(TP);
	return Result;
}

static char* GetWordFromConEmu(HWND WND, POINT Pt, DWORD *BeginPos)
{
	HWND hConsole = (HWND)(DWORD_PTR)GetWindowLongPtr(WND, 0);
	if (!hConsole || !IsWindow(hConsole))
		return NULL;
	
	RECT rcConEmu;
	if (!GetWindowRect(WND, &rcConEmu) || rcConEmu.right <= rcConEmu.left || rcConEmu.bottom <= rcConEmu.top)
		return NULL;
	RECT rcConsole;
	if (!GetClientRect(hConsole, &rcConsole) || rcConsole.right <= rcConsole.left || rcConsole.bottom <= rcConsole.top)
		return NULL;

	POINT ptReal = { (Pt.x - rcConEmu.left) * (rcConsole.right - rcConsole.left + 1) / ( rcConEmu.right - rcConEmu.left + 1),
					(Pt.y - rcConEmu.top) * (rcConsole.bottom - rcConsole.top + 1 ) / (rcConEmu.bottom - rcConEmu.top + 1) };
	ClientToScreen(hConsole, &ptReal);

	return GetWordFromConsole(hConsole, ptReal, BeginPos);
}

char* TryGetWordFromAnyWindow(TKnownWndClass WndType, HWND WND, POINT Pt, DWORD *BeginPos)
{
	typedef char* (*GetWordFunction_t)(HWND, POINT, DWORD*);
	const GetWordFunction_t GetWordFunction[]= {
		ExtractFromEverything,
		ExtractWordFromRichEditPos,
		ExtractWordFromEditPos,
		ExtractWordFromEditPos,
		ExtractWordFromIE,
		GetWordFromConsole,
		GetWordFromConEmu,
	};
	return GetWordFunction[WndType](WND, Pt, BeginPos);
}
