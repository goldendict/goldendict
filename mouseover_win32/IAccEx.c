#include <windows.h>
#include <winable.h>
#include "IAccExInt.h"

const long UIA_InvokePatternId              =   10000;
const long UIA_SelectionPatternId           =   10001;
const long UIA_ValuePatternId               =   10002;
const long UIA_RangeValuePatternId          =   10003;
const long UIA_ScrollPatternId              =   10004;
const long UIA_ExpandCollapsePatternId      =   10005;
const long UIA_GridPatternId                =   10006;
const long UIA_GridItemPatternId            =   10007;
const long UIA_MultipleViewPatternId        =   10008;
const long UIA_WindowPatternId              =   10009;
const long UIA_SelectionItemPatternId       =   10010;
const long UIA_DockPatternId                =   10011;
const long UIA_TablePatternId               =   10012;
const long UIA_TableItemPatternId           =   10013;
const long UIA_TextPatternId                =   10014;
const long UIA_TogglePatternId              =   10015;
const long UIA_TransformPatternId           =   10016;
const long UIA_ScrollItemPatternId          =   10017;
const long UIA_LegacyIAccessiblePatternId   =   10018;
const long UIA_ItemContainerPatternId       =   10019;
const long UIA_VirtualizedItemPatternId     =   10020;
const long UIA_SynchronizedInputPatternId   =   10021;

HRESULT GetIAccessibleExFromIAccessible( IAccessible *pAcc, long idChild, IAccessibleEx **ppaex )
{
*ppaex = NULL;
IAccessibleEx *paex;
IServiceProvider *pSp = NULL;
//char s[500];
    HRESULT hr = pAcc->lpVtbl->QueryInterface( pAcc, &IID_IServiceProvider, (void **)&pSp );
/*
    wsprintf(s,"GD:QueryInterface (IServiceProvider) return hr=%08X, ptr=%p\n", hr, pSp);
    OutputDebugString(s);
*/
    if( hr != S_OK ) return hr;
    if( pSp == NULL ) return E_NOINTERFACE;

    paex = NULL;
    hr = pSp->lpVtbl->QueryService( pSp, &IID_IAccessibleEx, &IID_IAccessibleEx, (void **)&paex );
    pSp->lpVtbl->Release( pSp );
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
        hr = paex->lpVtbl->GetObjectForChild( paex, idChild, &paexChild );
/*
        wsprintf(s,"GD: GetObjectForChild return hr=%08X, ptr=%p (ChildID=%i)\n", hr, paexChild, idChild);
        OutputDebugString(s);
*/
        paex->lpVtbl->Release( paex );
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

    hr = paex->lpVtbl->QueryInterface( paex, &IID_IRawElementProviderSimple, (void **)ppEl );
/*
    wsprintf(s,"GD:QueryInterface (IRawElementProviderSimple) return hr=%08X, ptr=%p\n", hr, ppEl);
    OutputDebugString(s);
*/
    paex->lpVtbl->Release( paex );
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
    hr = pel->lpVtbl->GetPatternProvider( pel, patternId, &pPatternObject );
/*
    wsprintf(s,"GD:GetPatternProvider return hr=%08X, ptr=%p\n", hr, pPatternObject);
    OutputDebugString(s);
*/
    pel->lpVtbl->Release( pel );
    if( hr != S_OK ) return hr;
    if( pPatternObject == NULL ) return E_NOINTERFACE;

    *ppv = NULL;
    hr = pPatternObject->lpVtbl->QueryInterface( pPatternObject, iid, ppv );
/*
    wsprintf(s,"GD:QueryInterface (TextPattern) return hr=%08X, ptr=%p\n", hr, ppv);
    OutputDebugString(s);
*/
    pPatternObject->lpVtbl->Release( pPatternObject );
    if( *ppv == NULL ) return E_NOINTERFACE;
    return hr;
}
