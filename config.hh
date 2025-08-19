/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CONFIG_HH_INCLUDED__
#define __CONFIG_HH_INCLUDED__

#include <QObject>
#include <QVector>
#include <QString>
#include <QSize>
#include <QDateTime>
#include <QKeySequence>
#include <QSet>
#include <QMetaType>
#include "cpp_features.hh"
#include "ex.hh"

#ifdef Q_OS_WIN
#include <QRect>
#endif

/// GoldenDict's configuration
namespace Config {

/// Dictionaries which are temporarily disabled via the dictionary bar.
typedef QSet< QString > MutedDictionaries;

#ifdef Q_OS_WIN
#pragma pack(push,4)
#endif

/// A path where to search for the dictionaries
struct Path
{
  QString path;
  bool recursive;

  Path(): recursive( false ) {}
  Path( QString const & path_, bool recursive_ ):
    path( path_ ), recursive( recursive_ ) {}

  bool operator == ( Path const & other ) const
  { return path == other.path && recursive == other.recursive; }
};

/// A list of paths where to search for the dictionaries
typedef QVector< Path > Paths;

/// A directory holding bunches of audiofiles, which is indexed into a separate
/// dictionary.
struct SoundDir
{
  QString path, name;
  QString iconFilename;

  SoundDir()
  {}

  SoundDir( QString const & path_, QString const & name_, QString iconFilename_ = "" ):
    path( path_ ), name( name_ ), iconFilename( iconFilename_ )
  {}

  bool operator == ( SoundDir const & other ) const
  { return path == other.path && name == other.name && iconFilename == other.iconFilename; }
};

/// A list of SoundDirs
typedef QVector< SoundDir > SoundDirs;

struct DictionaryRef
{
  QString id; // Dictionrary id, which is usually an md5 hash
  QString name; // Dictionary name, used to recover when its id changes

  DictionaryRef()
  {}

  DictionaryRef( QString const & id_, QString const & name_ ):
    id( id_ ), name( name_ ) {}

  bool operator == ( DictionaryRef const & other ) const
  { return id == other.id && name == other.name; }
};

/// A dictionary group
struct Group
{
  unsigned id;
  QString name, icon;
  QByteArray iconData;
  QKeySequence shortcut;
  QString favoritesFolder;
  QVector< DictionaryRef > dictionaries;
  Config::MutedDictionaries mutedDictionaries; // Disabled via dictionary bar
  Config::MutedDictionaries popupMutedDictionaries; // Disabled via dictionary bar in popup

  Group(): id( 0 ) {}

  bool operator == ( Group const & other ) const
  { return id == other.id && name == other.name && icon == other.icon &&
           favoritesFolder == other.favoritesFolder &&
           dictionaries == other.dictionaries && shortcut == other.shortcut &&
           mutedDictionaries == other.mutedDictionaries &&
           popupMutedDictionaries == other.popupMutedDictionaries &&
           iconData == other.iconData; }

  bool operator != ( Group const & other ) const
  { return ! operator == ( other ); }
};

/// All the groups
struct Groups: public QVector< Group >
{
  unsigned nextId; // Id to use to create the group next time

  Groups(): nextId( 1 )
  {}
};

/// Proxy server configuration
struct ProxyServer
{
  bool enabled;
  bool useSystemProxy;

  enum Type
  {
    Socks5 = 0,
    HttpConnect,
    HttpGet
  } type;

  QString host;
  unsigned port;
  QString user, password;
  QString systemProxyUser, systemProxyPassword;

  ProxyServer();
};

// A hotkey -- currently qt modifiers plus one or two keys
struct HotKey
{
  Qt::KeyboardModifiers modifiers;
  int key1, key2;

  HotKey();

  /// We use the first two keys of QKeySequence, with modifiers being stored
  /// in the first one.
  HotKey( QKeySequence const & );

  QKeySequence toKeySequence() const;
};

struct FullTextSearch
{
  int searchMode;
  bool matchCase;
  int maxArticlesPerDictionary;
  int maxDistanceBetweenWords;
  bool useMaxDistanceBetweenWords;
  bool useMaxArticlesPerDictionary;
  bool enabled;
  bool ignoreWordsOrder;
  bool ignoreDiacritics;
  quint32 maxDictionarySize;
  QByteArray dialogGeometry;
  QString disabledTypes;

