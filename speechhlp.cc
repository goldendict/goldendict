#define WINVER 0x0500 // At least WinXP required
#include <windows.h>
#include <limits.h>

#include "speechhlp.hh"
#include <string>
#include "sapi.hh"
#include "sphelper.hh"

using std::wstring;

struct _SpeechHelper
{
    ISpVoice * voice;
    wstring engineId;
    wstring engineName;
    bool willInvokeCoUninitialize;

    _SpeechHelper() :
        willInvokeCoUninitialize(false)
    {
        HRESULT hr;
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        willInvokeCoUninitialize = (hr != RPC_E_CHANGED_MODE);
        CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_ISpVoice, (void**)&voice);
    }

    ~_SpeechHelper()
    {
        if (voice)
            voice->Release();

        if (willInvokeCoUninitialize)
            CoUninitialize();
    }
};

SpeechHelper speechCreate(const wchar_t *engineId)
{
    SpeechHelper sp = new _SpeechHelper();
    HRESULT hr;
    ISpObjectToken * spToken;

    hr = SpGetTokenFromId(engineId, &spToken);

    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr) && sp->voice)
        {
            sp->voice->SetVoice(spToken);

            WCHAR * engineName = NULL;
            SpGetDescription( spToken, &engineName );
            sp->engineId = engineId;
            if (engineName)
            {
                sp->engineName = engineName;
                CoTaskMemFree(engineName);
            }
        }

        spToken->Release();
    }

    return sp;
}

void speechDestroy(SpeechHelper sp)
{
    delete sp;
}

bool speechAvailable(SpeechHelper sp)
{
    if (!sp)
        return false;

    return !!(sp->voice);
}

void speechEnumerateAvailableEngines(EnumerateCallback callback, void *userData)
{
    HRESULT hr;
    IEnumSpObjectTokens * enumSpTokens;
    ULONG count = 0;
    bool next = true;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool willInvokeCoUninitialize = (hr != RPC_E_CHANGED_MODE);
    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &enumSpTokens);
    if (SUCCEEDED(hr))
        hr = enumSpTokens->GetCount(&count);

    for (ULONG i = 0; i < count && next; i++)
    {
        ISpObjectToken * spToken = NULL;
        WCHAR * engineName = NULL;
        WCHAR * engineId = NULL;

        if (SUCCEEDED(hr))
            hr = enumSpTokens->Next(1, &spToken, NULL);
        if (SUCCEEDED(hr))
            hr = SpGetDescription(spToken, &engineName);
        if (SUCCEEDED(hr))
            hr = spToken->GetId(&engineId);

        if (SUCCEEDED(hr))
            next = callback(spToken, engineId, engineName, userData);

        if( spToken )
          spToken->Release();

        if (engineName)
            CoTaskMemFree(engineName);
        if (engineId)
            CoTaskMemFree(engineId);
    }

    if( enumSpTokens )
      enumSpTokens->Release();

    if (willInvokeCoUninitialize)
        CoUninitialize();
}

const wchar_t * speechEngineId(SpeechHelper sp)
{
    if (!sp)
        return NULL;

    return sp->engineId.c_str();
}

const wchar_t * speechEngineName(SpeechHelper sp)
{
    if (!sp)
        return NULL;

    return sp->engineName.c_str();
}

bool speechTell(SpeechHelper sp, const wchar_t *text)
{
    if (!sp || !sp->voice || !text)
        return false;

    HRESULT hr = sp->voice->Speak(text, SPF_ASYNC | SPF_IS_NOT_XML, 0);
    return !!SUCCEEDED(hr);
}

bool speechTellFinished(SpeechHelper sp)
{
    if (!sp || !sp->voice)
        return true;

    SPVOICESTATUS es;
    sp->voice->GetStatus(&es, NULL);
    return es.dwRunningState == SPRS_DONE;
}

bool speechSay(SpeechHelper sp, const wchar_t *text)
{
    if (!sp || !sp->voice || !text)
        return false;

    HRESULT hr = sp->voice->Speak(text, SPF_IS_NOT_XML, 0);
    return !!SUCCEEDED(hr);
}

int setSpeechVolume( SpeechHelper sp, int newVolume )
{
  if( !sp || !sp->voice || newVolume < 0 || newVolume > 100 )
    return -1;
  unsigned short oldVolume;
  HRESULT hr = sp->voice->GetVolume( &oldVolume );
  if( !SUCCEEDED( hr ) )
    return -1;
  sp->voice->SetVolume( (unsigned short) newVolume );
  return oldVolume;
}

int setSpeechRate( SpeechHelper sp, int newRate )
{
  if( !sp || !sp->voice || newRate < 0 || newRate > 100 )
    return -1;
  long oldRate;
  HRESULT hr = sp->voice->GetRate( &oldRate );
  if( !SUCCEEDED( hr ) )
    return -1;
  sp->voice->SetRate( ( newRate - 50 ) / 5 );
  return oldRate * 5 + 50;
}
