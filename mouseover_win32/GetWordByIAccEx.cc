#include <windows.h>
#include <wtypes.h>
#include "ThTypes.h"
#include "IAccExInt.h"
#include "GetWordByIAccEx.h"

GetPhysicalCursorPosFunc getPhysicalCursorPosFunc;

BOOL FindGetPhysicalCursorPos()
{
HMODULE hm;
	getPhysicalCursorPosFunc = NULL;
	hm = GetModuleHandle( "user32.dll" );
	if( hm != NULL ) {
		getPhysicalCursorPosFunc = (GetPhysicalCursorPosFunc)GetProcAddress( hm, "GetPhysicalCursorPos" );
	}
	return( getPhysicalCursorPosFunc != NULL );
}


HRESULT GetParentAccessibleObject( IAccessible* pAcc, IAccessible** ppAccParent )
{
IDispatch* pDispatch = NULL;

	*ppAccParent = NULL;
	if ( pAcc == NULL )
	       	return E_INVALIDARG;
	HRESULT hr = pAcc->get_accParent( &pDispatch );
	if ( ( hr == S_OK ) && ( pDispatch != NULL ) ) {
	        hr = pDispatch->QueryInterface( IID_IAccessible, (void**)ppAccParent );
	        pDispatch->Release();
	}
	return hr;
}


BOOL getWordByAccEx( POINT pt )
{
HRESULT hr;
IAccessible *pAcc, *pAccParent;
ITextProvider *pText;
ITextRangeProvider *pTextRange;
VARIANT var;
long idChild;
BSTR bstr = NULL;
int n;
UiaPoint upt;
POINT ppt = { 0, 0 };

	if( getPhysicalCursorPosFunc != NULL ) {
		getPhysicalCursorPosFunc( &ppt );
	} else {
		ppt = pt;
	}

	upt.x = ppt.x;
	upt.y = ppt.y;

	pAcc = NULL;
	hr = AccessibleObjectFromPoint( ppt, &pAcc, &var );
	idChild = var.lVal;

	if( hr != S_OK || pAcc == NULL) {
		VariantClear( &var );
		return FALSE;
	}

	pText = NULL;
	while( pAcc != NULL) {
		hr = GetPatternFromIAccessible( pAcc, 0, UIA_TextPatternId, IID_ITextProvider, (void **)&pText );
		if( hr == S_OK && pText != NULL )
			break;
		pAccParent = NULL;
		hr = GetParentAccessibleObject( pAcc, &pAccParent );
		pAcc->Release();
		pAcc = pAccParent;
	}
	if( pAcc == NULL ) 
		return FALSE;

	pAcc->Release();

	pTextRange = NULL;
	hr = pText->RangeFromPoint( upt, &pTextRange );
	if( hr != S_OK || pTextRange == NULL ) {
		pText->Release();
		return FALSE;
	}

	hr = pTextRange->ExpandToEnclosingUnit( TextUnit_Word );
	if( hr == S_OK) {
		bstr = NULL;
		hr = pTextRange->GetText( 255, &bstr );
		if (hr == S_OK) {
			n = SysStringLen( bstr );
			if( n != 0 ) {
				n = WideCharToMultiByte( CP_UTF8, 0, (LPCWSTR)bstr, n, GlobalData->CurMod.MatchedWord, sizeof( GlobalData->CurMod.MatchedWord ) - 1, NULL, NULL );
				GlobalData->CurMod.WordLen = n;
				GlobalData->CurMod.MatchedWord[n] = 0;
			}
			SysFreeString( bstr );
		}
	}
	pTextRange->Release();
	pText->Release();

	return TRUE;
}
