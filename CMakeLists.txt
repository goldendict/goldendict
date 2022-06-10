cmake_minimum_required(VERSION 3.16)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17.0")
    cmake_policy(SET CMP0100 NEW)
endif()

project(goldendict
    VERSION 1.5.0
    DESCRIPTION "A feature-rich dictionary lookup program, supporting multiple dictionary formats and online dictionaries"
    HOMEPAGE_URL goldendict.org
)

set(PROGRAM_VERSION_SUFFIX "-RC2+git")
set(PROGRAM_VERSION "${PROJECT_VERSION}${PROGRAM_VERSION_SUFFIX}")
set(LINUX_PROGRAM_DATA_DIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

set(REQUIRED_QT4_VERSION 4.7.0)
set(REQUIRED_QT5_VERSION 5.1.0)

if(UNIX)
    set(DESKTOP_FILE org.goldendict.GoldenDict.desktop)
    set(METAINFO_FILE org.goldendict.GoldenDict.metainfo.xml)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
set(CMAKE_WIN32_EXECUTABLE ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(USE_QT4 "Build GoldenDict with Qt 4" OFF)
option(USE_QT5 "Build GoldenDict with Qt 5" ON)
option(USE_OLD_HUNSPELL "Use old Hunspell API" ON)
option(WITH_EXTRA_TIFF_HANDLER "Enable extra TIFF handler" ON)
option(WITH_QTMULTIMEDIA_PLAYER "Enable support for QtMultiMedia player" ON)
option(WITH_FFMPEG_PLAYER "Enable support for FFMPEG player" ON)
option(WITH_EPWING "Enable support for Epwing dictionary format" OFF)
option(WITH_ZIM "Enable support for ZIM Wiki dictionary" ON)
option(WITH_OPENCC "Enable support of Open Chinese Conversion" OFF)

set(QAPPLICATION_CLASS QApplication)
add_subdirectory(third-party/singleapplication)

add_definitions(-DPROGRAM_VERSION=\"${PROGRAM_VERSION}\")
if(UNIX AND NOT APPLE)
    add_definitions(-DPROGRAM_DATA_DIR=\"${LINUX_PROGRAM_DATA_DIR}/locale\")
endif()

include(GNUInstallDirs)


find_package(PkgConfig REQUIRED)
pkg_check_modules(linux_base_libs REQUIRED IMPORTED_TARGET
    "vorbisfile" "vorbis" "ogg" "hunspell"
)
if(WITH_FFMPEG_PLAYER)
    pkg_check_modules(ffmpeg_libs REQUIRED IMPORTED_TARGET
        "ao" "libavutil" "libavformat" "libavcodec" "libswresample"
    )
endif()

if(USE_QT4)
    find_package(Qt4 ${REQUIRED_QT4_VERSION} REQUIRED COMPONENTS
        Core Gui Network Sql Svg Xml WebKit X11Extras
        LinguistTools # for qt_add_translation command
    )
    find_package(Qt4 ${REQUIRED_QT4_VERSION} CONFIG REQUIRED
        Help 
    )
elseif(USE_QT5)
    find_package(Qt5 ${REQUIRED_QT5_VERSION} REQUIRED COMPONENTS
        Core Gui Network Sql Svg Xml
        Help Widgets PrintSupport WebKitWidgets X11Extras
        LinguistTools
    )
    if(WITH_QTMULTIMEDIA_PLAYER)
        find_package(Qt5 ${REQUIRED_QT5_VERSION} REQUIRED COMPONENTS
            Multimedia
        )
    endif()
endif()

if(WITH_EXTRA_TIFF_HANDLER)
    find_package(TIFF)
endif()

if(WITH_ZIM)
    find_package(LibLZMA)
endif()

file(GLOB HELP_FILES ${CMAKE_SOURCE_DIR}/data/help/*.qch)
file(GLOB LOCALE_FILES ${CMAKE_SOURCE_DIR}/data/locale/*.ts)

qt_add_translation(QM_FILES ${LOCALE_FILES})

add_executable(${PROJECT_NAME}
    ${QM_FILES}
    resources.qrc
    data/flags/flags.qrc
    data/icons/icons.qrc
    data/style/style.qrc
    src/utils/broken_xrecord.cc
    src/utils/decompress.cc
    src/utils/dictzip.c
    src/utils/file.cc
    src/utils/filetype.cc
    src/utils/folding.cc
    src/utils/fsencoding.cc
    src/utils/gddebug.cc
    src/utils/iconv.cc
    src/utils/indexedzip.cc
    src/utils/splitfile.cc
    src/utils/tiff.cc
    src/utils/ufile.cc
    src/utils/utf8.cc
    src/utils/wstring.cc
    src/utils/wstring_qt.cc
    src/utils/zipfile.cc
    src/utils/zipsounds.cc
    src/lang/belarusiantranslit.cc
    src/lang/country.cc
    src/lang/german.cc
    src/lang/greektranslit.cc
    src/lang/hunspell.cc
    src/lang/langcoder.cc
    src/lang/language.cc
    src/lang/romaji.cc
    src/lang/russiantranslit.cc
    src/dict/aard.cc
    src/dict/bgl/bgl.cc
    src/dict/bgl/bgl_babylon.cc
    src/dict/btreeidx.cc 
    src/dict/dictdfiles.cc
    src/dict/dictionary.cc
    src/dict/dsl/dsl.cc
    src/dict/dsl/dsl_details.cc
    src/dict/gls.cc
    src/dict/mdict/mdictparser.cc
    src/dict/mdict/mdx.cc
    src/dict/sdict.cc
    src/dict/slob.cc
    src/dict/stardict.cc
    src/dict/xdxf/xdxf.cc
    src/dict/xdxf/xdxf2html.cc
    src/dict/zim.cc
    src/about.cc src/about.ui
    src/article_maker.cc
    src/article_netmgr.cc
    src/articleinspector.cc
    src/articleview.cc src/articleview.ui
    src/articlewebview.cc
    src/atomic_rename.cc
    src/audiolink.cc
    src/audioplayerfactory.cc
    src/audioplayerinterface.hh
    src/authentication.ui
    src/chunkedstorage.cc
    src/config.cc
    src/delegate.cc
    src/dictgroupwidget.ui
    src/dictheadwords.cc src/dictheadwords.ui
    src/dictinfo.cc src/dictinfo.ui
    src/dictionarybar.cc
    src/dictserver.cc
    src/dictspanewidget.hh
    src/editdictionaries.cc src/editdictionaries.ui
    src/externalaudioplayer.cc
    src/externalviewer.cc
    src/extlineedit.cc
    src/favoritespanewidget.cc
    src/ffmpegaudio.cc
    src/ffmpegaudioplayer.hh
    src/forvo.cc
    src/ftshelpers.cc
    src/fulltextsearch.cc src/fulltextsearch.ui
    src/gdappstyle.cc
    src/gestures.cc
    src/groupcombobox.cc
    src/groups.cc src/groups.ui
    src/groups_widgets.cc
    src/helpwindow.cc
    src/history.cc
    src/historypanewidget.cc
    src/hotkeyedit.cc
    src/hotkeywrapper.cc
    src/htmlescape.cc
    src/initializing.cc src/initializing.ui
    src/instances.cc
    src/keyboardstate.cc
    src/loaddictionaries.cc
    src/lsa.cc
    src/main.cc
    src/mainstatusbar.cc
    src/maintabwidget.cc
    src/mainwindow.cc src/mainwindow.ui
    src/mediawiki.cc
    src/mouseover.cc
    src/mruqmenu.cc
    src/multimediaaudioplayer.cc
    src/mutex.cc
    src/orderandprops.cc src/orderandprops.ui
    src/parsecmdline.cc
    src/preferences.cc src/preferences.ui
    src/processwrapper.cc
    src/programs.cc
    src/ripemd.cc
    src/scanpopup.cc src/scanpopup.ui
    src/sounddir.cc
    src/sources.cc src/sources.ui
    src/stylescombobox.cc
    src/termination.cc
    src/translatebox.cc
    src/transliteration.cc
    src/treeview.cc
    src/voiceengines.cc
    src/webmultimediadownload.cc
    src/website.cc
    src/wordfinder.cc
    src/wordlist.cc
)

if(USE_QT5)
    target_sources(${PROJECT_NAME} PUBLIC
        src/utils/wildcard.cc
    )
endif()

if(WITH_EPWING)
    target_sources(${PROJECT_NAME} PUBLIC
        src/dict/epwing/epwing.cc
        src/dict/epwing/epwing_book.cc
        src/dict/epwing/epwing_charmap.cc
    )
endif()

if(WITH_OPENCC)
    target_sources(${PROJECT_NAME} PUBLIC
        src/lang/chinese.cc
        src/chineseconversion.cc
        src/chineseconversion.ui
    )
endif()

if(UNIX)
    target_sources(${PROJECT_NAME} PUBLIC
        src/scanflag.cc
        src/scanflag.ui
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    third-party/qtsingleapplication/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SingleApplication::SingleApplication
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Sql
    Qt::Svg
    Qt::Xml
    Qt::X11Extras
    bz2
    lzo2
    z
)

if(USE_QT5)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Help
        Qt::PrintSupport
        Qt5::WebKitWidgets
        Qt::Widgets
    )
    if(WITH_QTMULTIMEDIA_PLAYER)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::Multimedia
        )
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            MAKE_QTMULTIMEDIA_PLAYER
        )
    endif()
endif()

if(USE_QT4)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::WebKit
    )
endif()

if(WITH_FFMPEG_PLAYER)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        PkgConfig::ffmpeg_libs
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        MAKE_FFMPEG_PLAYER
    )
endif()


if(WITH_ZIM)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        zstd lzma
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        MAKE_ZIM_SUPPORT
    )
endif()

if(WITH_EXTRA_TIFF_HANDLER)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        TIFF::TIFF
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        MAKE_EXTRA_TIFF_HANDLER
    )
endif()

if(WITH_EPWING)
    target_link_libraries(${PROJECT_NAME} PRIVATE eb)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE NO_EPWING_SUPPORT)
endif()

if(WITH_OPENCC)
    target_link_libraries(${PROJECT_NAME} PRIVATE opencc)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        MAKE_CHINESE_CONVERSION_SUPPORT
    )
endif()

if(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        PkgConfig::linux_base_libs
        X11 Xtst
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        HAVE_X11
    )
endif()

if(UNIX)
    install(TARGETS ${PROJECT_NAME})
    install(FILES ${QM_FILES} DESTINATION ${LINUX_PROGRAM_DATA_DIR}/locale)
    install(FILES data/${DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES data/${METAINFO_FILE} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
    install(FILES ${HELP_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${APPLICATION_NAME}/help)
endif()
