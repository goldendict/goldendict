/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "config.hh"
#include <QDir>
#include <QFile>
#include <QtXml>

#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

namespace Config {

namespace
{
  QDir getHomeDir()
  {
    QDir result = QDir::home();

    char const * pathInHome =
      #ifdef Q_OS_WIN32
      "Application Data/GoldenDict"
      #else
      ".goldendict"
      #endif
      ;

    result.mkpath( pathInHome );

    if ( !result.cd( pathInHome ) )
      throw exCantUseHomeDir();

    return result;
  }

  QString getConfigFileName()
  {
    return getHomeDir().absoluteFilePath( "config" );
  }
}

ProxyServer::ProxyServer(): enabled( false ), type( Socks5 ), port( 3128 )
{
}

HotKey::HotKey(): modifiers( 0 ), key1( 0 ), key2( 0 )
{
}

// Does anyone know how to separate modifiers from the keycode? We'll
// use our own mask.

uint32_t const keyMask = 0x01FFFFFF;

HotKey::HotKey( QKeySequence const & seq ):
  modifiers( seq[ 0 ] & ~keyMask ),
  key1( seq[ 0 ] & keyMask ),
  key2( seq[ 1 ] & keyMask )
{
}

QKeySequence HotKey::toKeySequence() const
{
  return QKeySequence( key1 | modifiers, key2 | modifiers );
}

Preferences::Preferences():
  newTabsOpenAfterCurrentOne( false ),
  newTabsOpenInBackground( true ),
  enableTrayIcon( true ),
  startToTray( false ),
  closeToTray( true ),
  autoStart( false ),

  enableMainWindowHotkey( true ),
  mainWindowHotkey( QKeySequence( "Ctrl+F11,F11" ) ),
  enableClipboardHotkey( true ),
  clipboardHotkey( QKeySequence( "Ctrl+Ins,Ins" ) ),

  enableScanPopup( true ),
  startWithScanPopupOn( false ),
  enableScanPopupModifiers( false ),
  scanPopupModifiers( 0 ),
  scanPopupAltMode( false ),
  scanPopupAltModeSecs( 3 ),
  pronounceOnLoadMain( false ),
  pronounceOnLoadPopup( false ),
  checkForNewReleases( true ),

