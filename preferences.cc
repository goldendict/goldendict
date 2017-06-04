#include "preferences.hh"
#include "keyboardstate.hh"
#include "language.hh"
#include "langcoder.hh"
#include <QMessageBox>
#include "broken_xrecord.hh"
#include "mainwindow.hh"


Preferences::Preferences( QWidget * parent, Config::Class & cfg_ ):
  QDialog( parent ), prevInterfaceLanguage( 0 )
, helpWindow( 0 )
, cfg( cfg_ )
, helpAction( this )
{
  Config::Preferences const & p = cfg_.preferences;
  ui.setupUi( this );

  connect( ui.enableScanPopup, SIGNAL( toggled( bool ) ),
           this, SLOT( enableScanPopupToggled( bool ) ) );

  connect( ui.enableScanPopupModifiers, SIGNAL( toggled( bool ) ),
           this, SLOT( enableScanPopupModifiersToggled( bool ) ) );

  connect( ui.altKey, SIGNAL( clicked( bool ) ),
           this, SLOT( wholeAltClicked( bool ) ) );
  connect( ui.ctrlKey, SIGNAL( clicked( bool ) ),
           this, SLOT( wholeCtrlClicked( bool ) ) );
  connect( ui.shiftKey, SIGNAL( clicked( bool ) ),
           this, SLOT( wholeShiftClicked( bool ) ) );

  connect( ui.leftAlt, SIGNAL( clicked( bool ) ),
           this, SLOT( sideAltClicked( bool ) ) );
  connect( ui.rightAlt, SIGNAL( clicked( bool ) ),
           this, SLOT( sideAltClicked( bool ) ) );
  connect( ui.leftCtrl, SIGNAL( clicked( bool ) ),
           this, SLOT( sideCtrlClicked( bool ) ) );
  connect( ui.rightCtrl, SIGNAL( clicked( bool ) ),
           this, SLOT( sideCtrlClicked( bool ) ) );
  connect( ui.leftShift, SIGNAL( clicked( bool ) ),
           this, SLOT( sideShiftClicked( bool ) ) );
  connect( ui.rightShift, SIGNAL( clicked( bool ) ),
           this, SLOT( sideShiftClicked( bool ) ) );

  connect( ui.buttonBox, SIGNAL( helpRequested() ),
           this, SLOT( helpRequested() ) );

  helpAction.setShortcut( QKeySequence( "F1" ) );
  helpAction.setShortcutContext( Qt::WidgetWithChildrenShortcut );

  connect( &helpAction, SIGNAL( triggered() ),
           this, SLOT( helpRequested() ) );

  addAction( &helpAction );

  // Load values into form

  ui.interfaceLanguage->addItem( tr( "System default" ), QString() );
  ui.interfaceLanguage->addItem( QIcon( ":/flags/us.png" ), Language::localizedNameForId( LangCoder::code2toInt( "en" ) ), QString( "en_US" ) );

  // See which other translations do we have

  QStringList availLocs = QDir( Config::getLocDir() ).entryList( QStringList( "*.qm" ),
                                                                 QDir::Files );

  // We need to sort by language name -- otherwise list looks really weird
  QMap< QString, QPair< QIcon, QString > > sortedLocs;

  for( QStringList::iterator i = availLocs.begin(); i != availLocs.end(); ++i )
  {
    // Here we assume the xx_YY naming, where xx is language and YY is region.
    QString lang = i->mid( 0, 2 );

    if ( lang == "qt" )
      continue; // We skip qt's own localizations

    sortedLocs.insertMulti(
      Language::localizedNameForId( LangCoder::code2toInt( lang.toLatin1().data() ) ),
      QPair< QIcon, QString >(
        QIcon( QString( ":/flags/%1.png" ).arg( i->mid( 3, 2 ).toLower() ) ),
        i->mid( 0, i->size() - 3 ) ) );
  }

  for( QMap< QString, QPair< QIcon, QString > >::iterator i = sortedLocs.begin();
       i != sortedLocs.end(); ++i )
    ui.interfaceLanguage->addItem( i.value().first, i.key(), i.value().second );

  for( int x = 0; x < ui.interfaceLanguage->count(); ++x )
    if ( ui.interfaceLanguage->itemData( x ).toString() == p.interfaceLanguage )
    {
      ui.interfaceLanguage->setCurrentIndex( x );
      prevInterfaceLanguage = x;
      break;
    }

  // Fill help languages combobox

  ui.helpLanguage->addItem( tr( "Default" ), QString() );

  // See which helps do we have

  QStringList availHelps = QDir( Config::getHelpDir() ).entryList( QStringList( "*.qch" ),
                                                                 QDir::Files );

  QMap< QString, QPair< QIcon, QString > > sortedHelps;

  for( QStringList::iterator i = availHelps.begin(); i != availHelps.end(); ++i )
  {
    QString loc = i->mid( 7, i->length() - 11 );
    QString lang = loc.mid( 0, 2 );
    QString reg;
    if(loc.length() >= 5 )
      reg = loc.mid( 3, 2 ).toLower();
    else
    {
      if( lang.compare( "en", Qt::CaseInsensitive ) == 0 )
        reg = "US";
      else
        reg = lang.toUpper();
    }

    sortedHelps.insertMulti(
      Language::localizedNameForId( LangCoder::code2toInt( lang.toLatin1().data() ) ),
      QPair< QIcon, QString >(
        QIcon( QString( ":/flags/%1.png" ).arg( reg.toLower() ) ), lang + "_" + reg ) );
  }

  for( QMap< QString, QPair< QIcon, QString > >::iterator i = sortedHelps.begin();
       i != sortedHelps.end(); ++i )
    ui.helpLanguage->addItem( i.value().first, i.key(), i.value().second );

  for( int x = 0; x < ui.helpLanguage->count(); ++x )
    if ( ui.helpLanguage->itemData( x ).toString() == p.helpLanguage )
    {
      ui.helpLanguage->setCurrentIndex( x );
      break;
    }

  ui.displayStyle->addItem( QIcon( ":/icons/programicon_old.png" ), tr( "Default" ), QString() );
  ui.displayStyle->addItem( QIcon( ":/icons/programicon.png" ), tr( "Modern" ), QString( "modern" ) );
  ui.displayStyle->addItem( QIcon( ":/icons/icon32_dsl.png" ), tr( "Lingvo" ), QString( "lingvo" ) );
  ui.displayStyle->addItem( QIcon( ":/icons/icon32_bgl.png" ), tr( "Babylon" ), QString( "babylon" ) );
  ui.displayStyle->addItem( QIcon( ":/icons/icon32_lingoes.png" ), tr( "Lingoes" ), QString( "lingoes" ) );
  ui.displayStyle->addItem( QIcon( ":/icons/icon32_lingoes.png" ), tr( "Lingoes-Blue" ), QString( "lingoes-blue" ) );

  for( int x = 0; x < ui.displayStyle->count(); ++x )
    if ( ui.displayStyle->itemData( x ).toString() == p.displayStyle )
    {
      ui.displayStyle->setCurrentIndex( x );
      break;
    }

  ui.newTabsOpenAfterCurrentOne->setChecked( p.newTabsOpenAfterCurrentOne );
  ui.newTabsOpenInBackground->setChecked( p.newTabsOpenInBackground );
  ui.hideSingleTab->setChecked( p.hideSingleTab );
  ui.mruTabOrder->setChecked( p.mruTabOrder );
  ui.enableTrayIcon->setChecked( p.enableTrayIcon );
  ui.startToTray->setChecked( p.startToTray );
  ui.closeToTray->setChecked( p.closeToTray );
  ui.cbAutostart->setChecked( p.autoStart );
  ui.doubleClickTranslates->setChecked( p.doubleClickTranslates );
  ui.selectBySingleClick->setChecked( p.selectWordBySingleClick);
  ui.escKeyHidesMainWindow->setChecked( p.escKeyHidesMainWindow );

  ui.enableMainWindowHotkey->setChecked( p.enableMainWindowHotkey );
  ui.mainWindowHotkey->setHotKey( p.mainWindowHotkey );
  ui.enableClipboardHotkey->setChecked( p.enableClipboardHotkey );
  ui.clipboardHotkey->setHotKey( p.clipboardHotkey );

  ui.enableScanPopup->setChecked( p.enableScanPopup );
  ui.startWithScanPopupOn->setChecked( p.startWithScanPopupOn );
  ui.enableScanPopupModifiers->setChecked( p.enableScanPopupModifiers );

  ui.altKey->setChecked( p.scanPopupModifiers & KeyboardState::Alt );
  ui.ctrlKey->setChecked( p.scanPopupModifiers & KeyboardState::Ctrl );
  ui.shiftKey->setChecked( p.scanPopupModifiers & KeyboardState::Shift );
  ui.winKey->setChecked( p.scanPopupModifiers & KeyboardState::Win );
  ui.leftAlt->setChecked( p.scanPopupModifiers & KeyboardState::LeftAlt );
  ui.rightAlt->setChecked( p.scanPopupModifiers & KeyboardState::RightAlt );
  ui.leftCtrl->setChecked( p.scanPopupModifiers & KeyboardState::LeftCtrl );
  ui.rightCtrl->setChecked( p.scanPopupModifiers & KeyboardState::RightCtrl );
  ui.leftShift->setChecked( p.scanPopupModifiers & KeyboardState::LeftShift );
  ui.rightShift->setChecked( p.scanPopupModifiers & KeyboardState::RightShift );

  ui.scanPopupAltMode->setChecked( p.scanPopupAltMode );
  ui.scanPopupAltModeSecs->setValue( p.scanPopupAltModeSecs );
  ui.scanToMainWindow->setChecked( p.scanToMainWindow );
  ui.showScanFlag->setChecked( p.showScanFlag);
  ui.scanPopupUseUIAutomation->setChecked( p.scanPopupUseUIAutomation );
  ui.scanPopupUseIAccessibleEx->setChecked( p.scanPopupUseIAccessibleEx );
  ui.scanPopupUseGDMessage->setChecked( p.scanPopupUseGDMessage );

  ui.storeHistory->setChecked( p.storeHistory );
  ui.historyMaxSizeField->setValue( p.maxStringsInHistory );
  ui.historySaveIntervalField->setValue( p.historyStoreInterval );
  ui.alwaysExpandOptionalParts->setChecked( p.alwaysExpandOptionalParts );

  ui.favoritesSaveIntervalField->setValue( p.favoritesStoreInterval );
  ui.confirmFavoritesDeletion->setChecked( p.confirmFavoritesDeletion );

  ui.collapseBigArticles->setChecked( p.collapseBigArticles );
  ui.articleSizeLimit->setValue( p.articleSizeLimit );

  ui.synonymSearchEnabled->setChecked( p.synonymSearchEnabled );

  ui.maxDictsInContextMenu->setValue( p.maxDictionaryRefsInContextMenu );

  // Different platforms have different keys available

#ifdef Q_OS_WIN32
  ui.winKey->hide();
#else
  ui.leftAlt->hide();
  ui.rightAlt->hide();
  ui.leftCtrl->hide();
  ui.rightCtrl->hide();
  ui.leftShift->hide();
  ui.rightShift->hide();
#ifdef Q_OS_MAC
  ui.altKey->setText( "Opt" );
  ui.winKey->setText( "Ctrl" );
  ui.ctrlKey->setText( "Cmd" );
#endif
#endif

//Platform-specific options

#ifndef Q_OS_WIN32
  ui.groupBox_ScanPopupTechnologies->hide();
//  ui.tabWidget->removeTab( 5 );
#endif

  // Sound

  ui.pronounceOnLoadMain->setChecked( p.pronounceOnLoadMain );
  ui.pronounceOnLoadPopup->setChecked( p.pronounceOnLoadPopup );

#ifdef DISABLE_INTERNAL_PLAYER
  ui.useInternalPlayer->hide();
#else
  if ( p.useInternalPlayer )
    ui.useInternalPlayer->setChecked( true );
  else
#endif
    ui.useExternalPlayer->setChecked( p.useExternalPlayer );

  ui.audioPlaybackProgram->setText( p.audioPlaybackProgram );

  if ( !isRECORDBroken() )
    ui.brokenXRECORD->hide();

  // Proxy server

  ui.useProxyServer->setChecked( p.proxyServer.enabled );

  ui.proxyType->addItem( "SOCKS5" );
  ui.proxyType->addItem( "HTTP Transp." );
  ui.proxyType->addItem( "HTTP Caching" );

  ui.proxyType->setCurrentIndex( p.proxyServer.type );

  ui.proxyHost->setText( p.proxyServer.host );
  ui.proxyPort->setValue( p.proxyServer.port );

  ui.proxyUser->setText( p.proxyServer.user );
  ui.proxyPassword->setText( p.proxyServer.password );

  if( p.proxyServer.useSystemProxy )
  {
    ui.systemProxy->setChecked( true );
    ui.customSettingsGroup->setEnabled( false );
  }
  else
  {
    ui.customProxy->setChecked( true );
    ui.customSettingsGroup->setEnabled( p.proxyServer.enabled );
  }

  connect( ui.customProxy, SIGNAL( toggled( bool ) ),
           this, SLOT( customProxyToggled( bool ) ) );

  connect( ui.useProxyServer, SIGNAL( toggled( bool ) ),
           this, SLOT( customProxyToggled( bool ) ) );

  ui.checkForNewReleases->setChecked( p.checkForNewReleases );
  ui.disallowContentFromOtherSites->setChecked( p.disallowContentFromOtherSites );
  ui.enableWebPlugins->setChecked( p.enableWebPlugins );
  ui.hideGoldenDictHeader->setChecked( p.hideGoldenDictHeader );

  // Add-on styles
  ui.addonStylesLabel->setVisible( ui.addonStyles->count() > 1 );
  ui.addonStyles->setCurrentStyle( p.addonStyle );

  // Full-text search parameters
  ui.ftsGroupBox->setChecked( p.fts.enabled );

  ui.allowAard->setChecked( !p.fts.disabledTypes.contains( "AARD", Qt::CaseInsensitive ) );
  ui.allowBGL->setChecked( !p.fts.disabledTypes.contains( "BGL", Qt::CaseInsensitive ) );
  ui.allowDictD->setChecked( !p.fts.disabledTypes.contains( "DICTD", Qt::CaseInsensitive ) );
  ui.allowDSL->setChecked( !p.fts.disabledTypes.contains( "DSL", Qt::CaseInsensitive ) );
  ui.allowMDict->setChecked( !p.fts.disabledTypes.contains( "MDICT", Qt::CaseInsensitive ) );
  ui.allowSDict->setChecked( !p.fts.disabledTypes.contains( "SDICT", Qt::CaseInsensitive ) );
  ui.allowSlob->setChecked( !p.fts.disabledTypes.contains( "SLOB", Qt::CaseInsensitive ) );
  ui.allowStardict->setChecked( !p.fts.disabledTypes.contains( "STARDICT", Qt::CaseInsensitive ) );
  ui.allowXDXF->setChecked( !p.fts.disabledTypes.contains( "XDXF", Qt::CaseInsensitive ) );
  ui.allowZim->setChecked( !p.fts.disabledTypes.contains( "ZIM", Qt::CaseInsensitive ) );
  ui.allowEpwing->setChecked( !p.fts.disabledTypes.contains( "EPWING", Qt::CaseInsensitive ) );
  ui.allowGls->setChecked( !p.fts.disabledTypes.contains( "GLS", Qt::CaseInsensitive ) );
#ifndef MAKE_ZIM_SUPPORT
  ui.allowZim->hide();
  ui.allowSlob->hide();
#endif
#ifdef NO_EPWING_SUPPORT
  ui.allowEpwing->hide();
#endif
  ui.maxDictionarySize->setValue( p.fts.maxDictionarySize );
}

