#ifndef HOTKEYWRAPPER_H
#define HOTKEYWRAPPER_H

#include <QtGui>

#ifdef Q_WS_X11

#include <X11/Xlib.h>
#include <QX11Info>

#endif


//////////////////////////////////////////////////////////////////////////

struct HotkeyStruct
{
  HotkeyStruct() {};
  HotkeyStruct(quint32 key, quint32 key2, quint32 modifier, const QObject *member, const char *receiver);

  quint32 key, key2;
  quint32 modifier;
  const QObject *member;
  const char *receiver;
};

//////////////////////////////////////////////////////////////////////////

class HotkeyWrapper : public QObject
{
  Q_OBJECT

  friend class QHotkeyApplication;

public:
  HotkeyWrapper(QObject *parent);
  virtual ~HotkeyWrapper();

  bool setGlobalKey(int key, int key2, Qt::KeyboardModifiers modifier, const QObject *member, const char *receiver);

signals:
  void hotkeyActivated(void);

protected slots:
  void waitKey2();

protected:
  void init();
  bool checkState(quint32 vk, quint32 mod);
  quint32 nativeKey(int key);

  QList<HotkeyStruct> hotkeys;

  bool state2;
  HotkeyStruct state2waiter;

#ifdef Q_OS_WIN32
  virtual bool winEvent ( MSG * message, long * result );
  HWND hwnd;
#else
  virtual bool x11Event ( XEvent * event );
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
#else
  virtual bool x11EventFilter ( XEvent * event );
#endif

  QList<HotkeyWrapper*> hotkeyWrappers;
};

//////////////////////////////////////////////////////////////////////////

#endif // HOTKEYWRAPPER_H
