/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "config.hh"
#include <QDir>
#include <QFile>
#include <QtXml>
#include "dprintf.hh"

#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

#ifdef Q_OS_WIN32
#include "shlobj.h"
#endif

#include "atomic_rename.hh"

namespace Config {

namespace
{
  QDir getHomeDir()
  {
    if ( isPortableVersion() )
      return QDir( QCoreApplication::applicationDirPath() + "/portable" );

    QDir result;

    result = QDir::home();
    #ifdef Q_OS_WIN32
      if ( result.cd( "Application Data/GoldenDict" ) )
        return result;
      char const * pathInHome = "GoldenDict";
      result = QDir::fromNativeSeparators( QString::fromWCharArray( _wgetenv( L"APPDATA" ) ) );
    #else
      char const * pathInHome = ".goldendict";
    #endif

    result.mkpath( pathInHome );

    if ( !result.cd( pathInHome ) )
      throw exCantUseHomeDir();

    return result;
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
  int v2 = key2 ? ( key2 | modifiers ): 0;

  return QKeySequence( key1 | modifiers, v2 );
}

Preferences::Preferences():
  newTabsOpenAfterCurrentOne( false ),
  newTabsOpenInBackground( true ),
  hideSingleTab( false ),
  mruTabOrder ( false ),
  hideMenubar( false ),
  enableTrayIcon( true ),
  startToTray( false ),
  closeToTray( true ),
  autoStart( false ),
  doubleClickTranslates( true ),
  selectWordBySingleClick( false ),
  escKeyHidesMainWindow( false ),
  alwaysOnTop ( false ),
  searchInDock ( false ),

  enableMainWindowHotkey( true ),
  mainWindowHotkey( QKeySequence( "Ctrl+F11,F11" ) ),
  enableClipboardHotkey( true ),
  clipboardHotkey( QKeySequence( "Ctrl+C,C" ) ),