Config::Preferences Preferences::getPreferences()
{
  Config::Preferences p;

  p.interfaceLanguage =
    ui.interfaceLanguage->itemData(
      ui.interfaceLanguage->currentIndex() ).toString();

  p.helpLanguage =
    ui.helpLanguage->itemData(
      ui.helpLanguage->currentIndex() ).toString();

  p.displayStyle =
    ui.displayStyle->itemData(
      ui.displayStyle->currentIndex() ).toString();

  p.newTabsOpenAfterCurrentOne = ui.newTabsOpenAfterCurrentOne->isChecked();
  p.newTabsOpenInBackground = ui.newTabsOpenInBackground->isChecked();
  p.hideSingleTab = ui.hideSingleTab->isChecked();
  p.mruTabOrder = ui.mruTabOrder->isChecked();
  p.enableTrayIcon = ui.enableTrayIcon->isChecked();
  p.startToTray = ui.startToTray->isChecked();
  p.closeToTray = ui.closeToTray->isChecked();
  p.autoStart = ui.cbAutostart->isChecked();
  p.doubleClickTranslates = ui.doubleClickTranslates->isChecked();
  p.selectWordBySingleClick = ui.selectBySingleClick->isChecked();
  p.escKeyHidesMainWindow = ui.escKeyHidesMainWindow->isChecked();

  p.enableMainWindowHotkey = ui.enableMainWindowHotkey->isChecked();
  p.mainWindowHotkey = ui.mainWindowHotkey->getHotKey();
  p.enableClipboardHotkey = ui.enableClipboardHotkey->isChecked();
  p.clipboardHotkey = ui.clipboardHotkey->getHotKey();

  p.enableScanPopup = ui.enableScanPopup->isChecked();
  p.startWithScanPopupOn = ui.startWithScanPopupOn->isChecked();
  p.enableScanPopupModifiers = ui.enableScanPopupModifiers->isChecked();

  p.scanPopupModifiers += ui.altKey->isChecked() ? KeyboardState::Alt : 0;
  p.scanPopupModifiers += ui.ctrlKey->isChecked() ? KeyboardState::Ctrl: 0;
  p.scanPopupModifiers += ui.shiftKey->isChecked() ? KeyboardState::Shift: 0;
  p.scanPopupModifiers += ui.winKey->isChecked() ? KeyboardState::Win: 0;
  p.scanPopupModifiers += ui.leftAlt->isChecked() ? KeyboardState::LeftAlt: 0;
  p.scanPopupModifiers += ui.rightAlt->isChecked() ? KeyboardState::RightAlt: 0;
  p.scanPopupModifiers += ui.leftCtrl->isChecked() ? KeyboardState::LeftCtrl: 0;
  p.scanPopupModifiers += ui.rightCtrl->isChecked() ? KeyboardState::RightCtrl: 0;
  p.scanPopupModifiers += ui.leftShift->isChecked() ? KeyboardState::LeftShift: 0;
  p.scanPopupModifiers += ui.rightShift->isChecked() ? KeyboardState::RightShift: 0;

  p.scanPopupAltMode = ui.scanPopupAltMode->isChecked();
  p.scanPopupAltModeSecs = ui.scanPopupAltModeSecs->value();
  p.scanToMainWindow = ui.scanToMainWindow->isChecked();
  p.showScanFlag= ui.showScanFlag->isChecked();
  p.scanPopupUseUIAutomation = ui.scanPopupUseUIAutomation->isChecked();
  p.scanPopupUseIAccessibleEx = ui.scanPopupUseIAccessibleEx->isChecked();
  p.scanPopupUseGDMessage = ui.scanPopupUseGDMessage->isChecked();

  p.storeHistory = ui.storeHistory->isChecked();
  p.maxStringsInHistory = ui.historyMaxSizeField->text().toUInt();
  p.historyStoreInterval = ui.historySaveIntervalField->text().toUInt();
  p.alwaysExpandOptionalParts = ui.alwaysExpandOptionalParts->isChecked();

  p.favoritesStoreInterval = ui.favoritesSaveIntervalField->text().toUInt();
  p.confirmFavoritesDeletion = ui.confirmFavoritesDeletion->isChecked();

  p.collapseBigArticles = ui.collapseBigArticles->isChecked();
  p.articleSizeLimit = ui.articleSizeLimit->text().toInt();

  p.synonymSearchEnabled = ui.synonymSearchEnabled->isChecked();

  p.maxDictionaryRefsInContextMenu = ui.maxDictsInContextMenu->text().toInt();

  p.pronounceOnLoadMain = ui.pronounceOnLoadMain->isChecked();
  p.pronounceOnLoadPopup = ui.pronounceOnLoadPopup->isChecked();
  p.useExternalPlayer = ui.useExternalPlayer->isChecked();
  p.useInternalPlayer = ui.useInternalPlayer->isChecked();
  p.audioPlaybackProgram = ui.audioPlaybackProgram->text();

  p.proxyServer.enabled = ui.useProxyServer->isChecked();
  p.proxyServer.useSystemProxy = ui.systemProxy->isChecked();

  p.proxyServer.type = ( Config::ProxyServer::Type ) ui.proxyType->currentIndex();

  p.proxyServer.host = ui.proxyHost->text();
  p.proxyServer.port = ( unsigned ) ui.proxyPort->value();

  p.proxyServer.user = ui.proxyUser->text();
  p.proxyServer.password = ui.proxyPassword->text();

  p.checkForNewReleases = ui.checkForNewReleases->isChecked();
  p.disallowContentFromOtherSites = ui.disallowContentFromOtherSites->isChecked();
  p.enableWebPlugins = ui.enableWebPlugins->isChecked();
  p.hideGoldenDictHeader = ui.hideGoldenDictHeader->isChecked();

  p.addonStyle = ui.addonStyles->getCurrentStyle();

  p.fts.enabled = ui.ftsGroupBox->isChecked();
  p.fts.maxDictionarySize = ui.maxDictionarySize->value();

  if( !ui.allowAard->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "AARD";
  }

  if( !ui.allowBGL->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "BGL";
  }

  if( !ui.allowDictD->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "DICTD";
  }

  if( !ui.allowDSL->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "DSL";
  }

  if( !ui.allowMDict->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "MDICT";
  }

  if( !ui.allowSDict->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "SDICT";
  }

  if( !ui.allowSlob->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "SLOB";
  }

  if( !ui.allowStardict->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "STARDICT";
  }

  if( !ui.allowXDXF->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "XDXF";
  }

  if( !ui.allowZim->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "ZIM";
  }

  if( !ui.allowEpwing->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "EPWING";
  }

  if( !ui.allowGls->isChecked() )
  {
    if( !p.fts.disabledTypes.isEmpty() )
      p.fts.disabledTypes += ',';
    p.fts.disabledTypes += "GLS";
  }

  return p;
}

