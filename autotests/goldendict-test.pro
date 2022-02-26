QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += qtestlib

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS+= \
        ../iconv.hh \
        ../wstring.hh \
        ../wstring_qt.hh
SOURCES += \
        test-qtextcodec-convert.cpp \
        ../iconv.cc \
        ../wstring_qt.cc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