  FullTextSearch() :
    searchMode( 0 ), matchCase( false ),
    maxArticlesPerDictionary( 100 ),
    maxDistanceBetweenWords( 2 ),
    useMaxDistanceBetweenWords( true ),
    useMaxArticlesPerDictionary( false ),
    enabled( true ),
    ignoreWordsOrder( false ),
    ignoreDiacritics( false ),
    maxDictionarySize( 0 )
  {}
};

/// This class encapsulates supported backend preprocessor logic,
/// discourages duplicating backend names in code, which is error-prone.
class InternalPlayerBackend
{
public:
  /// Returns true if at least one backend is available.
  static bool anyAvailable();
  /// Returns the default backend or null backend if none is available.
  static InternalPlayerBackend defaultBackend();
  /// Returns the name list of supported backends.
  static QStringList nameList();

  /// Returns true if built with FFmpeg player support and the name matches.
  bool isFfmpeg() const;
  /// Returns true if built with Qt Multimedia player support and the name matches.
  bool isQtmultimedia() const;

  QString const & uiName() const
  { return name; }

  void setUiName( QString const & name_ )
  { name = name_; }

  bool operator == ( InternalPlayerBackend const & other ) const
  { return name == other.name; }

  bool operator != ( InternalPlayerBackend const & other ) const
  { return ! operator == ( other ); }

private:
#ifdef MAKE_FFMPEG_PLAYER
  static InternalPlayerBackend ffmpeg()
  { return InternalPlayerBackend( "FFmpeg+libao" ); }
#endif

#ifdef MAKE_QTMULTIMEDIA_PLAYER
  static InternalPlayerBackend qtmultimedia()
  { return InternalPlayerBackend( "Qt Multimedia" ); }
#endif

  explicit InternalPlayerBackend( QString const & name_ ) : name( name_ )
  {}

  QString name;
};

#if defined( HAVE_X11 ) && QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  // The ScanPopup window flags customization code has been tested
  // only in X11 desktop environments and window managers.
  // None of the window flags configurations I have tried works perfectly well
  // in XFCE with Qt4. Let us enable customization code for Qt5 exclusively to
  // avoid regressions with Qt4.
  #define ENABLE_SPWF_CUSTOMIZATION
#endif

enum ScanPopupWindowFlags
{
  SPWF_default = 0,
  SPWF_Popup,
  SPWF_Tool
};
ScanPopupWindowFlags spwfFromInt( int id );

struct InputPhrase
{
  InputPhrase()
  {}

  InputPhrase( QString const & _phrase, QString const & _suffix ) :
    phrase( _phrase ),
    punctuationSuffix( _suffix )
  {}

  static InputPhrase fromPhrase( QString const & phrase )
  {
    return InputPhrase( phrase, QString() );
  }

  bool isValid() const { return !phrase.isEmpty(); }

  QString phraseWithSuffix() const { return phrase + punctuationSuffix; }

  QString phrase;
  QString punctuationSuffix;
};

inline bool operator == ( InputPhrase const & a, InputPhrase const & b )
{
  return a.phrase == b.phrase && a.punctuationSuffix == b.punctuationSuffix;
}
inline bool operator != ( InputPhrase const & a, InputPhrase const & b )
{
  return !( a == b );
}

/// Various user preferences
struct Preferences
{
  QString interfaceLanguage; // Empty value corresponds to system default
  QString helpLanguage; // Empty value corresponds to interface language
  QString displayStyle; // Empty value corresponds to the default one
  bool newTabsOpenAfterCurrentOne;
  bool newTabsOpenInBackground;
  bool hideSingleTab;
  bool mruTabOrder;
  bool hideMenubar;
  bool enableTrayIcon;
  bool startToTray;
  bool closeToTray;
  bool autoStart;
  bool doubleClickTranslates;
  bool selectWordBySingleClick;
  bool escKeyHidesMainWindow;
  bool alwaysOnTop;

  /// An old UI mode when tranlateLine and wordList
  /// are in the dockable side panel, not on the toolbar.
  bool searchInDock;