  enableScanPopup( true ),
  startWithScanPopupOn( false ),
  enableScanPopupModifiers( false ),
  scanPopupModifiers( 0 ),
  scanPopupAltMode( false ),
  scanPopupAltModeSecs( 3 ),
  scanPopupUseUIAutomation( true ),
  scanPopupUseIAccessibleEx( true ),
  scanPopupUseGDMessage( true ),
  scanToMainWindow( false ),
  pronounceOnLoadMain( false ),
  pronounceOnLoadPopup( false ),
#ifndef DISABLE_INTERNAL_PLAYER
  useExternalPlayer( false ),
  useInternalPlayer( true ),
#else
  useExternalPlayer( true ),
  useInternalPlayer( false ),
#endif
  checkForNewReleases( true ),
  disallowContentFromOtherSites( false ),
  enableWebPlugins( false ),
  hideGoldenDictHeader( false ),
  zoomFactor( 1 ),
  wordsZoomLevel( 0 ),
  maxStringsInHistory( 500 ),
  storeHistory( 1 ),
  alwaysExpandOptionalParts( true )
, historyStoreInterval( 0 )
, collapseBigArticles( false )
, articleSizeLimit( 2000 )
, maxDictionaryRefsInContextMenu ( 20 )
{
}

Romaji::Romaji():
  enable( false ),
  enableHepburn( true ),
  enableNihonShiki( false ),
  enableKunreiShiki( false ),
  enableHiragana( true ),
  enableKatakana( true )
{
}

Group * Class::getGroup( unsigned id )
{
  for( int x = 0; x < groups.size(); x++ )
    if( groups.at( x ).id == id )
      return &groups[ x ];
  return 0;
}

Group const * Class::getGroup( unsigned id ) const
{
  for( int x = 0; x < groups.size(); x++ )
    if( groups.at( x ).id == id )
      return &groups.at( x );
  return 0;
}

void Events::signalMutedDictionariesChanged()
{
  emit mutedDictionariesChanged();
}

namespace {

MediaWikis makeDefaultMediaWikis( bool enable )
{
  MediaWikis mw;

  mw.push_back( MediaWiki( "ae6f89aac7151829681b85f035d54e48", "English Wikipedia", "http://en.wikipedia.org/w", enable, "" ) );
  mw.push_back( MediaWiki( "affcf9678e7bfe701c9b071f97eccba3", "English Wiktionary", "http://en.wiktionary.org/w", false, ""  ) );
  mw.push_back( MediaWiki( "8e0c1c2b6821dab8bdba8eb869ca7176", "Russian Wikipedia", "http://ru.wikipedia.org/w", false, "" ) );
  mw.push_back( MediaWiki( "b09947600ae3902654f8ad4567ae8567", "Russian Wiktionary", "http://ru.wiktionary.org/w", false, "" ) );
  mw.push_back( MediaWiki( "a8a66331a1242ca2aeb0b4aed361c41d", "German Wikipedia", "http://de.wikipedia.org/w", false, "" ) );
  mw.push_back( MediaWiki( "21c64bca5ec10ba17ff19f3066bc962a", "German Wiktionary", "http://de.wiktionary.org/w", false, "" ) );
  mw.push_back( MediaWiki( "96957cb2ad73a20c7a1d561fc83c253a", "Portuguese Wikipedia", "http://pt.wikipedia.org/w", false, "" ) );
  mw.push_back( MediaWiki( "ed4c3929196afdd93cc08b9a903aad6a", "Portuguese Wiktionary", "http://pt.wiktionary.org/w", false, "" ) );
  mw.push_back( MediaWiki( "f3b4ec8531e52ddf5b10d21e4577a7a2", "Greek Wikipedia", "http://el.wikipedia.org/w", false, "" ) );
  mw.push_back( MediaWiki( "5d45232075d06e002dea72fe3e137da1", "Greek Wiktionary", "http://el.wiktionary.org/w", false, "" ) );

  return mw;
}

WebSites makeDefaultWebSites()
{
  WebSites ws;

  ws.push_back( WebSite( "b88cb2898e634c6638df618528284c2d", "Google En-En (Oxford)", "http://www.google.com/dictionary?aq=f&langpair=en|en&q=%GDWORD%&hl=en", false, "" ) );
  ws.push_back( WebSite( "f376365a0de651fd7505e7e5e683aa45", "Urban Dictionary", "http://www.urbandictionary.com/define.php?term=%GDWORD%", false, "" ) );
  ws.push_back( WebSite( "324ca0306187df7511b26d3847f4b07c", "Multitran (En)", "http://multitran.ru/c/m.exe?CL=1&l1=1&s=%GD1251%", false, "" ) );
  ws.push_back( WebSite( "924db471b105299c82892067c0f10787", "Lingvo (En-Ru)", "http://lingvopro.abbyyonline.com/en/Search/en-ru/%GDWORD%", false, "" ) );
  ws.push_back( WebSite( "087a6d65615fb047f4c80eef0a9465db", "Michaelis (Pt-En)", "http://michaelis.uol.com.br/moderno/ingles/index.php?lingua=portugues-ingles&palavra=%GDISO1%", false, "" ) );

  return ws;
}

Programs makeDefaultPrograms()
{
  Programs programs;

  // The following list doesn't make a lot of sense under Windows
#ifndef Q_WS_WIN
  programs.push_back( Program( false, Program::Audio, "428b4c2b905ef568a43d9a16f59559b0", "Festival", "festival --tts", "" ) );
  programs.push_back( Program( false, Program::Audio, "2cf8b3a60f27e1ac812de0b57c148340", "Espeak", "espeak %GDWORD%", "" ) );
  programs.push_back( Program( false, Program::Html, "4f898f7582596cea518c6b0bfdceb8b3", "Manpages", "man -a --html=/bin/cat %GDWORD%", "" ) );
#endif

  return programs;
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

Group loadGroup( QDomElement grp, unsigned * nextId = 0 )
{
  Group g;

  if ( grp.hasAttribute( "id" ) )
    g.id = grp.attribute( "id" ).toUInt();
  else
    g.id = nextId ? (*nextId)++ : 0;

  g.name = grp.attribute( "name" );
  g.icon = grp.attribute( "icon" );

  if ( !grp.attribute( "iconData" ).isEmpty() )
    g.iconData = QByteArray::fromBase64( grp.attribute( "iconData" ).toLatin1() );

  if ( !grp.attribute( "shortcut" ).isEmpty() )
    g.shortcut = QKeySequence::fromString( grp.attribute( "shortcut" ) );

  QDomNodeList dicts = grp.elementsByTagName( "dictionary" );

  for( unsigned y = 0; y < dicts.length(); ++y )
    g.dictionaries.push_back( DictionaryRef( dicts.item( y ).toElement().text(),
                                             dicts.item( y ).toElement().attribute( "name" ) ) );

  QDomNode muted = grp.namedItem( "mutedDictionaries" );
  dicts = muted.toElement().elementsByTagName( "mutedDictionary" );
  for( unsigned x = 0; x < dicts.length(); ++x )
    g.mutedDictionaries.insert( dicts.item( x ).toElement().text() );

  dicts = muted.toElement().elementsByTagName( "popupMutedDictionary" );
  for( unsigned x = 0; x < dicts.length(); ++x )
    g.popupMutedDictionaries.insert( dicts.item( x ).toElement().text() );

  return g;
}

MutedDictionaries loadMutedDictionaries( QDomNode mutedDictionaries )
{
  MutedDictionaries result;

  if ( !mutedDictionaries.isNull() )
  {
    QDomNodeList nl = mutedDictionaries.toElement().
                        elementsByTagName( "mutedDictionary" );

    for( unsigned x = 0; x < nl.length(); ++x )
      result.insert( nl.item( x ).toElement().text() );
  }

  return result;
}

void saveMutedDictionaries( QDomDocument & dd, QDomElement & muted,
                            MutedDictionaries const & mutedDictionaries )
{
  for( MutedDictionaries::const_iterator i = mutedDictionaries.begin();
       i != mutedDictionaries.end(); ++i )
  {
    QDomElement dict = dd.createElement( "mutedDictionary" );
    muted.appendChild( dict );

    QDomText value = dd.createTextNode( *i );
    dict.appendChild( value );
  }
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

    if ( QDir( "/usr/share/opendict/dictionaries" ).exists() )
      c.paths.push_back( Path( "/usr/share/opendict/dictionaries", true ) );

    if ( QDir( "/usr/share/goldendict-wordnet" ).exists() )
      c.paths.push_back( Path( "/usr/share/goldendict-wordnet", true ) );

    if ( QDir( "/usr/share/WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( "/usr/share/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );

    if ( QDir( "/usr/share/myspell/dicts" ).exists() )
      c.hunspell.dictionariesPath = "/usr/share/myspell/dicts";

    #endif

    #ifdef Q_OS_WIN32

    // get path to Program Files
    wchar_t buf[ MAX_PATH ];
    SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, 0, buf );
    QString pathToProgramFiles = QString::fromWCharArray( buf );
    if ( pathToProgramFiles.isEmpty() )
      pathToProgramFiles = "C:\\Program Files";

    if ( QDir( pathToProgramFiles + "\\StarDict\\dic" ).exists() )
      c.paths.push_back( Path( pathToProgramFiles + "\\StarDict\\dic", true ) );

    if ( QDir( pathToProgramFiles + "\\StarDict\\WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( pathToProgramFiles + "\\StarDict\\WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );
    else
    if ( QDir( pathToProgramFiles + "\\WyabdcRealPeopleTTS" ).exists() )
      c.soundDirs.push_back( SoundDir( pathToProgramFiles + "\\WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );

    // #### "C:/Program Files" is bad! will not work for German Windows etc.
    // #### should be replaced to system path

//    if ( QDir( "C:/Program Files/StarDict/dic" ).exists() )
//      c.paths.push_back( Path( "C:/Program Files/StarDict/dic", true ) );
//
//    if ( QDir( "C:/Program Files/StarDict/WyabdcRealPeopleTTS" ).exists() )
//      c.soundDirs.push_back( SoundDir( "C:/Program Files/StarDict/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );
//    else
//    if ( QDir( "C:/Program Files/WyabdcRealPeopleTTS" ).exists() )
//      c.soundDirs.push_back( SoundDir( "C:/Program Files/WyabdcRealPeopleTTS", "WyabdcRealPeopleTTS" ) );

    #endif

    #ifndef Q_OS_WIN32
    c.preferences.audioPlaybackProgram = "mplayer";
    #endif

    QString possibleMorphologyPath = getProgramDataDir() + "/content/morphology";

    if ( QDir( possibleMorphologyPath ).exists() )
      c.hunspell.dictionariesPath = possibleMorphologyPath;

    c.mediawikis = makeDefaultMediaWikis( true );
    c.webSites = makeDefaultWebSites();

    // Check if we have a template config file. If we do, load it instead

    configName = getProgramDataDir() + "/content/defconfig";
    loadFromTemplate = QFile( configName ).exists();

    if ( !loadFromTemplate )
    {
      save( c );

      return c;
    }
  }

  getStylesDir();

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
      DPRINTF( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
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
      DPRINTF( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
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
                  nl.item( x ).toElement().attribute( "name" ),
                  nl.item( x ).toElement().attribute( "icon" ) ) );
  }

  QDomNode dictionaryOrder = root.namedItem( "dictionaryOrder" );

  if ( !dictionaryOrder.isNull() )
    c.dictionaryOrder = loadGroup( dictionaryOrder.toElement() );

  QDomNode inactiveDictionaries = root.namedItem( "inactiveDictionaries" );

  if ( !inactiveDictionaries.isNull() )
    c.inactiveDictionaries = loadGroup( inactiveDictionaries.toElement() );

  QDomNode groups = root.namedItem( "groups" );

  if ( !groups.isNull() )
  {
    c.groups.nextId = groups.toElement().attribute( "nextId", "1" ).toUInt();

    QDomNodeList nl = groups.toElement().elementsByTagName( "group" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement grp = nl.item( x ).toElement();

      c.groups.push_back( loadGroup( grp, &c.groups.nextId ) );
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

  QDomNode transliteration = root.namedItem( "transliteration" );

  if ( !transliteration.isNull() )
  {
    applyBoolOption( c.transliteration.enableRussianTransliteration,
                     transliteration.namedItem( "enableRussianTransliteration" ) );

    applyBoolOption( c.transliteration.enableGermanTransliteration,
                     transliteration.namedItem( "enableGermanTransliteration" ) );

    applyBoolOption( c.transliteration.enableGreekTransliteration,
                     transliteration.namedItem( "enableGreekTransliteration" ) );

    applyBoolOption( c.transliteration.enableBelarusianTransliteration,
                     transliteration.namedItem( "enableBelarusianTransliteration" ) );

    QDomNode romaji = transliteration.namedItem( "romaji" );

    if ( !romaji.isNull() )
    {
      applyBoolOption( c.transliteration.romaji.enable, romaji.namedItem( "enable" ) );
      applyBoolOption( c.transliteration.romaji.enableHepburn, romaji.namedItem( "enableHepburn" ) );
      applyBoolOption( c.transliteration.romaji.enableNihonShiki, romaji.namedItem( "enableNihonShiki" ) );
      applyBoolOption( c.transliteration.romaji.enableKunreiShiki, romaji.namedItem( "enableKunreiShiki" ) );
      applyBoolOption( c.transliteration.romaji.enableHiragana, romaji.namedItem( "enableHiragana" ) );
      applyBoolOption( c.transliteration.romaji.enableKatakana, romaji.namedItem( "enableKatakana" ) );
    }
  }

  QDomNode forvo = root.namedItem( "forvo" );

  if ( !forvo.isNull() )
  {
    applyBoolOption( c.forvo.enable,
                     forvo.namedItem( "enable" ) );

    c.forvo.apiKey = forvo.namedItem( "apiKey" ).toElement().text();
    c.forvo.languageCodes = forvo.namedItem( "languageCodes" ).toElement().text();
  }
  else
    c.forvo.languageCodes = "en, ru"; // Default demo values

  QDomNode programs = root.namedItem( "programs" );

  if ( !programs.isNull() )
  {
    QDomNodeList nl = programs.toElement().elementsByTagName( "program" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement pr = nl.item( x ).toElement();

      Program p;

      p.id = pr.attribute( "id" );
      p.name = pr.attribute( "name" );
      p.commandLine = pr.attribute( "commandLine" );
      p.enabled = ( pr.attribute( "enabled" ) == "1" );
      p.type = (Program::Type)( pr.attribute( "type" ).toInt() );
      p.iconFilename = pr.attribute( "icon" );

      c.programs.push_back( p );
    }
  }
  else
    c.programs = makeDefaultPrograms();

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
      w.icon = mw.attribute( "icon" );

      c.mediawikis.push_back( w );
    }
  }
  else
  {
    // When upgrading, populate the list with some choices, but don't enable
    // anything.
    c.mediawikis = makeDefaultMediaWikis( false );
  }

  QDomNode wss = root.namedItem( "websites" );

  if ( !wss.isNull() )
  {
    QDomNodeList nl = wss.toElement().elementsByTagName( "website" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement ws = nl.item( x ).toElement();

      WebSite w;

      w.id = ws.attribute( "id" );
      w.name = ws.attribute( "name" );
      w.url = ws.attribute( "url" );
      w.enabled = ( ws.attribute( "enabled" ) == "1" );
      w.iconFilename = ws.attribute( "icon" );

      c.webSites.push_back( w );
    }
  }
  else
  {
    // Upgrading
    c.webSites = makeDefaultWebSites();
  }

  QDomNode ves = root.namedItem( "voiceEngines" );

  if ( !ves.isNull() )
  {
    QDomNodeList nl = ves.toElement().elementsByTagName( "voiceEngine" );

    for ( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement ve = nl.item( x ).toElement();
      VoiceEngine v;

      v.enabled = ve.attribute( "enabled" ) == "1";
      v.id = ve.attribute( "id" );
      v.name = ve.attribute( "name" );
      v.iconFilename = ve.attribute( "icon" );
      v.volume = ve.attribute( "volume", "50" ).toInt();
      if( v.volume < 0 || v.volume > 100 )
        v.volume = 50;
      v.rate = ve.attribute( "rate", "50" ).toInt();
      if( v.rate < 0 || v.rate > 100 )
        v.rate = 50;
      c.voiceEngines.push_back( v );
    }
  }

  c.mutedDictionaries = loadMutedDictionaries( root.namedItem( "mutedDictionaries" ) );
  c.popupMutedDictionaries = loadMutedDictionaries( root.namedItem( "popupMutedDictionaries" ) );

  QDomNode preferences = root.namedItem( "preferences" );

  if ( !preferences.isNull() )
  {
    c.preferences.interfaceLanguage = preferences.namedItem( "interfaceLanguage" ).toElement().text();
    c.preferences.displayStyle = preferences.namedItem( "displayStyle" ).toElement().text();
    c.preferences.newTabsOpenAfterCurrentOne = ( preferences.namedItem( "newTabsOpenAfterCurrentOne" ).toElement().text() == "1" );
    c.preferences.newTabsOpenInBackground = ( preferences.namedItem( "newTabsOpenInBackground" ).toElement().text() == "1" );
    c.preferences.hideSingleTab = ( preferences.namedItem( "hideSingleTab" ).toElement().text() == "1" );
    c.preferences.mruTabOrder = ( preferences.namedItem( "mruTabOrder" ).toElement().text() == "1" );
    c.preferences.hideMenubar = ( preferences.namedItem( "hideMenubar" ).toElement().text() == "1" );
    c.preferences.enableTrayIcon = ( preferences.namedItem( "enableTrayIcon" ).toElement().text() == "1" );
    c.preferences.startToTray = ( preferences.namedItem( "startToTray" ).toElement().text() == "1" );
    c.preferences.closeToTray = ( preferences.namedItem( "closeToTray" ).toElement().text() == "1" );
    c.preferences.autoStart = ( preferences.namedItem( "autoStart" ).toElement().text() == "1" );
    c.preferences.alwaysOnTop = ( preferences.namedItem( "alwaysOnTop" ).toElement().text() == "1" );
    c.preferences.searchInDock = ( preferences.namedItem( "searchInDock" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "doubleClickTranslates" ).isNull() )
      c.preferences.doubleClickTranslates = ( preferences.namedItem( "doubleClickTranslates" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "selectWordBySingleClick" ).isNull() )
      c.preferences.selectWordBySingleClick = ( preferences.namedItem( "selectWordBySingleClick" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "escKeyHidesMainWindow" ).isNull() )
      c.preferences.escKeyHidesMainWindow = ( preferences.namedItem( "escKeyHidesMainWindow" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "zoomFactor" ).isNull() )
      c.preferences.zoomFactor = preferences.namedItem( "zoomFactor" ).toElement().text().toDouble();

    if ( !preferences.namedItem( "wordsZoomLevel" ).isNull() )
      c.preferences.wordsZoomLevel = preferences.namedItem( "wordsZoomLevel" ).toElement().text().toInt();

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
    c.preferences.scanToMainWindow = ( preferences.namedItem( "scanToMainWindow" ).toElement().text() == "1" );
    c.preferences.scanPopupUseUIAutomation = ( preferences.namedItem( "scanPopupUseUIAutomation" ).toElement().text() == "1" );
    c.preferences.scanPopupUseIAccessibleEx = ( preferences.namedItem( "scanPopupUseIAccessibleEx" ).toElement().text() == "1" );
    c.preferences.scanPopupUseGDMessage = ( preferences.namedItem( "scanPopupUseGDMessage" ).toElement().text() == "1" );

    c.preferences.pronounceOnLoadMain = ( preferences.namedItem( "pronounceOnLoadMain" ).toElement().text() == "1" );
    c.preferences.pronounceOnLoadPopup = ( preferences.namedItem( "pronounceOnLoadPopup" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "useExternalPlayer" ).isNull() )
      c.preferences.useExternalPlayer = ( preferences.namedItem( "useExternalPlayer" ).toElement().text() == "1" );

#ifndef DISABLE_INTERNAL_PLAYER
    if ( !preferences.namedItem( "useInternalPlayer" ).isNull() )
      c.preferences.useInternalPlayer = ( preferences.namedItem( "useInternalPlayer" ).toElement().text() == "1" );
#else
    c.preferences.useInternalPlayer = false;
    c.preferences.useExternalPlayer = true;
#endif

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

    if ( !preferences.namedItem( "disallowContentFromOtherSites" ).isNull() )
      c.preferences.disallowContentFromOtherSites = ( preferences.namedItem( "disallowContentFromOtherSites" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "enableWebPlugins" ).isNull() )
      c.preferences.enableWebPlugins = ( preferences.namedItem( "enableWebPlugins" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "hideGoldenDictHeader" ).isNull() )
      c.preferences.hideGoldenDictHeader = ( preferences.namedItem( "hideGoldenDictHeader" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "maxStringsInHistory" ).isNull() )
      c.preferences.maxStringsInHistory = preferences.namedItem( "maxStringsInHistory" ).toElement().text().toUInt() ;

    if ( !preferences.namedItem( "storeHistory" ).isNull() )
      c.preferences.storeHistory = preferences.namedItem( "storeHistory" ).toElement().text().toUInt() ;

    if ( !preferences.namedItem( "alwaysExpandOptionalParts" ).isNull() )
      c.preferences.alwaysExpandOptionalParts = preferences.namedItem( "alwaysExpandOptionalParts" ).toElement().text().toUInt() ;

    if ( !preferences.namedItem( "addonStyle" ).isNull() )
      c.preferences.addonStyle = preferences.namedItem( "addonStyle" ).toElement().text();

    if ( !preferences.namedItem( "historyStoreInterval" ).isNull() )
      c.preferences.historyStoreInterval = preferences.namedItem( "historyStoreInterval" ).toElement().text().toUInt() ;

    if ( !preferences.namedItem( "collapseBigArticles" ).isNull() )
      c.preferences.collapseBigArticles = ( preferences.namedItem( "collapseBigArticles" ).toElement().text() == "1" );

    if ( !preferences.namedItem( "articleSizeLimit" ).isNull() )
      c.preferences.articleSizeLimit = preferences.namedItem( "articleSizeLimit" ).toElement().text().toUInt() ;

    if ( !preferences.namedItem( "maxDictionaryRefsInContextMenu" ).isNull() )
      c.preferences.maxDictionaryRefsInContextMenu = preferences.namedItem( "maxDictionaryRefsInContextMenu" ).toElement().text().toUShort();
  }

  c.lastMainGroupId = root.namedItem( "lastMainGroupId" ).toElement().text().toUInt();
  c.lastPopupGroupId = root.namedItem( "lastPopupGroupId" ).toElement().text().toUInt();

  QDomNode popupWindowState = root.namedItem( "popupWindowState" );

  if ( !popupWindowState.isNull() )
    c.popupWindowState = QByteArray::fromBase64( popupWindowState.toElement().text().toLatin1() );

  QDomNode popupWindowGeometry = root.namedItem( "popupWindowGeometry" );

  if ( !popupWindowGeometry.isNull() )
    c.popupWindowGeometry = QByteArray::fromBase64( popupWindowGeometry.toElement().text().toLatin1() );

  c.pinPopupWindow = ( root.namedItem( "pinPopupWindow" ).toElement().text() == "1" );

  QDomNode mainWindowState = root.namedItem( "mainWindowState" );

  if ( !mainWindowState.isNull() )
    c.mainWindowState = QByteArray::fromBase64( mainWindowState.toElement().text().toLatin1() );

  QDomNode mainWindowGeometry = root.namedItem( "mainWindowGeometry" );

  if ( !mainWindowGeometry.isNull() )
    c.mainWindowGeometry = QByteArray::fromBase64( mainWindowGeometry.toElement().text().toLatin1() );

#ifdef Q_OS_WIN
  QDomNode maximizedMainWindowGeometry = root.namedItem( "maximizedMainWindowGeometry" );

  if ( !maximizedMainWindowGeometry.isNull() )
  {
    int x = 0, y = 0, width = 0, height = 0;
    if( !maximizedMainWindowGeometry.namedItem( "x" ).isNull() )
      x = maximizedMainWindowGeometry.namedItem( "x" ).toElement().text().toInt();
    if( !maximizedMainWindowGeometry.namedItem( "y" ).isNull() )
      y = maximizedMainWindowGeometry.namedItem( "y" ).toElement().text().toInt();
    if( !maximizedMainWindowGeometry.namedItem( "width" ).isNull() )
      width = maximizedMainWindowGeometry.namedItem( "width" ).toElement().text().toInt();
    if( !maximizedMainWindowGeometry.namedItem( "height" ).isNull() )
      height = maximizedMainWindowGeometry.namedItem( "height" ).toElement().text().toInt();
    c.maximizedMainWindowGeometry = QRect( x, y, width, height );
  }
#endif

  QDomNode dictInfoGeometry = root.namedItem( "dictInfoGeometry" );

  if ( !dictInfoGeometry.isNull() )
    c.dictInfoGeometry = QByteArray::fromBase64( dictInfoGeometry.toElement().text().toLatin1() );

  QDomNode inspectorGeometry = root.namedItem( "inspectorGeometry" );

  if ( !inspectorGeometry.isNull() )
    c.inspectorGeometry = QByteArray::fromBase64( inspectorGeometry.toElement().text().toLatin1() );

  QDomNode timeForNewReleaseCheck = root.namedItem( "timeForNewReleaseCheck" );

  if ( !timeForNewReleaseCheck.isNull() )
    c.timeForNewReleaseCheck = QDateTime::fromString( timeForNewReleaseCheck.toElement().text(),
                                                      Qt::ISODate );

  c.skippedRelease = root.namedItem( "skippedRelease" ).toElement().text();

  c.showingDictBarNames = ( root.namedItem( "showingDictBarNames" ).toElement().text() == "1" );

  c.usingSmallIconsInToolbars = ( root.namedItem( "usingSmallIconsInToolbars" ).toElement().text() == "1" );

  if ( !root.namedItem( "historyExportPath" ).isNull() )
    c.historyExportPath = root.namedItem( "historyExportPath" ).toElement().text();

  if ( !root.namedItem( "resourceSavePath" ).isNull() )
    c.resourceSavePath = root.namedItem( "resourceSavePath" ).toElement().text();

  if ( !root.namedItem( "articleSavePath" ).isNull() )
    c.articleSavePath = root.namedItem( "articleSavePath" ).toElement().text();

  if ( !root.namedItem( "editDictionaryCommandLine" ).isNull() )
    c.editDictionaryCommandLine = root.namedItem( "editDictionaryCommandLine" ).toElement().text();

  if ( !root.namedItem( "maxPictureWidth" ).isNull() )
    c.maxPictureWidth = root.namedItem( "maxPictureWidth" ).toElement().text().toInt();

  if ( !root.namedItem( "maxHeadwordSize" ).isNull() )
  {
    unsigned int value = root.namedItem( "maxHeadwordSize" ).toElement().text().toUInt();
    if ( value != 0 ) // 0 is invalid value for our purposes
    {
      c.maxHeadwordSize = value;
    }
  }

  return c;
}

namespace {
void saveGroup( Group const & data, QDomElement & group )
{
  QDomDocument dd = group.ownerDocument();

  QDomAttr id = dd.createAttribute( "id" );

  id.setValue( QString::number( data.id ) );

  group.setAttributeNode( id );

  QDomAttr name = dd.createAttribute( "name" );

  name.setValue( data.name );

  group.setAttributeNode( name );

  if ( data.icon.size() )
  {
    QDomAttr icon = dd.createAttribute( "icon" );

    icon.setValue( data.icon );

    group.setAttributeNode( icon );
  }

  if ( data.iconData.size() )
  {
    QDomAttr iconData = dd.createAttribute( "iconData" );

    iconData.setValue( QString::fromLatin1( data.iconData.toBase64() ) );

    group.setAttributeNode( iconData );
  }

  if ( !data.shortcut.isEmpty() )
  {
    QDomAttr shortcut = dd.createAttribute( "shortcut" );

    shortcut.setValue(  data.shortcut.toString() );

    group.setAttributeNode( shortcut );
  }

  for( QVector< DictionaryRef >::const_iterator j = data.dictionaries.begin(); j != data.dictionaries.end(); ++j )
  {
    QDomElement dictionary = dd.createElement( "dictionary" );

    group.appendChild( dictionary );

    QDomText value = dd.createTextNode( j->id );

    dictionary.appendChild( value );

    QDomAttr name = dd.createAttribute( "name" );

    name.setValue( j->name );

    dictionary.setAttributeNode( name );
  }

  QDomElement muted = dd.createElement( "mutedDictionaries" );
  group.appendChild( muted );

  for( MutedDictionaries::const_iterator i = data.mutedDictionaries.begin();
       i != data.mutedDictionaries.end(); ++i )
  {
    QDomElement dict = dd.createElement( "mutedDictionary" );
    muted.appendChild( dict );

    QDomText value = dd.createTextNode( *i );
    dict.appendChild( value );
  }

  for( MutedDictionaries::const_iterator i = data.popupMutedDictionaries.begin();
       i != data.popupMutedDictionaries.end(); ++i )
  {
    QDomElement dict = dd.createElement( "popupMutedDictionary" );
    muted.appendChild( dict );

    QDomText value = dd.createTextNode( *i );
    dict.appendChild( value );
  }
}

}

void save( Class const & c ) throw( exError )
{
  QFile configFile( getConfigFileName() + ".tmp" );

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

      QDomAttr icon = dd.createAttribute( "icon" );
      icon.setValue( i->iconFilename );
      soundDir.setAttributeNode( icon );

      QDomText value = dd.createTextNode( i->path );

      soundDir.appendChild( value );
    }
  }

  {
    QDomElement dictionaryOrder = dd.createElement( "dictionaryOrder" );
    root.appendChild( dictionaryOrder );
    saveGroup( c.dictionaryOrder, dictionaryOrder );
  }

  {
    QDomElement inactiveDictionaries = dd.createElement( "inactiveDictionaries" );
    root.appendChild( inactiveDictionaries );
    saveGroup( c.inactiveDictionaries, inactiveDictionaries );
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

      saveGroup( *i, group );
    }
  }

