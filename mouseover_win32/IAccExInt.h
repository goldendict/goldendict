#ifndef __UIAUTO_HH_INCLUDED__
#define __UIAUTO_HH_INCLUDED__

//#include <objbase.h>
#include <unknwn.h>
#include <oaidl.h>
#include <oleacc.h>
#include <servprov.h>

#ifdef INTERFACE
#undef INTERFACE
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern const IID IID_IAccessibleEx;
extern const IID IID_IRawElementProviderSimple;
extern const IID IID_ITextProvider;
extern const IID IID_ITextRangeProvider;

typedef int PROPERTYID;
typedef int PATTERNID;
typedef int TEXTATTRIBUTEID;

typedef struct _UiaPoint
{
    double x;
    double y;
} UiaPoint;

typedef enum
{
    ProviderOptions_ClientSideProvider      = 0x1,
    ProviderOptions_ServerSideProvider      = 0x2,
    ProviderOptions_NonClientAreaProvider   = 0x4,
    ProviderOptions_OverrideProvider        = 0x8,
    ProviderOptions_ProviderOwnsSetFocus    = 0x10,
    ProviderOptions_UseComThreading         = 0x20
} ProviderOptions;

typedef enum
{
    SupportedTextSelection_None     = 0,
    SupportedTextSelection_Single   = 1,
    SupportedTextSelection_Multiple = 2
} SupportedTextSelection;

typedef enum
{
    TextUnit_Character  = 0,
    TextUnit_Format     = 1,
    TextUnit_Word       = 2,
    TextUnit_Line       = 3,
    TextUnit_Paragraph  = 4,
    TextUnit_Page       = 5,
    TextUnit_Document   = 6
} TextUnit;

typedef enum
{
    TextPatternRangeEndpoint_Start  = 0,
    TextPatternRangeEndpoint_End    = 1
} TextPatternRangeEndpoint;

/* UIA_PatternIds */
extern const long UIA_InvokePatternId;
extern const long UIA_SelectionPatternId;
extern const long UIA_ValuePatternId;
extern const long UIA_RangeValuePatternId;
extern const long UIA_ScrollPatternId;
extern const long UIA_ExpandCollapsePatternId;
extern const long UIA_GridPatternId;
extern const long UIA_GridItemPatternId;
extern const long UIA_MultipleViewPatternId;
extern const long UIA_WindowPatternId;
extern const long UIA_SelectionItemPatternId;
extern const long UIA_DockPatternId;
extern const long UIA_TablePatternId;
extern const long UIA_TableItemPatternId;
extern const long UIA_TextPatternId;
extern const long UIA_TogglePatternId;
extern const long UIA_TransformPatternId;
extern const long UIA_ScrollItemPatternId;
extern const long UIA_LegacyIAccessiblePatternId;
extern const long UIA_ItemContainerPatternId;
extern const long UIA_VirtualizedItemPatternId;
extern const long UIA_SynchronizedInputPatternId;

#define INTERFACE IRawElementProviderSimple
DECLARE_INTERFACE_(IRawElementProviderSimple, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    STDMETHOD(get_ProviderOptions)(THIS_ ProviderOptions *) PURE;
    STDMETHOD(GetPatternProvider)(THIS_ PATTERNID, IUnknown **) PURE;
    STDMETHOD(GetPropertyValue)(THIS_ PROPERTYID, VARIANT *) PURE;
    STDMETHOD(get_HostRawElementProvider)(THIS_ IRawElementProviderSimple **) PURE;
};
#undef INTERFACE

#define INTERFACE ITextRangeProvider
DECLARE_INTERFACE_(ITextRangeProvider, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    STDMETHOD(Clone)(THIS_ ITextRangeProvider **) PURE;
    STDMETHOD(Compare)(THIS_ ITextRangeProvider *, BOOL *) PURE;
    STDMETHOD(CompareEndpoints)(THIS_ TextPatternRangeEndpoint, ITextRangeProvider *, TextPatternRangeEndpoint, int *) PURE;
    STDMETHOD(ExpandToEnclosingUnit)(THIS_ TextUnit) PURE;
    STDMETHOD(FindAttribute)(THIS_ TEXTATTRIBUTEID, VARIANT, BOOL, ITextRangeProvider **) PURE;
    STDMETHOD(FindText)(THIS_ BSTR, BOOL, BOOL, ITextRangeProvider **) PURE;
    STDMETHOD(GetAttributeValue)(THIS_ TEXTATTRIBUTEID, VARIANT *) PURE;
    STDMETHOD(GetBoundingRectangles)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(GetEnclosingElement)(THIS_ IRawElementProviderSimple **) PURE;
    STDMETHOD(GetText)(THIS_ int, BSTR *) PURE;
    STDMETHOD(Move)(THIS_ TextUnit, int, int *) PURE;
    STDMETHOD(MoveEndpointByUnit)(THIS_ TextPatternRangeEndpoint, TextUnit, int *) PURE;
    STDMETHOD(MoveEndpointByRange)(THIS_ TextPatternRangeEndpoint, ITextRangeProvider *, TextPatternRangeEndpoint) PURE;
    STDMETHOD(Select)(THIS) PURE;
    STDMETHOD(AddToSelection)(THIS) PURE;
    STDMETHOD(RemoveFromSelection)(THIS) PURE;
    STDMETHOD(ScrollIntoView)(THIS_ BOOL) PURE;
    STDMETHOD(GetChildren)(THIS_ SAFEARRAY **) PURE;
};
#undef INTERFACE

#define INTERFACE ITextProvider
DECLARE_INTERFACE_(ITextProvider, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    STDMETHOD(GetSelection)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(GetVisibleRanges)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(RangeFromChild)(THIS_ IRawElementProviderSimple *, ITextRangeProvider **) PURE;
    STDMETHOD(RangeFromPoint)(THIS_ UiaPoint, ITextRangeProvider **pRetVal) PURE;
    STDMETHOD(get_DocumentRange)(THIS_ ITextRangeProvider **) PURE;
    STDMETHOD(get_SupportedTextSelection)(THIS_ SupportedTextSelection *) PURE;
};
#undef INTERFACE

#define INTERFACE IAccessibleEx
DECLARE_INTERFACE_(IAccessibleEx, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

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
