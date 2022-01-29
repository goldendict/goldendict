#include "mouseover.hh"
#include "utf8.hh"
#include <QCoreApplication>
#include <QDir>
#include <algorithm>

#ifdef Q_OS_WIN32
#undef WINVER
#define WINVER 0x0500
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>
#include "wordbyauto.hh"
#include "x64.hh"
#endif

MouseOver & MouseOver::instance()
{
  static MouseOver m;

  return m;
}

#ifdef Q_OS_WIN32
const UINT WM_MY_SHOW_TRANSLATION = WM_USER + 301;
static wchar_t className[] = L"GoldenDictMouseover";
typedef BOOL ( WINAPI  *ChangeWindowMessageFilterFunc )( UINT, DWORD );

#ifndef _MSC_VER
typedef struct tagCHANGEFILTERSTRUCT {
  DWORD cbSize;
  DWORD ExtStatus;
} CHANGEFILTERSTRUCT, *PCHANGEFILTERSTRUCT;
#endif

typedef BOOL ( WINAPI  *ChangeWindowMessageFilterExFunc )( HWND, UINT, DWORD, PCHANGEFILTERSTRUCT );

#endif // Q_OS_WIN32

#ifdef Q_OS_WIN32

#ifndef ConvertStringSecurityDescriptorToSecurityDescriptor

extern "C" BOOL WINAPI ConvertStringSecurityDescriptorToSecurityDescriptorW(
                        LPCWSTR StringSecurityDescriptor,
                        DWORD StringSDRevision,
                        PSECURITY_DESCRIPTOR *SecurityDescriptor,
                        PULONG SecurityDescriptorSize );

#endif

static void SetLowLabelToGDSynchroObjects()
{
// The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity
#define LOW_INTEGRITY_SDDL_SACL_W L"S:(ML;;NW;;;LW)"
//    DWORD dwErr = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pSD = NULL;

    PACL pSacl = NULL; // not allocated
    BOOL fSaclPresent = FALSE;
    BOOL fSaclDefaulted = FALSE;
#ifdef Q_OS_WIN64
    LPCWSTR pwszMapFileName64 = L"GoldenDictTextOutHookSharedMem64";
#endif
    LPCWSTR pwszMapFileName = L"GoldenDictTextOutHookSharedMem";
    LPCWSTR pwszSpyMutexName = L"GoldenDictTextOutSpyMutex";

    if( ConvertStringSecurityDescriptorToSecurityDescriptorW( LOW_INTEGRITY_SDDL_SACL_W, 1 /* SDDL_REVISION_1 */, &pSD, NULL ) )
    {
        if( GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted))
        {
// Note that psidOwner, psidGroup, and pDacl are
// all NULL and set the new LABEL_SECURITY_INFORMATION

#ifdef Q_OS_WIN64
           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszMapFileName64,
                     SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);
#endif
           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszMapFileName,
                    SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);

           /* dwErr = */ SetNamedSecurityInfoW( (LPWSTR)pwszSpyMutexName,
                    SE_KERNEL_OBJECT, LABEL_SECURITY_INFORMATION, NULL, NULL, NULL, pSacl);

        }
        LocalFree(pSD);
    }
}

#endif // Q_OS_WIN32

MouseOver::MouseOver() :
  pPref(NULL)
{

}

void MouseOver::enableMouseOver()
{
#ifdef Q_OS_WIN32
  if ( !mouseOverEnabled && activateSpyFn )
  {
    activateSpyFn( true );
    installx64Hooks();
    mouseOverEnabled = true;
  }
#endif
}

void MouseOver::disableMouseOver()
{
#ifdef Q_OS_WIN32
  if ( mouseOverEnabled && activateSpyFn )
  {
    activateSpyFn( false );
    removex64Hooks();
    mouseOverEnabled = false;
  }
#endif
}

#ifdef Q_OS_WIN32

LRESULT MouseOver::makeScanBitMask()
{
LRESULT res = 0;
    if( pPref == NULL )
        return 0;

    return res;
}

LRESULT CALLBACK MouseOver::eventHandler( HWND hwnd_, UINT msg,
                                          WPARAM wparam, LPARAM lparam )
{
  if ( msg == WM_MY_SHOW_TRANSLATION )
  {
    LRESULT res = instance().makeScanBitMask();

    if( res == 0 )
        return 0;  // Don't handle word without necessity

    if( wparam != 0) //Ask for methods of word retrieving
        return res;

    int wordSeqPos = 0;
    QString wordSeq;

    // Now locate the word inside the sequence

    QString word;

    if ( wordSeq[ wordSeqPos ].isSpace() )
    {
      // Currently we ignore such cases
      return 0;
    }
    else
    if ( !wordSeq[ wordSeqPos ].isLetterOrNumber() )
    {
      // Special case: the cursor points to something which doesn't look like a
      // middle of the word -- assume that it's something that joins two words
      // together.

      int begin = wordSeqPos;

      for( ; begin; --begin )
        if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
          break;

      int end = wordSeqPos;

      while( ++end < wordSeq.size() )
        if ( !wordSeq[ end ].isLetterOrNumber() )
          break;

      if ( end - begin == 1 )
      {
        // Well, turns out it was just a single non-letter char, discard it
        return 0;
      }

      word = wordSeq.mid( begin, end - begin );
    }
    else
    {
      // Cursor points to a letter -- cut the word it points to

      int begin = wordSeqPos;

      for( ; begin; --begin )
        if ( !wordSeq[ begin - 1 ].isLetterOrNumber() )
          break;

      int end = wordSeqPos;

      while( ++end < wordSeq.size() )
      {
        if ( !wordSeq[ end ].isLetterOrNumber() )
          break;
      }
      word = wordSeq.mid( begin, end - begin );
    }

    // See if we have an RTL char. Reverse the whole string if we do.

    if( lparam == 0 )
    {
      for( int x = 0; x < word.size(); ++x )
      {
        QChar::Direction d = word[ x ].direction();

        if ( d == QChar::DirR || d == QChar::DirAL ||
             d == QChar::DirRLE || d == QChar::DirRLO )
        {
          std::reverse( word.begin(), word.end() );
          break;
        }
      }
    }

    bool forcePopup = false;
    emit instance().hovered( word, forcePopup );
    return 0;
  }

  return DefWindowProc( hwnd_, msg, wparam, lparam );
}

#endif

MouseOver::~MouseOver()
{
#ifdef Q_OS_WIN32


#endif
}

