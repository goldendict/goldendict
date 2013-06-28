/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEVIEW_HH_INCLUDED__
#define __ARTICLEVIEW_HH_INCLUDED__

#include <QWebView>
#include <QUrl>
#include <QMap>
#include <list>
#include "article_netmgr.hh"
#include "instances.hh"
#include "groupcombobox.hh"
#include "ui_articleview.h"

class ResourceToSaveHandler;

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

  QAction pasteAction, articleUpAction, articleDownAction,
          goBackAction, goForwardAction, openSearchAction, selectCurrentArticleAction,
          copyAsTextAction, inspectAction;
  bool searchIsOpened;
  bool expandOptionalParts;
  QString articleToJump;

  /// Any resource we've decided to download off the dictionary gets stored here.
  /// Full vector capacity is used for search requests, where we have to make
  /// a multitude of requests.
  std::list< sptr< Dictionary::DataRequest > > resourceDownloadRequests;
  /// Url of the resourceDownloadRequests
  QUrl resourceDownloadUrl;

  /// For resources opened via desktop services
  QString desktopOpenedTempFile;

  QAction * dictionaryBarToggled;
  GroupComboBox const * groupComboBox;

public:
  /// The popupView flag influences contents of the context menus to be
  /// appropriate to the context of the view.
  /// The groups aren't copied -- rather than that, the reference is kept
  ArticleView( QWidget * parent,
               ArticleNetworkAccessManager &,
               std::vector< sptr< Dictionary::Class > > const & allDictionaries,
               Instances::Groups const &,
               bool popupView,
               Config::Class const & cfg,
               QAction * dictionaryBarToggled = 0,
               GroupComboBox const * groupComboBox = 0 );

  /// Sets the currently active group combo box. When looking up selections,
  /// this allows presenting a choice of looking up in the currently chosen
  /// group. Setting this to 0 disables this. It is 0 by default.
  void setGroupComboBox( GroupComboBox const * );

  virtual QSize minimumSizeHint() const;

  ~ArticleView();

  typedef QMap< QString, QString > Contexts;

  /// Shows the definition of the given word with the given group.
  /// scrollTo can be optionally set to a "gdfrom-xxxx" identifier to position
  /// the page to that article on load.
  /// contexts is an optional map of context values to be passed for dictionaries.
  /// The only values to pass here are ones obtained from showDefinitionInNewTab()
  /// signal or none at all.
  void showDefinition( QString const & word, unsigned group,
                       QString const & scrollTo = QString(),
                       Contexts const & contexts = Contexts() );

  /// Clears the view and sets the application-global waiting cursor,
  /// which will be restored when some article loads eventually.
  void showAnticipation();

  /// Opens the given link. Supposed to be used in response to
  /// openLinkInNewTab() signal. The link scheme is therefore supposed to be
  /// one of the internal ones.
  /// contexts is an optional map of context values to be passed for dictionaries.
  /// The only values to pass here are ones obtained from showDefinitionInNewTab()
  /// signal or none at all.
  void openLink( QUrl const & url, QUrl const & referrer,
                 QString const & scrollTo = QString(),
                 Contexts const & contexts = Contexts() );

  /// Called when the state of dictionary bar changes and the view is active.
  /// The function reloads content if the change affects it.
  void updateMutedContents();

  bool canGoBack();
  bool canGoForward();

  /// Called when preference changes
  void setSelectionBySingleClick( bool set );

public slots:

  /// Goes back in history
  void back();

  /// Goes forward in history
  void forward();

public:

  /// Takes the focus to the view
  void focus()
  { ui.definition->setFocus( Qt::ShortcutFocusReason ); }

  /// Reloads the view
  void reload()
  { ui.definition->reload(); }

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

  /// Closes search if it's open and returns true. Returns false if it
  /// wasn't open.
  bool closeSearch();

  bool isSearchOpened();

  /// Jumps to the article specified by the dictionary id,
  /// by executing a javascript code.
  void jumpToDictionary( QString const &, bool force );

  /// Returns all articles currently present in view, as a list of dictionary
  /// string ids.
  QStringList getArticlesList();

  /// Returns the dictionary id of the currently active article in the view.
  QString getActiveArticleId();

  std::vector< ResourceToSaveHandler * > saveResource( const QUrl & url, const QString & fileName );
  std::vector< ResourceToSaveHandler * > saveResource( const QUrl & url, const QUrl & ref, const QString & fileName );

