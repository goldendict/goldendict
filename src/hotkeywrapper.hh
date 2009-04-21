#ifndef HOTKEYWRAPPER_H
#define HOTKEYWRAPPER_H

#include <QtGui>

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <X11/extensions/record.h>
#include <QX11Info>

#endif

#include "ex.hh"

//////////////////////////////////////////////////////////////////////////

struct HotkeyStruct
{
  HotkeyStruct() {};
  HotkeyStruct( quint32 key, quint32 key2, quint32 modifier, int handle, int id );

  quint32 key, key2;
  quint32 modifier;
  int handle;
  int id;
};

//////////////////////////////////////////////////////////////////////////

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

private slots:

  bool checkState( quint32 vk, quint32 mod );

private:

  void init();
  quint32 nativeKey(int key);

  QList<HotkeyStruct> hotkeys;

  bool state2;
  HotkeyStruct state2waiter;

#ifdef Q_OS_WIN32
  virtual bool winEvent ( MSG * message, long * result );
  HWND hwnd;
#else

  static void recordEventCallback( XPointer, XRecordInterceptData * );

  /// Called by recordEventCallback()
  void handleRecordEvent( XRecordInterceptData * );

  void run(); // QThread

  // We do one-time init of those, translating keysyms to keycodes
  KeyCode lShiftCode, rShiftCode, lCtrlCode, rCtrlCode, lAltCode, rAltCode;

  quint32 currentModifiers;

  Display * dataDisplay;
  XRecordRange * recordRange;
  XRecordContext recordContext;
  XRecordClientSpec recordClientSpec;

signals:

  /// Emitted from the thread
  void keyRecorded( quint32 vk, quint32 mod );

#endif
};

//////////////////////////////////////////////////////////////////////////

class QHotkeyApplication : public QApplication
{
  friend class HotkeyWrapper;

public:
  QHotkeyApplication(int & argc, char ** argv);

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
