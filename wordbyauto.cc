#include <windows.h>
#include <servprov.h>
#include "wordbyauto.hh"
#include "uiauto.hh"

#include <cstdio>
#include "gddebug.hh"

class GDAutomationClient {
public:
    GDAutomationClient();
    ~GDAutomationClient();
    bool getWordAtPoint( POINT pt );
    WCHAR *getText() { return buffer; }
private:
    WCHAR buffer[256];
    IUIAutomation *pGDAutomation;
    IUIAutomationTreeWalker *pTree;
};

GDAutomationClient gdAuto;

GDAutomationClient::GDAutomationClient()
{
HRESULT hr;
    CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    hr = CoCreateInstance( CLSID_CUIAutomation , NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&pGDAutomation );
    if( hr != S_OK ) pGDAutomation = NULL;
    pTree = NULL;
    if( pGDAutomation != NULL )
        hr = pGDAutomation->get_RawViewWalker( &pTree );
    memset( buffer, 0, sizeof(buffer) );
}

GDAutomationClient::~GDAutomationClient()
{
    if( pTree != NULL ) pTree->Release();
    if( pGDAutomation != NULL ) pGDAutomation->Release();
    CoUninitialize();
}

bool GDAutomationClient::getWordAtPoint( POINT pt )
{
HRESULT hr;
IUIAutomationTextPattern *pTextPattern;
IUIAutomationTextRange *pTextRange;
IUIAutomationElement *pElement, *pParent;
BSTR bstr;
RECT r = { 0, 0, 0, 0 };
bool bGoUp;

    GD_DPRINTF("\nEntering getWordAtPoint\n");

    if( pGDAutomation == NULL ) return false;

    buffer[0] = 0;
    pElement = NULL;
    hr = pGDAutomation->ElementFromPoint( pt, &pElement );
    GD_DPRINTF("ElementFromPoint return hr=%08lX, ptr=%p\n", hr, pElement);
    if( hr != S_OK || pElement == NULL )
        return false;

    pTextPattern = NULL;
    bGoUp = false;
    while( pElement != NULL ) {
        hr = pElement->GetCurrentPatternAs( UIA_TextPatternId, IID_IUIAutomationTextPattern, (void**)&pTextPattern );
        if( hr == S_OK && pTextPattern != NULL )
            break;
        if( pTree == NULL ) {
            pElement->Release();
            return false;
        }
        pParent = NULL;
        hr = pTree->GetParentElement( pElement, &pParent );
        pElement->Release();
        pElement = pParent;
        bGoUp = TRUE;
    }
    if( pElement == NULL )
        return false;

    if( !bGoUp ) {
        hr = pElement->get_CurrentBoundingRectangle( &r );
        if( hr == S_OK) {
            pt.x -= r.left;
            pt.y -= r.top;
        }
    }
    pElement->Release();

    pTextRange = NULL;
    hr = pTextPattern->RangeFromPoint( pt, &pTextRange );
    pTextPattern->Release();
    if( hr != S_OK || pTextRange == NULL )
        return false;

    hr = pTextRange->ExpandToEnclosingUnit( TextUnit_Word );
    if( hr == S_OK) {
        hr = pTextRange->GetText( 255, &bstr );
        if (hr == S_OK) {
            wsprintfW( buffer, L"%s", (LPCWSTR)bstr );
            SysFreeString( bstr );
        }
    }
    pTextRange->Release();

    return ( buffer[0] != 0 );
}

WCHAR *gdGetWordAtPointByAutomation( POINT pt )
{
    if( gdAuto.getWordAtPoint( pt ) ) return gdAuto.getText();
    else return NULL;
}
