/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CONFIG_HH_INCLUDED__
#define __CONFIG_HH_INCLUDED__

#include <QVector>
#include <QString>
#include <QSize>
#include <QDateTime>
#include <QKeySequence>
#include <QSet>
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
  QVector< DictionaryRef > dictionaries;
  Config::MutedDictionaries mutedDictionaries; // Disabled via dictionary bar
  Config::MutedDictionaries popupMutedDictionaries; // Disabled via dictionary bar in popup

  Group(): id( 0 ) {}

  bool operator == ( Group const & other ) const
  { return id == other.id && name == other.name && icon == other.icon &&
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

  enum Type
  {
    Socks5 = 0,
    HttpConnect,
    HttpGet
  } type;

  QString host;
  unsigned port;
  QString user, password;

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

/// Various user preferences
struct Preferences
{
  QString interfaceLanguage; // Empty value corresponds to system default
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
  bool scanPopupUseUIAutomation;
  bool scanPopupUseIAccessibleEx;
  bool scanPopupUseGDMessage;
  bool scanToMainWindow;

  // Whether the word should be pronounced on page load, in main window/popup
  bool pronounceOnLoadMain, pronounceOnLoadPopup;
  QString audioPlaybackProgram;
  bool useExternalPlayer;
  bool useInternalPlayer;

  ProxyServer proxyServer;

  bool checkForNewReleases;
  bool disallowContentFromOtherSites;
  bool enableWebPlugins;
  bool hideGoldenDictHeader;

  qreal zoomFactor;
  int wordsZoomLevel;

  unsigned maxStringsInHistory;
  unsigned storeHistory;
  bool alwaysExpandOptionalParts;

  unsigned historyStoreInterval;

  bool collapseBigArticles;
  int articleSizeLimit;

  unsigned short maxDictionaryRefsInContextMenu;

  QString addonStyle;

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

  WebSite(): enabled( false )
  {}

  WebSite( QString const & id_, QString const & name_, QString const & url_,
           bool enabled_, QString const & iconFilename_ ):
    id( id_ ), name( name_ ), url( url_ ), enabled( enabled_ ), iconFilename( iconFilename_ ) {}

  bool operator == ( WebSite const & other ) const
  { return id == other.id && name == other.name && url == other.url &&
           enabled == other.enabled && iconFilename == other.iconFilename; }
};

/// All the WebSites
typedef QVector< WebSite > WebSites;

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
  Romaji romaji;

  bool operator == ( Transliteration const & other ) const
  { return enableRussianTransliteration == other.enableRussianTransliteration &&
           romaji == other.romaji &&
           enableGermanTransliteration == other.enableGermanTransliteration &&
           enableGreekTransliteration == other.enableGreekTransliteration &&
           enableBelarusianTransliteration == other.enableBelarusianTransliteration;
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

  QString historyExportPath; // Path for export/import history
  QString resourceSavePath;  // Path to save images/audio
  QString articleSavePath;   // Path to save articles

  bool pinPopupWindow; // Last pin status

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

#ifdef Q_OS_WIN
  QRect maximizedMainWindowGeometry;
#endif

  QString editDictionaryCommandLine; // Command line to call external editor for dictionary

  Class(): lastMainGroupId( 0 ), lastPopupGroupId( 0 ),
           pinPopupWindow( false ), showingDictBarNames( false ),
           usingSmallIconsInToolbars( false ),
           maxPictureWidth( 0 ), maxHeadwordSize ( 256U )
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
DEF_EX( exCantUseHomeDir, "Can't use home directory to store GoldenDict preferences", exError )
DEF_EX( exCantUseIndexDir, "Can't use index directory to store GoldenDict index files", exError )
DEF_EX( exCantReadConfigFile, "Can't read the configuration file", exError )
DEF_EX( exCantWriteConfigFile, "Can't write the configuration file", exError )
DEF_EX( exMalformedConfigFile, "The configuration file is malformed", exError )

/// Loads the configuration, or creates the default one if none is present
Class load() throw( exError );

/// Saves the configuration
void save( Class const & ) throw( exError );

/// Returns the configuration file name.
QString getConfigFileName();

/// Returns the main configuration directory.
QString getConfigDir() throw( exError );

/// Returns the index directory, where the indices are to be stored.
QString getIndexDir() throw( exError );

/// Returns the filename of a .pid file which should store current pid of
/// the process.
QString getPidFileName() throw( exError );

/// Returns the filename of a history file which stores search history.
QString getHistoryFileName() throw( exError );

/// Returns the user .css file name.
QString getUserCssFileName() throw( exError );

/// Returns the user .css file name used for printing only.
QString getUserCssPrintFileName() throw( exError );

/// Returns the user .css file name for the Qt interface customization.
QString getUserQtCssFileName() throw( exError );

/// Returns the program's data dir. Under Linux that would be something like
/// /usr/share/apps/goldendict, under Windows C:/Program Files/GoldenDict.
QString getProgramDataDir() throw();

/// Returns the directory storing program localizized files (.qm).
QString getLocDir() throw();

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

}

#endif

