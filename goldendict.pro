TEMPLATE = app
TARGET = goldendict
VERSION = 22.5.22-alpha

# Generate version file. We do this here and in a build rule described later.
# The build rule is required since qmake isn't run each time the project is
# rebuilt; and doing it here is required too since any other way the RCC
# compiler would complain if version.txt wouldn't exist (fresh checkouts).

system(git describe --tags --always --dirty): hasGit=1

!isEmpty(hasGit){
    GIT_HASH=$$system(git rev-parse --short=8 HEAD )
}

win32{
# date /T output is locale aware.
    DD=$$system(date /T)
    DATE =$$replace(DD, / , )
}
else{
    DATE=$$system(date '+%y%m%d')
}

system(echo $${VERSION}.$${GIT_HASH} on $${DATE} > version.txt)

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
INCLUDEPATH += .

QT += core \
      gui \
      xml \
      network \
      svg \
      widgets \
      webenginewidgets\
      webchannel\
      printsupport \
      help

greaterThan(QT_MAJOR_VERSION, 5): QT += webenginecore core5compat

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050F00

# QMediaPlayer is not available in Qt4.
!CONFIG( no_qtmultimedia_player ) {
  QT += multimedia
  DEFINES += MAKE_QTMULTIMEDIA_PLAYER
}

!CONFIG( no_ffmpeg_player ) {
  DEFINES += MAKE_FFMPEG_PLAYER
}

CONFIG += exceptions \
    rtti \
    stl  \
    c++17 \
    lrelease \
    embed_translations

mac {
    DEBUG:CONFIG += app_bundle
}

QM_FILES_RESOURCE_PREFIX = /locale/
OBJECTS_DIR = build
UI_DIR = build
MOC_DIR = build
#RCC_DIR = build
LIBS += \
        -lz \
        -lbz2 \
        -llzo2

CONFIG+=utf8_source

CONFIG+=force_debug_info

win32 {
    TARGET = GoldenDict

    win32-msvc* {
        VERSION = 22.5.22 # VS does not recognize 22.number.alpha,cause errors during compilation under MSVC++
        DEFINES += __WIN32 _CRT_SECURE_NO_WARNINGS
        contains(QMAKE_TARGET.arch, x86_64) {
            DEFINES += NOMINMAX __WIN64
        }
        LIBS += -L$${PWD}/winlibs/lib/msvc
        # silence the warning C4290: C++ exception specification ignored
        QMAKE_CXXFLAGS += /wd4290 /Zc:__cplusplus /std:c++17 /permissive- 
        # QMAKE_LFLAGS_RELEASE += /OPT:REF /OPT:ICF

        # QMAKE_CXXFLAGS_RELEASE += /GL # slows down the linking significantly
        LIBS += -lshell32 -luser32 -lsapi -lole32
        Debug: LIBS+= -lhunspelld
        Release: LIBS+= -lhunspell
        HUNSPELL_LIB = hunspell
    }

    LIBS += -lwsock32 \
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
            -lswresample \
            -lavutil \
            -lavformat \
            -lavcodec
    }

    RC_ICONS += icons/programicon.ico icons/programicon_old.ico
    INCLUDEPATH += winlibs/include

    # Enable console in Debug mode on Windows, with useful logging messages
    Debug:CONFIG += console

    Release:DEFINES += NO_CONSOLE

    gcc48:QMAKE_CXXFLAGS += -Wno-unused-local-typedefs

    !CONFIG( no_chinese_conversion_support ) {
        CONFIG += chinese_conversion_support
    }
}

