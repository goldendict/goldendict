#ifndef __UIAUTO_HH_INCLUDED__
#define __UIAUTO_HH_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include <oleacc.h>

EXTERN_C const IID IID_IUIAutomation;
EXTERN_C const IID CLSID_CUIAutomation;
EXTERN_C const IID IID_IUIAutomationElement;
EXTERN_C const IID IID_IUIAutomationTextPattern;
EXTERN_C const IID IID_IUIAutomationTextRange;
EXTERN_C const IID IID_IUIAutomationTreeWalker;

typedef interface IUIAutomationElement IUIAutomationElement;
typedef interface IUIAutomationElementArray IUIAutomationElementArray;
typedef interface IUIAutomationTextPattern IUIAutomationTextPattern;
typedef interface IUIAutomationTextRange IUIAutomationTextRange;
typedef interface IUIAutomationTextRangeArray IUIAutomationTextRangeArray;
typedef interface IUIAutomationCacheRequest IUIAutomationCacheRequest;
typedef interface IUIAutomationTreeWalker IUIAutomationTreeWalker;
typedef interface IUIAutomationCondition IUIAutomationCondition;
typedef interface IUIAutomationEventHandler IUIAutomationEventHandler;
typedef interface IUIAutomationPropertyChangedEventHandler IUIAutomationPropertyChangedEventHandler;
typedef interface IUIAutomationStructureChangedEventHandler IUIAutomationStructureChangedEventHandler;
typedef interface IUIAutomationFocusChangedEventHandler IUIAutomationFocusChangedEventHandler;
typedef interface IUIAutomationProxyFactory IUIAutomationProxyFactory;
typedef interface IUIAutomationProxyFactoryEntry IUIAutomationProxyFactoryEntry;
typedef interface IUIAutomationProxyFactoryMapping IUIAutomationProxyFactoryMapping;

typedef void *UIA_HWND;
typedef int PROPERTYID;
typedef int EVENTID;
typedef int PATTERNID;
typedef int CONTROLTYPEID;
typedef int TEXTATTRIBUTEID;

enum TreeScope
{
    TreeScope_Element       = 0x1,
    TreeScope_Children      = 0x2,
    TreeScope_Descendants   = 0x4,
    TreeScope_Parent        = 0x8,
    TreeScope_Ancestors     = 0x10,
    TreeScope_Subtree       = ( ( TreeScope_Element | TreeScope_Children )  | TreeScope_Descendants )
};

enum PropertyConditionFlags
{
    PropertyConditionFlags_None = 0,
    PropertyConditionFlags_IgnoreCase = 0x1
};

enum OrientationType
{
    OrientationType_None        = 0,
    OrientationType_Horizontal  = 1,
    OrientationType_Vertical    = 2
};

enum SupportedTextSelection
{
    SupportedTextSelection_None     = 0,
    SupportedTextSelection_Single   = 1,
    SupportedTextSelection_Multiple = 2
};