  bool enableMainWindowHotkey;
  HotKey mainWindowHotkey;
  bool enableClipboardHotkey;
  HotKey clipboardHotkey;

  bool enableScanPopup;
  bool startWithScanPopupOn;
  bool enableScanPopupModifiers;
  unsigned long scanPopupModifiers; // Combination of KeyboardState::Modifier
  bool scanPopupAltMode; // When you press modifier shortly after the selection
  unsigned scanPopupAltModeSecs;
  bool ignoreOwnClipboardChanges;
  bool scanPopupUseUIAutomation;
  bool scanPopupUseIAccessibleEx;
  bool scanPopupUseGDMessage;
  ScanPopupWindowFlags scanPopupUnpinnedWindowFlags;
  bool scanPopupUnpinnedBypassWMHint;
  bool scanToMainWindow;
  bool ignoreDiacritics;
#ifdef HAVE_X11
  bool showScanFlag;
#endif

  // Whether the word should be pronounced on page load, in main window/popup
  bool pronounceOnLoadMain, pronounceOnLoadPopup;
  bool useInternalPlayer;
  InternalPlayerBackend internalPlayerBackend;
  QString audioPlaybackProgram;

  ProxyServer proxyServer;

  bool checkForNewReleases;
  bool disallowContentFromOtherSites;
  bool enableWebPlugins;
  bool hideGoldenDictHeader;
  int maxNetworkCacheSize;
  bool clearNetworkCacheOnExit;

  qreal zoomFactor;
  qreal helpZoomFactor;
  int wordsZoomLevel;

  unsigned maxStringsInHistory;
  unsigned storeHistory;
  bool alwaysExpandOptionalParts;

  unsigned historyStoreInterval;
  unsigned favoritesStoreInterval;

  bool confirmFavoritesDeletion;

  bool collapseBigArticles;
  int articleSizeLimit;

  bool limitInputPhraseLength;
  int inputPhraseLengthLimit;
  InputPhrase sanitizeInputPhrase( QString const & inputPhrase ) const;

  unsigned short maxDictionaryRefsInContextMenu;
#ifndef Q_WS_X11
  bool trackClipboardChanges;
#endif

  bool synonymSearchEnabled;

  QString addonStyle;

  FullTextSearch fts;

  Preferences();
};

/// A MediaWiki network dictionary definition
struct MediaWiki
{
  QString id, name, url;
  bool enabled;
  QString icon;

  MediaWiki(): enabled( false )
  {}

  MediaWiki( QString const & id_, QString const & name_, QString const & url_,
             bool enabled_, QString const & icon_ ):
    id( id_ ), name( name_ ), url( url_ ), enabled( enabled_ ), icon( icon_ ) {}

  bool operator == ( MediaWiki const & other ) const
  { return id == other.id && name == other.name && url == other.url &&
           enabled == other.enabled && icon == other.icon ; }
};

/// Any website which can be queried though a simple template substitution
struct WebSite
{
  QString id, name, url;
  bool enabled;
  QString iconFilename;
  bool inside_iframe;

  WebSite(): enabled( false )
  {}

  WebSite( QString const & id_, QString const & name_, QString const & url_,
           bool enabled_, QString const & iconFilename_, bool inside_iframe_ ):
    id( id_ ), name( name_ ), url( url_ ), enabled( enabled_ ), iconFilename( iconFilename_ ),
    inside_iframe( inside_iframe_ ) {}

  bool operator == ( WebSite const & other ) const
  { return id == other.id && name == other.name && url == other.url &&
           enabled == other.enabled && iconFilename == other.iconFilename &&
           inside_iframe == other.inside_iframe; }
};

/// All the WebSites
typedef QVector< WebSite > WebSites;

/// Any DICT server
struct DictServer
{
  QString id, name, url;
  bool enabled;
  QString databases;
  QString strategies;
  QString iconFilename;

  DictServer(): enabled( false )
  {}

  DictServer( QString const & id_, QString const & name_, QString const & url_,
              bool enabled_, QString const & databases_, QString const & strategies_,
              QString const & iconFilename_ ):
    id( id_ ), name( name_ ), url( url_ ), enabled( enabled_ ), databases( databases_ ),
    strategies( strategies_ ), iconFilename( iconFilename_ )  {}