  {
    QDomElement hunspell = dd.createElement( "hunspell" );
    QDomAttr path = dd.createAttribute( "dictionariesPath" );
    path.setValue( c.hunspell.dictionariesPath );
    hunspell.setAttributeNode( path );
    root.appendChild( hunspell );

    for( int x = 0; x < c.hunspell.enabledDictionaries.size(); ++x )
    {
      QDomElement en = dd.createElement( "enabled" );
      QDomText value = dd.createTextNode( c.hunspell.enabledDictionaries.at( x ) );

      en.appendChild( value );
      hunspell.appendChild( en );
    }
  }

  {
    // Russian translit

    QDomElement transliteration = dd.createElement( "transliteration" );
    root.appendChild( transliteration );

    QDomElement opt = dd.createElement( "enableRussianTransliteration" );
    opt.appendChild( dd.createTextNode( c.transliteration.enableRussianTransliteration ? "1":"0" ) );
    transliteration.appendChild( opt );

    // German translit

    opt = dd.createElement( "enableGermanTransliteration" );
    opt.appendChild( dd.createTextNode( c.transliteration.enableGermanTransliteration ? "1":"0" ) );
    transliteration.appendChild( opt );

    // Greek translit

    opt = dd.createElement( "enableGreekTransliteration" );
    opt.appendChild( dd.createTextNode( c.transliteration.enableGreekTransliteration ? "1":"0" ) );
    transliteration.appendChild( opt );

    // Belarusian translit

    opt = dd.createElement( "enableBelarusianTransliteration" );
    opt.appendChild( dd.createTextNode( c.transliteration.enableBelarusianTransliteration ? "1":"0" ) );
    transliteration.appendChild( opt );

    // Romaji

    QDomElement romaji = dd.createElement( "romaji" );
    transliteration.appendChild( romaji );

    opt = dd.createElement( "enable" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enable ? "1":"0" ) );
    romaji.appendChild( opt );

