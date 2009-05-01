/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CONFIG_HH_INCLUDED__
#define __CONFIG_HH_INCLUDED__

#include <vector>
#include <QString>
#include <QSize>
#include <QDateTime>
#include <QKeySequence>
#include "ex.hh"

/// GoldenDict's configuration
namespace Config {

using std::vector;

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
typedef vector< Path > Paths;

/// A directory holding bunches of audiofiles, which is indexed into a separate
/// dictionary.
struct SoundDir
{
  QString path, name;

  SoundDir()
  {}

  SoundDir( QString const & path_, QString const & name_ ):
    path( path_ ), name( name_ )
  {}

  bool operator == ( SoundDir const & other ) const
  { return path == other.path && name == other.name; }
};

/// A list of SoundDirs
typedef vector< SoundDir > SoundDirs;

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
  vector< DictionaryRef > dictionaries;

  Group(): id( 0 ) {}

  bool operator == ( Group const & other ) const
  { return id == other.id && name == other.name && icon == other.icon &&
           dictionaries == other.dictionaries; }
};

/// All the groups
struct Groups: public vector< Group >
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
  bool newTabsOpenAfterCurrentOne;
  bool newTabsOpenInBackground;
  bool enableTrayIcon;
  bool startToTray;
  bool closeToTray;
  bool autoStart;

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

  // Whether the word should be pronounced on page load, in main window/popup
  bool pronounceOnLoadMain, pronounceOnLoadPopup;
  QString audioPlaybackProgram;

  ProxyServer proxyServer;

  bool checkForNewReleases;

  qreal zoomFactor;

  Preferences();
};

/// A MediaWiki network dictionary definition
struct MediaWiki
{
  QString id, name, url;
  bool enabled;

  MediaWiki(): enabled( false )
  {}

  MediaWiki( QString const & id_, QString const & name_, QString const & url_,
             bool enabled_ ):
    id( id_ ), name( name_ ), url( url_ ), enabled( enabled_ ) {}

  bool operator == ( MediaWiki const & other ) const
  { return id == other.id && name == other.name && url == other.url &&
           enabled == other.enabled; }
};

/// Hunspell configuration
struct Hunspell
{
  QString dictionariesPath;

  typedef vector< QString > Dictionaries;

  Dictionaries enabledDictionaries;

  bool operator == ( Hunspell const & other ) const
  { return dictionariesPath == other.dictionariesPath &&
    enabledDictionaries == other.enabledDictionaries; }

  bool operator != ( Hunspell const & other ) const
  { return ! operator == ( other ); }
};

/// All the MediaWikis
typedef vector< MediaWiki > MediaWikis;

struct Class
{
  Paths paths;
  SoundDirs soundDirs;
  Groups groups;
  Preferences preferences;
  MediaWikis mediawikis;
  Hunspell hunspell;

  unsigned lastMainGroupId; // Last used group in main window
  unsigned lastPopupGroupId; // Last used group in popup window
  QSize lastPopupSize;

  QByteArray mainWindowState; // Binary state saved by QMainWindow
  QByteArray mainWindowGeometry; // Geometry saved by QMainWindow

  QDateTime timeForNewReleaseCheck; // Only effective if
                                    // preferences.checkForNewReleases is set

  Class(): lastMainGroupId( 0 ), lastPopupGroupId( 0 )
  {}
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

/// Returns the index directory, where the indices are to be stored.
QString getIndexDir() throw( exError );

/// Returns the user .css file name.
QString getUserCssFileName() throw( exError );

/// Returns the user .css file name used for printing only.
QString getUserCssPrintFileName() throw( exError );

/// Returns the user .css file name for the Qt interface customization.
QString getUserQtCssFileName() throw( exError );

/// Returns the program's data dir. Under Linux that would be something like
/// /usr/share/apps/goldendict, under Windows C:/Program Files/GoldenDict.
QString getProgramDataDir() throw();

}

#endif