  bool operator == ( DictServer const & other ) const
  { return id == other.id && name == other.name && url == other.url
           && enabled == other.enabled && databases == other.databases
           && strategies == other.strategies
           && iconFilename == other.iconFilename; }
};

/// All the DictServers
typedef QVector< DictServer > DictServers;

/// Hunspell configuration
struct Hunspell
{
  QString dictionariesPath;

  typedef QVector< QString > Dictionaries;

  Dictionaries enabledDictionaries;

  bool operator == ( Hunspell const & other ) const
  { return dictionariesPath == other.dictionariesPath &&
    enabledDictionaries == other.enabledDictionaries; }

  bool operator != ( Hunspell const & other ) const
  { return ! operator == ( other ); }
};

/// All the MediaWikis
typedef QVector< MediaWiki > MediaWikis;

#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
/// Chinese transliteration configuration
struct Chinese
{
  bool enable;

  bool enableSCToTWConversion;
  bool enableSCToHKConversion;
  bool enableTCToSCConversion;

  Chinese();

  bool operator == ( Chinese const & other ) const
  { return enable == other.enable &&
           enableSCToTWConversion == other.enableSCToTWConversion &&
           enableSCToHKConversion == other.enableSCToHKConversion &&
           enableTCToSCConversion == other.enableTCToSCConversion; }

  bool operator != ( Chinese const & other ) const
  { return ! operator == ( other ); }

};
#endif

/// Romaji transliteration configuration
struct Romaji
{
  bool enable;

  bool enableHepburn;
  bool enableNihonShiki;
  bool enableKunreiShiki;
  bool enableHiragana;
  bool enableKatakana;

  Romaji();

  bool operator == ( Romaji const & other ) const
  { return enable == other.enable &&
           enableHepburn == other.enableHepburn &&
           enableNihonShiki == other.enableNihonShiki &&
           enableKunreiShiki == other.enableKunreiShiki &&
           enableHiragana == other.enableHiragana &&
           enableKatakana == other.enableKatakana; }

  bool operator != ( Romaji const & other ) const
  { return ! operator == ( other ); }

};

struct Transliteration
{
  bool enableRussianTransliteration;
  bool enableGermanTransliteration;
  bool enableGreekTransliteration;
  bool enableBelarusianTransliteration;
#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
  Chinese chinese;
#endif
  Romaji romaji;

  bool operator == ( Transliteration const & other ) const
  { return enableRussianTransliteration == other.enableRussianTransliteration &&
           enableGermanTransliteration == other.enableGermanTransliteration &&
           enableGreekTransliteration == other.enableGreekTransliteration &&
           enableBelarusianTransliteration == other.enableBelarusianTransliteration &&
#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
           chinese == other.chinese &&
#endif
           romaji == other.romaji;
  }

  bool operator != ( Transliteration const & other ) const
  { return ! operator == ( other ); }

  Transliteration():
      enableRussianTransliteration( false ),
      enableGermanTransliteration( false ),
      enableGreekTransliteration( false ),
      enableBelarusianTransliteration( false )
  {}
};

struct Forvo
{
  bool enable;
  QString apiKey;
  QString languageCodes;

  Forvo(): enable( false )
  {}

  bool operator == ( Forvo const & other ) const
  { return enable == other.enable &&
           apiKey == other.apiKey &&
           languageCodes == other.languageCodes;
  }

  bool operator != ( Forvo const & other ) const
  { return ! operator == ( other ); }
};

struct Program
{
  bool enabled;
  enum Type
  {
    Audio,
    PlainText,
    Html,
    PrefixMatch,
    MaxTypeValue
  } type;
  QString id, name, commandLine;
  QString iconFilename;

  Program(): enabled( false )
  {}

  Program( bool enabled_, Type type_, QString const & id_,
          QString const & name_, QString const & commandLine_, QString const & iconFilename_ ):
    enabled( enabled_ ), type( type_ ), id( id_ ), name( name_ ),
    commandLine( commandLine_ ), iconFilename( iconFilename_ ) {}

