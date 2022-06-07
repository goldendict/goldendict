#ifndef __SPEECHHLP_H__
#define __SPEECHHLP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SpeechHelper *SpeechHelper;
typedef bool (*EnumerateCallback)(void *token, const wchar_t *id, const wchar_t *name, void *userData);

SpeechHelper speechCreate(const wchar_t *engineId);
void speechDestroy(SpeechHelper sp);
bool speechAvailable(SpeechHelper sp);
void speechEnumerateAvailableEngines(EnumerateCallback callback, void *userData);
const wchar_t * speechEngineId(SpeechHelper sp);
const wchar_t * speechEngineName(SpeechHelper sp);
bool speechTell(SpeechHelper sp, const wchar_t *text);
bool speechTellFinished(SpeechHelper sp);
bool speechSay(SpeechHelper sp, const wchar_t *text);
int setSpeechVolume( SpeechHelper sp, int newVolume );
int setSpeechRate( SpeechHelper sp, int newRate );

#ifdef __cplusplus
}
#endif

#endif // __SPEECHHLP_H__
