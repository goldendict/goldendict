#ifndef __SPHELPER_HH_INCLUDED__
#define __SPHELPER_HH_INCLUDED__

#ifndef SR_LOCALIZED_DESCRIPTION
#define SR_LOCALIZED_DESCRIPTION L"Description"
#endif

#ifndef REG_MUI_STRING_TRUNCATE
#define REG_MUI_STRING_TRUNCATE     0x00000001
#endif

#ifndef SPERR_NOT_FOUND
#define FACILITY_SAPI      FACILITY_ITF
#define SAPI_ERROR_BASE    0x5000
#define MAKE_SAPI_HRESULT(sev, err)    MAKE_HRESULT(sev, FACILITY_SAPI, err)
#define MAKE_SAPI_ERROR(err)           MAKE_SAPI_HRESULT(SEVERITY_ERROR, err + SAPI_ERROR_BASE)
#define SPERR_NOT_FOUND                MAKE_SAPI_ERROR(0x03a)
#endif

#ifdef _SAPI_VER
#undef _SAPI_VER
#endif
#define _SAPI_VER 0x053

inline void SpHexFromUlong(WCHAR * psz, ULONG ul)
{
    // If for some reason we cannot convert a number, set it to 0

    if (_ultow(ul, psz, 16))
    {
        psz[0] = L'0';
        psz[1] = 0;
    }
}

inline HRESULT SpGetTokenFromId(
    const WCHAR * pszTokenId,
    ISpObjectToken ** ppToken,
    BOOL fCreateIfNotExist = FALSE)
{
    HRESULT hr;
    ISpObjectToken * cpToken;
    hr = CoCreateInstance(CLSID_SpObjectToken, NULL, CLSCTX_INPROC_SERVER,
                          IID_ISpObjectToken, (void**)&cpToken);

    if (SUCCEEDED(hr))
    {
        hr = cpToken->SetId(NULL, pszTokenId, fCreateIfNotExist);
        if (SUCCEEDED(hr))
        {
            *ppToken = cpToken;
        }
        else
            cpToken->Release();
    }

    return hr;
}

inline HRESULT SpGetCategoryFromId(
    const WCHAR * pszCategoryId,
    ISpObjectTokenCategory ** ppCategory,
    BOOL fCreateIfNotExist = FALSE)
{
    HRESULT hr;

    ISpObjectTokenCategory * cpTokenCategory;
    hr = CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_INPROC_SERVER,
                          IID_ISpObjectTokenCategory, (void**)&cpTokenCategory );

    if (SUCCEEDED(hr))
    {
      hr = cpTokenCategory->SetId(pszCategoryId, fCreateIfNotExist);
      if (SUCCEEDED(hr))
      {
        *ppCategory = cpTokenCategory;
      }
      else
        cpTokenCategory->Release();
    }

    return hr;
}

HRESULT SpEnumTokens(
    const WCHAR * pszCategoryId,
    const WCHAR * pszReqAttribs,
    const WCHAR * pszOptAttribs,
    IEnumSpObjectTokens ** ppEnum)
{
    HRESULT hr = S_OK;

    ISpObjectTokenCategory * cpCategory;
    hr = SpGetCategoryFromId(pszCategoryId, &cpCategory);

    if (SUCCEEDED(hr))
    {
        hr = cpCategory->EnumTokens(
                    pszReqAttribs,
                    pszOptAttribs,
                    ppEnum);
        cpCategory->Release();
    }

    return hr;
}