  bool operator == ( Program const & other ) const
  { return enabled == other.enabled &&
           type == other.type &&
           name == other.name &&
           commandLine == other.commandLine &&
           iconFilename == other.iconFilename;
  }

  bool operator != ( Program const & other ) const
  { return ! operator == ( other ); }
};

typedef QVector< Program > Programs;

struct VoiceEngine
{
  bool enabled;
  QString id;
  QString name;
  QString iconFilename;
  int volume; // 0-100 allowed
  int rate;   // 0-100 allowed

  VoiceEngine(): enabled( false )
    , volume( 50 )
    , rate( 50 )
  {}
  VoiceEngine( QString id_, QString name_, int volume_, int rate_ ):
    enabled( false )
    , id( id_ )
    , name( name_ )
    , volume( volume_ )
    , rate( rate_ )
  {}

  bool operator == ( VoiceEngine const & other ) const
  {
    return enabled == other.enabled &&
           id == other.id &&
           name == other.name &&
           iconFilename == other.iconFilename &&
           volume == other.volume &&
           rate == other.rate;
  }

  bool operator != ( VoiceEngine const & other ) const
    { return ! operator == ( other ); }
};

typedef QVector< VoiceEngine> VoiceEngines;

struct HeadwordsDialog
{
  int searchMode;
  bool matchCase;
  bool autoApply;
  QString headwordsExportPath;
  QByteArray headwordsDialogGeometry;

  HeadwordsDialog() :
    searchMode( 0 ), matchCase( false )
    , autoApply( false )
  {}
};

struct Class
{
  Paths paths;
  SoundDirs soundDirs;
  Group dictionaryOrder;
  Group inactiveDictionaries;
  Groups groups;
  Preferences preferences;
  MediaWikis mediawikis;
  WebSites webSites;
  DictServers dictServers;
  Hunspell hunspell;
  Transliteration transliteration;
  Forvo forvo;
  Programs programs;
  VoiceEngines voiceEngines;

  unsigned lastMainGroupId; // Last used group in main window
  unsigned lastPopupGroupId; // Last used group in popup window

  QByteArray popupWindowState; // Binary state saved by QMainWindow
  QByteArray popupWindowGeometry; // Geometry saved by QMainWindow
  QByteArray dictInfoGeometry; // Geometry of "Dictionary info" window
  QByteArray inspectorGeometry; // Geometry of WebKit inspector window
  QByteArray dictionariesDialogGeometry; // Geometry of Dictionaries dialog
  QByteArray printPreviewDialogGeometry; // Geometry of Print Preview dialog
  QByteArray helpWindowGeometry; // Geometry of help window
  QByteArray helpSplitterState; // Geometry of help splitter

  QString historyExportPath; // Path for export/import history
  QString resourceSavePath;  // Path to save images/audio
  QString articleSavePath;   // Path to save articles

  bool pinPopupWindow; // Last pin status
  bool popupWindowAlwaysOnTop; // Last status of pinned popup window

  QByteArray mainWindowState; // Binary state saved by QMainWindow
  QByteArray mainWindowGeometry; // Geometry saved by QMainWindow

  MutedDictionaries mutedDictionaries; // Disabled via dictionary bar
  MutedDictionaries popupMutedDictionaries; // Disabled via dictionary bar in popup

  QDateTime timeForNewReleaseCheck; // Only effective if
                                    // preferences.checkForNewReleases is set
  QString skippedRelease; // Empty by default

  bool showingDictBarNames;

  bool usingSmallIconsInToolbars;

  int maxPictureWidth; // Maximum picture width

  /// Maximum size for the headwords.
  /// Bigger headwords won't be indexed. For now, only in DSL.
  unsigned int maxHeadwordSize;

  unsigned int maxHeadwordsToExpand;

  HeadwordsDialog headwordsDialog;

#ifdef Q_OS_WIN
  QRect maximizedMainWindowGeometry;
  QRect normalMainWindowGeometry;
#endif

  QString editDictionaryCommandLine; // Command line to call external editor for dictionary

