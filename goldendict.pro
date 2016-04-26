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

# DEPENDPATH += . generators
INCLUDEPATH += .

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
} else {
    QT += webkit
    CONFIG += help
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

!isEmpty(DISABLE_INTERNAL_PLAYER): DEFINES += DISABLE_INTERNAL_PLAYER

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
          CONFIG(gcc48) {
            LIBS += -lhunspell-1.3.2
          } else {
            greaterThan(QT_MAJOR_VERSION, 4) {
              lessThan(QT_MINOR_VERSION, 1) {
                LIBS += -lhunspell-1.3-sjlj
              } else {
                LIBS += -lhunspell-1.3-dw2
              }
            } else {
              LIBS += -lhunspell-1.3.2
            }
          }
        } else {
          LIBS += -l$$HUNSPELL_LIB
        }
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
    isEmpty(DISABLE_INTERNAL_PLAYER) {
        LIBS += -lao \
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
    isEmpty(DISABLE_INTERNAL_PLAYER) {
        PKGCONFIG += ao \
            libavutil \
            libavformat \
            libavcodec
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
    locale.files = locale/*.qm
    INSTALLS += target \
        locale
    icons.path = $$PREFIX/share/pixmaps
    icons.files = redist/icons/*.*
    INSTALLS += icons
    desktops.path = $$PREFIX/share/applications
    desktops.files = redist/*.desktop
    INSTALLS += desktops
    helps.path = $$PREFIX/share/goldendict/help/
    helps.files = help/*.qch
    INSTALLS += helps
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
        -lhunspell-1.2 \
        -llzo2
    isEmpty(DISABLE_INTERNAL_PLAYER) {
        LIBS += -lao \
            -lavutil-gd \
            -lavformat-gd \
            -lavcodec-gd
    }
    INCLUDEPATH = $${PWD}/maclibs/include
    LIBS += -L$${PWD}/maclibs/lib -framework AppKit -framework Carbon
    OBJECTIVE_SOURCES += lionsupport.mm \
                         machotkeywrapper.mm \
                         macmouseover.mm \
                         speechclient_mac.mm
    ICON = icons/macicon.icns
    QMAKE_INFO_PLIST = myInfo.plist
    QMAKE_POST_LINK = mkdir -p GoldenDict.app/Contents/Frameworks & \
                      cp -nR $${PWD}/maclibs/lib/ GoldenDict.app/Contents/Frameworks/ & \
                      mkdir -p GoldenDict.app/Contents/MacOS/locale & \
                      cp -R locale/*.qm GoldenDict.app/Contents/MacOS/locale/ & \
                      mkdir -p GoldenDict.app/Contents/MacOS/help & \
                      cp -R $${PWD}/help/*.qch GoldenDict.app/Contents/MacOS/help/

    CONFIG += zim_support
    !CONFIG( no_chinese_conversion_support ) {
        CONFIG += chinese_conversion_support
        CONFIG( x86_64 ) {
            QMAKE_POST_LINK += & mkdir -p GoldenDict.app/Contents/MacOS/opencc & \
                                 cp -R $${PWD}/opencc/x64/*.json GoldenDict.app/Contents/MacOS/opencc/ & \
                                 cp -R $${PWD}/opencc/x64/*.ocd GoldenDict.app/Contents/MacOS/opencc/
        } else {
            QMAKE_POST_LINK += & mkdir -p GoldenDict.app/Contents/MacOS/opencc & \
                                 cp -R $${PWD}/opencc/*.json GoldenDict.app/Contents/MacOS/opencc/ & \
                                 cp -R $${PWD}/opencc/*.ocd GoldenDict.app/Contents/MacOS/opencc/
        }
    }
}
DEFINES += PROGRAM_VERSION=\\\"$$VERSION\\\"

# Input
HEADERS += folding.hh \
    inc_case_folding.hh \
    inc_diacritic_folding.hh \
    mainwindow.hh \
    sptr.hh \
    dictionary.hh \
    ex.hh \
    config.hh \
    sources.hh \
    utf8.hh \
    file.hh \
    bgl_babylon.hh \
    bgl.hh \
    initializing.hh \
    article_netmgr.hh \
    dictzip.h \
    btreeidx.hh \
    stardict.hh \
    chunkedstorage.hh \
    xdxf2html.hh \
    iconv.hh \
    lsa.hh \
    htmlescape.hh \
    dsl.hh \
    dsl_details.hh \
    filetype.hh \
    fsencoding.hh \
    groups.hh \
    groups_widgets.hh \
    instances.hh \
    article_maker.hh \
    scanpopup.hh \
    articleview.hh \
    externalviewer.hh \
    wordfinder.hh \
    groupcombobox.hh \
    keyboardstate.hh \
    mouseover.hh \
    preferences.hh \
    mutex.hh \
    mediawiki.hh \
    sounddir.hh \
    hunspell.hh \
    dictdfiles.hh \
    audiolink.hh \
    wstring.hh \
    wstring_qt.hh \
    processwrapper.hh \
    hotkeywrapper.hh \
    searchpanewidget.hh \
    hotkeyedit.hh \
    langcoder.hh \
    editdictionaries.hh \
    loaddictionaries.hh \
    transliteration.hh \
    romaji.hh \
    belarusiantranslit.hh \
    russiantranslit.hh \
    german.hh \
    website.hh \
    orderandprops.hh \
    language.hh \
    dictionarybar.hh \
    broken_xrecord.hh \
    history.hh \
    atomic_rename.hh \
    articlewebview.hh \
    zipfile.hh \
    indexedzip.hh \
    termination.hh \
    greektranslit.hh \
    webmultimediadownload.hh \
    forvo.hh \
    country.hh \
    about.hh \
    programs.hh \
    parsecmdline.hh \
    dictspanewidget.hh \
    maintabwidget.hh \
    mainstatusbar.hh \
    gdappstyle.hh \
    ufile.hh \
    xdxf.hh \
    sdict.hh \
    decompress.hh \
    aard.hh \
    mruqmenu.hh \
    dictinfo.hh \
    zipsounds.hh \
    stylescombobox.hh \
    extlineedit.hh \
    translatebox.hh \
    historypanewidget.hh \
    wordlist.hh \
    mdictparser.hh \
    mdx.hh \
    voiceengines.hh \
    ffmpegaudio.hh \
    articleinspector.hh \
    delegate.hh \
    zim.hh \
    gddebug.hh \
    qt4x5.hh \
    gestures.hh \
    tiff.hh \
    dictheadwords.hh \
    fulltextsearch.hh \
    ftshelpers.hh \
    dictserver.hh \
    helpwindow.hh \
    slob.hh \
    ripemd.hh

FORMS += groups.ui \
    dictgroupwidget.ui \
    mainwindow.ui \
    sources.ui \
    initializing.ui \
    groupselectorwidget.ui \
    scanpopup.ui \
    articleview.ui \
    preferences.ui \
    about.ui \
    editdictionaries.ui \
    orderandprops.ui \
    dictinfo.ui \
    dictheadwords.ui \
    authentication.ui \
    fulltextsearch.ui

SOURCES += folding.cc \
    main.cc \
    dictionary.cc \
    config.cc \
    sources.cc \
    mainwindow.cc \
    utf8.cc \
    file.cc \
    bgl_babylon.cc \
    bgl.cc \
    initializing.cc \
    article_netmgr.cc \
    dictzip.c \
    btreeidx.cc \
    stardict.cc \
    chunkedstorage.cc \
    xdxf2html.cc \
    iconv.cc \
    lsa.cc \
    htmlescape.cc \
    dsl.cc \
    dsl_details.cc \
    filetype.cc \
    fsencoding.cc \
    groups.cc \
    groups_widgets.cc \
    instances.cc \
    article_maker.cc \
    scanpopup.cc \
    articleview.cc \
    externalviewer.cc \
    wordfinder.cc \
    groupcombobox.cc \
    keyboardstate.cc \
    mouseover.cc \
    preferences.cc \
    mutex.cc \
    mediawiki.cc \
    sounddir.cc \
    hunspell.cc \
    dictdfiles.cc \
    audiolink.cc \
    wstring.cc \
    wstring_qt.cc \
    processwrapper.cc \
    hotkeywrapper.cc \
    hotkeyedit.cc \
    langcoder.cc \
    editdictionaries.cc \
    loaddictionaries.cc \
    transliteration.cc \
    romaji.cc \
    belarusiantranslit.cc \
    russiantranslit.cc \
    german.cc \
    website.cc \
    orderandprops.cc \
    language.cc \
    dictionarybar.cc \
    broken_xrecord.cc \
    history.cc \
    atomic_rename.cc \
    articlewebview.cc \
    zipfile.cc \
    indexedzip.cc \
    termination.cc \
    greektranslit.cc \
    webmultimediadownload.cc \
    forvo.cc \
    country.cc \
    about.cc \
    programs.cc \
    parsecmdline.cc \
    maintabwidget.cc \
    mainstatusbar.cc \
    gdappstyle.cc \
    ufile.cc \
    xdxf.cc \
    sdict.cc \
    decompress.cc \
    aard.cc \
    mruqmenu.cc \
    dictinfo.cc \
    zipsounds.cc \
    stylescombobox.cc \
    extlineedit.cc \
    translatebox.cc \
    historypanewidget.cc \
    wordlist.cc \
    mdictparser.cc \
    mdx.cc \
    voiceengines.cc \
    ffmpegaudio.cc \
    articleinspector.cc \
    delegate.cc \
    zim.cc \
    gddebug.cc \
    gestures.cc \
    tiff.cc \
    dictheadwords.cc \
    fulltextsearch.cc \
    ftshelpers.cc \
    dictserver.cc \
    helpwindow.cc \
    slob.cc \
    ripemd.cc

win32 {
    FORMS   += texttospeechsource.ui
    SOURCES += mouseover_win32/ThTypes.c \
               wordbyauto.cc \
               guids.c \
               x64.cc \
               speechclient_win.cc \
               texttospeechsource.cc \
               speechhlp.cc
    HEADERS += mouseover_win32/ThTypes.h \
               wordbyauto.hh \
               uiauto.hh \
               x64.hh \
               texttospeechsource.hh \
               sapi.hh \
               sphelper.hh \
               speechclient.hh \
               speechhlp.hh
}

mac {
    HEADERS += macmouseover.hh \
               texttospeechsource.hh \
               speechclient.hh
    FORMS   += texttospeechsource.ui
    SOURCES += texttospeechsource.cc
}

CONFIG( zim_support ) {
  DEFINES += MAKE_ZIM_SUPPORT
  LIBS += -llzma
}

!CONFIG( no_extra_tiff_handler ) {
  DEFINES += MAKE_EXTRA_TIFF_HANDLER
  LIBS += -ltiff
}

CONFIG( no_epwing_support ) {
  DEFINES += NO_EPWING_SUPPORT
}

!CONFIG( no_epwing_support ) {
  HEADERS += epwing.hh \
             epwing_book.hh \
             epwing_charmap.hh
  SOURCES += epwing.cc \
             epwing_book.cc \
             epwing_charmap.cc
  LIBS += -leb
}

CONFIG( chinese_conversion_support ) {
  DEFINES += MAKE_CHINESE_CONVERSION_SUPPORT
  FORMS   += chineseconversion.ui
  HEADERS += chinese.hh \
             chineseconversion.hh
  SOURCES += chinese.cc \
             chineseconversion.cc
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

RESOURCES += resources.qrc \
    flags.qrc
TRANSLATIONS += locale/ru_RU.ts \
    locale/zh_CN.ts \
    locale/cs_CZ.ts \
    locale/de_DE.ts \
    locale/el_GR.ts \
    locale/bg_BG.ts \
    locale/ar_SA.ts \
    locale/lt_LT.ts \
    locale/uk_UA.ts \
    locale/vi_VN.ts \
    locale/it_IT.ts \
    locale/pl_PL.ts \
    locale/ja_JP.ts \
    locale/zh_TW.ts \
    locale/sq_AL.ts \
    locale/pt_BR.ts \
    locale/es_AR.ts \
    locale/es_BO.ts \
    locale/es_ES.ts \
    locale/sk_SK.ts \
    locale/tr_TR.ts \
    locale/qu_WI.ts \
    locale/tg_TJ.ts \
    locale/ay_WI.ts \
    locale/be_BY.ts \
    locale/be_BY@latin.ts \
    locale/fr_FR.ts \
    locale/ko_KR.ts \
    locale/nl_NL.ts \
    locale/sr_SR.ts \
    locale/sv_SE.ts \
    locale/tk_TM.ts \
    locale/fa_IR.ts \
    locale/mk_MK.ts

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
updateqm.output = locale/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE \
    ${QMAKE_FILE_IN} \
    -qm \
    ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm
TS_OUT = $$TRANSLATIONS
TS_OUT ~= s/.ts/.qm/g
PRE_TARGETDEPS += $$TS_OUT

include( qtsingleapplication/src/qtsingleapplication.pri )

