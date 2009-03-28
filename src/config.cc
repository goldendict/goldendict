/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "config.hh"
#include <QDir>
#include <QFile>
#include <QtXml>

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

Preferences::Preferences():
  enableTrayIcon( false ),
  startToTray( false ),
  enableScanPopup( false ),
  startWithScanPopupOn( false ),
  enableScanPopupModifiers( false ),
  scanPopupModifiers( 0 )
{
}

namespace {

MediaWikis makeDefaultMediaWikis( bool enable )
{
  MediaWikis mw;

  mw.push_back( MediaWiki( "ae6f89aac7151829681b85f035d54e48", "English Wikipedia", "http://en.wikipedia.org/w", enable ) );
  mw.push_back( MediaWiki( "affcf9678e7bfe701c9b071f97eccba3", "English Wiktionary", "http://en.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "8e0c1c2b6821dab8bdba8eb869ca7176", "Russian Wikipedia", "http://ru.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "b09947600ae3902654f8ad4567ae8567", "Russain Wiktionary", "http://ru.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "a8a66331a1242ca2aeb0b4aed361c41d", "German Wikipedia", "http://de.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "21c64bca5ec10ba17ff19f3066bc962a", "German Wiktionary", "http://de.wiktionary.org/w", false ) );
  mw.push_back( MediaWiki( "96957cb2ad73a20c7a1d561fc83c253a", "Portuguese Wikipedia", "http://pt.wikipedia.org/w", false ) );
  mw.push_back( MediaWiki( "ed4c3929196afdd93cc08b9a903aad6a", "Portuguese Wiktionary", "http://pt.wiktionary.org/w", false ) );

  return mw;
}

}

Class load() throw( exError )
{
  QString configName  = getConfigFileName();

  if ( !QFile::exists( configName ) )
  {
    // Make the default config, save it and return it
    Class c;

    #ifdef Q_OS_LINUX

    if ( QDir( "/usr/share/stardict/dic" ).exists() )
      c.paths.push_back( Path( "/usr/share/stardict/dic", true ) );

    #endif

    c.mediawikis = makeDefaultMediaWikis( true );

    save( c );

    return c;
  }

  QFile configFile( configName );

  if ( !configFile.open( QFile::ReadOnly ) )
    throw exCantReadConfigFile();

  QDomDocument dd;

  QString errorStr;
  int errorLine, errorColumn;

  if ( !dd.setContent( &configFile, false, &errorStr, &errorLine, &errorColumn  ) )
  {
    printf( "Error: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
      throw exMalformedConfigFile();
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

  QDomNode groups = root.namedItem( "groups" );

  if ( !groups.isNull() )
  {
    QDomNodeList nl = groups.toElement().elementsByTagName( "group" );

    for( unsigned x = 0; x < nl.length(); ++x )
    {
      QDomElement grp = nl.item( x ).toElement();

      Group g;

      g.name = grp.attribute( "name" );
      g.icon = grp.attribute( "icon" );

      QDomNodeList dicts = grp.elementsByTagName( "dictionary" );

      for( unsigned y = 0; y < dicts.length(); ++y )
        g.dictionaries.push_back( dicts.item( y ).toElement().text() );

      c.groups.push_back( g );
    }
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
    c.preferences.enableTrayIcon = ( preferences.namedItem( "enableTrayIcon" ).toElement().text() == "1" );
    c.preferences.startToTray = ( preferences.namedItem( "startToTray" ).toElement().text() == "1" );
    c.preferences.closeToTray = ( preferences.namedItem( "closeToTray" ).toElement().text() == "1" );
    c.preferences.enableScanPopup = ( preferences.namedItem( "enableScanPopup" ).toElement().text() == "1" );
    c.preferences.startWithScanPopupOn = ( preferences.namedItem( "startWithScanPopupOn" ).toElement().text() == "1" );
    c.preferences.enableScanPopupModifiers = ( preferences.namedItem( "enableScanPopupModifiers" ).toElement().text() == "1" );
    c.preferences.scanPopupModifiers = ( preferences.namedItem( "scanPopupModifiers" ).toElement().text().toULong() );    
  }

  c.lastMainGroup = root.namedItem( "lastMainGroup" ).toElement().text();
  c.lastPopupGroup = root.namedItem( "lastPopupGroup" ).toElement().text();

  QDomNode lastPopupWidth = root.namedItem( "lastPopupWidth" );
  QDomNode lastPopupHeight = root.namedItem( "lastPopupHeight" );

  if ( !lastPopupWidth.isNull() && !lastPopupHeight.isNull() )
  {
    c.lastPopupSize = QSize( lastPopupWidth.toElement().text().toULong(),
                             lastPopupHeight.toElement().text().toULong() );
  }

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
    QDomElement groups = dd.createElement( "groups" );
    root.appendChild( groups );

    for( Groups::const_iterator i = c.groups.begin(); i != c.groups.end(); ++i )
    {
      QDomElement group = dd.createElement( "group" );
      groups.appendChild( group );

      QDomAttr name = dd.createAttribute( "name" );

      name.setValue( i->name );

      group.setAttributeNode( name );

      if ( i->icon.size() )
      {
        QDomAttr icon = dd.createAttribute( "icon" );

        icon.setValue( i->icon );

        group.setAttributeNode( icon );
      }

      for( vector< QString >::const_iterator j = i->dictionaries.begin(); j != i->dictionaries.end(); ++j )
      {
        QDomElement dictionary = dd.createElement( "dictionary" );

        group.appendChild( dictionary );

        QDomText value = dd.createTextNode( *j );

        dictionary.appendChild( value );
      }
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

    QDomElement opt = dd.createElement( "enableTrayIcon" );
    opt.appendChild( dd.createTextNode( c.preferences.enableTrayIcon ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "startToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.startToTray ? "1":"0" ) );
    preferences.appendChild( opt );

    opt = dd.createElement( "closeToTray" );
    opt.appendChild( dd.createTextNode( c.preferences.closeToTray ? "1":"0" ) );
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
  }

  {
    QDomElement opt = dd.createElement( "lastMainGroup" );
    opt.appendChild( dd.createTextNode( c.lastMainGroup ) );
    root.appendChild( opt );

    opt = dd.createElement( "lastPopupGroup" );
    opt.appendChild( dd.createTextNode( c.lastPopupGroup ) );
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

}
