TEMPLATE = app
TARGET = goldendict
VERSION = 1.5.0-RC2+git

# Generate version file. We do this here and in a build rule described later.
# The build rule is required since qmake isn't run each time the project is
# rebuilt; and doing it here is required too since any other way the RCC
# compiler would complain if version.txt wouldn't exist (fresh checkouts).

system(git describe --tags --always --dirty > version.txt): hasGit=1

isEmpty( hasGit ) {
  message(Failed to precisely describe the version via Git -- using the default version string)
  system(echo $$VERSION > version.txt)
}

!CONFIG( verbose_build_output ) {
  !win32|*-msvc* {
    # Reduce build log verbosity except for MinGW builds (mingw-make cannot
    # execute "@echo ..." commands inserted by qmake).
    CONFIG += silent
  }
}

CONFIG( release, debug|release ) {
  DEFINES += NDEBUG
}

# DEPENDPATH += . generators
INCLUDEPATH += . src

QT += core \
      gui \
      xml \
      network \
      svg

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets \
          webkitwidgets \
          printsupport \
          help

    # QMediaPlayer is not available in Qt4.
    !CONFIG( no_qtmultimedia_player ) {
      QT += multimedia
      DEFINES += MAKE_QTMULTIMEDIA_PLAYER
    }
} else {
    QT += webkit
    CONFIG += help
}

!CONFIG( no_ffmpeg_player ) {
  DEFINES += MAKE_FFMPEG_PLAYER
}

QT += sql
CONFIG += exceptions \
    rtti \
    stl
OBJECTS_DIR = build
UI_DIR = build
MOC_DIR = build
RCC_DIR = build
LIBS += \
        -lz \
        -lbz2 \
        -llzo2

win32 {
    TARGET = GoldenDict

    win32-msvc* {
        VERSION = 1.5.0 # More complicated things cause errors during compilation under MSVC++
        DEFINES += __WIN32 _CRT_SECURE_NO_WARNINGS
        contains(QMAKE_TARGET.arch, x86_64) {
            DEFINES += NOMINMAX __WIN64
        }
        LIBS += -L$${PWD}/winlibs/lib/msvc
        QMAKE_CXXFLAGS += /wd4290 # silence the warning C4290: C++ exception specification ignored
        QMAKE_LFLAGS_RELEASE += /OPT:REF /OPT:ICF
        DEFINES += GD_NO_MANIFEST
        # QMAKE_CXXFLAGS_RELEASE += /GL # slows down the linking significantly
        LIBS += -lshell32 -luser32 -lsapi -lole32
        Debug: LIBS+= -lhunspelld
        Release: LIBS+= -lhunspell
        HUNSPELL_LIB = hunspell
    } else {
        CONFIG(gcc48) {
            x64 {
                LIBS += -L$${PWD}/winlibs/lib64-48
                QMAKE_CXXFLAGS += -m64
                QMAKE_CFLAGS += -m64
            } else {
                LIBS += -L$${PWD}/winlibs/lib32-48
            }
        } else {
            LIBS += -L$${PWD}/winlibs/lib
        }
        !x64:QMAKE_LFLAGS += -Wl,--large-address-aware

        isEmpty(HUNSPELL_LIB) {
          LIBS += -lhunspell-1.6.1
        } else {
          LIBS += -l$$HUNSPELL_LIB
        }
        QMAKE_CXXFLAGS += -Wextra -Wempty-body
    }

    LIBS += -liconv \
        -lwsock32 \
        -lpsapi \
        -lole32 \
        -loleaut32 \
        -ladvapi32 \
        -lcomdlg32
    LIBS += -lvorbisfile \
        -lvorbis \
        -logg
    !CONFIG( no_ffmpeg_player ) {
        LIBS += -lao \
            -lswresample-gd \
            -lavutil-gd \
            -lavformat-gd \
            -lavcodec-gd
    }


    RC_FILE = goldendict.rc
    INCLUDEPATH += winlibs/include

    # Enable console in Debug mode on Windows, with useful logging messages
    Debug:CONFIG += console

    Release:DEFINES += NO_CONSOLE

    gcc48:QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

    CONFIG += zim_support

    !CONFIG( no_chinese_conversion_support ) {
        CONFIG += chinese_conversion_support
    }

    greaterThan(QT_MAJOR_VERSION, 4) {
      LIBS += -luxtheme
    }
}

