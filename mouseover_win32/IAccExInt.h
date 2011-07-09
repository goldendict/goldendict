#ifndef __UIAUTO_HH_INCLUDED__
#define __UIAUTO_HH_INCLUDED__

//#include <objbase.h>
#include <unknwn.h>
#include <oaidl.h>
#include <oleacc.h>
#include <servprov.h>

#ifdef __cplusplus
extern "C" {
#endif

EXTERN_C const IID IID_IAccessibleEx;
EXTERN_C const IID IID_IRawElementProviderSimple;
EXTERN_C const IID IID_ITextProvider;
EXTERN_C const IID IID_ITextRangeProvider;

typedef interface IAccessibleEx IAccessibleEx;
typedef interface IRawElementProviderSimple IRawElementProviderSimple;
typedef interface ITextProvider ITextProvider;
typedef interface ITextRangeProvider ITextRangeProvider;

typedef int PROPERTYID;
typedef int PATTERNID;
typedef int TEXTATTRIBUTEID;

struct UiaPoint
{
    double x;
    double y;
};

enum ProviderOptions
{
    ProviderOptions_ClientSideProvider      = 0x1,
    ProviderOptions_ServerSideProvider      = 0x2,
    ProviderOptions_NonClientAreaProvider   = 0x4,
    ProviderOptions_OverrideProvider        = 0x8,
    ProviderOptions_ProviderOwnsSetFocus    = 0x10,
    ProviderOptions_UseComThreading         = 0x20
};

enum SupportedTextSelection
{
    SupportedTextSelection_None     = 0,
    SupportedTextSelection_Single   = 1,
    SupportedTextSelection_Multiple = 2
};

enum TextUnit
{
    TextUnit_Character  = 0,
    TextUnit_Format     = 1,
    TextUnit_Word       = 2,
    TextUnit_Line       = 3,
    TextUnit_Paragraph  = 4,
    TextUnit_Page       = 5,
    TextUnit_Document   = 6
};

enum TextPatternRangeEndpoint
{
    TextPatternRangeEndpoint_Start  = 0,
    TextPatternRangeEndpoint_End    = 1
};

/* UIA_PatternIds */
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

#define INTERFACE ITextProvider
DECLARE_INTERFACE_(ITextProvider, IUnknown)
{
    STDMETHOD(GetSelection)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(GetVisibleRanges)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(RangeFromChild)(THIS_ IRawElementProviderSimple *, ITextRangeProvider **) PURE;
    STDMETHOD(RangeFromPoint)(THIS_ struct UiaPoint, ITextRangeProvider **pRetVal) PURE;
    STDMETHOD(get_DocumentRange)(THIS_ ITextRangeProvider **) PURE;
    STDMETHOD(get_SupportedTextSelection)(THIS_ enum SupportedTextSelection *) PURE;
};
#undef INTERFACE

#define INTERFACE ITextRangeProvider
DECLARE_INTERFACE_(ITextRangeProvider, IUnknown)
{
    STDMETHOD(Clone)(THIS_ ITextRangeProvider **) PURE;
    STDMETHOD(Compare)(THIS_ ITextRangeProvider *, BOOL *) PURE;
    STDMETHOD(CompareEndpoints)(THIS_ enum TextPatternRangeEndpoint, ITextRangeProvider *, enum TextPatternRangeEndpoint, int *) PURE;
    STDMETHOD(ExpandToEnclosingUnit)(THIS_ enum TextUnit) PURE;
    STDMETHOD(FindAttribute)(THIS_ TEXTATTRIBUTEID, VARIANT, BOOL, ITextRangeProvider **) PURE;
    STDMETHOD(FindText)(THIS_ BSTR, BOOL, BOOL, ITextRangeProvider **) PURE;
    STDMETHOD(GetAttributeValue)(THIS_ TEXTATTRIBUTEID, VARIANT *) PURE;
    STDMETHOD(GetBoundingRectangles)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(GetEnclosingElement)(THIS_ IRawElementProviderSimple **) PURE;
    STDMETHOD(GetText)(THIS_ int, BSTR *) PURE;
    STDMETHOD(Move)(THIS_ enum TextUnit, int, int *) PURE;
    STDMETHOD(MoveEndpointByUnit)(THIS_ enum TextPatternRangeEndpoint, enum TextUnit, int *) PURE;
    STDMETHOD(MoveEndpointByRange)(THIS_ enum TextPatternRangeEndpoint, ITextRangeProvider *, enum TextPatternRangeEndpoint) PURE;
    STDMETHOD(Select)(THIS) PURE;
    STDMETHOD(AddToSelection)(THIS) PURE;
    STDMETHOD(RemoveFromSelection)(THIS) PURE;
    STDMETHOD(ScrollIntoView)(THIS_ BOOL) PURE;
    STDMETHOD(GetChildren)(THIS_ SAFEARRAY **) PURE;
};
#undef INTERFACE

#define INTERFACE IRawElementProviderSimple
DECLARE_INTERFACE_(IRawElementProviderSimple, IUnknown)
{
    STDMETHOD(get_ProviderOptions)(THIS_ enum ProviderOptions *) PURE;
    STDMETHOD(GetPatternProvider)(THIS_ PATTERNID, IUnknown **) PURE;
    STDMETHOD(GetPropertyValue)(THIS_ PROPERTYID, VARIANT *) PURE;
    STDMETHOD(get_HostRawElementProvider)(THIS_ IRawElementProviderSimple **) PURE;
};
#undef INTERFACE


#define INTERFACE IAccessibleEx
DECLARE_INTERFACE_(IAccessibleEx, IUnknown)
{
    STDMETHOD(GetObjectForChild)(THIS_ long, IAccessibleEx **) PURE;
    STDMETHOD(GetIAccessiblePair)(THIS_ IAccessible **, long *) PURE;
    STDMETHOD(GetRuntimeId)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(ConvertReturnedElement)(THIS_ IRawElementProviderSimple *, IAccessibleEx **) PURE;
};
#undef INTERFACE

HRESULT GetIAccessibleExFromIAccessible( IAccessible *pAcc, long idChild, IAccessibleEx **ppaex );
HRESULT GetIRawElementProviderFromIAccessible( IAccessible * pAcc, long idChild, IRawElementProviderSimple **ppEl );
HRESULT GetPatternFromIAccessible( IAccessible * pAcc, long idChild, PATTERNID patternId, REFIID iid, void **ppv );

#ifdef __cplusplus
}
#endif

#endif // UIAUTO_HH
