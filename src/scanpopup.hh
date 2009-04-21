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

/// This is a popup dialog to show translations when clipboard scanning mode
/// is enabled.
class ScanPopup: public QDialog, KeyboardState
{
  Q_OBJECT

public:

  ScanPopup( QWidget * parent,
             Config::Class & cfg,
             ArticleNetworkAccessManager &,
             std::vector< sptr< Dictionary::Class > > const & allDictionaries,
             Instances::Groups const & );

  ~ScanPopup();
  
  /// Enables scanning. When the object is created, the scanning is disabled
  /// initially.
  void enableScanning();
  /// Disables scanning.
  void disableScanning();

public slots:

  /// Translates the word from the clipboard, showing the window etc.
  void translateWordFromClipboard();

private:

  Config::Class & cfg;
  bool isScanningEnabled;
  std::vector< sptr< Dictionary::Class > > const & allDictionaries;
  Instances::Groups const & groups;
  Ui::ScanPopup ui;
  ArticleView * definition;
  QString pendingInputWord, inputWord;
  WordFinder wordFinder;

  bool mouseEnteredOnce;

  QPoint startPos; // For window moving

  QTimer hideTimer; // When mouse leaves the window, a grace period is
                    // given for it to return back. If it doesn't before
                    // this timer expires, the window gets hidden.
  QTimer altModeExpirationTimer, altModePollingTimer; // Timers for alt mode

  void handleInputWord( QString const & );
  void engagePopup();
  void initiateTranslation();

  vector< sptr< Dictionary::Class > > const & getActiveDicts();

  virtual void mousePressEvent( QMouseEvent * );
  virtual void mouseMoveEvent( QMouseEvent * );
  virtual void mouseReleaseEvent( QMouseEvent * );
  virtual void leaveEvent( QEvent * event );
  virtual void enterEvent( QEvent * event );
  virtual void resizeEvent( QResizeEvent * event );
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

  void hideTimerExpired();
  void altModeExpired();
  void altModePoll();

  void pageLoaded();
};

#endif