unix:!mac {
  DEFINES += HAVE_X11
  # This is to keep symbols for backtraces
  QMAKE_CXXFLAGS += -rdynamic
  QMAKE_LFLAGS += -rdynamic

    greaterThan(QT_MAJOR_VERSION, 4) {
      greaterThan(QT_MINOR_VERSION, 0) {
        QT += x11extras
      }
    }

    CONFIG += link_pkgconfig
    PKGCONFIG += vorbisfile \
        vorbis \
        ogg \
        hunspell
    !CONFIG( no_ffmpeg_player ) {
        PKGCONFIG += ao \
            libavutil \
            libavformat \
            libavcodec \
            libswresample \
    }
    arm {
        LIBS += -liconv
    } else {
        LIBS += -lX11 -lXtst
    }

    # Install prefix: first try to use qmake's PREFIX variable,
    # then $PREFIX from system environment, and if both fails,
    # use the hardcoded /usr/local.
    PREFIX = $${PREFIX}
    isEmpty( PREFIX ):PREFIX = $$(PREFIX)
    isEmpty( PREFIX ):PREFIX = /usr/local
    message(Install Prefix is: $$PREFIX)

    DEFINES += PROGRAM_DATA_DIR=\\\"$$PREFIX/share/goldendict/\\\"
    target.path = $$PREFIX/bin/
    locale.path = $$PREFIX/share/goldendict/locale/
    locale.files = data/locale/*.qm
    INSTALLS += target \
        locale
    icons.path = $$PREFIX/share/pixmaps
    icons.files = data/icons/goldendict.png
    INSTALLS += icons
    desktops.path = $$PREFIX/share/applications
    desktops.files = data/*.desktop
    INSTALLS += desktops
    metainfo.path = $$PREFIX/share/metainfo
    metainfo.files = data/*.metainfo.xml
    INSTALLS += metainfo
    helps.path = $$PREFIX/share/goldendict/help/
    helps.files = data/help/*.qch
    INSTALLS += helps
}
freebsd {
    LIBS += -liconv -lexecinfo
}
mac {
    TARGET = GoldenDict
    # Uncomment this line to make a universal binary.
    # You will need to use Xcode 3 and Qt Carbon SDK
    # if you want the support for PowerPC and/or Mac OS X 10.4
    # CONFIG += x86 x86_64 ppc
    LIBS = -lz \
        -lbz2 \
        -liconv \
        -lvorbisfile \
        -lvorbis \
        -logg \
        -lhunspell-1.6.1 \
        -llzo2
    !CONFIG( no_ffmpeg_player ) {
        LIBS += -lao \
            -lswresample-gd \
            -lavutil-gd \
            -lavformat-gd \
            -lavcodec-gd
    }
    INCLUDEPATH = $${PWD}/maclibs/include
    LIBS += -L$${PWD}/maclibs/lib -framework AppKit -framework Carbon
    OBJECTIVE_SOURCES += src/lionsupport.mm \
                         src/machotkeywrapper.mm \
                         src/macmouseover.mm \
                         src/speechclient_mac.mm
    ICON = icons/macicon.icns
    QMAKE_INFO_PLIST = myInfo.plist
    QMAKE_POST_LINK = mkdir -p GoldenDict.app/Contents/Frameworks & \
                      cp -nR $${PWD}/maclibs/lib/ GoldenDict.app/Contents/Frameworks/ & \
                      mkdir -p GoldenDict.app/Contents/MacOS/locale & \
                      cp -R data/locale/*.qm GoldenDict.app/Contents/MacOS/locale/ & \
                      mkdir -p GoldenDict.app/Contents/MacOS/help & \
                      cp -R $${PWD}/data/help/*.qch GoldenDict.app/Contents/MacOS/help/

    CONFIG += zim_support
    !CONFIG( no_chinese_conversion_support ) {
        CONFIG += chinese_conversion_support
        CONFIG( x86 ) {
            QMAKE_POST_LINK += & mkdir -p GoldenDict.app/Contents/MacOS/opencc & \
                                 cp -R $${PWD}/opencc/*.json GoldenDict.app/Contents/MacOS/opencc/ & \
                                 cp -R $${PWD}/opencc/*.ocd GoldenDict.app/Contents/MacOS/opencc/
        } else {
            QMAKE_POST_LINK += & mkdir -p GoldenDict.app/Contents/MacOS/opencc & \
                                 cp -R $${PWD}/opencc/x64/*.json GoldenDict.app/Contents/MacOS/opencc/ & \
                                 cp -R $${PWD}/opencc/x64/*.ocd GoldenDict.app/Contents/MacOS/opencc/
        }
    }
}
DEFINES += PROGRAM_VERSION=\\\"$$VERSION\\\"

# Input
HEADERS += src/folding.hh \
    src/inc_case_folding.hh \
    src/inc_diacritic_folding.hh \
    src/mainwindow.hh \
    src/sptr.hh \
    src/dictionary.hh \
    src/ex.hh \
    src/config.hh \
    src/sources.hh \
    src/utf8.hh \
    src/file.hh \
    src/bgl_babylon.hh \
    src/bgl.hh \
    src/initializing.hh \
    src/article_netmgr.hh \
    src/dictzip.h \
    src/btreeidx.hh \
    src/stardict.hh \
    src/chunkedstorage.hh \
    src/xdxf2html.hh \
    src/iconv.hh \
    src/lsa.hh \
    src/htmlescape.hh \
    src/dsl.hh \
    src/dsl_details.hh \
    src/filetype.hh \
    src/fsencoding.hh \
    src/groups.hh \
    src/groups_widgets.hh \
    src/instances.hh \
    src/article_maker.hh \
    src/scanpopup.hh \
    src/articleview.hh \
    src/audioplayerinterface.hh \
    src/audioplayerfactory.hh \
    src/ffmpegaudioplayer.hh \
    src/multimediaaudioplayer.hh \
    src/externalaudioplayer.hh \
    src/externalviewer.hh \
    src/wordfinder.hh \
    src/groupcombobox.hh \
    src/keyboardstate.hh \
    src/mouseover.hh \
    src/preferences.hh \
    src/mutex.hh \
    src/mediawiki.hh \
    src/sounddir.hh \
    src/hunspell.hh \
    src/dictdfiles.hh \
    src/audiolink.hh \
    src/wstring.hh \
    src/wstring_qt.hh \
    src/processwrapper.hh \
    src/hotkeywrapper.hh \
    src/searchpanewidget.hh \
    src/hotkeyedit.hh \
    src/langcoder.hh \
    src/editdictionaries.hh \
    src/loaddictionaries.hh \
    src/transliteration.hh \
    src/romaji.hh \
    src/belarusiantranslit.hh \
    src/russiantranslit.hh \
    src/german.hh \
    src/website.hh \
    src/orderandprops.hh \
    src/language.hh \
    src/dictionarybar.hh \
    src/broken_xrecord.hh \
    src/history.hh \
    src/atomic_rename.hh \
    src/articlewebview.hh \
    src/zipfile.hh \
    src/indexedzip.hh \
    src/termination.hh \
    src/greektranslit.hh \
    src/webmultimediadownload.hh \
    src/forvo.hh \
    src/country.hh \
    src/about.hh \
    src/programs.hh \
    src/parsecmdline.hh \
    src/dictspanewidget.hh \
    src/maintabwidget.hh \
    src/mainstatusbar.hh \
    src/gdappstyle.hh \
    src/ufile.hh \
    src/xdxf.hh \
    src/sdict.hh \
    src/decompress.hh \
    src/aard.hh \
    src/mruqmenu.hh \
    src/dictinfo.hh \
    src/zipsounds.hh \
    src/stylescombobox.hh \
    src/extlineedit.hh \
    src/translatebox.hh \
    src/historypanewidget.hh \
    src/wordlist.hh \
    src/mdictparser.hh \
    src/mdx.hh \
    src/voiceengines.hh \
    src/ffmpegaudio.hh \
    src/articleinspector.hh \
    src/delegate.hh \
    src/zim.hh \
    src/gddebug.hh \
    src/qt4x5.hh \
    src/gestures.hh \
    src/tiff.hh \
    src/dictheadwords.hh \
    src/fulltextsearch.hh \
    src/ftshelpers.hh \
    src/dictserver.hh \
    src/helpwindow.hh \
    src/slob.hh \
    src/ripemd.hh \
    src/gls.hh \
    src/splitfile.hh \
    src/favoritespanewidget.hh \
    src/cpp_features.hh \
    src/treeview.hh

FORMS += src/groups.ui \
    src/dictgroupwidget.ui \
    src/mainwindow.ui \
    src/sources.ui \
    src/initializing.ui \
    src/scanpopup.ui \
    src/articleview.ui \
    src/preferences.ui \
    src/about.ui \
    src/editdictionaries.ui \
    src/orderandprops.ui \
    src/dictinfo.ui \
    src/dictheadwords.ui \
    src/authentication.ui \
    src/fulltextsearch.ui

SOURCES += src/folding.cc \
    src/main.cc \
    src/dictionary.cc \
    src/config.cc \
    src/sources.cc \
    src/mainwindow.cc \
    src/utf8.cc \
    src/file.cc \
    src/bgl_babylon.cc \
    src/bgl.cc \
    src/initializing.cc \
    src/article_netmgr.cc \
    src/dictzip.c \
    src/btreeidx.cc \
    src/stardict.cc \
    src/chunkedstorage.cc \
    src/xdxf2html.cc \
    src/iconv.cc \
    src/lsa.cc \
    src/htmlescape.cc \
    src/dsl.cc \
    src/dsl_details.cc \
    src/filetype.cc \
    src/fsencoding.cc \
    src/groups.cc \
    src/groups_widgets.cc \
    src/instances.cc \
    src/article_maker.cc \
    src/scanpopup.cc \
    src/articleview.cc \
    src/audioplayerfactory.cc \
    src/multimediaaudioplayer.cc \
    src/externalaudioplayer.cc \
    src/externalviewer.cc \
    src/wordfinder.cc \
    src/groupcombobox.cc \
    src/keyboardstate.cc \
    src/mouseover.cc \
    src/preferences.cc \
    src/mutex.cc \
    src/mediawiki.cc \
    src/sounddir.cc \
    src/hunspell.cc \
    src/dictdfiles.cc \
    src/audiolink.cc \
    src/wstring.cc \
    src/wstring_qt.cc \
    src/processwrapper.cc \
    src/hotkeywrapper.cc \
    src/hotkeyedit.cc \
    src/langcoder.cc \
    src/editdictionaries.cc \
    src/loaddictionaries.cc \
    src/transliteration.cc \
    src/romaji.cc \
    src/belarusiantranslit.cc \
    src/russiantranslit.cc \
    src/german.cc \
    src/website.cc \
    src/orderandprops.cc \
    src/language.cc \
    src/dictionarybar.cc \
    src/broken_xrecord.cc \
    src/history.cc \
    src/atomic_rename.cc \
    src/articlewebview.cc \
    src/zipfile.cc \
    src/indexedzip.cc \
    src/termination.cc \
    src/greektranslit.cc \
    src/webmultimediadownload.cc \
    src/forvo.cc \
    src/country.cc \
    src/about.cc \
    src/programs.cc \
    src/parsecmdline.cc \
    src/maintabwidget.cc \
    src/mainstatusbar.cc \
    src/gdappstyle.cc \
    src/ufile.cc \
    src/xdxf.cc \
    src/sdict.cc \
    src/decompress.cc \
    src/aard.cc \
    src/mruqmenu.cc \
    src/dictinfo.cc \
    src/zipsounds.cc \
    src/stylescombobox.cc \
    src/extlineedit.cc \
    src/translatebox.cc \
    src/historypanewidget.cc \
    src/wordlist.cc \
    src/mdictparser.cc \
    src/mdx.cc \
    src/voiceengines.cc \
    src/ffmpegaudio.cc \
    src/articleinspector.cc \
    src/delegate.cc \
    src/zim.cc \
    src/gddebug.cc \
    src/gestures.cc \
    src/tiff.cc \
    src/dictheadwords.cc \
    src/fulltextsearch.cc \
    src/ftshelpers.cc \
    src/dictserver.cc \
    src/helpwindow.cc \
    src/slob.cc \
    src/ripemd.cc \
    src/gls.cc \
    src/splitfile.cc \
    src/favoritespanewidget.cc \
    src/treeview.cc

win32 {
    FORMS   += src/texttospeechsource.ui
    SOURCES += src/win32/ThTypes.c \
               src/wordbyauto.cc \
               src/guids.c \
               src/x64.cc \
               src/speechclient_win.cc \
               src/texttospeechsource.cc \
               src/speechhlp.cc
    HEADERS += src/win32/ThTypes.h \
               src/wordbyauto.hh \
               src/uiauto.hh \
               src/x64.hh \
               src/texttospeechsource.hh \
               src/sapi.hh \
               src/sphelper.hh \
               src/speechclient.hh \
               src/speechhlp.hh \
               src/hotkeys.h
}

mac {
    HEADERS += src/macmouseover.hh \
               src/texttospeechsource.hh \
               src/speechclient.hh
    FORMS   += src/texttospeechsource.ui
    SOURCES += src/texttospeechsource.cc
}

unix:!mac {
    HEADERS += src/scanflag.hh
    FORMS   += src/scanflag.ui
    SOURCES += src/scanflag.cc
}

greaterThan(QT_MAJOR_VERSION, 4) {
    HEADERS += src/wildcard.hh
    SOURCES += src/wildcard.cc
}

CONFIG( zim_support ) {
  DEFINES += MAKE_ZIM_SUPPORT
  LIBS += -llzma -lzstd
}

!CONFIG( no_extra_tiff_handler ) {
  DEFINES += MAKE_EXTRA_TIFF_HANDLER
  LIBS += -ltiff
}

CONFIG( no_epwing_support ) {
  DEFINES += NO_EPWING_SUPPORT
}

!CONFIG( no_epwing_support ) {
  HEADERS += src/epwing.hh \
             src/epwing_book.hh \
             src/epwing_charmap.hh
  SOURCES += src/epwing.cc \
             src/epwing_book.cc \
             src/epwing_charmap.cc
  LIBS += -leb
}

CONFIG( chinese_conversion_support ) {
  DEFINES += MAKE_CHINESE_CONVERSION_SUPPORT
  FORMS   += src/chineseconversion.ui
  HEADERS += src/chinese.hh \
             src/chineseconversion.hh
  SOURCES += src/chinese.cc \
             src/chineseconversion.cc
  win32-msvc* {
    Debug:   LIBS += -lopenccd
    Release: LIBS += -lopencc
  } else {
    mac {
      LIBS += -lopencc.2
    } else {
      LIBS += -lopencc
    }
  }
}

CONFIG( old_hunspell ) {
  DEFINES += OLD_HUNSPELL_INTERFACE
}

RESOURCES += resources.qrc \
    data/flags/flags.qrc \
    data/icons/icons.qrc \
    data/style/style.qrc

TRANSLATIONS += data/locale/ru_RU.ts \
    data/locale/zh_CN.ts \
    data/locale/cs_CZ.ts \
    data/locale/de_DE.ts \
    data/locale/el_GR.ts \
    data/locale/bg_BG.ts \
    data/locale/ar_SA.ts \
    data/locale/lt_LT.ts \
    data/locale/uk_UA.ts \
    data/locale/vi_VN.ts \
    data/locale/it_IT.ts \
    data/locale/pl_PL.ts \
    data/locale/ja_JP.ts \
    data/locale/zh_TW.ts \
    data/locale/sq_AL.ts \
    data/locale/pt_BR.ts \
    data/locale/es_AR.ts \
    data/locale/es_BO.ts \
    data/locale/es_ES.ts \
    data/locale/sk_SK.ts \
    data/locale/tr_TR.ts \
    data/locale/qu_WI.ts \
    data/locale/tg_TJ.ts \
    data/locale/ay_WI.ts \
    data/locale/be_BY.ts \
    data/locale/be_BY@latin.ts \
    data/locale/fr_FR.ts \
    data/locale/ko_KR.ts \
    data/locale/nl_NL.ts \
    data/locale/sr_SR.ts \
    data/locale/sv_SE.ts \
    data/locale/tk_TM.ts \
    data/locale/fa_IR.ts \
    data/locale/mk_MK.ts \
    data/locale/eo_EO.ts \
    data/locale/fi_FI.ts \
    data/locale/jb_JB.ts \
    data/locale/hi_IN.ts \
    data/locale/ie_001.ts

# Build version file
!isEmpty( hasGit ) {
  QMAKE_EXTRA_TARGETS += revtarget
  PRE_TARGETDEPS      += $$PWD/version.txt
  revtarget.target     = $$PWD/version.txt

  !win32 {
    revtarget.commands   = cd $$PWD; git describe --tags --always --dirty > $$revtarget.target
  } else {
    revtarget.commands   = git --git-dir=\"$$PWD/.git\" describe --tags --always --dirty > $$revtarget.target
  }

  ALL_SOURCES = $$SOURCES $$HEADERS $$FORMS
  for(src, ALL_SOURCES) {
    QUALIFIED_SOURCES += $${PWD}/$${src}
  }
  revtarget.depends = $$QUALIFIED_SOURCES
}

# This makes qmake generate translations

win32:# Windows doesn't seem to have *-qt4 symlinks
isEmpty(QMAKE_LRELEASE):QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
isEmpty(QMAKE_LRELEASE):QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease-qt4

# The *.qm files might not exist when qmake is run for the first time,
# causing the standard install rule to be ignored, and no translations
# will be installed. With this, we create the qm files during qmake run.
!win32 {
  system($${QMAKE_LRELEASE} -silent $${_PRO_FILE_} 2> /dev/null)
}

updateqm.input = TRANSLATIONS
updateqm.output = data/locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE \
    ${QMAKE_FILE_IN} \
    -qm \
    ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
TS_OUT = $$TRANSLATIONS
TS_OUT ~= s/.ts/.qm/g
PRE_TARGETDEPS += $$TS_OUT

include( src/third-party/qtsingleapplication/src/qtsingleapplication.pri )