    opt = dd.createElement( "enableHepburn" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enableHepburn ? "1":"0" ) );
    romaji.appendChild( opt );

    opt = dd.createElement( "enableNihonShiki" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enableNihonShiki ? "1":"0" ) );
    romaji.appendChild( opt );

    opt = dd.createElement( "enableKunreiShiki" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enableKunreiShiki ? "1":"0" ) );
    romaji.appendChild( opt );

    opt = dd.createElement( "enableHiragana" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enableHiragana ? "1":"0" ) );
    romaji.appendChild( opt );

    opt = dd.createElement( "enableKatakana" );
    opt.appendChild( dd.createTextNode( c.transliteration.romaji.enableKatakana ? "1":"0" ) );
    romaji.appendChild( opt );
  }

  {
    // Forvo

    QDomElement forvo = dd.createElement( "forvo" );
    root.appendChild( forvo );

    QDomElement opt = dd.createElement( "enable" );
    opt.appendChild( dd.createTextNode( c.forvo.enable ? "1":"0" ) );
    forvo.appendChild( opt );

    opt = dd.createElement( "apiKey" );
    opt.appendChild( dd.createTextNode( c.forvo.apiKey ) );
    forvo.appendChild( opt );

    opt = dd.createElement( "languageCodes" );
    opt.appendChild( dd.createTextNode( c.forvo.languageCodes ) );
    forvo.appendChild( opt );
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

      QDomAttr icon = dd.createAttribute( "icon" );
      icon.setValue( i->icon );
      mw.setAttributeNode( icon );
    }
  }

  {
    QDomElement wss = dd.createElement( "websites" );
    root.appendChild( wss );

    for( WebSites::const_iterator i = c.webSites.begin(); i != c.webSites.end(); ++i )
    {
      QDomElement ws = dd.createElement( "website" );
      wss.appendChild( ws );

      QDomAttr id = dd.createAttribute( "id" );
      id.setValue( i->id );
      ws.setAttributeNode( id );

      QDomAttr name = dd.createAttribute( "name" );
      name.setValue( i->name );
      ws.setAttributeNode( name );

      QDomAttr url = dd.createAttribute( "url" );
      url.setValue( i->url );
      ws.setAttributeNode( url );

      QDomAttr enabled = dd.createAttribute( "enabled" );
      enabled.setValue( i->enabled ? "1" : "0" );
      ws.setAttributeNode( enabled );

      QDomAttr icon = dd.createAttribute( "icon" );
      icon.setValue( i->iconFilename );
      ws.setAttributeNode( icon );
    }
  }

  {
    QDomElement programs = dd.createElement( "programs" );
    root.appendChild( programs );

    for( Programs::const_iterator i = c.programs.begin(); i != c.programs.end(); ++i )
    {
      QDomElement p = dd.createElement( "program" );
      programs.appendChild( p );

      QDomAttr id = dd.createAttribute( "id" );
      id.setValue( i->id );
      p.setAttributeNode( id );

      QDomAttr name = dd.createAttribute( "name" );
      name.setValue( i->name );
      p.setAttributeNode( name );

      QDomAttr commandLine = dd.createAttribute( "commandLine" );
      commandLine.setValue( i->commandLine );
      p.setAttributeNode( commandLine );

      QDomAttr enabled = dd.createAttribute( "enabled" );
      enabled.setValue( i->enabled ? "1" : "0" );
      p.setAttributeNode( enabled );

      QDomAttr type = dd.createAttribute( "type" );
      type.setValue( QString::number( i->type ) );
      p.setAttributeNode( type );

      QDomAttr icon = dd.createAttribute( "icon" );
      icon.setValue( i->iconFilename );
      p.setAttributeNode( icon );
    }
  }

  {
    QDomNode ves = dd.createElement( "voiceEngines" );
    root.appendChild( ves );

    for ( VoiceEngines::const_iterator i = c.voiceEngines.begin(); i != c.voiceEngines.end(); ++i )
    {
      QDomElement v = dd.createElement( "voiceEngine" );
      ves.appendChild( v );

      QDomAttr id = dd.createAttribute( "id" );
      id.setValue( i->id );
      v.setAttributeNode( id );

      QDomAttr name = dd.createAttribute( "name" );
      name.setValue( i->name );
      v.setAttributeNode( name );

      QDomAttr enabled = dd.createAttribute( "enabled" );
      enabled.setValue( i->enabled ? "1" : "0" );
      v.setAttributeNode( enabled );

      QDomAttr icon = dd.createAttribute( "icon" );
      icon.setValue( i->iconFilename );
      v.setAttributeNode( icon );

      QDomAttr volume = dd.createAttribute( "volume" );
      volume.setValue( QString::number( i->volume ) );
      v.setAttributeNode( volume );

      QDomAttr rate = dd.createAttribute( "rate" );
      rate.setValue( QString::number( i->rate ) );
      v.setAttributeNode( rate );
    }
  }

  {
    QDomElement muted = dd.createElement( "mutedDictionaries" );
    root.appendChild( muted );
    saveMutedDictionaries( dd, muted, c.mutedDictionaries );
  }

  {
    QDomElement muted = dd.createElement( "popupMutedDictionaries" );
    root.appendChild( muted );
    saveMutedDictionaries( dd, muted, c.popupMutedDictionaries );
  }

  {
    QDomElement preferences = dd.createElement( "preferences" );
    root.appendChild( preferences );

    QDomElement opt = dd.createElement( "interfaceLanguage" );
    opt.appendChild( dd.createTextNode( c.preferences.interfaceLanguage ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "displayStyle" );
    opt.appendChild( dd.createTextNode( c.preferences.displayStyle ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "newTabsOpenAfterCurrentOne" );
    opt.appendChild( dd.createTextNode( c.preferences.newTabsOpenAfterCurrentOne ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "newTabsOpenInBackground" );
    opt.appendChild( dd.createTextNode( c.preferences.newTabsOpenInBackground ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "hideSingleTab" );
    opt.appendChild( dd.createTextNode( c.preferences.hideSingleTab ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "mruTabOrder" );
    opt.appendChild( dd.createTextNode( c.preferences.mruTabOrder ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "hideMenubar" );
    opt.appendChild( dd.createTextNode( c.preferences.hideMenubar ? "1":"0" ) );
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

    opt = dd.createElement( "doubleClickTranslates" );
    opt.appendChild( dd.createTextNode( c.preferences.doubleClickTranslates ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "selectWordBySingleClick" );
    opt.appendChild( dd.createTextNode( c.preferences.selectWordBySingleClick ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "escKeyHidesMainWindow" );
    opt.appendChild( dd.createTextNode( c.preferences.escKeyHidesMainWindow ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "zoomFactor" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.zoomFactor ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "wordsZoomLevel" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.wordsZoomLevel ) ) );
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

    opt = dd.createElement( "scanToMainWindow" );
    opt.appendChild( dd.createTextNode( c.preferences.scanToMainWindow ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupUseUIAutomation" );
    opt.appendChild( dd.createTextNode( c.preferences.scanPopupUseUIAutomation ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupUseIAccessibleEx" );
    opt.appendChild( dd.createTextNode( c.preferences.scanPopupUseIAccessibleEx ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "scanPopupUseGDMessage" );
    opt.appendChild( dd.createTextNode( c.preferences.scanPopupUseGDMessage ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "pronounceOnLoadMain" );
    opt.appendChild( dd.createTextNode( c.preferences.pronounceOnLoadMain ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "pronounceOnLoadPopup" );
    opt.appendChild( dd.createTextNode( c.preferences.pronounceOnLoadPopup ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "useExternalPlayer" );
    opt.appendChild( dd.createTextNode( c.preferences.useExternalPlayer ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "useInternalPlayer" );
    opt.appendChild( dd.createTextNode( c.preferences.useInternalPlayer ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "audioPlaybackProgram" );
    opt.appendChild( dd.createTextNode( c.preferences.audioPlaybackProgram ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "alwaysOnTop" );
    opt.appendChild( dd.createTextNode( c.preferences.alwaysOnTop ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "searchInDock" );
    opt.appendChild( dd.createTextNode( c.preferences.searchInDock ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "historyStoreInterval" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.historyStoreInterval ) ) );
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

    opt = dd.createElement( "disallowContentFromOtherSites" );
    opt.appendChild( dd.createTextNode( c.preferences.disallowContentFromOtherSites ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "enableWebPlugins" );
    opt.appendChild( dd.createTextNode( c.preferences.enableWebPlugins ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "hideGoldenDictHeader" );
    opt.appendChild( dd.createTextNode( c.preferences.hideGoldenDictHeader ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "maxStringsInHistory" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.maxStringsInHistory ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "storeHistory" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.storeHistory ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "alwaysExpandOptionalParts" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.alwaysExpandOptionalParts ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "addonStyle" );
    opt.appendChild( dd.createTextNode( c.preferences.addonStyle ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "collapseBigArticles" );
    opt.appendChild( dd.createTextNode( c.preferences.collapseBigArticles ? "1" : "0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "articleSizeLimit" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.articleSizeLimit ) ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "maxDictionaryRefsInContextMenu" );
    opt.appendChild( dd.createTextNode( QString::number( c.preferences.maxDictionaryRefsInContextMenu ) ) );
    preferences.appendChild( opt ); }

  {
    QDomElement opt = dd.createElement( "lastMainGroupId" );
    opt.appendChild( dd.createTextNode( QString::number( c.lastMainGroupId ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "lastPopupGroupId" );
    opt.appendChild( dd.createTextNode( QString::number( c.lastPopupGroupId ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "popupWindowState" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.popupWindowState.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "popupWindowGeometry" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.popupWindowGeometry.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "pinPopupWindow" );
    opt.appendChild( dd.createTextNode( c.pinPopupWindow ? "1" : "0" ) );
    root.appendChild( opt );

    opt = dd.createElement( "mainWindowState" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.mainWindowState.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "mainWindowGeometry" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.mainWindowGeometry.toBase64() ) ) );
    root.appendChild( opt );

#ifdef Q_OS_WIN
    {
      QDomElement maximizedMainWindowGeometry = dd.createElement( "maximizedMainWindowGeometry" );
      root.appendChild( maximizedMainWindowGeometry );

      opt = dd.createElement( "x" );
      opt.appendChild( dd.createTextNode( QString::number( c.maximizedMainWindowGeometry.x() ) ) );
      maximizedMainWindowGeometry.appendChild( opt );

      opt = dd.createElement( "y" );
      opt.appendChild( dd.createTextNode( QString::number( c.maximizedMainWindowGeometry.y() ) ) );
      maximizedMainWindowGeometry.appendChild( opt );

      opt = dd.createElement( "width" );
      opt.appendChild( dd.createTextNode( QString::number( c.maximizedMainWindowGeometry.width() ) ) );
      maximizedMainWindowGeometry.appendChild( opt );

      opt = dd.createElement( "height" );
      opt.appendChild( dd.createTextNode( QString::number( c.maximizedMainWindowGeometry.height() ) ) );
      maximizedMainWindowGeometry.appendChild( opt );
    }
#endif

    opt = dd.createElement( "dictInfoGeometry" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.dictInfoGeometry.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "inspectorGeometry" );
    opt.appendChild( dd.createTextNode( QString::fromLatin1( c.inspectorGeometry.toBase64() ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "timeForNewReleaseCheck" );
    opt.appendChild( dd.createTextNode( c.timeForNewReleaseCheck.toString( Qt::ISODate ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "skippedRelease" );
    opt.appendChild( dd.createTextNode( c.skippedRelease ) );
    root.appendChild( opt );

    opt = dd.createElement( "showingDictBarNames" );
    opt.appendChild( dd.createTextNode( c.showingDictBarNames ? "1" : "0" ) );
    root.appendChild( opt );

    opt = dd.createElement( "usingSmallIconsInToolbars" );
    opt.appendChild( dd.createTextNode( c.usingSmallIconsInToolbars ? "1" : "0" ) );
    root.appendChild( opt );

    if( !c.historyExportPath.isEmpty() )
    {
        opt = dd.createElement( "historyExportPath" );
        opt.appendChild( dd.createTextNode( c.historyExportPath ) );
        root.appendChild( opt );
    }

    if( !c.resourceSavePath.isEmpty() )
    {
        opt = dd.createElement( "resourceSavePath" );
        opt.appendChild( dd.createTextNode( c.resourceSavePath ) );
        root.appendChild( opt );
    }

    if( !c.articleSavePath.isEmpty() )
    {
        opt = dd.createElement( "articleSavePath" );
        opt.appendChild( dd.createTextNode( c.articleSavePath ) );
        root.appendChild( opt );
    }

    opt = dd.createElement( "editDictionaryCommandLine" );
    opt.appendChild( dd.createTextNode( c.editDictionaryCommandLine ) );
    root.appendChild( opt );

    opt = dd.createElement( "maxPictureWidth" );
    opt.appendChild( dd.createTextNode( QString::number( c.maxPictureWidth ) ) );
    root.appendChild( opt );

    opt = dd.createElement( "maxHeadwordSize" );
    opt.appendChild( dd.createTextNode( QString::number( c.maxHeadwordSize ) ) );
    root.appendChild( opt );
  }

  QByteArray result( dd.toByteArray() );

  if ( configFile.write( result ) != result.size() )
    throw exCantWriteConfigFile();

  configFile.close();

  renameAtomically( configFile.fileName(), getConfigFileName() );
}

QString getConfigFileName()
{
  return getHomeDir().absoluteFilePath( "config" );
}

QString getConfigDir() throw( exError )
{
  return getHomeDir().path() + QDir::separator();
}

QString getIndexDir() throw( exError )
{
  QDir result = getHomeDir();

  result.mkpath( "index" );

  if ( !result.cd( "index" ) )
    throw exCantUseIndexDir();

  return result.path() + QDir::separator();
}

QString getPidFileName() throw( exError )
{
  return getHomeDir().filePath( "pid" );
}

QString getHistoryFileName() throw( exError )
{
  return getHomeDir().filePath( "history" );
}

QString getUserCssFileName() throw( exError )
{
  return getHomeDir().filePath( "article-style.css" );
}

QString getUserCssPrintFileName() throw( exError )
{
  return getHomeDir().filePath( "article-style-print.css" );
}

QString getUserQtCssFileName() throw( exError )
{
  return getHomeDir().filePath( "qt-style.css" );
}

QString getProgramDataDir() throw()
{
  if ( isPortableVersion() )
    return QCoreApplication::applicationDirPath();

  #ifdef PROGRAM_DATA_DIR
  return PROGRAM_DATA_DIR;
  #else
  return QCoreApplication::applicationDirPath();
  #endif
}

QString getLocDir() throw()
{
  if ( QDir( getProgramDataDir() ).cd( "locale" ) )
    return getProgramDataDir() + "/locale";
  else
    return QCoreApplication::applicationDirPath() + "/locale";
}

bool isPortableVersion() throw()
{
  struct IsPortable
  {
    bool isPortable;

    IsPortable(): isPortable( QFileInfo( QCoreApplication::applicationDirPath() + "/portable" ).isDir() )
    {}
  };

  static IsPortable p;

  return p.isPortable;
}

QString getPortableVersionDictionaryDir() throw()
{
  if ( isPortableVersion() )
    return getProgramDataDir() + "/content";
  else
    return QString();
}

QString getPortableVersionMorphoDir() throw()
{
  if ( isPortableVersion() )
    return getPortableVersionDictionaryDir() + "/morphology";
  else
    return QString();
}

QString getStylesDir() throw()
{
  QDir result = getHomeDir();

  result.mkpath( "styles" );

  if ( !result.cd( "styles" ) )
    return QString();

  return result.path() + QDir::separator();
}

}