void Preferences::enableScanPopupToggled( bool b )
{
  ui.scanPopupModifiers->setEnabled( b && ui.enableScanPopupModifiers->isChecked() );
}

void Preferences::enableScanPopupModifiersToggled( bool b )
{
  ui.scanPopupModifiers->setEnabled( b && ui.enableScanPopup->isChecked() );
}

void Preferences::wholeAltClicked( bool b )
{
  if ( b )
  {
    ui.leftAlt->setChecked( false );
    ui.rightAlt->setChecked( false );
  }
}

void Preferences::wholeCtrlClicked( bool b )
{
  if ( b )
  {
    ui.leftCtrl->setChecked( false );
    ui.rightCtrl->setChecked( false );
  }
}

void Preferences::wholeShiftClicked( bool b )
{
  if ( b )
  {
    ui.leftShift->setChecked( false );
    ui.rightShift->setChecked( false );
  }
}

void Preferences::sideAltClicked( bool )
{
  if ( ui.leftAlt->isChecked() || ui.rightAlt->isChecked() )
    ui.altKey->setChecked( false );
}

void Preferences::sideCtrlClicked( bool )
{
  if ( ui.leftCtrl->isChecked() || ui.rightCtrl->isChecked() )
    ui.ctrlKey->setChecked( false );
}

