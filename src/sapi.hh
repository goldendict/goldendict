#ifndef __SAPI_HH_INCLUDED__
#define __SAPI_HH_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <oleacc.h>

#undef INTERFACE

#define SPCAT_VOICES  L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices"

typedef void __stdcall SPNOTIFYCALLBACK( WPARAM wParam, LPARAM lParam );

typedef struct SPEVENTSOURCEINFO
{
  ULONGLONG ullEventInterest;
  ULONGLONG ullQueuedInterest;
  ULONG ulCount;
} SPEVENTSOURCEINFO;

typedef enum SPEVENTENUM
{
  SPEI_UNDEFINED              = 0,
  SPEI_START_INPUT_STREAM     = 1,
  SPEI_END_INPUT_STREAM       = 2,
  SPEI_VOICE_CHANGE           = 3,
  SPEI_TTS_BOOKMARK           = 4,
  SPEI_WORD_BOUNDARY          = 5,
  SPEI_PHONEME                = 6,
  SPEI_SENTENCE_BOUNDARY      = 7,
  SPEI_VISEME                 = 8,
  SPEI_TTS_AUDIO_LEVEL        = 9,
  SPEI_TTS_PRIVATE            = 15,
  SPEI_MIN_TTS                = 1,
  SPEI_MAX_TTS                = 15,
  SPEI_END_SR_STREAM          = 34,
  SPEI_SOUND_START            = 35,
  SPEI_SOUND_END              = 36,
  SPEI_PHRASE_START           = 37,
  SPEI_RECOGNITION            = 38,
  SPEI_HYPOTHESIS             = 39,
  SPEI_SR_BOOKMARK            = 40,
  SPEI_PROPERTY_NUM_CHANGE    = 41,
  SPEI_PROPERTY_STRING_CHANGE = 42,
  SPEI_FALSE_RECOGNITION      = 43,
  SPEI_INTERFERENCE           = 44,
  SPEI_REQUEST_UI             = 45,
  SPEI_RECO_STATE_CHANGE      = 46,
  SPEI_ADAPTATION             = 47,
  SPEI_START_SR_STREAM        = 48,
  SPEI_RECO_OTHER_CONTEXT     = 49,
  SPEI_SR_AUDIO_LEVEL         = 50,
  SPEI_SR_RETAINEDAUDIO       = 51,
  SPEI_SR_PRIVATE             = 52,
  SPEI_RESERVED4              = 53,
  SPEI_RESERVED5              = 54,
  SPEI_RESERVED6              = 55,
  SPEI_MIN_SR                 = 34,
  SPEI_MAX_SR                 = 55,
  SPEI_RESERVED1              = 30,
  SPEI_RESERVED2              = 33,
  SPEI_RESERVED3              = 63
} SPEVENTENUM;

typedef enum SPEVENTLPARAMTYPE
{
  SPET_LPARAM_IS_UNDEFINED  = 0,
  SPET_LPARAM_IS_TOKEN      = ( SPET_LPARAM_IS_UNDEFINED + 1 ) ,
  SPET_LPARAM_IS_OBJECT     = ( SPET_LPARAM_IS_TOKEN + 1 ) ,
  SPET_LPARAM_IS_POINTER    = ( SPET_LPARAM_IS_OBJECT + 1 ) ,
  SPET_LPARAM_IS_STRING     = ( SPET_LPARAM_IS_POINTER + 1 )
} SPEVENTLPARAMTYPE;

typedef struct SPEVENT
{
  SPEVENTENUM        eEventId : 16;
  SPEVENTLPARAMTYPE  elParamType : 16;
  ULONG              ulStreamNum;
  ULONGLONG          ullAudioStreamOffset;
  WPARAM             wParam;
  LPARAM             lParam;
} SPEVENT;

typedef WCHAR SPPHONEID;

