#include "speechhlp.h"

#include <string>
#include <sapi.h>
#include <sphelper.h>

using std::wstring;

struct _SpeechHelper
{
    CComPtr<ISpVoice> voice;
    wstring engineId;
    wstring engineName;
    bool willInvokeCoUninitialize;

    _SpeechHelper() : willInvokeCoUninitialize(false)
    {
        HRESULT hr;
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        willInvokeCoUninitialize = (hr != RPC_E_CHANGED_MODE);
        voice.CoCreateInstance(CLSID_SpVoice);
    }

    ~_SpeechHelper()
    {
        if (voice)
            voice.Release();

        if (willInvokeCoUninitialize)
            CoUninitialize();
    }
};

static bool findByEngineName(void *token, const wchar_t *id, const wchar_t *name, void *userData)
{
    SpeechHelper sp = (SpeechHelper)userData;
    if (sp->engineId == id)
    {
        sp->voice->SetVoice((ISpObjectToken *)token);
        sp->engineName = name;
        return TRUE;
    }

    return FALSE;
}

SPEECHHLP_EXPORTS SpeechHelper
speechCreate(const wchar_t *engineId)
{
    SpeechHelper sp = new _SpeechHelper();

    sp->engineId = engineId;
    speechEnumerateAvailableEngines(findByEngineName, sp);
    return sp;
}

SPEECHHLP_EXPORTS void
speechDestroy(SpeechHelper sp)
{
    delete sp;
}

SPEECHHLP_EXPORTS bool
speechAvailable(SpeechHelper sp)
{
    if (!sp)
        return false;

    return !!(sp->voice);
}

SPEECHHLP_EXPORTS void
speechEnumerateAvailableEngines(EnumerateCallback callback, void *userData)
{
    HRESULT hr;
    CComPtr<IEnumSpObjectTokens> enumSpTokens;
    ULONG count = 0;
    bool next = true;
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    bool willInvokeCoUninitialize = (hr != RPC_E_CHANGED_MODE);
    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &enumSpTokens);
    hr = enumSpTokens->GetCount(&count);

    for (ULONG i = 0; i < count && next; i++)
    {
        CComPtr<ISpObjectToken> spToken;
        WCHAR * engineName = NULL;
        WCHAR * engineId = NULL;

        if (SUCCEEDED(hr))
            hr = enumSpTokens->Next(1, &spToken, NULL);
        if (SUCCEEDED(hr))
            hr = SpGetDescription(spToken, &engineName);
        if (SUCCEEDED(hr))
            hr = spToken->GetId(&engineId);

        if (SUCCEEDED(hr))
        {
            next = callback(spToken, engineId, engineName, userData);
        }

        spToken.Release();

        if (engineName)
            CoTaskMemFree(engineName);
        if (engineId)
            CoTaskMemFree(engineId);
    }

    if (willInvokeCoUninitialize)
        CoUninitialize();
}

SPEECHHLP_EXPORTS const wchar_t *
speechEngineId(SpeechHelper sp)
{
    if (!sp)
        return NULL;

    return sp->engineId.c_str();
}

SPEECHHLP_EXPORTS const wchar_t *
speechEngineName(SpeechHelper sp)
{
    if (!sp)
        return NULL;

    return sp->engineName.c_str();
}

SPEECHHLP_EXPORTS bool
speechTell(SpeechHelper sp, const wchar_t *text)
{
    if (!sp || !sp->voice || !text)
        return false;

    HRESULT hr = sp->voice->Speak(text, SPF_ASYNC | SPF_IS_NOT_XML, 0);
    return !!SUCCEEDED(hr);
}

SPEECHHLP_EXPORTS bool
speechTellFinished(SpeechHelper sp)
{
    if (!sp || !sp->voice)
        return true;

    SPVOICESTATUS es;
    sp->voice->GetStatus(&es, NULL);
    return es.dwRunningState == SPRS_DONE;
}

SPEECHHLP_EXPORTS bool
speechSay(SpeechHelper sp, const wchar_t *text)
{
    if (!sp || !sp->voice || !text)
        return false;

    HRESULT hr = sp->voice->Speak(text, SPF_IS_NOT_XML, 0);
    return !!SUCCEEDED(hr);
}