  Class(): lastMainGroupId( 0 ), lastPopupGroupId( 0 ),
           pinPopupWindow( false ), showingDictBarNames( false ),
           usingSmallIconsInToolbars( false ),
           maxPictureWidth( 0 ), maxHeadwordSize ( 256U ),
           maxHeadwordsToExpand( 0 )
  {}
  Group * getGroup( unsigned id );
  Group const * getGroup( unsigned id ) const;
};

#ifdef Q_OS_WIN
#pragma pack(pop)
#endif

/// Configuration-specific events. Some parts of the program need to react
/// to specific changes in configuration. The object of this class is used
/// to emit signals when such events happen -- and the listeners connect to
/// them to be notified of them.
/// This class is separate from the main Class since QObjects can't be copied.
class Events: public QObject
{
  Q_OBJECT

public:

  /// Signals that the value of the mutedDictionaries has changed.
  /// This emits mutedDictionariesChanged() signal, so the subscribers will
  /// be notified.
  void signalMutedDictionariesChanged();

signals:

  /// THe value of the mutedDictionaries has changed.
  void mutedDictionariesChanged();

private:
};

DEF_EX( exError, "Error with the program's configuration", std::exception )
DEF_EX( exCantUseDataDir, "Can't use XDG_DATA_HOME directory to store GoldenDict data", exError )
DEF_EX( exCantUseHomeDir, "Can't use home directory to store GoldenDict preferences", exError )
DEF_EX( exCantUseIndexDir, "Can't use index directory to store GoldenDict index files", exError )
DEF_EX( exCantReadConfigFile, "Can't read the configuration file", exError )
DEF_EX( exCantWriteConfigFile, "Can't write the configuration file", exError )
DEF_EX( exMalformedConfigFile, "The configuration file is malformed", exError )

/// Loads the configuration, or creates the default one if none is present
Class load() THROW_SPEC( exError );

/// Saves the configuration
void save( Class const & ) THROW_SPEC( exError );

/// Returns the configuration file name.
QString getConfigFileName();

/// Returns the main configuration directory.
QString getConfigDir() THROW_SPEC( exError );

/// Returns the index directory, where the indices are to be stored.
QString getIndexDir() THROW_SPEC( exError );

/// Returns the filename of a .pid file which should store current pid of
/// the process.
QString getPidFileName() THROW_SPEC( exError );

/// Returns the filename of a history file which stores search history.
QString getHistoryFileName() THROW_SPEC( exError );

/// Returns the filename of a favorities file.
QString getFavoritiesFileName() THROW_SPEC( exError );

/// Returns the user .css file name.
QString getUserCssFileName() THROW_SPEC( exError );

/// Returns the user .css file name used for printing only.
QString getUserCssPrintFileName() THROW_SPEC( exError );

/// Returns the user .css file name for the Qt interface customization.
QString getUserQtCssFileName() THROW_SPEC( exError );

/// Returns the program's data dir. Under Linux that would be something like
/// /usr/share/apps/goldendict, under Windows C:/Program Files/GoldenDict.
QString getProgramDataDir() throw();

/// Returns the directory storing program localizized files (.qm).
QString getLocDir() throw();

/// Returns the directory storing program help files (.qch).
QString getHelpDir() throw();

#ifdef MAKE_CHINESE_CONVERSION_SUPPORT
/// Returns the directory storing OpenCC configuration and dictionary files (.json and .ocd).
QString getOpenCCDir() throw();
#endif

#ifdef MAKE_ZIM_SUPPORT
/// Return the path of the MimeTex executable
QString getTexCgiPath() throw();
#endif

/// Returns true if the program is configured as a portable version. In that
/// mode, all the settings and indices are kept in the program's directory.
bool isPortableVersion() throw();

/// Returns directory with dictionaries for portable version. It is content/
/// in the application's directory.
QString getPortableVersionDictionaryDir() throw();

/// Returns directory with morpgologies for portable version. It is
/// content/morphology in the application's directory.
QString getPortableVersionMorphoDir() throw();

/// Returns the add-on styles directory.
QString getStylesDir() throw();

/// Returns the directory where user-specific non-essential (cached) data should be written.
QString getCacheDir() throw();

/// Returns the article network disk cache directory.
QString getNetworkCacheDir() throw();

}

Q_DECLARE_METATYPE( Config::InputPhrase )

#endif
