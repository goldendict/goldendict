#ifndef __SPEECHHLP_H__
#define __SPEECHHLP_H__

#ifdef SPEECHHLP_DLL
#   define SPEECHHLP_EXPORTS extern __declspec(dllexport)
#else
#   define SPEECHHLP_EXPORTS extern __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SpeechHelper *SpeechHelper;
typedef bool (*EnumerateCallback)(void *token, const wchar_t *id, const wchar_t *name, void *userData);

SPEECHHLP_EXPORTS SpeechHelper speechCreate(const wchar_t *engineId);
SPEECHHLP_EXPORTS void speechDestroy(SpeechHelper sp);
SPEECHHLP_EXPORTS bool speechAvailable(SpeechHelper sp);
SPEECHHLP_EXPORTS void speechEnumerateAvailableEngines(EnumerateCallback callback, void *userData);
SPEECHHLP_EXPORTS const wchar_t * speechEngineId(SpeechHelper sp);
SPEECHHLP_EXPORTS const wchar_t * speechEngineName(SpeechHelper sp);
SPEECHHLP_EXPORTS bool speechTell(SpeechHelper sp, const wchar_t *text);
SPEECHHLP_EXPORTS bool speechTellFinished(SpeechHelper sp);
SPEECHHLP_EXPORTS bool speechSay(SpeechHelper sp, const wchar_t *text);

#ifdef __cplusplus
}
#endif

#endif // __SPEECHHLP_H__