signals:

  void iconChanged( ArticleView *, QIcon const & icon );

  void titleChanged( ArticleView *, QString const & title );

  void pageLoaded( ArticleView * );

  /// Signals that the following link was requested to be opened in new tab
  void openLinkInNewTab( QUrl const &, QUrl const & referrer,
                         QString const & fromArticle,
                         ArticleView::Contexts const & contexts );
  /// Signals that the following definition was requested to be showed in new tab
  void showDefinitionInNewTab( QString const & word, unsigned group,
                               QString const & fromArticle,
                               ArticleView::Contexts const & contexts );

  /// Put translated word into history
  void sendWordToHistory( QString const & word );

  /// Emitted when user types a text key. This should typically be used to
  /// switch focus to word input.
  void typingEvent( QString const & text );

  void statusBarMessage( QString const & message, int timeout = 0, QPixmap const & pixmap = QPixmap());

  /// Signals that the dictionaries pane was requested to be showed
  void showDictsPane( );

  /// Emitted when an article becomes active,
  /// typically in response to user actions
  /// (clicking on the article or using shortcuts).
  /// id - the dictionary id of the active article.
  void activeArticleChanged ( QString const & id );

  /// Signal to add word to history even if history is disabled
  void forceAddWordToHistory( const QString & word);

  /// Signal to close popup menu
  void closePopupMenu();

  /// Signal to set optional parts expand mode
  void setExpandMode ( bool  expand );

  void sendWordToInputLine( QString const & word );

  void storeResourceSavePath(QString const & );

public slots:

  void on_searchPrevious_clicked();
  void on_searchNext_clicked();

  void onJsActiveArticleChanged(QString const & id);

  /// Handles F3 and Shift+F3 for search navigation
  bool handleF3( QObject * obj, QEvent * ev );

  /// Control optional parts expanding
  void receiveExpandOptionalParts( bool expand );
  void switchExpandOptionalParts();

  /// Selects an entire text of the current article
  void selectCurrentArticle();

private slots:

  void loadFinished( bool ok );
  void handleTitleChanged( QString const & title );
  void handleUrlChanged( QUrl const & url );
  void attachToJavaScript();
  void linkClicked( QUrl const & );
  void linkHovered( const QString & link, const QString & title, const QString & textContent );
  void contextMenuRequested( QPoint const & );

  void resourceDownloadFinished();

  /// We handle pasting by attempting to define the word in clipboard.
  void pasteTriggered();

  /// Nagivates to the previous article relative to the active one.
  void moveOneArticleUp();

  /// Nagivates to the next article relative to the active one.
  void moveOneArticleDown();

  /// Opens the search area
  void openSearch();

  void on_searchText_textEdited();
  void on_searchText_returnPressed();
  void on_searchCloseButton_clicked();
  void on_searchCaseSensitive_clicked();
  void on_highlightAllButton_clicked();

  /// Handles the double-click from the definition.
  void doubleClicked();

  /// Handles audio player error message
  void audioPlayerError( QString const & message );

  /// Copy current selection as plain text
  void copyAsText();

  /// Inspect element
  void inspect();

private:

  /// Deduces group from the url. If there doesn't seem to be any group,
  /// returns 0.
  unsigned getGroup( QUrl const & );


  /// Returns current article in the view, in the form of "gdfrom-xxx" id.
  QString getCurrentArticle();

  /// Sets the current article by executing a javascript code.
  /// If moveToIt is true, it moves the focus to it as well.
  void setCurrentArticle( QString const &, bool moveToIt = false );

  /// Checks if the given article in form of "gdfrom-xxx" is inside a "website"
  /// frame.
  bool isFramedArticle( QString const & );

  /// Checks if the given link is to be opened externally, as opposed to opening
  /// it in-place.
  bool isExternalLink( QUrl const & url );

  /// Sees if the last clicked link is from a website frame. If so, changes url
  /// to point to url text translation instead, and saves the original
  /// url to the appropriate "contexts" entry.
  void tryMangleWebsiteClickedUrl( QUrl & url, Contexts & contexts );

  /// Use the known information about the current frame to update the current
  /// article's value.
  void updateCurrentArticleFromCurrentFrame( QWebFrame * frame = 0 );

  /// Saves current article and scroll position for the current history item.
  /// Should be used when leaving the page.
  void saveHistoryUserData();

  /// Attempts removing last temporary file created.
  void cleanupTemp();

  bool eventFilter( QObject * obj, QEvent * ev );

  void performFindOperation( bool restart, bool backwards, bool checkHighlight = false );

  void reloadStyleSheet();

  /// Returns the comma-separated list of dictionary ids which should be muted
  /// for the given group. If there are none, returns empty string.
  QString getMutedForGroup( unsigned group );

protected:

  // We need this to hide the search bar when we're showed
  void showEvent( QShowEvent * );

#ifdef Q_OS_WIN32

  /// Search inside web page for word under cursor

private:
  QString insertSpans( QString const & html );
  void readTag( QString const & from, QString & to, int & count );
  QString checkElement( QWebElement & elem, const QPoint & pt );
public:
  QString wordAtPoint( int x, int y );
#endif

};

class ResourceToSaveHandler: public QObject
{
  Q_OBJECT

public:
  explicit ResourceToSaveHandler( ArticleView * view, sptr< Dictionary::DataRequest > req,
                                  QString const & fileName );

signals:
  void done();
  void statusBarMessage( QString const & message, int timeout = 0, QPixmap const & pixmap = QPixmap() );

private slots:
  void downloadFinished();

private:
  sptr< Dictionary::DataRequest > req;
  QString fileName;
};

#endif