typedef enum SPVISEMES
{
  SP_VISEME_0   = 0,
  SP_VISEME_1   = ( SP_VISEME_0 + 1 ) ,
  SP_VISEME_2   = ( SP_VISEME_1 + 1 ) ,
  SP_VISEME_3   = ( SP_VISEME_2 + 1 ) ,
  SP_VISEME_4   = ( SP_VISEME_3 + 1 ) ,
  SP_VISEME_5   = ( SP_VISEME_4 + 1 ) ,
  SP_VISEME_6   = ( SP_VISEME_5 + 1 ) ,
  SP_VISEME_7   = ( SP_VISEME_6 + 1 ) ,
  SP_VISEME_8   = ( SP_VISEME_7 + 1 ) ,
  SP_VISEME_9   = ( SP_VISEME_8 + 1 ) ,
  SP_VISEME_10  = ( SP_VISEME_9 + 1 ) ,
  SP_VISEME_11  = ( SP_VISEME_10 + 1 ) ,
  SP_VISEME_12  = ( SP_VISEME_11 + 1 ) ,
  SP_VISEME_13  = ( SP_VISEME_12 + 1 ) ,
  SP_VISEME_14  = ( SP_VISEME_13 + 1 ) ,
  SP_VISEME_15  = ( SP_VISEME_14 + 1 ) ,
  SP_VISEME_16  = ( SP_VISEME_15 + 1 ) ,
  SP_VISEME_17  = ( SP_VISEME_16 + 1 ) ,
  SP_VISEME_18  = ( SP_VISEME_17 + 1 ) ,
  SP_VISEME_19  = ( SP_VISEME_18 + 1 ) ,
  SP_VISEME_20  = ( SP_VISEME_19 + 1 ) ,
  SP_VISEME_21  = ( SP_VISEME_20 + 1 )
} SPVISEMES;

typedef enum SPDATAKEYLOCATION
{
  SPDKL_DefaultLocation = 0,
  SPDKL_CurrentUser     = 1,
  SPDKL_LocalMachine    = 2,
  SPDKL_CurrentConfig   = 5
} SPDATAKEYLOCATION;

typedef struct SPVOICESTATUS
{
  ULONG ulCurrentStream;
  ULONG ulLastStreamQueued;
  HRESULT hrLastResult;
  DWORD dwRunningState;
  ULONG ulInputWordPos;
  ULONG ulInputWordLen;
  ULONG ulInputSentPos;
  ULONG ulInputSentLen;
  LONG lBookmarkId;
  SPPHONEID PhonemeId;
  SPVISEMES VisemeId;
  DWORD dwReserved1;
  DWORD dwReserved2;
} SPVOICESTATUS;

typedef enum SPVPRIORITY
{
  SPVPRI_NORMAL = 0,
  SPVPRI_ALERT  = ( 1L << 0 ) ,
  SPVPRI_OVER   = ( 1L << 1 )
} SPVPRIORITY;

typedef enum SPEAKFLAGS
{
  SPF_DEFAULT           = 0,
  SPF_ASYNC             = ( 1L << 0 ) ,
  SPF_PURGEBEFORESPEAK  = ( 1L << 1 ) ,
  SPF_IS_FILENAME       = ( 1L << 2 ) ,
  SPF_IS_XML            = ( 1L << 3 ) ,
  SPF_IS_NOT_XML        = ( 1L << 4 ) ,
  SPF_PERSIST_XML       = ( 1L << 5 ) ,
  SPF_NLP_SPEAK_PUNC    = ( 1L << 6 ) ,
  SPF_PARSE_SAPI        = ( 1L << 7 ) ,
  SPF_PARSE_SSML        = ( 1L << 8 ) ,
  SPF_PARSE_AUTODETECT  = 0,
  SPF_NLP_MASK          = SPF_NLP_SPEAK_PUNC,
  SPF_PARSE_MASK        = ( SPF_PARSE_SAPI | SPF_PARSE_SSML ) ,
  SPF_VOICE_MASK        = ( ( ( ( ( ( ( SPF_ASYNC | SPF_PURGEBEFORESPEAK )  | SPF_IS_FILENAME )  | SPF_IS_XML )  | SPF_IS_NOT_XML )  | SPF_NLP_MASK )  | SPF_PERSIST_XML )  | SPF_PARSE_MASK ) ,
  SPF_UNUSED_FLAGS      = ~SPF_VOICE_MASK
} SPEAKFLAGS;

typedef enum SPRUNSTATE
{
  SPRS_DONE         = ( 1L << 0 ) ,
  SPRS_IS_SPEAKING  = ( 1L << 1 )
} SPRUNSTATE;

EXTERN_C const IID CLSID_SpVoice;
EXTERN_C const IID IID_ISpVoice;
EXTERN_C const IID IID_ISpObjectToken;
EXTERN_C const IID CLSID_SpObjectToken;
EXTERN_C const IID IID_IEnumSpObjectTokens;
EXTERN_C const IID IID_ISpEventSource;
EXTERN_C const IID IID_ISpNotifySource;
EXTERN_C const IID IID_ISpNotifySink;
EXTERN_C const IID IID_ISpObjectTokenCategory;
EXTERN_C const IID CLSID_SpObjectTokenCategory;

