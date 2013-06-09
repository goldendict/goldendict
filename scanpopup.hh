/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SCANPOPUP_HH_INCLUDED__
#define __SCANPOPUP_HH_INCLUDED__

#include "article_netmgr.hh"
#include "articleview.hh"
#include "wordfinder.hh"
#include "keyboardstate.hh"
#include "config.hh"
#include "ui_scanpopup.h"
#include <QDialog>
#include <QClipboard>
#include "history.hh"
#include "dictionarybar.hh"
#include "mainstatusbar.hh"

/// This is a popup dialog to show translations when clipboard scanning mode
/// is enabled.
class ScanPopup: public QMainWindow, KeyboardState
{
  Q_OBJECT

public:

  ScanPopup( QWidget * parent,
             Config::Class & cfg,
             ArticleNetworkAccessManager &,
             std::vector< sptr< Dictionary::Class > > const & allDictionaries,
             Instances::Groups const &,
             History & );

  ~ScanPopup();
  
  /// Enables scanning. When the object is created, the scanning is disabled
  /// initially.
  void enableScanning();
  /// Disables scanning.
  void disableScanning();

  /// Applies current zoom factor to the popup's view. Should be called when
  /// it's changed.
  void applyZoomFactor();
  /// Translate the word
  void translateWord( QString const & word );

  void setDictionaryIconSize();

signals:

  /// Forwarded from the dictionary bar, so that main window could act on this.
  void editGroupRequested( unsigned id );
  /// Send word to main window
  void sendWordToMainWindow( QString const & word );
  /// Close opened menus when window hide
  void closeMenu();
  /// Signals to set expand optional parts mode (retranslation from/to MainWindow and dictionary bar)
  void setExpandMode( bool expand );
  void setViewExpandMode( bool expand );
  /// Signal to switch expand optional parts mode
  void switchExpandMode();
  /// Signal to add word to history even if history is disabled
  void forceAddWordToHistory( const QString & word);
  /// Retranslate signal from dictionary bar
  void showDictionaryInfo( QString const & id );
  void openDictionaryFolder( QString const & id );
  /// Put translated word into history
  void sendWordToHistory( QString const & word );

#ifdef Q_OS_WIN32
  /// Ask for source window is current translate tab
  bool isGoldenDictWindow( HWND hwnd );
#endif

public slots:
  void requestWindowFocus();

  /// Translates the word from the clipboard, showing the window etc.
  void translateWordFromClipboard();
  /// Translates the word from the clipboard selection
  void translateWordFromSelection();
  /// From the dictionary bar.
  void editGroupRequested();

private:

  // Translates the word from the clipboard or the clipboard selection
  void translateWordFromClipboard(QClipboard::Mode m);

  // Hides the popup window, effectively closing it.
  void hideWindow();

  // Grabs mouse and installs global event filter to track it thoroughly.
  void interceptMouse();
  // Ungrabs mouse and uninstalls global event filter.
  void uninterceptMouse();

  void updateDictionaryBar();

  Config::Class & cfg;
  bool isScanningEnabled;
  std::vector< sptr< Dictionary::Class > > const & allDictionaries;
  std::vector< sptr< Dictionary::Class > > dictionariesUnmuted;
  Instances::Groups const & groups;
  History & history;
  Ui::ScanPopup ui;
  ArticleView * definition;
  QAction escapeAction, switchExpandModeAction, focusTranslateLineAction;
  QString pendingInputWord, inputWord;
  WordFinder wordFinder;
  Config::Events configEvents;
  DictionaryBar dictionaryBar;
  MainStatusBar * mainStatusBar;

  bool mouseEnteredOnce;
  bool mouseIntercepted;

  QPoint startPos; // For window moving

  QTimer hideTimer; // When mouse leaves the window, a grace period is
                    // given for it to return back. If it doesn't before
                    // this timer expires, the window gets hidden.
  QTimer altModeExpirationTimer, altModePollingTimer; // Timers for alt mode

  QTimer mouseGrabPollTimer;

  void handleInputWord( QString const & , bool forcePopup = false );
  void engagePopup( bool forcePopup, bool giveFocus = false );

  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  virtual bool eventFilter( QObject * watched, QEvent * event );

  /// Called from event filter or from mouseGrabPoll to handle mouse event
  /// while it is being intercepted.
  void reactOnMouseMove( QPoint const & p );

  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void leaveEvent( QEvent * event );
  virtual void enterEvent( QEvent * event );
  virtual void showEvent( QShowEvent * );

  /// Returns inputWord, chopped with appended ... if it's too long/
  QString elideInputWord();

  void updateBackForwardButtons();

  void showTranslationFor( QString const & inputWord );

private slots:

  void clipboardChanged( QClipboard::Mode );
  void mouseHovered( QString const & , bool forcePopup);
  void currentGroupChanged( QString const & );
  void prefixMatchFinished();
  void on_pronounceButton_clicked();
  void pinButtonClicked( bool checked );
  void on_showDictionaryBar_clicked( bool checked );
  void showStatusBarMessage ( QString const &, int, QPixmap const & );
  void on_sendWordButton_clicked();
  void on_goBackButton_clicked();
  void on_goForwardButton_clicked();

  void hideTimerExpired();
  void altModeExpired();
  void altModePoll();

  /// Called repeatedly once the popup is initially engaged and we monitor the
  /// mouse as it may move away from the window. This simulates mouse grab, in
  /// essense, but seems more reliable. Once the mouse enters the window, the
  /// polling stops.
  void mouseGrabPoll();

  void pageLoaded( ArticleView * );

  void escapePressed();

  void mutedDictionariesChanged();

  void switchExpandOptionalPartsMode();

  void translateInputChanged(QString const & text);
  void translateInputFinished();
  void wordListItemActivated( QListWidgetItem * );

  void focusTranslateLine();

  void typingEvent( QString const & );
};

#endif