void Preferences::sideShiftClicked( bool )
{
  if ( ui.leftShift->isChecked() || ui.rightShift->isChecked() )
    ui.shiftKey->setChecked( false );
}

void Preferences::on_enableMainWindowHotkey_toggled( bool checked )
{
  ui.mainWindowHotkey->setEnabled( checked );
}

void Preferences::on_enableClipboardHotkey_toggled( bool checked )
{
  ui.clipboardHotkey->setEnabled( checked );
}

void Preferences::on_buttonBox_accepted()
{
  if ( prevInterfaceLanguage != ui.interfaceLanguage->currentIndex() )
    QMessageBox::information( this, tr( "Changing Language" ),
                              tr( "Restart the program to apply the language change." ) );
}

void Preferences::on_useExternalPlayer_toggled( bool enabled )
{
  ui.audioPlaybackProgram->setEnabled( enabled );
}

void Preferences::customProxyToggled( bool )
{
  ui.customSettingsGroup->setEnabled( ui.customProxy->isChecked()
                                      && ui.useProxyServer->isChecked() );
}

void Preferences::helpRequested()
{
  if( !helpWindow )
  {
    MainWindow * mainWindow = qobject_cast< MainWindow * >( parentWidget() );
    if( mainWindow )
      mainWindow->closeGDHelp();

    helpWindow = new Help::HelpWindow( this, cfg );

    if( helpWindow )
    {
      helpWindow->setWindowFlags( Qt::Window );

      connect( helpWindow, SIGNAL( needClose() ),
               this, SLOT( closeHelp() ) );
      helpWindow->showHelpFor( "Preferences" );
      helpWindow->show();
    }
  }
  else
  {
    if( !helpWindow->isVisible() )
      helpWindow->show();

    helpWindow->activateWindow();
  }
}

void Preferences::closeHelp()
{
  if( helpWindow )
    helpWindow->hide();
}
