#ifndef __MOUSEOVER_HH_INCLUDED__
#define __MOUSEOVER_HH_INCLUDED__

#include <QObject>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

/// This is a mouseover feature interface, where you can point your mouse at
/// any word in any window and wait a little, and it would provide that word
/// for the translation.
/// This interface always exists, even on platforms that don't support that
/// feature -- it just remains dormant on them.
/// 
/// The Windows platform is the only one supported; it works with the help of
/// two external .dll files,
class MouseOver: public QObject
{
  Q_OBJECT

public:

  /// The class is a singleton.
  static MouseOver & instance();

  /// Enables mouseover. The mouseover is initially disabled.
  void enableMouseOver();
  /// Disables mouseover.
  void disableMouseOver();
  
signals:

  /// Emitted when there was some text under cursor which was hovered over.
  void hovered( QString const & );

private:

  MouseOver();
  ~MouseOver();

#ifdef Q_OS_WIN32

  static LRESULT CALLBACK eventHandler( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

  typedef void ( *ActivateSpyFn )( bool );
  ActivateSpyFn activateSpyFn;
  HINSTANCE spyDll;
  bool mouseOverEnabled;

#endif

};

#endif