typedef interface ISpVoice ISpVoice;
typedef interface ISpObjectToken ISpObjectToken;
typedef interface ISpObjectTokenCategory ISpObjectTokenCategory;
typedef interface IEnumSpObjectTokens IEnumSpObjectTokens;
typedef interface ISpEventSource ISpEventSource;
typedef interface ISpNotifySource ISpNotifySource;
typedef interface ISpNotifySink ISpNotifySink;
typedef interface ISpNotifyCallback ISpNotifyCallback;
typedef interface ISpDataKey ISpDataKey;
typedef interface ISpStreamFormat ISpStreamFormat;

#define INTERFACE ISpNotifyCallback
DECLARE_INTERFACE( ISpNotifyCallback )
{
  STDMETHOD( NotifyCallback )( THIS_ WPARAM, LPARAM ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpNotifySink
DECLARE_INTERFACE_(ISpNotifySink, IUnknown )
{
  STDMETHOD(Notify)( THIS) PURE;
};
#undef INTERFACE

#define INTERFACE ISpNotifySource
DECLARE_INTERFACE_(ISpNotifySource, IUnknown )
{
  STDMETHOD( SetNotifySink )( THIS_ ISpNotifySink * ) PURE;
  STDMETHOD( SetNotifyWindowMessage )( THIS_ HWND, UINT, WPARAM, LPARAM ) PURE;
  STDMETHOD( SetNotifyCallbackFunction )( THIS_ SPNOTIFYCALLBACK ) PURE;
  STDMETHOD( SetNotifyCallbackInterface )( THIS_ ISpNotifyCallback *, WPARAM, LPARAM ) PURE;
  STDMETHOD( SetNotifyWin32Event )( THIS) PURE;
  STDMETHOD( WaitForNotifyEvent )( THIS_ DWORD ) PURE;
  STDMETHOD( GetNotifyEventHandle )( THIS_ DWORD ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpDataKey
DECLARE_INTERFACE_( ISpDataKey, IUnknown )
{
  STDMETHOD( SetData )( THIS_ LPCWSTR, ULONG, const BYTE * ) PURE;
  STDMETHOD( GetData )( THIS_ LPCWSTR, ULONG *, const BYTE * ) PURE;
  STDMETHOD( SetStringValue )( THIS_ LPCWSTR, LPCWSTR ) PURE;
  STDMETHOD( GetStringValue )( THIS_ LPCWSTR, LPWSTR * ) PURE;
  STDMETHOD( SetDWORD )( THIS_ LPCWSTR, DWORD ) PURE;
  STDMETHOD( GetDWORD )( THIS_ LPCWSTR, DWORD * ) PURE;
  STDMETHOD( OpenKey )( THIS_ LPCWSTR, ISpDataKey * * ) PURE;
  STDMETHOD( CreateKey )( THIS_ LPCWSTR, ISpDataKey * * ) PURE;
  STDMETHOD( DeleteKey )( THIS_ LPCWSTR ) PURE;
  STDMETHOD( DeleteValue )( THIS_ LPCWSTR ) PURE;
  STDMETHOD( EnumKeys )( THIS_ ULONG, LPWSTR * ) PURE;
  STDMETHOD( EnumValues )( THIS_ ULONG, LPWSTR * ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpEventSource
DECLARE_INTERFACE_( ISpEventSource, ISpNotifySource )
{
  STDMETHOD( SetInterest )( THIS_ ULONGLONG, ULONGLONG ) PURE;
  STDMETHOD( GetEvents )( THIS_ ULONG, SPEVENT *, ULONG ) PURE;
  STDMETHOD( GetInfo )( THIS_ SPEVENTSOURCEINFO * ) PURE;
};
#undef INTERFACE

#define INTERFACE IEnumSpObjectTokens
DECLARE_INTERFACE_( IEnumSpObjectTokens, IUnknown )
{
  STDMETHOD( Next )( THIS_ ULONG, ISpObjectToken * *, ULONG * ) PURE;
  STDMETHOD( Skip )( THIS_ ULONG ) PURE;
  STDMETHOD( Reset )( THIS ) PURE;
  STDMETHOD( Clone )( THIS_ IEnumSpObjectTokens * * ) PURE;
  STDMETHOD( Item )( THIS_ ULONG, ISpObjectToken * * ) PURE;
  STDMETHOD( GetCount )( THIS_ ULONG * ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpObjectToken
DECLARE_INTERFACE_( ISpObjectToken, ISpDataKey )
{
  STDMETHOD( SetId )( THIS_ LPCWSTR, LPCWSTR, BOOL ) PURE;
  STDMETHOD( GetId )( THIS_ LPWSTR * ) PURE;
  STDMETHOD( GetCategory )( THIS_ ISpObjectTokenCategory * * ) PURE;
  STDMETHOD( CreateInstance )( THIS_ IUnknown *, DWORD, REFIID, void * * ) PURE;
  STDMETHOD( GetStorageFileName )( THIS_ REFCLSID, LPCWSTR, LPCWSTR, ULONG, LPWSTR * ) PURE;
  STDMETHOD( RemoveStorageFileName )( THIS_ REFCLSID, LPCWSTR, BOOL ) PURE;
  STDMETHOD( Remove )( THIS_ const CLSID * ) PURE;
  STDMETHOD( IsUISupported )( THIS_ LPCWSTR, void *, ULONG, IUnknown *, BOOL * ) PURE;
  STDMETHOD( DisplayUI )( THIS_ HWND, LPCWSTR, LPCWSTR, void *, ULONG, IUnknown * ) PURE;
  STDMETHOD( MatchesAttributes )( THIS_ LPCWSTR, BOOL * ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpObjectTokenCategory
DECLARE_INTERFACE_(ISpObjectTokenCategory, ISpDataKey)
{
  STDMETHOD(SetId)( THIS_ LPCWSTR, BOOL) PURE;
  STDMETHOD(GetId)( THIS_ LPWSTR *) PURE;
  STDMETHOD(GetDataKey)( THIS_ SPDATAKEYLOCATION, ISpDataKey * *) PURE;
  STDMETHOD(EnumTokens)( THIS_ LPCWSTR, LPCWSTR, IEnumSpObjectTokens * *) PURE;
  STDMETHOD(SetDefaultTokenId)( THIS_ LPCWSTR) PURE;
  STDMETHOD(GetDefaultTokenId)( THIS_ LPWSTR *) PURE;
};
#undef INTERFACE

#define INTERFACE ISpStreamFormat
DECLARE_INTERFACE_( ISpStreamFormat, IStream )
{
  STDMETHOD( GetFormat )( THIS_ GUID *, WAVEFORMATEX * * ) PURE;
};
#undef INTERFACE

#define INTERFACE ISpVoice
DECLARE_INTERFACE_( ISpVoice, ISpEventSource )
{
  STDMETHOD( SetOutput )( THIS_ IUnknown *, BOOL ) PURE;
  STDMETHOD( GetOutputObjectToken )( THIS_ ISpObjectToken * * ) PURE;
  STDMETHOD( GetOutputStream )( THIS_ ISpStreamFormat * * ) PURE;
  STDMETHOD( Pause )( THIS ) PURE;
  STDMETHOD( Resume )( THIS ) PURE;
  STDMETHOD( SetVoice )( THIS_ ISpObjectToken * ) PURE;
  STDMETHOD( GetVoice )( THIS_ ISpObjectToken * * ) PURE;
  STDMETHOD( Speak )( THIS_ LPCWSTR, DWORD, ULONG * ) PURE;
  STDMETHOD( SpeakStream )( THIS_ IStream *, DWORD, ULONG * ) PURE;
  STDMETHOD( GetStatus )( THIS_ SPVOICESTATUS *, LPWSTR * ) PURE;
  STDMETHOD( Skip )( THIS_ LPCWSTR, long, ULONG * ) PURE;
  STDMETHOD( SetPriority )( THIS_ SPVPRIORITY ) PURE;
  STDMETHOD( GetPriority )( THIS_ SPVPRIORITY * ) PURE;
  STDMETHOD( SetAlertBoundary )( THIS_ SPEVENTENUM ) PURE;
  STDMETHOD( GetAlertBoundary )( THIS_ SPEVENTENUM * ) PURE;
  STDMETHOD( SetRate )( THIS_ long ) PURE;
  STDMETHOD( GetRate )( THIS_ long * ) PURE;
  STDMETHOD( SetVolume )( THIS_ USHORT ) PURE;
  STDMETHOD( GetVolume )( THIS_ USHORT * ) PURE;
  STDMETHOD( WaitUntilDone )( THIS_ ULONG ) PURE;
  STDMETHOD( SetSyncSpeakTimeout )( THIS_ ULONG ) PURE;
  STDMETHOD( GetSyncSpeakTimeout )( THIS_ ULONG * ) PURE;
  STDMETHOD( SpeakCompleteEvent )( THIS ) PURE;
  STDMETHOD( IsUISupported )( THIS_ LPCWSTR, void *, ULONG, BOOL * ) PURE;
  STDMETHOD( DisplayUI )( THIS_ HWND, LPCWSTR, LPCWSTR, LPCWSTR, void *, ULONG ) PURE;
};
#undef INTERFACE

#ifdef __cplusplus
}
#endif

#endif
