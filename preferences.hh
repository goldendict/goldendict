#ifndef __PREFERENCES_HH_INCLUDED__
#define __PREFERENCES_HH_INCLUDED__

#include <QDialog>
#include "config.hh"
#include "helpwindow.hh"
#include "ui_preferences.h"

/// Preferences dialog -- allows changing various program options.
class Preferences: public QDialog
{
  Q_OBJECT

  int prevInterfaceLanguage;
  bool prevOffTheRecordWebProfile;

  Help::HelpWindow * helpWindow;
  Config::Class & cfg;
  QAction helpAction;

public:

  Preferences( QWidget * parent, Config::Class & cfg_ );
  ~Preferences()
  { if( helpWindow ) delete helpWindow; }

  Config::Preferences getPreferences();

private:

  Ui::Preferences ui;

private slots:

  void enableScanPopupToggled( bool );
  void enableScanPopupModifiersToggled( bool );
  void showScanFlagToggled( bool b );
  void on_scanPopupUnpinnedWindowFlags_currentIndexChanged( int index );

  void wholeAltClicked( bool );
  void wholeCtrlClicked( bool );
  void wholeShiftClicked( bool );

  void sideAltClicked( bool );
  void sideCtrlClicked( bool );
  void sideShiftClicked( bool );

  void on_enableMainWindowHotkey_toggled( bool checked );
  void on_enableClipboardHotkey_toggled( bool checked );

  void on_buttonBox_accepted();

  void on_useExternalPlayer_toggled( bool enabled );

  void customProxyToggled( bool );
  void on_maxNetworkCacheSize_valueChanged( int value );

  void on_collapseBigArticles_toggled( bool checked );
  void on_limitInputPhraseLength_toggled( bool checked );

  void helpRequested();
  void closeHelp();
};

#endif

