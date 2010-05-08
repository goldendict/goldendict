/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
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
  
signals:

  /// Forwarded from the dictionary bar, so that main window could act on this.
  void editGroupRequested( unsigned id );

public slots:

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
  QAction escapeAction;
  QString pendingInputWord, inputWord;
  WordFinder wordFinder;
  Config::Events configEvents;
  DictionaryBar dictionaryBar;

  bool mouseEnteredOnce;
  bool mouseIntercepted;

  QPoint startPos; // For window moving

  QTimer hideTimer; // When mouse leaves the window, a grace period is
                    // given for it to return back. If it doesn't before
                    // this timer expires, the window gets hidden.
  QTimer altModeExpirationTimer, altModePollingTimer; // Timers for alt mode

  void handleInputWord( QString const & );
  void engagePopup( bool giveFocus = false );
  void initiateTranslation();

  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  virtual bool eventFilter( QObject * watched, QEvent * event );
  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void leaveEvent( QEvent * event );
  virtual void enterEvent( QEvent * event );
  virtual void showEvent( QShowEvent * );

  /// Returns inputWord, chopped with appended ... if it's too long/
  QString elideInputWord();

private slots:

  void clipboardChanged( QClipboard::Mode );
  void mouseHovered( QString const & );
  void currentGroupChanged( QString const & );
  void prefixMatchFinished();
  void on_wordListButton_clicked();
  void on_pronounceButton_clicked();
  void pinButtonClicked( bool checked );
  void on_showDictionaryBar_clicked( bool checked );

  void hideTimerExpired();
  void altModeExpired();
  void altModePoll();

  void pageLoaded( ArticleView * );

  void escapePressed();

  void mutedDictionariesChanged();
};

#endif
