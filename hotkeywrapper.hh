#ifndef HOTKEYWRAPPER_H
#define HOTKEYWRAPPER_H

#include <QtGui>

#ifdef Q_WS_X11

#include <set>

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <QX11Info>

#endif

#ifdef Q_OS_MACX
#define __SECURITYHI__
#include <Carbon/Carbon.h>
#endif

#include "ex.hh"
#include "qtsingleapplication.h"

//////////////////////////////////////////////////////////////////////////

struct HotkeyStruct
{
  HotkeyStruct() {};
  HotkeyStruct( quint32 key, quint32 key2, quint32 modifier, int handle, int id );

  quint32 key, key2;
  quint32 modifier;
  int handle;
  int id;
#ifdef Q_OS_MACX
  EventHotKeyRef hkRef, hkRef2;
#endif
};

//////////////////////////////////////////////////////////////////////////

#if !defined(Q_WS_QWS)
class HotkeyWrapper : public QThread // Thread is actually only used on X11
{
  Q_OBJECT

  friend class QHotkeyApplication;

public:

  DEF_EX( exInit, "Hotkey wrapper failed to init", std::exception )

  HotkeyWrapper(QObject *parent);
  virtual ~HotkeyWrapper();

  /// The handle is passed back in hotkeyActivated() to inform which hotkey
  /// was activated.
  bool setGlobalKey( int key, int key2, Qt::KeyboardModifiers modifier,
                     int handle );

  /// Unregisters everything
  void unregister();

signals:

  void hotkeyActivated( int );

protected slots:

  void waitKey2();

#ifndef Q_OS_MACX
private slots:

  bool checkState( quint32 vk, quint32 mod );
#endif

private:

  void init();
  quint32 nativeKey(int key);

  QList<HotkeyStruct> hotkeys;

  bool state2;
  HotkeyStruct state2waiter;

#ifdef Q_OS_WIN32
  virtual bool winEvent ( MSG * message, long * result );
  HWND hwnd;

#elif defined(Q_OS_MACX)

public:
  void activated( int hkId );
private:
  void sendCmdC();

  static EventHandlerUPP hotKeyFunction;
  quint32 keyC;
  EventHandlerRef handlerRef;

#else

  static void recordEventCallback( XPointer, XRecordInterceptData * );

  /// Called by recordEventCallback()
  void handleRecordEvent( XRecordInterceptData * );

  void run(); // QThread

  // We do one-time init of those, translating keysyms to keycodes
  KeyCode lShiftCode, rShiftCode, lCtrlCode, rCtrlCode, lAltCode, rAltCode,
          cCode, insertCode, kpInsertCode;

  quint32 currentModifiers;

  Display * dataDisplay;
  XRecordRange * recordRange;
  XRecordContext recordContext;
  XRecordClientSpec recordClientSpec;

  /// Holds all the keys currently grabbed.
  /// The first value is keycode, the second is modifiers
  typedef std::set< std::pair< quint32, quint32 > > GrabbedKeys;
  GrabbedKeys grabbedKeys;

  GrabbedKeys::iterator keyToUngrab; // Used for second stage grabs

  /// Returns true if the given key is usually used to copy from clipboard,
  /// false otherwise.
  bool isCopyToClipboardKey( quint32 keyCode, quint32 modifiers ) const;
  /// Returns true if the given key is grabbed, false otherwise
  bool isKeyGrabbed( quint32 keyCode, quint32 modifiers ) const;
  /// Grabs the given key, recording the fact in grabbedKeys. If the key's
  /// already grabbed, does nothing.
  /// Returns the key's iterator in grabbedKeys.
  GrabbedKeys::iterator grabKey( quint32 keyCode, quint32 modifiers );
  /// Ungrabs the given key. erasing it from grabbedKeys. The key's provided
  /// as an interator inside the grabbedKeys set.
  void ungrabKey( GrabbedKeys::iterator );

signals:

  /// Emitted from the thread
  void keyRecorded( quint32 vk, quint32 mod );

#endif
};
#else

class HotkeyWrapper : public QObject
{
  Q_OBJECT

  friend class QHotkeyApplication;

public:

  DEF_EX( exInit, "Hotkey wrapper failed to init", std::exception )

  HotkeyWrapper(QObject *parent): QObject( parent )
  {}

  bool setGlobalKey( int key, int key2, Qt::KeyboardModifiers modifier,
                     int handle )
  { return true; }

  void unregister()
  {}

signals:

  void hotkeyActivated( int );
};
#endif

//////////////////////////////////////////////////////////////////////////

class DataCommitter
{
public:

  virtual void commitData( QSessionManager & )=0;
  virtual ~DataCommitter()
  {}
};

class QHotkeyApplication : public QtSingleApplication
{
  friend class HotkeyWrapper;

  QList< DataCommitter * > dataCommitters;

public:
  QHotkeyApplication( int & argc, char ** argv );
  QHotkeyApplication( QString const & id, int & argc, char ** argv );

  void addDataCommiter( DataCommitter & );
  void removeDataCommiter( DataCommitter & );

  /// This calls all data committers.
  virtual void commitData( QSessionManager & );

protected:
  void registerWrapper(HotkeyWrapper *wrapper);
  void unregisterWrapper(HotkeyWrapper *wrapper);

#ifdef Q_OS_WIN32
  virtual bool winEventFilter ( MSG * message, long * result );
#endif

  QList<HotkeyWrapper*> hotkeyWrappers;
};

//////////////////////////////////////////////////////////////////////////

#endif // HOTKEYWRAPPER_H