unix:!mac {
    DEFINES += HAVE_X11

    lessThan(QT_MAJOR_VERSION, 6):     QT += x11extras

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
    !arm {
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
    metainfo.path = $$PREFIX/share/metainfo
    metainfo.files = redist/*.metainfo.xml
    INSTALLS += metainfo
    helps.path = $$PREFIX/share/goldendict/help/
    helps.files = help/*.qch
    INSTALLS += helps
}
freebsd {
    LIBS +=   -lexecinfo
}
mac {
    TARGET = GoldenDict
    # Uncomment this line to make a universal binary.
    # You will need to use Xcode 3 and Qt Carbon SDK
    # if you want the support for PowerPC and/or Mac OS X 10.4
    # CONFIG += x86 x86_64 ppc
    LIBS = -lz \
        -lbz2 \
        -lvorbisfile \
        -lvorbis \
        -logg \
        -lhunspell \
        -llzo2
    !CONFIG( no_ffmpeg_player ) {
        LIBS += -lao \
            -lswresample \
            -lavutil \
            -lavformat \
            -lavcodec
    }
    QT_CONFIG -= no-pkg-config 
    CONFIG += link_pkgconfig
    INCLUDEPATH = $${PWD}/maclibs/include
    LIBS += -L$${PWD}/maclibs/lib -framework AppKit -framework Carbon
    OBJECTIVE_SOURCES += lionsupport.mm \
                         machotkeywrapper.mm \
                         macmouseover.mm \
                         speechclient_mac.mm
    ICON = icons/macicon.icns
    QMAKE_INFO_PLIST = myInfo.plist
    QMAKE_POST_LINK = mkdir -p GoldenDict.app/Contents/Frameworks && \
                      cp -nR $${PWD}/maclibs/lib/ GoldenDict.app/Contents/Frameworks/ && \
                      mkdir -p GoldenDict.app/Contents/MacOS/help && \
                      cp -R $${PWD}/help/*.qch GoldenDict.app/Contents/MacOS/help/

    CONFIG += zim_support
    !CONFIG( no_chinese_conversion_support ) {
        CONFIG += chinese_conversion_support
        QMAKE_POST_LINK += && mkdir -p GoldenDict.app/Contents/MacOS/opencc && \
                             cp -R $${PWD}/opencc/*.* GoldenDict.app/Contents/MacOS/opencc/
    }
}
DEFINES += PROGRAM_VERSION=\\\"$$VERSION\\\"

# Input
HEADERS += folding.hh \
    ankiconnector.h \
    article_inspect.h \
    articlewebpage.h \
    globalbroadcaster.h \
    iframeschemehandler.h \
    inc_case_folding.hh \
    inc_diacritic_folding.hh \
    mainwindow.hh \
    resourceschemehandler.h \
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
    weburlrequestinterceptor.h \
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
    audioplayerinterface.hh \
    audioplayerfactory.hh \
    ffmpegaudioplayer.hh \
    multimediaaudioplayer.hh \
    externalaudioplayer.hh \
    externalviewer.hh \
    wordfinder.hh \
    groupcombobox.hh \
    keyboardstate.hh \
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
    delegate.hh \
    zim.hh \
    gddebug.hh \
    utils.hh \
    gestures.hh \
    tiff.hh \
    dictheadwords.hh \
    fulltextsearch.hh \
    ftshelpers.hh \
    dictserver.hh \
    helpwindow.hh \
    slob.hh \
    ripemd.hh \
    gls.hh \
    splitfile.hh \
    favoritespanewidget.hh \
    treeview.hh

FORMS += groups.ui \
    dictgroupwidget.ui \
    mainwindow.ui \
    sources.ui \
    initializing.ui \
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
    ankiconnector.cpp \
    article_inspect.cpp \
    articlewebpage.cpp \
    globalbroadcaster.cpp \
    iframeschemehandler.cpp \
    main.cc \
    dictionary.cc \
    config.cc \
    resourceschemehandler.cpp \
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
    weburlrequestinterceptor.cpp \
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
    audioplayerfactory.cc \
    multimediaaudioplayer.cc \
    externalaudioplayer.cc \
    externalviewer.cc \
    wordfinder.cc \
    groupcombobox.cc \
    keyboardstate.cc \
    preferences.cc \
    mutex.cc \
    mediawiki.cc \
    sounddir.cc \
    hunspell.cc \
    dictdfiles.cc \
    audiolink.cc \
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
    ripemd.cc \
    gls.cc \
    splitfile.cc \
    favoritespanewidget.cc \
    treeview.cc

win32 {
    FORMS   += texttospeechsource.ui
    SOURCES += guids.c \
               speechclient_win.cc \
               texttospeechsource.cc \
               speechhlp.cc
    HEADERS += texttospeechsource.hh \
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

unix:!mac {
    HEADERS += scanflag.hh
    FORMS   += scanflag.ui
    SOURCES += scanflag.cc
}


    HEADERS += wildcard.hh
    SOURCES += wildcard.cc


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
  LIBS += -lopencc
}

RESOURCES += resources.qrc \
    flags.qrc
#EXTRA_TRANSLATIONS += thirdparty/qwebengine_ts/qtwebengine_zh_CN.ts
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
    locale/mk_MK.ts \
    locale/eo_EO.ts \
    locale/fi_FI.ts \
    locale/jb_JB.ts \
    locale/hi_IN.ts \
    locale/ie_001.ts

# Build version file
!isEmpty( hasGit ) {
  PRE_TARGETDEPS      += $$PWD/version.txt
}



include( thirdparty/qtsingleapplication/src/qtsingleapplication.pri )

