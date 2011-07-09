#include <windows.h>
#include <winable.h>
#include "IAccExInt.h"

HRESULT GetIAccessibleExFromIAccessible( IAccessible *pAcc, long idChild, IAccessibleEx **ppaex )
{
*ppaex = NULL;
IAccessibleEx *paex;
IServiceProvider *pSp = NULL;
//char s[500];
    HRESULT hr = pAcc->QueryInterface( IID_IServiceProvider, (void **)&pSp );
/*
    wsprintf(s,"GD:QueryInterface (IServiceProvider) return hr=%08X, ptr=%p\n", hr, pSp);
    OutputDebugString(s);
*/
    if( hr != S_OK ) return hr;
    if( pSp == NULL ) return E_NOINTERFACE;

    paex = NULL;
    hr = pSp->QueryService( IID_IAccessibleEx, IID_IAccessibleEx, (void **)&paex );
    pSp->Release();
/*
    wsprintf(s,"GD:QueryService (IAccessibleEx) return hr=%08X, ptr=%p\n", hr, paex);
    OutputDebugString(s);
*/
    if( hr != S_OK ) return hr;
    if( paex == NULL ) return E_NOINTERFACE;

    if(idChild == CHILDID_SELF) {
        *ppaex = paex;
    } else {
        IAccessibleEx *paexChild = NULL;
        hr = paex->GetObjectForChild( idChild, &paexChild );
/*
        wsprintf(s,"GD: GetObjectForChild return hr=%08X, ptr=%p (ChildID=%i)\n", hr, paexChild, idChild);
        OutputDebugString(s);
*/
        paex->Release();
        if( hr != S_OK ) return hr;
        if(paexChild == NULL) return E_NOINTERFACE;
        *ppaex = paexChild;
    }
    return S_OK;
}

HRESULT GetIRawElementProviderFromIAccessible( IAccessible * pAcc, long idChild, IRawElementProviderSimple **ppEl )
{
*ppEl = NULL;
IAccessibleEx *paex;
//char s[500];
    HRESULT hr = GetIAccessibleExFromIAccessible( pAcc, idChild, &paex );
    if( hr != S_OK ) return hr;

    hr = paex->QueryInterface( IID_IRawElementProviderSimple, (void **)ppEl );
/*
    wsprintf(s,"GD:QueryInterface (IRawElementProviderSimple) return hr=%08X, ptr=%p\n", hr, ppEl);
    OutputDebugString(s);
*/
    paex->Release();
    return hr;
}

HRESULT GetPatternFromIAccessible( IAccessible * pAcc, long idChild, PATTERNID patternId, REFIID iid, void **ppv )
{
IRawElementProviderSimple * pel;
//char s[500];
    HRESULT hr = GetIRawElementProviderFromIAccessible( pAcc, idChild, &pel );
    if( hr != S_OK ) return hr;
    if( pel == NULL ) return E_NOINTERFACE;

    IUnknown * pPatternObject = NULL;
    hr = pel->GetPatternProvider( patternId, &pPatternObject );
/*
    wsprintf(s,"GD:GetPatternProvider return hr=%08X, ptr=%p\n", hr, pPatternObject);
    OutputDebugString(s);
*/
    pel->Release();
    if( hr != S_OK ) return hr;
    if( pPatternObject == NULL ) return E_NOINTERFACE;

    *ppv = NULL;
    hr = pPatternObject->QueryInterface( iid, ppv );
/*
    wsprintf(s,"GD:QueryInterface (TextPattern) return hr=%08X, ptr=%p\n", hr, ppv);
    OutputDebugString(s);
*/
    pPatternObject->Release();
    if( *ppv == NULL ) return E_NOINTERFACE;
    return hr;
}
