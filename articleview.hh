/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ARTICLEVIEW_HH_INCLUDED__
#define __ARTICLEVIEW_HH_INCLUDED__

#include <QWebView>
#include <QHash>
#include <QMap>
#include <QUrl>
#include <QSet>
#include <list>
#include "article_netmgr.hh"
#include "audioplayerinterface.hh"
#include "instances.hh"
#include "groupcombobox.hh"
#include "ui_articleview.h"

class ArticleViewJsProxy;
class ResourceToSaveHandler;

/// A widget with the web view tailored to view and handle articles -- it
/// uses the appropriate netmgr, handles link clicks, rmb clicks etc
class ArticleView: public QFrame
{
  Q_OBJECT

  ArticleNetworkAccessManager & articleNetMgr;
  AudioPlayerPtr const & audioPlayer;
  std::vector< sptr< Dictionary::Class > > const & allDictionaries;
  Instances::Groups const & groups;
  bool popupView;
  Config::Class const & cfg;

  Ui::ArticleView ui;

  ArticleViewJsProxy * const jsProxy;

  QStringList articleList; ///< All articles currently present in the view as a list of dictionary ids.
  QHash< QString, QString > audioLinks;
  QString firstAudioLink;
  QString currentArticle; ///< Current article in the view, in the form of "gdfrom-xxx"
                          ///< (scrollTo) id. If empty, there is no current article.

  QAction pasteAction, articleUpAction, articleDownAction,
          goBackAction, goForwardAction, selectCurrentArticleAction,
          copyAsTextAction, inspectAction;
  QAction & openSearchAction;
  bool searchIsOpened;
  bool expandOptionalParts;
  QString rangeVarName;

  /// Any resource we've decided to download off the dictionary gets stored here.
  /// Full vector capacity is used for search requests, where we have to make
  /// a multitude of requests.
  std::list< sptr< Dictionary::DataRequest > > resourceDownloadRequests;
  /// Url of the resourceDownloadRequests
  QUrl resourceDownloadUrl;

  /// For resources opened via desktop services
  QSet< QString > desktopOpenedTempFiles;

  QAction * dictionaryBarToggled;
  GroupComboBox const * groupComboBox;

  /// Search in results of full-text search
  QStringList allMatches;
  QStringList uniqueMatches;
  bool ftsSearchIsOpened, ftsSearchMatchCase;
  int ftsPosition;

  void highlightFTSResults();
  void highlightAllFtsOccurences( QWebPage::FindFlags flags );
  void performFtsFindOperation( bool backwards );

public:
  /// The popupView flag influences contents of the context menus to be
  /// appropriate to the context of the view.
  /// The groups aren't copied -- rather than that, the reference is kept
  ArticleView( QWidget * parent,
               ArticleNetworkAccessManager &,
               AudioPlayerPtr const &,
               std::vector< sptr< Dictionary::Class > > const & allDictionaries,
               Instances::Groups const &,
               bool popupView,
               Config::Class const & cfg,
               QAction & openSearchAction_,
               QAction * dictionaryBarToggled = 0,
               GroupComboBox const * groupComboBox = 0 );

  /// Sets the currently active group combo box. When looking up selections,
  /// this allows presenting a choice of looking up in the currently chosen
  /// group. Setting this to 0 disables this. It is 0 by default.
  void setGroupComboBox( GroupComboBox const * );

  virtual QSize minimumSizeHint() const;

  ~ArticleView();

  typedef QMap< QString, QString > Contexts;

  /// Returns "gdfrom-" + dictionaryId.
  static QString scrollToFromDictionaryId( QString const & dictionaryId );

  /// Shows the definition of the given word with the given group.
  /// scrollTo can be optionally set to a "gdfrom-xxxx" identifier to position
  /// the page to that article on load.
  /// contexts is an optional map of context values to be passed for dictionaries.
  /// The only values to pass here are ones obtained from showDefinitionInNewTab()
  /// signal or none at all.
  void showDefinition( Config::InputPhrase const & phrase, unsigned group,
                       QString const & scrollTo = QString(),
                       Contexts const & contexts = Contexts() );

  void showDefinition( QString const & word, unsigned group,
                       QString const & scrollTo = QString(),
                       Contexts const & contexts = Contexts() );

  void showDefinition( QString const & word, QStringList const & dictIDs,
                       QRegExp const & searchRegExp, unsigned group,
                       bool ignoreDiacritics );

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

  /// Takes the focus to the view
  void focus()
  { ui.definition->setFocus( Qt::ShortcutFocusReason ); }

public:

  /// Reloads the view
  void reload();

  /// Returns true if there's an audio reference on the page, false otherwise.
  bool hasSound() const;

  /// Plays the first audio reference on the page, if any.
  void playSound();

  void setZoomFactor( qreal factor )
  { ui.definition->setZoomFactor( factor ); }

  /// Returns current article's text in .html format
  QString toHtml();

  /// Returns current article's title
  QString getTitle();

  /// Returns the phrase translated by the current article.
  Config::InputPhrase getPhrase() const;

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
  QStringList getArticleList() const;

  /// Returns the dictionary id of the currently active article in the view.
  QString getActiveArticleId();

  void saveResource( QUrl const & url, ResourceToSaveHandler & handler );

  /// Return group id of the view
  unsigned getViewGroup()
  { return getGroup( ui.definition->url() ); }

signals:

  void iconChanged( ArticleView *, QIcon const & icon );

  void titleChanged( ArticleView *, QString const & title );

  void pageUnloaded( ArticleView * );
  void articleLoaded( ArticleView *, QString const & id, bool isActive );
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
  void activeArticleChanged ( ArticleView const *, QString const & id );

  /// Signal to add word to history even if history is disabled
  void forceAddWordToHistory( const QString & word);

  /// Signal to close popup menu
  void closePopupMenu();

  /// Signal to set optional parts expand mode
  void setExpandMode ( bool  expand );

  void sendWordToInputLine( QString const & word );

  void storeResourceSavePath(QString const & );

  void zoomIn();
  void zoomOut();

public slots:

  void on_searchPrevious_clicked();
  void on_searchNext_clicked();

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

  void on_ftsSearchPrevious_clicked();
  void on_ftsSearchNext_clicked();

  /// Handles the double-click from the definition.
  void doubleClicked( QPoint pos );

  /// Handles audio player error message
  void audioPlayerError( QString const & message );

  /// Copy current selection as plain text
  void copyAsText();

  /// Inspect element
  void inspect();

private:
  /// <JavaScript interface>

  friend class ArticleViewJsProxy;

  void onJsPageInitStarted();

  void onJsArticleLoaded( QString const & id, QString const & audioLink, bool isActive );

  void onJsActiveArticleChanged( QString const & id );

  /// </JavaScript interface>

  /// Clears selection on the current web page. This function can be called to
  /// reset the start position of the page's findText().
  void clearPageSelection();

  /// Deduces group from the url. If there doesn't seem to be any group,
  /// returns 0.
  unsigned getGroup( QUrl const & );

  /// Sets the current article by executing a javascript code.
  /// If moveToIt is true, it moves the focus to it as well.
  /// Returns true in case of success, false otherwise.
  bool setCurrentArticle( QString const &, bool moveToIt = false );

  void setValidCurrentArticleNoJs( QString const & id );

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

  void initCurrentArticleAndScroll();

  /// Saves current article and scroll position for the current history item.
  /// Should be used when leaving the page.
  void saveHistoryUserData();

  /// Loads a page at @p url into view.
  void load( QUrl const & url );

  /// Attempts removing last temporary file created.
  void cleanupTemp();

  bool eventFilter( QObject * obj, QEvent * ev );

  void performFindOperation( bool restart, bool backwards, bool checkHighlight = false );

  void reloadStyleSheet();

  /// Returns the comma-separated list of dictionary ids which should be muted
  /// for the given group. If there are none, returns empty string.
  QString getMutedForGroup( unsigned group );

  void saveResource( QUrl const & url, QUrl const & ref, ResourceToSaveHandler * handler );

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

/// Downloads the specified resource and saves it to a file at the specified path.
/// Emits done() and destroys itself using deleteLater() when finished.
class ResourceToSaveHandler: public QObject
{
  Q_OBJECT

public:
  explicit ResourceToSaveHandler( ArticleView * view, QString const & fileName );
  void addRequest( sptr< Dictionary::DataRequest > req );
  bool isEmpty()
  { return downloadRequests.empty(); }

signals:
  /// Is emitted when the resource is downloaded. Allows to modify the resource data before it is saved to a file.
  /// Connect via Qt::DirectConnection, because with Qt::QueuedConnection @p resourceData would be a dangling pointer.
  void downloaded( QString const & fileName, QByteArray * resourceData );
  void done();
  void statusBarMessage( QString const & message, int timeout = 0, QPixmap const & pixmap = QPixmap() );

public slots:
  void downloadFinished();

private:
  std::list< sptr< Dictionary::DataRequest > > downloadRequests;
  QString fileName;
  bool alreadyDone;
};

#endif