enum TextPatternRangeEndpoint
{
    TextPatternRangeEndpoint_Start  = 0,
    TextPatternRangeEndpoint_End    = 1
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

enum ProviderOptions
{
    ProviderOptions_ClientSideProvider      = 0x1,
    ProviderOptions_ServerSideProvider      = 0x2,
    ProviderOptions_NonClientAreaProvider   = 0x4,
    ProviderOptions_OverrideProvider        = 0x8,
    ProviderOptions_ProviderOwnsSetFocus    = 0x10,
    ProviderOptions_UseComThreading         = 0x20
} ;

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

#ifdef INTERFACE
#undef INTERFACE
#endif

#define INTERFACE IUIAutomation
DECLARE_INTERFACE_(IUIAutomation, IUnknown)
{
    STDMETHOD(CompareElements)(THIS_ IUIAutomationElement *, IUIAutomationElement *, BOOL *) PURE;
    STDMETHOD(CompareRuntimeIds)(THIS_ SAFEARRAY *, SAFEARRAY *, BOOL *) PURE;
    STDMETHOD(GetRootElement)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(ElementFromHandle)(THIS_ UIA_HWND, IUIAutomationElement **) PURE;
    STDMETHOD(ElementFromPoint)(THIS_ POINT, IUIAutomationElement **) PURE;
    STDMETHOD(GetFocusedElement)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(GetRootElementBuildCache)(THIS_ IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(ElementFromHandleBuildCache)(THIS_ UIA_HWND, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(ElementFromPointBuildCache)(THIS_ POINT, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetFocusedElementBuildCache)(THIS_ IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(CreateTreeWalker)(THIS_ IUIAutomationCondition *, IUIAutomationTreeWalker **) PURE;
    STDMETHOD(get_ControlViewWalker)(THIS_ IUIAutomationTreeWalker **) PURE;
    STDMETHOD(get_ContentViewWalker)(THIS_ IUIAutomationTreeWalker **) PURE;
    STDMETHOD(get_RawViewWalker)(THIS_ IUIAutomationTreeWalker **) PURE;
    STDMETHOD(get_RawViewCondition)(THIS_ IUIAutomationCondition **) PURE;
    STDMETHOD(get_ControlViewCondition)(THIS_ IUIAutomationCondition **) PURE;
    STDMETHOD(get_ContentViewCondition)(THIS_ IUIAutomationCondition **) PURE;
    STDMETHOD(CreateCacheRequest)(THIS_ IUIAutomationCacheRequest **) PURE;
    STDMETHOD(CreateTrueCondition)(THIS_ IUIAutomationCondition **) PURE;
    STDMETHOD(CreateFalseCondition)(THIS_ IUIAutomationCondition **) PURE;
    STDMETHOD(CreatePropertyCondition)(THIS_ PROPERTYID, VARIANT, IUIAutomationCondition **) PURE;
    STDMETHOD(CreatePropertyConditionEx)(THIS_ PROPERTYID, VARIANT, enum PropertyConditionFlags, IUIAutomationCondition **) PURE;
    STDMETHOD(CreateAndCondition)(THIS_ IUIAutomationCondition *, IUIAutomationCondition *, IUIAutomationCondition **) PURE;
    STDMETHOD(CreateAndConditionFromArray)(THIS_ SAFEARRAY *, IUIAutomationCondition **) PURE;
    STDMETHOD(CreateAndConditionFromNativeArray)(THIS_ IUIAutomationCondition **, int , IUIAutomationCondition **) PURE;
    STDMETHOD(CreateOrCondition)(THIS_ IUIAutomationCondition *, IUIAutomationCondition *, IUIAutomationCondition **) PURE;
    STDMETHOD(CreateOrConditionFromArray)(THIS_ SAFEARRAY *, IUIAutomationCondition **) PURE;
    STDMETHOD(CreateOrConditionFromNativeArray)(THIS_ IUIAutomationCondition **, int , IUIAutomationCondition **) PURE;
    STDMETHOD(CreateNotCondition)(THIS_ IUIAutomationCondition *, IUIAutomationCondition **) PURE;
    STDMETHOD(AddAutomationEventHandler)(THIS_ EVENTID, IUIAutomationElement *, enum TreeScope, IUIAutomationCacheRequest *, IUIAutomationEventHandler *) PURE;
    STDMETHOD(RemoveAutomationEventHandler)(THIS_ EVENTID, IUIAutomationElement *, IUIAutomationEventHandler *) PURE;
    STDMETHOD(AddPropertyChangedEventHandlerNativeArray)(THIS_ IUIAutomationElement *, enum TreeScope, IUIAutomationCacheRequest *,
                                                         IUIAutomationPropertyChangedEventHandler *, PROPERTYID *, int) PURE;
    STDMETHOD(AddPropertyChangedEventHandler)(THIS_ IUIAutomationElement *, enum TreeScope, EVENTID, IUIAutomationCacheRequest *,
                                              IUIAutomationPropertyChangedEventHandler *, SAFEARRAY *) PURE;
    STDMETHOD(RemovePropertyChangedEventHandler)(THIS_ IUIAutomationElement *, IUIAutomationPropertyChangedEventHandler *) PURE;
    STDMETHOD(AddStructureChangedEventHandler)(THIS_ IUIAutomationElement *, enum TreeScope, IUIAutomationCacheRequest *, IUIAutomationStructureChangedEventHandler *) PURE;
    STDMETHOD(RemoveStructureChangedEventHandler)(THIS_ IUIAutomationElement *, IUIAutomationStructureChangedEventHandler *) PURE;
    STDMETHOD(AddFocusChangedEventHandler)(THIS_ IUIAutomationCacheRequest *, IUIAutomationFocusChangedEventHandler *) PURE;
    STDMETHOD(RemoveFocusChangedEventHandler)(THIS_ IUIAutomationFocusChangedEventHandler *) PURE;
    STDMETHOD(RemoveAllEventHandlers)(THIS) PURE;
    STDMETHOD(IntNativeArrayToSafeArray)(THIS_ int *, int, SAFEARRAY **) PURE;
    STDMETHOD(IntSafeArrayToNativeArray)(THIS_ SAFEARRAY *, int **, int *) PURE;
    STDMETHOD(RectToVariant)(THIS_ RECT, VARIANT *) PURE;
    STDMETHOD(VariantToRect)(THIS_ VARIANT, RECT *) PURE;
    STDMETHOD(SafeArrayToRectNativeArray)(THIS_ SAFEARRAY *, RECT **, int *) PURE;
    STDMETHOD(CreateProxyFactoryEntry)(THIS_ IUIAutomationProxyFactory *, IUIAutomationProxyFactoryEntry **) PURE;
    STDMETHOD(get_ProxyFactoryMapping)(THIS_ IUIAutomationProxyFactoryMapping **) PURE;
    STDMETHOD(GetPropertyProgrammaticName)(THIS_ PROPERTYID, BSTR *) PURE;
    STDMETHOD(GetPatternProgrammaticName)(THIS_ PATTERNID, BSTR *) PURE;
    STDMETHOD(PollForPotentialSupportedPatterns)(THIS_ IUIAutomationElement *, SAFEARRAY **, SAFEARRAY **) PURE;
    STDMETHOD(PollForPotentialSupportedProperties)(THIS_ IUIAutomationElement *, SAFEARRAY **, SAFEARRAY **) PURE;
    STDMETHOD(CheckNotSupported)(THIS_ VARIANT, BOOL *) PURE;
    STDMETHOD(get_ReservedNotSupportedValue)(THIS_ IUnknown **) PURE;
    STDMETHOD(get_ReservedMixedAttributeValue)(THIS_ IUnknown **) PURE;
    STDMETHOD(ElementFromIAccessible)(THIS_ IAccessible *, int, IUIAutomationElement **) PURE;
    STDMETHOD(ElementFromIAccessibleBuildCache)(THIS_ IAccessible *, int, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
};
#undef INTERFACE

#define INTERFACE IUIAutomationElement
DECLARE_INTERFACE_(IUIAutomationElement, IUnknown)
{
    STDMETHOD(SetFocus)(THIS) PURE;
    STDMETHOD(GetRuntimeId)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(FindFirst)(THIS_ enum TreeScope, IUIAutomationCondition *, IUIAutomationElement **) PURE;
    STDMETHOD(FindAll)(THIS_ enum TreeScope, IUIAutomationCondition *, IUIAutomationElementArray **) PURE;
    STDMETHOD(FindFirstBuildCache)(THIS_ enum TreeScope, IUIAutomationCondition *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(FindAllBuildCache)(THIS_ enum TreeScope, IUIAutomationCondition *, IUIAutomationCacheRequest *, IUIAutomationElementArray **) PURE;
    STDMETHOD(BuildUpdatedCache)(THIS_ IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetCurrentPropertyValue)(THIS_ PROPERTYID, VARIANT *) PURE;
    STDMETHOD(GetCurrentPropertyValueEx)(THIS_ PROPERTYID, BOOL, VARIANT *) PURE;
    STDMETHOD(GetCachedPropertyValue)(THIS_ PROPERTYID, VARIANT *) PURE;
    STDMETHOD(GetCachedPropertyValueEx)(THIS_ PROPERTYID, BOOL, VARIANT *) PURE;
    STDMETHOD(GetCurrentPatternAs)(THIS_ PATTERNID, REFIID, void **) PURE;
    STDMETHOD(GetCachedPatternAs)(THIS_ PATTERNID, REFIID, void **) PURE;
    STDMETHOD(GetCurrentPattern)(THIS_ PATTERNID, IUnknown **) PURE;
    STDMETHOD(GetCachedPattern)(THIS_ PATTERNID, IUnknown **) PURE;
    STDMETHOD(GetCachedParent)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(GetCachedChildren)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(get_CurrentProcessId)(THIS_ int *) PURE;
    STDMETHOD(get_CurrentControlType)(THIS_ CONTROLTYPEID *) PURE;
    STDMETHOD(get_CurrentLocalizedControlType)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentName)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentAcceleratorKey)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentAccessKey)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentHasKeyboardFocus)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentIsKeyboardFocusable)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentIsEnabled)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentAutomationId)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentClassName)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentHelpText)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentCulture)(THIS_ int *) PURE;
    STDMETHOD(get_CurrentIsControlElement)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentIsContentElement)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentIsPassword)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentNativeWindowHandle)(THIS_ UIA_HWND *) PURE;
    STDMETHOD(get_CurrentItemType)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentIsOffscreen)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentOrientation)(THIS_ enum OrientationType *) PURE;
    STDMETHOD(get_CurrentFrameworkId)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentIsRequiredForForm)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentItemStatus)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentBoundingRectangle)(THIS_ RECT *) PURE;
    STDMETHOD(get_CurrentLabeledBy)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(get_CurrentAriaRole)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentAriaProperties)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CurrentIsDataValidForForm)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CurrentControllerFor)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CurrentDescribedBy)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CurrentFlowsTo)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CurrentProviderDescription)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedProcessId)(THIS_ int *) PURE;
    STDMETHOD(get_CachedControlType)(THIS_ CONTROLTYPEID *) PURE;
    STDMETHOD(get_CachedLocalizedControlType)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedName)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedAcceleratorKey)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedAccessKey)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedHasKeyboardFocus)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedIsKeyboardFocusable)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedIsEnabled)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedAutomationId)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedClassName)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedHelpText)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedCulture)(THIS_ int *) PURE;
    STDMETHOD(get_CachedIsControlElement)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedIsContentElement)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedIsPassword)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedNativeWindowHandle)(THIS_ UIA_HWND *) PURE;
    STDMETHOD(get_CachedItemType)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedIsOffscreen)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedOrientation)(THIS_ enum OrientationType *) PURE;
    STDMETHOD(get_CachedFrameworkId)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedIsRequiredForForm)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedItemStatus)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedBoundingRectangle)(THIS_ RECT *) PURE;
    STDMETHOD(get_CachedLabeledBy)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(get_CachedAriaRole)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedAriaProperties)(THIS_ BSTR *) PURE;
    STDMETHOD(get_CachedIsDataValidForForm)(THIS_ BOOL *) PURE;
    STDMETHOD(get_CachedControllerFor)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CachedDescribedBy)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CachedFlowsTo)(THIS_ IUIAutomationElementArray **) PURE;
    STDMETHOD(get_CachedProviderDescription)(THIS_ BSTR *) PURE;
};
#undef INTERFACE

