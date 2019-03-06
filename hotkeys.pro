#-------------------------------------------------
#
# DLL for global hotkeys handling
# Should be build for Windows only
# Place GdHotkey.dll beside GoldenDict.exe
#
#-------------------------------------------------

win32 {
  QT       -= core gui

  TARGET = GdHotkeys
  TEMPLATE = lib

  DEFINES += HOTKEYS_LIBRARY

  SOURCES += \
          hotkeys.c

  HEADERS += \
          hotkeys.h \
}
