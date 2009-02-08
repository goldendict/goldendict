/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEVIEW_HH_INCLUDED__
#define __ARTICLEVIEW_HH_INCLUDED__

#include <QWebView>
#include <QUrl>
#include "article_netmgr.hh"
#include "instances.hh"
#include "ui_articleview.h"

/// A widget with the web view tailored to view and handle articles -- it
/// uses the appropriate netmgr, handles link clicks, rmb clicks etc
class ArticleView: public QFrame
{
  Q_OBJECT

  ArticleNetworkAccessManager & articleNetMgr;
  Instances::Groups const & groups;
  bool popupView;

  Ui::ArticleView ui;

#ifdef Q_OS_WIN32
  // Used in Windows only
  vector< char > winWavData;
#endif

  // For resources opened via desktop services
  sptr< QTemporaryFile > desktopOpenedTempFile;

public:
  /// The popupView flag influences contents of the context menus to be
  /// appropriate to the context of the view.
  /// The groups aren't copied -- rather than that, the reference is kept
  ArticleView( QWidget * parent,
               ArticleNetworkAccessManager &,
               Instances::Groups const &,
               bool popupView );

  ~ArticleView();

  /// Shows the definition of the given word with the given group
  void showDefinition( QString const & word, QString const & group );

  /// Shows the page stating that the given word could not be found.
  void showNotFound( QString const & word, QString const & group );

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

signals:

  void iconChanged( ArticleView *, QIcon const & icon );

  void titleChanged( ArticleView *, QString const & title );

  /// Singals that the following link was requested to be opened in new tab
  void openLinkInNewTab( QUrl const &, QUrl const & referrer );  
  /// Singals that the following definition was requested to be showed in new tab
  void showDefinitionInNewTab( QString const & word, QString const & group );

private slots:

  void handleTitleChanged( QString const & title );
  void handleUrlChanged( QUrl const & url );
  void linkClicked( QUrl const & );
  void contextMenuRequested( QPoint const & );

private:

  /// Deduces group from the url. If there doesn't seem to be any group,
  /// returns empty string.
  QString getGroup( QUrl const & );

protected:

  // We need this to hide the search bar when we're showed
  void showEvent( QShowEvent * );
};

#endif