#define INTERFACE IUIAutomationTextPattern
DECLARE_INTERFACE_(IUIAutomationTextPattern, IUnknown)
{
    STDMETHOD(RangeFromPoint)(THIS_ POINT, IUIAutomationTextRange **) PURE;
    STDMETHOD(RangeFromChild)(THIS_ IUIAutomationElement *, IUIAutomationTextRange **) PURE;
    STDMETHOD(GetSelection)(THIS_ IUIAutomationTextRangeArray **) PURE;
    STDMETHOD(GetVisibleRanges)(THIS_ IUIAutomationTextRangeArray **) PURE;
    STDMETHOD(get_DocumentRange)(THIS_ IUIAutomationTextRange **) PURE;
    STDMETHOD(get_SupportedTextSelection)(THIS_ enum SupportedTextSelection *) PURE;
};
#undef INTERFACE

#define INTERFACE IUIAutomationTreeWalker
DECLARE_INTERFACE_(IUIAutomationTreeWalker, IUnknown)
{
    STDMETHOD(GetParentElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(GetFirstChildElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(GetLastChildElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(GetNextSiblingElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(GetPreviousSiblingElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(NormalizeElement)(THIS_ IUIAutomationElement *, IUIAutomationElement **) PURE;
    STDMETHOD(GetParentElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetFirstChildElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetLastChildElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetNextSiblingElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(GetPreviousSiblingElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(NormalizeElementBuildCache)(THIS_ IUIAutomationElement *, IUIAutomationCacheRequest *, IUIAutomationElement **) PURE;
    STDMETHOD(get_Condition)(THIS_ IUIAutomationCondition **) PURE;
};
#undef INTERFACE

#define INTERFACE IUIAutomationTextRange
DECLARE_INTERFACE_(IUIAutomationTextRange, IUnknown)
{
    STDMETHOD(Clone)(THIS_ IUIAutomationTextRange **) PURE;
    STDMETHOD(Compare)(THIS_ IUIAutomationTextRange *, BOOL *) PURE;
    STDMETHOD(CompareEndpoints)(THIS_ enum TextPatternRangeEndpoint, IUIAutomationTextRange *, enum TextPatternRangeEndpoint, int *) PURE;
    STDMETHOD(ExpandToEnclosingUnit)(THIS_ enum TextUnit) PURE;
    STDMETHOD(FindAttribute)(THIS_ TEXTATTRIBUTEID, VARIANT, BOOL, IUIAutomationTextRange **) PURE;
    STDMETHOD(FindText)(THIS_ BSTR, BOOL, BOOL, IUIAutomationTextRange **) PURE;
    STDMETHOD(GetAttributeValue)(THIS_ TEXTATTRIBUTEID, VARIANT *) PURE;
    STDMETHOD(GetBoundingRectangles)(THIS_ SAFEARRAY **) PURE;
    STDMETHOD(GetEnclosingElement)(THIS_ IUIAutomationElement **) PURE;
    STDMETHOD(GetText)(THIS_ int, BSTR *) PURE;
    STDMETHOD(Move)(THIS_ enum TextUnit, int, int *) PURE;
    STDMETHOD(MoveEndpointByUnit)(THIS_ enum TextPatternRangeEndpoint, enum TextUnit, int *) PURE;
    STDMETHOD(MoveEndpointByRange)(THIS_ enum TextPatternRangeEndpoint, IUIAutomationTextRange *, enum TextPatternRangeEndpoint) PURE;
    STDMETHOD(Select)(THIS) PURE;
    STDMETHOD(AddToSelection)(THIS) PURE;
    STDMETHOD(RemoveFromSelection)(THIS) PURE;
    STDMETHOD(ScrollIntoView)(THIS_ BOOL) PURE;
    STDMETHOD(GetChildren)(THIS_ IUIAutomationElementArray **) PURE;
};
#undef INTERFACE

#ifdef __cplusplus
}
#endif

#endif // UIAUTO_HH