inline HRESULT SpGetDescription(ISpObjectToken * pObjToken, WCHAR ** ppszDescription, LANGID Language = GetUserDefaultUILanguage())
{
    WCHAR szLangId[10];
    HRESULT hr = S_OK;

    if (ppszDescription == NULL)
    {
        return E_POINTER;
    }
    *ppszDescription = NULL;

#if _SAPI_VER >= 0x053
    WCHAR* pRegKeyPath = 0;
    WCHAR* pszTemp = 0;
    HKEY   Handle = NULL;

    // Windows Vista does not encourage localized strings in the registry
    // When running on Windows Vista query the localized engine name from a resource dll
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof( ver );

    if( ( ::GetVersionEx( &ver ) == TRUE ) && ( ver.dwMajorVersion >= 6 ) )
    {
        // If we reach this code we are running under Windows Vista
        HMODULE hmodAdvapi32Dll = NULL;
        typedef HRESULT (WINAPI* LPFN_RegLoadMUIStringW)(HKEY, LPCWSTR, LPWSTR, DWORD, LPDWORD, DWORD, LPCWSTR);
        LPFN_RegLoadMUIStringW pfnRegLoadMUIStringW = NULL;

        // Delay bind with RegLoadMUIStringW since this function is not supported on previous versions of advapi32.dll
        // RegLoadMUIStringW is supported only on advapi32.dll that ships with Windows Vista  and above
        // Calling RegLoadMUIStringW directly makes the loader try to resolve the function reference at load time which breaks,
        // hence we manually load advapi32.dll, query for the function pointer and invoke it.
        hmodAdvapi32Dll = ::LoadLibrary(TEXT("advapi32.dll"));
        if(hmodAdvapi32Dll)
        {
            pfnRegLoadMUIStringW = (LPFN_RegLoadMUIStringW) ::GetProcAddress(hmodAdvapi32Dll, "RegLoadMUIStringW");
            if (!pfnRegLoadMUIStringW)
            {
                // This should not happen in Vista
                // _ASSERT (pfnRegLoadMUIStringW);
                hr = TYPE_E_DLLFUNCTIONNOTFOUND;
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND);
        }

        if (SUCCEEDED(hr))
        {
            hr = pObjToken->GetId(&pszTemp);
        }

        if (SUCCEEDED(hr))
        {
            LONG   lErrorCode = ERROR_SUCCESS;

            pRegKeyPath = wcschr(pszTemp, L'\\');   // Find the first occurance of '\\' in the absolute registry key path
            if(pRegKeyPath)
            {
                *pRegKeyPath = L'\0';
                pRegKeyPath++;                         // pRegKeyPath now points to the path to the recognizer token under the HKLM or HKCR hive
                *ppszDescription = 0;

                // Open the registry key for read and get the handle
                if (wcsncmp(pszTemp, L"HKEY_LOCAL_MACHINE", MAX_PATH) == 0)
                {
                    lErrorCode = RegOpenKeyExW(HKEY_LOCAL_MACHINE, pRegKeyPath, 0, KEY_QUERY_VALUE, &Handle);
                }
                else if (wcsncmp(pszTemp, L"HKEY_CURRENT_USER", MAX_PATH) == 0)
                {
                    lErrorCode = RegOpenKeyExW(HKEY_CURRENT_USER, pRegKeyPath, 0, KEY_QUERY_VALUE, &Handle);
                }
                else
                {
                    lErrorCode = ERROR_BAD_ARGUMENTS;
                }

                // Use MUI RegLoadMUIStringW API to load the localized string
                if(ERROR_SUCCESS == lErrorCode)
                {
                    *ppszDescription = (WCHAR*) CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR)); // This should be enough memory to allocate the localized Engine Name
                    lErrorCode = (*pfnRegLoadMUIStringW) (Handle, SR_LOCALIZED_DESCRIPTION, *ppszDescription, MAX_PATH * sizeof(WCHAR), NULL, REG_MUI_STRING_TRUNCATE, NULL);
                }
            }
            else
            {
                // pRegKeyPath should never be 0 if we are querying for relative hkey path
                lErrorCode = ERROR_BAD_ARGUMENTS;
            }

            hr = HRESULT_FROM_WIN32(lErrorCode);
        }

        // Close registry key handle
        if(Handle)
        {
            RegCloseKey(Handle);
        }
        // Free memory allocated to locals
        if(pszTemp)
        {
            CoTaskMemFree(pszTemp);
        }
        if (hmodAdvapi32Dll)
        {
            ::FreeLibrary(hmodAdvapi32Dll);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }

    // If running on OSes released before Windows Vista query the localized string from the registry
    // If RegLoadMUIStringW failed to retrieved the localized Engine name retrieve the localized string from the fallback (Default) attribute
#else
    hr = E_FAIL;
#endif // _SAPI_VER >= 0x053
    if (FAILED(hr))
    {
        // Free memory allocated above if necessary
        if (*ppszDescription != NULL)
        {
            CoTaskMemFree(*ppszDescription);
            *ppszDescription = NULL;
        }

        SpHexFromUlong(szLangId, Language);
        hr = pObjToken->GetStringValue(szLangId, ppszDescription);
        if (hr == SPERR_NOT_FOUND)
        {
            hr = pObjToken->GetStringValue(NULL, ppszDescription);
        }
    }

    return hr;
}

#endif