  zoomFactor( 1 )
{
}

namespace {

MediaWikis makeDefaultMediaWikis( bool enable )
{
  MediaWikis mw;

  mw.push_back( MediaWiki( "ae6f89aac7151829681b85f035d54e48", "English Wikipedia", "http://en.wikipedia.org/w", enable ) );
  mw.push_back( MediaWiki( "affcf9678e7bfe701c9b071f97eccba3", "English Wiktionary", "http://en.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "8e0c1c2b6821dab8bdba8eb869ca7176", "Russian Wikipedia", "http://ru.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "b09947600ae3902654f8ad4567ae8567", "Russian Wiktionary", "http://ru.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "a8a66331a1242ca2aeb0b4aed361c41d", "German Wikipedia", "http://de.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "21c64bca5ec10ba17ff19f3066bc962a", "German Wiktionary", "http://de.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "96957cb2ad73a20c7a1d561fc83c253a", "Portuguese Wikipedia", "http://pt.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "ed4c3929196afdd93cc08b9a903aad6a", "Portuguese Wiktionary", "http://pt.wiktionary.org/w", false ) );

  return mw;
}

/// Sets option to true of false if node is "1" or "0" respectively, or leaves
/// it intact if it's neither "1" nor "0".
void applyBoolOption( bool & option, QDomNode const & node )
{
  QString value = node.toElement().text();

  if ( value == "1" )
    option = true;
  else
  if ( value == "0" )
    option = false;
}

}

Class load() throw( exError )
{
  QString configName  = getConfigFileName();

  bool loadFromTemplate = false;

  if ( !QFile::exists( configName ) )
  {
    // Make the default config, save it and return it
    Class c;

    #ifdef Q_OS_LINUX
    if ( QDir( "/usr/share/stardict/dic" ).exists() )
      c.paths.push_back( Path( "/usr/share/stardict/dic", true ) );

    if ( QDir( "/usr/share/dictd" ).exists() )
      c.paths.push_back( Path( "/usr/share/dictd", true ) );

    if ( QDir( "/usr/share/WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( "/usr/share/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );

    if ( QDir( "/usr/share/myspell/dicts" ).exists() )
      c.hunspell.dictionariesPath = "/usr/share/myspell/dicts";

    #endif

    #ifdef Q_OS_WIN32

    // #### "C:/Program Files" is bad! will not work for German Windows etc.
    // #### should be replaced to system path

    if ( QDir( "C:/Program Files/StarDict/dic" ).exists() )
      c.paths.push_back( Path( "C:/Program Files/StarDict/dic", true ) );

    if ( QDir( "C:/Program Files/StarDict/WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( "C:/Program Files/StarDict/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );
    else
    if ( QDir( "C:/Program Files/WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( "C:/Program Files/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );
    #endif

    #ifndef Q_OS_WIN32
    c.preferences.audioPlaybackProgram = "mplayer";
    #endif

    c.mediawikis = makeDefaultMediaWikis( true );

    // Check if we have a template config file. If we do, load it instead

    configName = getProgramDataDir() + "/content/defconfig";
    loadFromTemplate = QFile( configName ).exists();

    if ( !loadFromTemplate )
    {
      save( c );

      return c;
    }
  }

  QFile configFile( configName );

  if ( !configFile.open( QFile::ReadOnly ) )
    throw exCantReadConfigFile();

  QDomDocument dd;

  QString errorStr;
  int errorLine, errorColumn;

  if ( !loadFromTemplate )
  {
    // Load the config as usual
    if ( !dd.setContent( &configFile, false, &errorStr, &errorLine, &errorColumn  ) )
    {
      printf( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
        throw exMalformedConfigFile();
    }
  }
  else
  {
    // We need to replace all %PROGRAMDIR% with the program data dir
    QByteArray data = configFile.readAll();

    data.replace( "%PROGRAMDIR%", getProgramDataDir().toUtf8() );

    QBuffer bufferedData( &data );

    if ( !dd.setContent( &bufferedData, false, &errorStr, &errorLine, &errorColumn  ) )
    {
      printf( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
        throw exMalformedConfigFile();
    }
  }

  configFile.close();

  QDomNode root = dd.namedItem( "config" );

  Class c;

  QDomNode paths = root.namedItem( "paths" );

  if ( !paths.isNull() )
  {
    QDomNodeList nl = paths.toElement().elementsByTagName( "path" );

    for( unsigned x = 0; x < nl.length(); ++x )
      c.paths.push_back(
        Path( nl.item( x ).toElement().text(),
              nl.item( x ).toElement().attribute( "recursive" ) == "1" ) );
  }

  QDomNode soundDirs = root.namedItem( "sounddirs" );

  if ( !soundDirs.isNull() )
  {
    QDomNodeList nl = soundDirs.toElement().elementsByTagName( "sounddir" );

    for( unsigned x = 0; x < nl.length(); ++x )
      c.soundDirs.push_back(
        SoundDir( nl.item( x ).toElement().text(),
                  nl.item( x ).toElement().attribute( "name" ) ) );
  }

  QDomNode groups = root.namedItem( "groups" );

  if ( !groups.isNull() )
  {
    c.groups.nextId = groups.toElement().attribute( "nextId", "1" ).toUInt();

    QDomNodeList nl = groups.toElement().elementsByTagName( "group" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement grp = nl.item( x ).toElement();

      Group g;

      if ( grp.hasAttribute( "id" ) )
        g.id = grp.attribute( "id" ).toUInt();
      else
        g.id = c.groups.nextId++;

      g.name = grp.attribute( "name" );
      g.icon = grp.attribute( "icon" );

      QDomNodeList dicts = grp.elementsByTagName( "dictionary" );

      for( unsigned y = 0; y < dicts.length(); ++y )
        g.dictionaries.push_back( DictionaryRef( dicts.item( y ).toElement().text(),
                                                 dicts.item( y ).toElement().attribute( "name" ) ) );

      c.groups.push_back( g );
    }
  }

  QDomNode hunspell = root.namedItem( "hunspell" );

  if ( !hunspell.isNull() )
  {
    c.hunspell.dictionariesPath = hunspell.toElement().attribute( "dictionariesPath" );

    QDomNodeList nl = hunspell.toElement().elementsByTagName( "enabled" );

    for( unsigned x = 0; x < nl.length(); ++x )
      c.hunspell.enabledDictionaries.push_back( nl.item( x ).toElement().text() );
  }

  QDomNode mws = root.namedItem( "mediawikis" );

  if ( !mws.isNull() )
  {
    QDomNodeList nl = mws.toElement().elementsByTagName( "mediawiki" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement mw = nl.item( x ).toElement();

      MediaWiki w;

      w.id = mw.attribute( "id" );
      w.name = mw.attribute( "name" );
      w.url = mw.attribute( "url" );
      w.enabled = ( mw.attribute( "enabled" ) == "1" );

      c.mediawikis.push_back( w );
    }
  }
  else
  {
    // When upgrading, populate the list with some choices, but don't enable
    // anything.
    c.mediawikis = makeDefaultMediaWikis( false );
  }

  QDomNode preferences = root.namedItem( "preferences" );

  if ( !preferences.isNull() )
  {
    c.preferences.interfaceLanguage = preferences.namedItem( "interfaceLanguage" ).toElement().text();
    c.preferences.newTabsOpenAfterCurrentOne = ( preferences.namedItem( "newTabsOpenAfterCurrentOne" ).toElement().text() == "1" );
    c.preferences.newTabsOpenInBackground = ( preferences.namedItem( "newTabsOpenInBackground" ).toElement().text() == "1" );
    c.preferences.enableTrayIcon = ( preferences.namedItem( "enableTrayIcon" ).toElement().text() == "1" );
    c.preferences.startToTray = ( preferences.namedItem( "startToTray" ).toElement().text() == "1" );
    c.preferences.closeToTray = ( preferences.namedItem( "closeToTray" ).toElement().text() == "1" );
    c.preferences.autoStart = ( preferences.namedItem( "autoStart" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "zoomFactor" ).isNull() )
    {
      c.preferences.zoomFactor = preferences.namedItem( "zoomFactor" ).toElement().text().toDouble();
      if ( c.preferences.zoomFactor < 0.5 )
        c.preferences.zoomFactor = 0.5;
      else if ( c.preferences.zoomFactor > 5 )
        c.preferences.zoomFactor = 5;
    }

    applyBoolOption( c.preferences.enableMainWindowHotkey, preferences.namedItem( "enableMainWindowHotkey" ) );
    if ( !preferences.namedItem( "mainWindowHotkey" ).isNull() )
      c.preferences.mainWindowHotkey = QKeySequence::fromString( preferences.namedItem( "mainWindowHotkey" ).toElement().text() );
    applyBoolOption( c.preferences.enableClipboardHotkey, preferences.namedItem( "enableClipboardHotkey" ) );
    if ( !preferences.namedItem( "clipboardHotkey" ).isNull() )
      c.preferences.clipboardHotkey = QKeySequence::fromString( preferences.namedItem( "clipboardHotkey" ).toElement().text() );

    c.preferences.enableScanPopup = ( preferences.namedItem( "enableScanPopup" ).toElement().text() == "1" );
    c.preferences.startWithScanPopupOn = ( preferences.namedItem( "startWithScanPopupOn" ).toElement().text() == "1" );
    c.preferences.enableScanPopupModifiers = ( preferences.namedItem( "enableScanPopupModifiers" ).toElement().text() == "1" );
    c.preferences.scanPopupModifiers = ( preferences.namedItem( "scanPopupModifiers" ).toElement().text().toULong() );
    c.preferences.scanPopupAltMode = ( preferences.namedItem( "scanPopupAltMode" ).toElement().text() == "1" );
    if ( !preferences.namedItem( "scanPopupAltModeSecs" ).isNull() )
      c.preferences.scanPopupAltModeSecs = preferences.namedItem( "scanPopupAltModeSecs" ).toElement().text().toUInt();

    c.preferences.pronounceOnLoadMain = ( preferences.namedItem( "pronounceOnLoadMain" ).toElement().text() == "1" );
    c.preferences.pronounceOnLoadPopup = ( preferences.namedItem( "pronounceOnLoadPopup" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "audioPlaybackProgram" ).isNull() )
      c.preferences.audioPlaybackProgram = preferences.namedItem( "audioPlaybackProgram" ).toElement().text();
    else
      c.preferences.audioPlaybackProgram = "mplayer";

    QDomNode proxy = preferences.namedItem( "proxyserver" );

    if ( !proxy.isNull() )
    {
      c.preferences.proxyServer.enabled = ( proxy.toElement().attribute( "enabled" ) == "1" );
      c.preferences.proxyServer.type = ( ProxyServer::Type ) proxy.namedItem( "type" ).toElement().text().toULong();
      c.preferences.proxyServer.host = proxy.namedItem( "host" ).toElement().text();
      c.preferences.proxyServer.port = proxy.namedItem( "port" ).toElement().text().toULong();
      c.preferences.proxyServer.user = proxy.namedItem( "user" ).toElement().text();
      c.preferences.proxyServer.password = proxy.namedItem( "password" ).toElement().text();
    }

    if ( !preferences.namedItem( "checkForNewReleases" ).isNull() )
      c.preferences.checkForNewReleases = ( preferences.namedItem( "checkForNewReleases" ).toElement().text() == "1" );
  }

  c.lastMainGroupId = root.namedItem( "lastMainGroupId" ).toElement().text().toUInt();
  c.lastPopupGroupId = root.namedItem( "lastPopupGroupId" ).toElement().text().toUInt();

  QDomNode lastPopupWidth = root.namedItem( "lastPopupWidth" );
  QDomNode lastPopupHeight = root.namedItem( "lastPopupHeight" );

  if ( !lastPopupWidth.isNull() && !lastPopupHeight.isNull() )
  {
    c.lastPopupSize = QSize( lastPopupWidth.toElement().text().toULong(),
                             lastPopupHeight.toElement().text().toULong() );
  }

  QDomNode mainWindowState = root.namedItem( "mainWindowState" );

  if ( !mainWindowState.isNull() )
    c.mainWindowState = QByteArray::fromBase64( mainWindowState.toElement().text().toLatin1() );

  QDomNode mainWindowGeometry = root.namedItem( "mainWindowGeometry" );

  if ( !mainWindowGeometry.isNull() )
    c.mainWindowGeometry = QByteArray::fromBase64( mainWindowGeometry.toElement().text().toLatin1() );

  QDomNode timeForNewReleaseCheck = root.namedItem( "timeForNewReleaseCheck" );

  if ( !timeForNewReleaseCheck.isNull() )
    c.timeForNewReleaseCheck = QDateTime::fromString( timeForNewReleaseCheck.toElement().text(),
                                                      Qt::ISODate );

  return c;
}

void save( Class const & c ) throw( exError )
{
  QFile configFile( getConfigFileName() );

  if ( !configFile.open( QFile::WriteOnly ) )
    throw exCantWriteConfigFile();

  QDomDocument dd;

  QDomElement root = dd.createElement( "config" );
  dd.appendChild( root );

  {
    QDomElement paths = dd.createElement( "paths" );
    root.appendChild( paths );

    for( Paths::const_iterator i = c.paths.begin(); i != c.paths.end(); ++i )
    {
      QDomElement path = dd.createElement( "path" );
      paths.appendChild( path );

      QDomAttr recursive = dd.createAttribute( "recursive" );
      recursive.setValue( i->recursive ? "1" : "0" );
      path.setAttributeNode( recursive );

      QDomText value = dd.createTextNode( i->path );

      path.appendChild( value );
    }
  }

  {
    QDomElement soundDirs = dd.createElement( "sounddirs" );
    root.appendChild( soundDirs );

    for( SoundDirs::const_iterator i = c.soundDirs.begin(); i != c.soundDirs.end(); ++i )
    {
      QDomElement soundDir = dd.createElement( "sounddir" );
      soundDirs.appendChild( soundDir );

      QDomAttr name = dd.createAttribute( "name" );
      name.setValue( i->name );
      soundDir.setAttributeNode( name );

      QDomText value = dd.createTextNode( i->path );

      soundDir.appendChild( value );
    }
  }

  {
    QDomElement groups = dd.createElement( "groups" );
    root.appendChild( groups );

    QDomAttr nextId = dd.createAttribute( "nextId" );
    nextId.setValue( QString::number( c.groups.nextId ) );
    groups.setAttributeNode( nextId );

    for( Groups::const_iterator i = c.groups.begin(); i != c.groups.end(); ++i )
    {
      QDomElement group = dd.createElement( "group" );
      groups.appendChild( group );

      QDomAttr id = dd.createAttribute( "id" );

      id.setValue( QString::number( i->id ) );

      group.setAttributeNode( id );

      QDomAttr name = dd.createAttribute( "name" );

      name.setValue( i->name );

      group.setAttributeNode( name );

      if ( i->icon.size() )
      {
        QDomAttr icon = dd.createAttribute( "icon" );

        icon.setValue( i->icon );

        group.setAttributeNode( icon );
      }

      for( vector< DictionaryRef >::const_iterator j = i->dictionaries.begin(); j != i->dictionaries.end(); ++j )
      {
        QDomElement dictionary = dd.createElement( "dictionary" );

        group.appendChild( dictionary );

        QDomText value = dd.createTextNode( j->id );

        dictionary.appendChild( value );

        QDomAttr name = dd.createAttribute( "name" );

        name.setValue( j->name );

        dictionary.setAttributeNode( name );
      }
    }
  }

  {
    QDomElement hunspell = dd.createElement( "hunspell" );
    QDomAttr path = dd.createAttribute( "dictionariesPath" );
    path.setValue( c.hunspell.dictionariesPath );
    hunspell.setAttributeNode( path );
    root.appendChild( hunspell );

    for( unsigned x = 0; x < c.hunspell.enabledDictionaries.size(); ++x )
    {
      QDomElement en = dd.createElement( "enabled" );
      QDomText value = dd.createTextNode( c.hunspell.enabledDictionaries[ x ] );

      en.appendChild( value );
      hunspell.appendChild( en );
    }
  }

  {
    QDomElement mws = dd.createElement( "mediawikis" );
    root.appendChild( mws );

    for( MediaWikis::const_iterator i = c.mediawikis.begin(); i != c.mediawikis.end(); ++i )
    {
      QDomElement mw = dd.createElement( "mediawiki" );
      mws.appendChild( mw );

      QDomAttr id = dd.createAttribute( "id" );
      id.setValue( i->id );
      mw.setAttributeNode( id );

      QDomAttr name = dd.createAttribute( "name" );
      name.setValue( i->name );
      mw.setAttributeNode( name );

      QDomAttr url = dd.createAttribute( "url" );
      url.setValue( i->url );
      mw.setAttributeNode( url );

      QDomAttr enabled = dd.createAttribute( "enabled" );
      enabled.setValue( i->enabled ? "1" : "0" );
      mw.setAttributeNode( enabled );
    }
  }

  {
    QDomElement preferences = dd.createElement( "preferences" );
    root.appendChild( preferences );

    QDomElement opt = dd.createElement( "interfaceLanguage" );
    opt.appendChild( dd.createTextNode( c.preferences.interfaceLanguage ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "newTabsOpenAfterCurrentOne" );
    opt.appendChild( dd.createTextNode( c.preferences.newTabsOpenAfterCurrentOne ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "newTabsOpenInBackground" );
    opt.appendChild( dd.createTextNode( c.preferences.newTabsOpenInBackground ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableTrayIcon" );
    opt.appendChild( dd.createTextNode( c.preferences.enableTrayIcon ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "startToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.startToTray ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "closeToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.closeToTray ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "autoStart" );
    opt.appendChild( dd.createTextNode( c.preferences.autoStart ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "zoomFactor" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.zoomFactor ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableMainWindowHotkey" );
    opt.appendChild( dd.createTextNode( c.preferences.enableMainWindowHotkey ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "mainWindowHotkey" );
    opt.appendChild( dd.createTextNode( c.preferences.mainWindowHotkey.toKeySequence().toString() ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableClipboardHotkey" );
    opt.appendChild( dd.createTextNode( c.preferences.enableClipboardHotkey ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "clipboardHotkey" );
    opt.appendChild( dd.createTextNode( c.preferences.clipboardHotkey.toKeySequence().toString() ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableScanPopup" );
    opt.appendChild( dd.createTextNode( c.preferences.enableScanPopup ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "startWithScanPopupOn" );
    opt.appendChild( dd.createTextNode( c.preferences.startWithScanPopupOn ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableScanPopupModifiers" );
    opt.appendChild( dd.createTextNode( c.preferences.enableScanPopupModifiers ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupModifiers" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.scanPopupModifiers ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupAltMode" );
    opt.appendChild( dd.createTextNode( c.preferences.scanPopupAltMode ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupAltModeSecs" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.scanPopupAltModeSecs ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "pronounceOnLoadMain" );
    opt.appendChild( dd.createTextNode( c.preferences.pronounceOnLoadMain ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "pronounceOnLoadPopup" );
    opt.appendChild( dd.createTextNode( c.preferences.pronounceOnLoadPopup ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "audioPlaybackProgram" );
    opt.appendChild( dd.createTextNode( c.preferences.audioPlaybackProgram ) );
    preferences.appendChild( opt );

    {
      QDomElement proxy = dd.createElement( "proxyserver" );
      preferences.appendChild( proxy );

      QDomAttr enabled = dd.createAttribute( "enabled" );
      enabled.setValue( c.preferences.proxyServer.enabled ? "1" : "0" );
      proxy.setAttributeNode( enabled );

      opt = dd.createElement( "type" );
      opt.appendChild( dd.createTextNode( QString::number( c.preferences.proxyServer.type ) ) );
      proxy.appendChild( opt );

      opt = dd.createElement( "host" );
      opt.appendChild( dd.createTextNode( c.preferences.proxyServer.host ) );
      proxy.appendChild( opt );

      opt = dd.createElement( "port" );
      opt.appendChild( dd.createTextNode( QString::number( c.preferences.proxyServer.port ) ) );
      proxy.appendChild( opt );

      opt = dd.createElement( "user" );
      opt.appendChild( dd.createTextNode( c.preferences.proxyServer.user ) );
      proxy.appendChild( opt );

      opt = dd.createElement( "password" );
      opt.appendChild( dd.createTextNode( c.preferences.proxyServer.password ) );
      proxy.appendChild( opt );
    }

    opt = dd.createElement( "checkForNewReleases" );
    opt.appendChild( dd.createTextNode( c.preferences.checkForNewReleases ? "1" : "0" ) );
    preferences.appendChild( opt );
  }

  {
    QDomElement opt = dd.createElement( "lastMainGroupId" );
    opt.appendChild( dd.createTextNode( QString::number( c.lastMainGroupId ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "lastPopupGroupId" );
    opt.appendChild( dd.createTextNode( QString::number( c.lastPopupGroupId ) ) );
    root.appendChild( opt );

    if ( c.lastPopupSize.isValid() )
    {
      opt = dd.createElement( "lastPopupWidth" );
      opt.appendChild( dd.createTextNode( QString::number( c.lastPopupSize.width() ) ) );
      root.appendChild( opt );

      opt = dd.createElement( "lastPopupHeight" );
      opt.appendChild( dd.createTextNode( QString::number( c.lastPopupSize.height() ) ) );
      root.appendChild( opt );
    }

    opt = dd.createElement( "mainWindowState" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.mainWindowState.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "mainWindowGeometry" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.mainWindowGeometry.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "timeForNewReleaseCheck" );
    opt.appendChild( dd.createTextNode( c.timeForNewReleaseCheck.toString( Qt::ISODate ) ) );
    root.appendChild( opt );
  }

  configFile.write( dd.toByteArray() );
}

QString getIndexDir() throw( exError )
{
  QDir result = getHomeDir();

  result.mkpath( "index" );

  if ( !result.cd( "index" ) )
    throw exCantUseIndexDir();

  return result.path() + QDir::separator();
}

QString getUserCssFileName() throw( exError )
{
  return getHomeDir().filePath( "article-style.css" );
}

QString getUserQtCssFileName() throw( exError )
{
  return getHomeDir().filePath( "qt-style.css" );
}

QString getProgramDataDir() throw()
{
  #ifdef PROGRAM_DATA_DIR
  return PROGRAM_DATA_DIR;
  #else
  return QCoreApplication::applicationDirPath();
  #endif
}

}
