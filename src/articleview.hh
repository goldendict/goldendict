/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEVIEW_HH_INCLUDED__
#define __ARTICLEVIEW_HH_INCLUDED__

#include <QWebView>
#include <QUrl>
#include <list>
#include "article_netmgr.hh"
#include "instances.hh"
#include "ui_articleview.h"

/// A widget with the web view tailored to view and handle articles -- it
/// uses the appropriate netmgr, handles link clicks, rmb clicks etc
class ArticleView: public QFrame
{
  Q_OBJECT

  ArticleNetworkAccessManager & articleNetMgr;
  std::vector< sptr< Dictionary::Class > > const & allDictionaries;
  Instances::Groups const & groups;
  bool popupView;
  Config::Class const & cfg;

  Ui::ArticleView ui;

#ifdef Q_OS_WIN32
  // Used in Windows only
  vector< char > winWavData;
#endif

  /// Any resource we've decided to download off the dictionary gets stored here.
  /// Full vector capacity is used for search requests, where we have to make
  /// a multitude of requests.
  std::list< sptr< Dictionary::DataRequest > > resourceDownloadRequests;
  /// Url of the resourceDownloadRequests
  QUrl resourceDownloadUrl;

  /// For resources opened via desktop services
  QString desktopOpenedTempFile;

public:
  /// The popupView flag influences contents of the context menus to be
  /// appropriate to the context of the view.
  /// The groups aren't copied -- rather than that, the reference is kept
  ArticleView( QWidget * parent,
               ArticleNetworkAccessManager &,
               std::vector< sptr< Dictionary::Class > > const & allDictionaries,
               Instances::Groups const &,
               bool popupView,
               Config::Class const & cfg );

  ~ArticleView();

  /// Shows the definition of the given word with the given group
  void showDefinition( QString const & word, unsigned group );

  /// Clears the view and sets the application-global waiting cursor,
  /// which will be restored when some article loads eventually.
  void showAnticipation();

  /// Opens the given link. Supposed to be used in response to
  /// openLinkInNewTab() signal. The link scheme is therefore supposed to be
  /// one of the internal ones.
  void openLink( QUrl const & url, QUrl const & referrer );

  /// Goes back in history
  void back()
  { ui.definition->back(); }

  /// Goes forward in history
  void forward()
  { ui.definition->forward(); }

  /// Takes the focus to the view
  void focus()
  { ui.definition->setFocus( Qt::ShortcutFocusReason ); }

  /// Returns true if there's an audio reference on the page, false otherwise.
  bool hasSound();

  /// Plays the first audio reference on the page, if any.
  void playSound();

  void setZoomFactor( qreal factor )
  { ui.definition->setZoomFactor( factor ); }

  /// Returns current article's text in .html format
  QString toHtml();

  /// Returns current article's title
  QString getTitle();

  /// Prints current article
  void print( QPrinter * ) const;
  
signals:

  void iconChanged( ArticleView *, QIcon const & icon );

  void titleChanged( ArticleView *, QString const & title );

  void pageLoaded();

  /// Singals that the following link was requested to be opened in new tab
  void openLinkInNewTab( QUrl const &, QUrl const & referrer );
  /// Singals that the following definition was requested to be showed in new tab
  void showDefinitionInNewTab( QString const & word, unsigned group );

private slots:

  void loadFinished( bool ok );
  void handleTitleChanged( QString const & title );
  void handleUrlChanged( QUrl const & url );
  void linkClicked( QUrl const & );
  void contextMenuRequested( QPoint const & );

  void resourceDownloadFinished();

private:

  /// Deduces group from the url. If there doesn't seem to be any group,
  /// returns 0.
  unsigned getGroup( QUrl const & );

  /// Attempts removing last temporary file created.
  void cleanupTemp();

protected:

  // We need this to hide the search bar when we're showed
  void showEvent( QShowEvent * );
};

#endif
