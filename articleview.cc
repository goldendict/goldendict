/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articleview.hh"
#include <map>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileDialog>
#include "articlewebpage.hh"
#include "webkit_or_webengine.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include "webmultimediadownload.hh"
#include "programs.hh"
#include "gddebug.hh"
#include <QDebug>
#include <QCryptographicHash>
#include "gestures.hh"
#include "fulltextsearch.hh"

#ifdef USE_QTWEBKIT
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebHistory>
#include <QWebHitTestResult>
#else
#include <QColor>
#include <QWebChannel>
#include <QWebEngineContextMenuData>
#include <QWebEngineFindTextResult>
#include <QWebEngineHistory>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#endif

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#include "wildcard.hh"
#endif

#include "qt4x5.hh"

#include <assert.h>

#ifdef Q_OS_WIN32
#include <windows.h>

#include <QPainter>
#endif

#include <QBuffer>

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
#include "speechclient.hh"
#endif

using std::map;
using std::list;

/// This class exposes only slim, minimal API to JavaScript clients in order to
/// reduce attack surface available to potentionally malicious external scripts.
class ArticleViewJsProxy: public QObject
{
  Q_OBJECT
public:
  /// Note: view becomes the parent of this proxy object.
  explicit ArticleViewJsProxy( ArticleView & view ):
    QObject( &view ), articleView( view )
  {}

public slots:
#ifdef USE_QTWEBKIT
  void onJsPageInitStarted()
  { articleView.onJsPageInitStarted(); }

  void onJsActiveArticleChanged( QString const & id )
  { articleView.onJsActiveArticleChanged( id ); }
#else
  void onJsPageInitStarted( QStringList const & loadedArticles, QStringList const & loadedAudioLinks,
                            int activeArticleIndex, bool hasPageInitFinished, QDateTime const & pageTimestamp )
  {
    articleView.onJsPageInitStarted( loadedArticles, loadedAudioLinks,
                                     activeArticleIndex, hasPageInitFinished, pageTimestamp );
  }

  void onJsPageInitFinished()
  { articleView.onJsPageInitFinished(); }

  void onJsActiveArticleChanged( QString const & id, QDateTime const & currentArticleTimestamp )
  { articleView.onJsActiveArticleChanged( id, currentArticleTimestamp ); }

  void onJsFirstLeftButtonMouseDown()
  { articleView.onJsFirstLeftButtonMouseDown(); }

  void onJsDoubleClicked( QString const & imageUrl )
  { articleView.onJsDoubleClicked( imageUrl ); }
#endif

  void onJsArticleLoaded( QString const & id, QString const & audioLink, bool isActive )
  { articleView.onJsArticleLoaded( id, audioLink, isActive ); }

  void onJsLocationHashChanged()
  { articleView.onJsLocationHashChanged(); }

private:
  ArticleView & articleView;
};

/// AccentMarkHandler class
///
/// Remove accent marks from text
/// and mirror position in normalized text to original text

class AccentMarkHandler
{
protected:
  QString normalizedString;
  QVector< int > accentMarkPos;
public:
  AccentMarkHandler()
  {}
  virtual ~AccentMarkHandler()
  {}
  static QChar accentMark()
  { return QChar( 0x301 ); }

  /// Create text without accent marks
  /// and store mark positions
  virtual void setText( QString const & baseString )
  {
    accentMarkPos.clear();
    normalizedString.clear();
    int pos = 0;
    QChar mark = accentMark();

    for( int x = 0; x < baseString.length(); x++ )
    {
      if( baseString.at( x ) == mark )
      {
        accentMarkPos.append( pos );
        continue;
      }
      normalizedString.append( baseString.at( x ) );
      pos++;
    }
  }

  /// Return text without accent marks
  QString const & normalizedText() const
  { return normalizedString; }

  /// Convert position into position in original text
  int mirrorPosition( int const & pos ) const
  {
    int newPos = pos;
    for( int x = 0; x < accentMarkPos.size(); x++ )
    {
      if( accentMarkPos.at( x ) < pos )
        newPos++;
      else
        break;
    }
    return newPos;
  }
};

/// End of DslAccentMark class

/// DiacriticsHandler class
///
/// Remove diacritics from text
/// and mirror position in normalized text to original text

class DiacriticsHandler : public AccentMarkHandler
{
public:
  DiacriticsHandler()
  {}
  ~DiacriticsHandler()
  {}

  /// Create text without diacriticss
  /// and store diacritic marks positions
  virtual void setText( QString const & baseString )
  {
    accentMarkPos.clear();
    normalizedString.clear();

    gd::wstring baseText = gd::toWString( baseString );
    gd::wstring normText;

    int pos = 0;
    normText.reserve( baseText.size() );

    gd::wchar const * nextChar = baseText.data();
    size_t consumed;

    for( size_t left = baseText.size(); left; )
    {
      if( *nextChar >= 0x10000 )
      {
        // Will be translated into surrogate pair
        normText.push_back( *nextChar );
        pos += 2;
        nextChar++; left--;
        continue;
      }

      gd::wchar ch = Folding::foldedDiacritic( nextChar, left, consumed );

      if( Folding::isCombiningMark( ch ) )
      {
        accentMarkPos.append( pos );
        nextChar++; left--;
        continue;
      }

      if( consumed > 1 )
      {
        for( size_t i = 1; i < consumed; i++ )
          accentMarkPos.append( pos );
      }

      normText.push_back( ch );
      pos += 1;
      nextChar += consumed;
      left -= consumed;
    }
    normalizedString = gd::toQString( normText );
  }
};

/// End of DiacriticsHandler class

#ifdef USE_QTWEBKIT
static QVariant evaluateJavaScriptVariableSafe( QWebFrame * frame, const QString & variable )
{
  return frame->evaluateJavaScript(
        QString( "( typeof( %1 ) !== 'undefined' && %1 !== undefined ) ? %1 : null;" )
        .arg( variable ) );
}
#endif

namespace {

char const * const scrollToPrefix = "gdfrom-";

bool isScrollTo( QString const & id )
{
  return id.startsWith( scrollToPrefix );
}

QString dictionaryIdFromScrollTo( QString const & scrollTo )
{
  Q_ASSERT( isScrollTo( scrollTo ) );
  const int scrollToPrefixLength = 7;
  return scrollTo.mid( scrollToPrefixLength );
}

WebPage::FindFlags caseSensitivityFindFlags( bool matchCase )
{
  return matchCase ? WebPage::FindCaseSensitively : WebPage::FindFlags();
}

QString searchStatusMessageNoMatches()
{
  return ArticleView::tr( "Phrase not found" );
}

QString searchStatusMessage( int activeMatch, int matchCount )
{
  Q_ASSERT( matchCount > 0 );
  Q_ASSERT( activeMatch > 0 );
  Q_ASSERT( activeMatch <= matchCount );
  return ArticleView::tr( "%1 of %2 matches" ).arg( activeMatch ).arg( matchCount );
}

std::string makeBlankPage( ArticleNetworkAccessManager const & articleNetMgr, WebPage & webPage )
{
#ifdef USE_QTWEBKIT
  Q_UNUSED( webPage )
  return articleNetMgr.makeBlankPage();
#else
  // Loading the default blank page instantly via QWebEnginePage::setHtml() alone is not sufficient to prevent
  // white background flashes when the background color in the current article style is not white. The white
  // background is visible for a particularly long time when a large page is loaded in a new foreground tab.
  // Setting QWebEnginePage::backgroundColor to Qt::transparent replaces the white background flashes with gray
  // background flashes. Eliminate the flashes by finding the page background color in the article style sheets and
  // assigning it to QWebEnginePage::backgroundColor. See also the documentation for QWebEnginePage::backgroundColor.
  QColor pageBackgroundColor;
  auto blankPage = articleNetMgr.makeBlankPage( &pageBackgroundColor );
  if( pageBackgroundColor.isValid() )
    webPage.setBackgroundColor( pageBackgroundColor );
  return blankPage;
#endif
}

QLatin1String javaScriptBool( bool value )
{
  return QLatin1String( value ? "true" : "false" );
}

#ifndef USE_QTWEBKIT
/// @return the string representation of @p date in the ISO 8601 format required by JavaScript Date constructor.
/// @note Qt::ISODateWithMs format is used to include milliseconds. The alternative Qt::ISODate format's
///       one-second precision is definitely too coarse for timestamp-based synchronization.
/// @note A JavaScript function, connected to a C++ signal with a QDateTime parameter, receives a value of the type
///       String rather than of the expected type Date in Qt 5.15.5. Explicitly sending date&time strings in the
///       required format will keep working correctly even if the type conversion issue is fixed in a future Qt version.
QString javaScriptDateString( QDateTime const & date )
{
  return date.toString( Qt::ISODateWithMs );
}

QString pageReloadingScriptName()
{
  return QStringLiteral( "PageReloading" );
}

QString pageReloadingScriptSourceCode( QString const & currentArticle, bool scrollToCurrentArticle )
{
  return QLatin1String( "const gdCurrentArticleBeforePageReloading = '%1';\n"
                        "const gdScrollToCurrentArticleAfterPageReloading = %2;" )
      .arg( currentArticle, javaScriptBool( scrollToCurrentArticle ) );
}

QWebEngineScript createScript( QString const & name, QString const & sourceCode )
{
  QWebEngineScript script;
  script.setInjectionPoint( QWebEngineScript::DocumentCreation );
  script.setRunsOnSubFrames( false );
  script.setWorldId( QWebEngineScript::MainWorld );

  script.setName( name );
  script.setSourceCode( sourceCode );

  return script;
}

QWebEngineScript createPageReloadingScript()
{
  return createScript( pageReloadingScriptName(), pageReloadingScriptSourceCode( QString{}, false ) );
}

QString variableDeclarationFromAssignmentScript( QString const & assignmentScript )
{
  return QLatin1String( "let " ) + assignmentScript;
}

QString selectWordBySingleClickScriptName()
{
  return QStringLiteral( "SelectWordBySingleClick" );
}

QString selectWordBySingleClickAssignmentScript( bool selectWordBySingleClick )
{
  return QLatin1String( "gdSelectWordBySingleClick = %1;" ).arg( javaScriptBool( selectWordBySingleClick ) );
}

QWebEngineScript createSelectWordBySingleClickScript()
{
  return createScript( selectWordBySingleClickScriptName(),
                       variableDeclarationFromAssignmentScript( selectWordBySingleClickAssignmentScript( false ) ) );
}
#endif // USE_QTWEBKIT

} // unnamed namespace

#ifndef USE_QTWEBKIT
class ArticleView::MouseEventInfo
{
public:
  void assign( QObject * target_, QMouseEvent const & event )
  {
    Q_ASSERT( target_ );

    target = target_;
    localPos = event.localPos();
    windowPos = event.windowPos();
    screenPos = event.screenPos();
    buttons = event.buttons();
    modifiers = event.modifiers();
  }

  void invalidate()
  { target = nullptr; }

  bool isValid() const
  { return target != nullptr; }

  void synthesize( QEvent::Type type, Qt::MouseButton button ) const
  {
    QMouseEvent event( type, localPos, windowPos, screenPos, button, buttons, modifiers,
                       Qt::MouseEventSynthesizedByApplication );
    QApplication::sendEvent( target, &event );
  }

private:
  QObject * target = nullptr;
  QPointF localPos;
  QPointF windowPos;
  QPointF screenPos;
  Qt::MouseButtons buttons;
  Qt::KeyboardModifiers modifiers;
};
#endif

QString ArticleView::scrollToFromDictionaryId( QString const & dictionaryId )
{
  Q_ASSERT( !isScrollTo( dictionaryId ) );
  return scrollToPrefix + dictionaryId;
}

ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm,
                          AudioPlayerPtr const & audioPlayer_,
                          std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                          Instances::Groups const & groups_, bool popupView_,
                          Config::Class const & cfg_,
                          QAction & openSearchAction_,
                          QAction * dictionaryBarToggled_,
                          GroupComboBox const * groupComboBox_ ):
  QFrame( parent ),
  articleNetMgr( nm ),
  audioPlayer( audioPlayer_ ),
  allDictionaries( allDictionaries_ ),
  groups( groups_ ),
  popupView( popupView_ ),
  cfg( cfg_ ),
  jsProxy( new ArticleViewJsProxy( *this ) ),
  pasteAction( this ),
  articleUpAction( this ),
  articleDownAction( this ),
  goBackAction( this ),
  goForwardAction( this ),
  selectCurrentArticleAction( this ),
  copyAsTextAction( this ),
  inspectAction( this ),
  openSearchAction( openSearchAction_ ),
  searchIsOpened( false ),
  dictionaryBarToggled( dictionaryBarToggled_ ),
  groupComboBox( groupComboBox_ ),
  ftsSearchIsOpened( false ),
  ftsSearchMatchCase( false ),
  ftsPosition( 0 )
{
  ui.setupUi( this );

  ArticleWebPage * const webPage = new ArticleWebPage( ui.definition );
  ui.definition->setPage( webPage );
#ifdef USE_QTWEBKIT
  ui.definition->setUp( const_cast< Config::Class * >( &cfg ) );
#endif

  goBackAction.setShortcut( QKeySequence( "Alt+Left" ) );
  ui.definition->addAction( &goBackAction );
  connect( &goBackAction, SIGNAL( triggered() ),
           this, SLOT( back() ) );

  goForwardAction.setShortcut( QKeySequence( "Alt+Right" ) );
  ui.definition->addAction( &goForwardAction );
  connect( &goForwardAction, SIGNAL( triggered() ),
           this, SLOT( forward() ) );

  QAction * const copyAction = ui.definition->pageAction( WebPage::Copy );
  copyAction->setShortcut( QKeySequence::Copy );
  ui.definition->addAction( copyAction );

  QAction * selectAll = ui.definition->pageAction( WebPage::SelectAll );
  selectAll->setShortcut( QKeySequence::SelectAll );
  selectAll->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  ui.definition->addAction( selectAll );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

#ifdef USE_QTWEBKIT
  // ArticleWebPage always delegates all links in the Qt WebEngine version.
  ui.definition->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  // ArticleUrlSchemeHandler implements this using articleNetMgr in the Qt WebEngine version.
  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  // QWebView::loadFinished() is emitted immediately after the JavaScript load event, which always
  // occurs after the page's JavaScript code finishes executing, so at the time of its emission
  // onJsArticleLoaded() has already obtained the data required by loadFinished(). An exception:
  // loadFinished() for the initial Welcome! page is emitted before the page's JavaScript code finishes
  // executing. This exception is not a problem in practice as the Welcome! page contains neither
  // dictionary articles nor audio links, so onJsArticleLoaded() isn't called at all on this page.
  // Calling ArticleView::loadFinished() before QWebView::loadFinished() is emitted, for example,
  // as soon as the page's JavaScript code finishes executing, breaks scrolling to current article
  // after ArticleView::reload(), i.e. causes a wrong vertical scroll position.
  connect( ui.definition, SIGNAL( loadFinished( bool ) ),
           this, SLOT( loadFinished( bool ) ) );

  attachToJavaScript();
  connect( ui.definition->page()->mainFrame(), SIGNAL( javaScriptWindowObjectCleared() ),
           this, SLOT( attachToJavaScript() ) );
#else
  auto * const webChannel = new QWebChannel( webPage );
  webPage->setWebChannel( webChannel, QWebEngineScript::MainWorld );
  webChannel->registerObject( QStringLiteral( "gdArticleView" ), jsProxy );

  webPage->scripts().insert( createPageReloadingScript() );
  webPage->scripts().insert( createSelectWordBySingleClickScript() );

  connect( webPage, &QWebEnginePage::findTextFinished, this, &ArticleView::findTextFinished );

  // QWebEnginePage always highlights all occurences, and this cannot be disabled => hide the useless button.
  ui.highlightAllButton->hide();
#endif // USE_QTWEBKIT

  connect( ui.definition, SIGNAL( titleChanged( QString const & ) ),
           this, SLOT( handleTitleChanged( QString const & ) ) );

  connect( ui.definition, SIGNAL( urlChanged( QUrl const & ) ),
           this, SLOT( handleUrlChanged( QUrl const & ) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( webPage, SIGNAL( linkClicked( QUrl const & ) ),
           this, SLOT( linkClicked( QUrl const & ) ) );

#ifdef USE_QTWEBKIT
  connect( ui.definition, SIGNAL( doubleClicked( QPoint ) ), this, SLOT( doubleClicked( QPoint ) ) );

  connect( ui.definition->page(), SIGNAL( linkHovered ( const QString &, const QString &, const QString & ) ),
           this, SLOT( linkHovered ( const QString &, const QString &, const QString & ) ) );
#else
  connect( ui.definition->page(), &QWebEnginePage::linkHovered,
           this, [ this ]( QString const & url ) { linkHovered( url ); } );
#endif

  pasteAction.setShortcut( QKeySequence::Paste  );
  ui.definition->addAction( &pasteAction );
  connect( &pasteAction, SIGNAL( triggered() ), this, SLOT( pasteTriggered() ) );

  articleUpAction.setShortcut( QKeySequence( "Alt+Up" ) );
  ui.definition->addAction( &articleUpAction );
  connect( &articleUpAction, SIGNAL( triggered() ), this, SLOT( moveOneArticleUp() ) );

  articleDownAction.setShortcut( QKeySequence( "Alt+Down" ) );
  ui.definition->addAction( &articleDownAction );
  connect( &articleDownAction, SIGNAL( triggered() ), this, SLOT( moveOneArticleDown() ) );

  ui.definition->addAction( &openSearchAction );
  connect( &openSearchAction, SIGNAL( triggered() ), this, SLOT( openSearch() ) );

  selectCurrentArticleAction.setShortcut( QKeySequence( "Ctrl+Shift+A" ));
  selectCurrentArticleAction.setText( tr( "Select Current Article" ) );
  ui.definition->addAction( &selectCurrentArticleAction );
  connect( &selectCurrentArticleAction, SIGNAL( triggered() ),
           this, SLOT( selectCurrentArticle() ) );

  copyAsTextAction.setShortcut( QKeySequence( "Ctrl+Shift+C" ) );
  copyAsTextAction.setText( tr( "Copy as text" ) );
  ui.definition->addAction( &copyAsTextAction );
  connect( &copyAsTextAction, SIGNAL( triggered() ),
           this, SLOT( copyAsText() ) );

  inspectAction.setShortcut( QKeySequence( Qt::Key_F12 ) );
  inspectAction.setText( tr( "Inspect" ) );
  ui.definition->addAction( &inspectAction );
  connect( &inspectAction, SIGNAL( triggered() ), this, SLOT( inspect() ) );

#ifdef USE_QTWEBKIT
  ui.definition->installEventFilter( this );
#else
  ui.definition->setEventFilter( this );
#endif
  ui.searchFrame->installEventFilter( this );
  ui.ftsSearchFrame->installEventFilter( this );

#ifdef USE_QTWEBKIT
  // registerArticleUrlSchemes() is responsible for similar configuration in the Qt WebEngine version.
  QWebSettings * settings = ui.definition->page()->settings();
  settings->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  settings->setAttribute( QWebSettings::LocalContentCanAccessFileUrls, true );
#endif

  std::string const blankPage = makeBlankPage( articleNetMgr, *webPage );
  // Load the default blank page instantly, so there would be no flicker.
  // When the correct WebEnginePage::backgroundColor is set, loading this blank page is less important,
  // but still prevents brief white background flashes when a new empty tab is created and switched to,
  // or when the scan popup is opened for the first time.
  ui.definition->setHtml( QString::fromUtf8( blankPage.data(), blankPage.size() ),
                          QUrl( "gdlookup://localhost?blank=1" ) );

  expandOptionalParts = cfg.preferences.alwaysExpandOptionalParts;

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
  ui.definition->grabGesture( Gestures::GDPinchGestureType );
  ui.definition->grabGesture( Gestures::GDSwipeGestureType );
#endif

#ifdef USE_QTWEBKIT
  // Variable name for store current selection range
  rangeVarName = QString( "sr_%1" ).arg( QString::number( (quint64)this, 16 ) );
#endif
}

// explicitly report the minimum size, to avoid
// sidebar widgets' improper resize during restore
QSize ArticleView::minimumSizeHint() const
{
  return ui.searchFrame->minimumSizeHint();
}

void ArticleView::setGroupComboBox( GroupComboBox const * g )
{
  groupComboBox = g;
}

ArticleView::~ArticleView()
{
  cleanupTemp();
  audioPlayer->stop();

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
  ui.definition->ungrabGesture( Gestures::GDPinchGestureType );
  ui.definition->ungrabGesture( Gestures::GDSwipeGestureType );
#endif
}

void ArticleView::showDefinition( Config::InputPhrase const & phrase, unsigned group,
                                  QString const & scrollTo,
                                  Contexts const & contexts_ )
{
  // first, let's stop the player
  audioPlayer->stop();

  QUrl req;
  Contexts contexts( contexts_ );

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  Qt4x5::Url::addQueryItem( req, "word", phrase.phrase );
  if ( !phrase.punctuationSuffix.isEmpty() )
    Qt4x5::Url::addQueryItem( req, "punctuation_suffix", phrase.punctuationSuffix );
  Qt4x5::Url::addQueryItem( req, "group", QString::number( group ) );
  if( cfg.preferences.ignoreDiacritics )
    Qt4x5::Url::addQueryItem( req, "ignore_diacritics", "1" );

  if ( scrollTo.size() )
    Qt4x5::Url::addQueryItem( req, "scrollto", scrollTo );

  Contexts::Iterator pos = contexts.find( "gdanchor" );
  if( pos != contexts.end() )
  {
    Qt4x5::Url::addQueryItem( req, "gdanchor", contexts[ "gdanchor" ] );
    contexts.erase( pos );
  }

  if ( contexts.size() )
  {
    QBuffer buf;

    buf.open( QIODevice::WriteOnly );

    QDataStream stream( &buf );

    stream << contexts;

    buf.close();

    Qt4x5::Url::addQueryItem( req,  "contexts", QString::fromLatin1( buf.buffer().toBase64() ) );
  }

  QString mutedDicts = getMutedForGroup( group );

  if ( mutedDicts.size() )
    Qt4x5::Url::addQueryItem( req,  "muted", mutedDicts );

  // Update headwords history
  emit sendWordToHistory( phrase.phrase );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

  load( req );

  //QApplication::setOverrideCursor( Qt::WaitCursor );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::showDefinition( QString const & word, unsigned group,
                                  QString const & scrollTo,
                                  Contexts const & contexts_ )
{
  showDefinition( Config::InputPhrase::fromPhrase( word ), group, scrollTo, contexts_ );
}

void ArticleView::showDefinition( Config::InputPhrase const & phrase, QStringList const & dictIDs,
                                  QRegExp const & searchRegExp, unsigned group, bool ignoreDiacritics )
{
  if( dictIDs.isEmpty() )
    return;

  // first, let's stop the player
  audioPlayer->stop();

  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  Qt4x5::Url::addQueryItem( req, "word", phrase.phrase );
  if( !phrase.punctuationSuffix.isEmpty() )
    Qt4x5::Url::addQueryItem( req, "punctuation_suffix", phrase.punctuationSuffix );
  Qt4x5::Url::addQueryItem( req, "dictionaries", dictIDs.join( ",") );
  Qt4x5::Url::addQueryItem( req, "regexp", searchRegExp.pattern() );
  if( searchRegExp.caseSensitivity() == Qt::CaseSensitive )
    Qt4x5::Url::addQueryItem( req, "matchcase", "1" );
  if( searchRegExp.patternSyntax() == QRegExp::WildcardUnix )
    Qt4x5::Url::addQueryItem( req, "wildcards", "1" );
  Qt4x5::Url::addQueryItem( req, "group", QString::number( group ) );
  if( ignoreDiacritics )
    Qt4x5::Url::addQueryItem( req, "ignore_diacritics", "1" );

  // Update headwords history
  emit sendWordToHistory( phrase.phrase );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

  load( req );

  //QApplication::setOverrideCursor( Qt::WaitCursor );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::showDefinition( QString const & word, QStringList const & dictIDs,
                                  QRegExp const & searchRegExp, unsigned group, bool ignoreDiacritics )
{
  showDefinition( Config::InputPhrase::fromPhrase( word ), dictIDs, searchRegExp, group, ignoreDiacritics );
}

#ifdef USE_QTWEBKIT

// This function relies on Qt WebKit API unavailable in Qt WebEngine.
// The code is useful only for website dictionaries.
static void expandFrames( QWebView & view )
{
  // See if we have any iframes in need of expansion

  QWebFrame & mainFrame = *view.page()->mainFrame();

  QList< QWebFrame * > frames = mainFrame.childFrames();

  bool wereFrames = false;

  for( QList< QWebFrame * >::iterator i = frames.begin(); i != frames.end(); ++i )
  {
    if ( (*i)->frameName().startsWith( "gdexpandframe-" ) )
    {
      //DPRINTF( "Name: %s\n", (*i)->frameName().toUtf8().data() );
      //DPRINTF( "Size: %d\n", (*i)->contentsSize().height() );
      //DPRINTF( ">>>>>>>>Height = %s\n", (*i)->evaluateJavaScript( "document.body.offsetHeight;" ).toString().toUtf8().data() );

      // Set the height
      mainFrame.evaluateJavaScript( QString( "document.getElementById('%1').height = %2;" ).
        arg( (*i)->frameName() ).
        arg( (*i)->contentsSize().height() ) );

      // Show it
      mainFrame.evaluateJavaScript( QString( "document.getElementById('%1').style.display = 'block';" ).
        arg( (*i)->frameName() ) );

      (*i)->evaluateJavaScript( "var gdLastUrlText;" );
      (*i)->evaluateJavaScript( "document.addEventListener( 'click', function() { gdLastUrlText = window.event.srcElement.textContent; }, true );" );
      (*i)->evaluateJavaScript( "document.addEventListener( 'contextmenu', function() { gdLastUrlText = window.event.srcElement.textContent; }, true );" );

      wereFrames = true;
    }
  }

  if ( wereFrames )
  {
    // There's some sort of glitch -- sometimes you need to move a mouse

    QMouseEvent ev( QEvent::MouseMove, QPoint(), Qt::MouseButton(), Qt::MouseButtons(), Qt::KeyboardModifiers() );

    qApp->sendEvent( &view, &ev );
  }
}

// JavaScript code implements this feature in the Qt WebEngine version.
void ArticleView::initCurrentArticleAndScroll()
{
  QVariant userDataVariant = ui.definition->history()->currentItem().userData();
  if ( userDataVariant.type() == QVariant::Map )
  {
    QMap< QString, QVariant > userData = userDataVariant.toMap();

    double sx = 0, sy = 0;
    bool moveToCurrentArticle = true;

    if ( userData.value( "sx" ).type() == QVariant::Double )
    {
      sx = userData.value( "sx" ).toDouble();
      moveToCurrentArticle = false;
    }

    if ( userData.value( "sy" ).type() == QVariant::Double )
    {
      sy = userData.value( "sy" ).toDouble();
      moveToCurrentArticle = false;
    }

    QString const savedCurrentArticle = userData.value( "currentArticle" ).toString();
    if( !savedCurrentArticle.isEmpty() )
    {
      // There's a current article saved, so set it to be current.
      // If a scroll position was stored - even (0, 0) - don't move to the
      // current article.
      setCurrentArticle( savedCurrentArticle, moveToCurrentArticle );
    }

    if ( sx != 0 || sy != 0 )
    {
      // Restore scroll position if at least one non-zero coordinate was stored.
      // Moving to (0, 0) is a no-op, so don't restore it.
      ui.definition->page()->mainFrame()->evaluateJavaScript(
          QString( "window.scroll( %1, %2 );" ).arg( sx ).arg( sy ) );
    }
  }
  else
  {
    QString const scrollTo = Qt4x5::Url::queryItemValue( ui.definition->url(), "scrollto" );
    if( isScrollTo( scrollTo ) )
    {
      // There is no active article saved in history, but we have it as a parameter.
      // setCurrentArticle will save it and scroll there.
      setCurrentArticle( scrollTo, true );
    }
  }
}

// JavaScript code implements this feature in the Qt WebEngine version.
static void scrollToGdAnchor( QWebView const & view )
{
  QUrl url = view.url();

  if( !Qt4x5::Url::queryItemValue( url, "gdanchor" ).isEmpty() )
  {
    QWebFrame & mainFrame = *view.page()->mainFrame();

    QString anchor = QUrl::fromPercentEncoding( Qt4x5::Url::encodedQueryItemValue( url, "gdanchor" ) );

    // Find GD anchor on page

    int n = anchor.indexOf( '_' );
    if( n == 33 )
      // MDict pattern: ("g" + dictionary ID (33 chars total))_(articleID(quint64, hex))_(original anchor)
      n = anchor.indexOf( '_', n + 1 );
    else
      n = 0;

    if( n > 0 )
    {
      QString originalAnchor = anchor.mid( n + 1 );

      QRegExp rx;
      rx.setMinimal( true );
      rx.setPattern( anchor.left( 34 ) + "[0-9a-f]*_" + originalAnchor );

      QWebElementCollection coll = mainFrame.findAllElements( "a[name]" );
      coll += mainFrame.findAllElements( "a[id]" );

      for( QWebElementCollection::iterator it = coll.begin(); it != coll.end(); ++it )
      {
        QString name = (*it).attribute( "name" );
        QString id = (*it).attribute( "id" );
        if( ( !name.isEmpty() && rx.indexIn( name ) >= 0 )
            || ( !id.isEmpty() && rx.indexIn( id ) >= 0 ))
        {
          // Anchor found, jump to it

          url.clear();
          url.setFragment( rx.cap( 0 ) );
          mainFrame.evaluateJavaScript(
             QString( "window.location.hash = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );

          break;
        }
      }
    }
    else
    {
      url.clear();
      url.setFragment( anchor );
      mainFrame.evaluateJavaScript(
         QString( "window.location.hash = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
  }
}

#endif // USE_QTWEBKIT

void ArticleView::loadFinished( bool )
{
#ifdef USE_QTWEBKIT
  expandFrames( *ui.definition );

  // If true, the user has managed to activate an article already.
  // The code below does not override this explicit user choice.
  bool const wasCurrentArticleSetExplicitly = evaluateJavaScriptVariableSafe(
        ui.definition->page()->mainFrame(), "gdWasCurrentArticleSetExplicitly" ).toBool();

  // Don't set the current article or scroll if the user has activated an article already.
  // However, if this was a back/forward navigation or page reloading, the user also has to
  // scroll the mouse wheel to prevent QWebPage from restoring its saved scroll position.
  if( !wasCurrentArticleSetExplicitly )
    initCurrentArticleAndScroll();
#endif

  ui.definition->unsetCursor();
  //QApplication::restoreOverrideCursor();

#ifdef USE_QTWEBKIT
  if( !wasCurrentArticleSetExplicitly )
    scrollToGdAnchor( *ui.definition );
#endif

  emit pageLoaded( this );

  if( Qt4x5::Url::hasQueryItem( ui.definition->url(), "regexp" ) )
    highlightFTSResults();
}

void ArticleView::handleTitleChanged( QString const & title )
{
  // Qt 5.x WebKit raise signal titleChanges(QString()) while navigation within page.
  // The Qt WebEngine-specific code below assumes title is not empty.
  if( title.isEmpty() )
    return;

#ifndef USE_QTWEBKIT
  // When a page is loaded, QWebEngineView::title first becomes equal to the URL,
  // then quickly changes to the HTML <title>. Ignore the temporary title change
  // to prevent the flashing of the main window title.

  auto const url = ui.definition->url().toString();
  if( title == url )
    return;

  constexpr QChar leftToRightEmbedding( u'\u202A' );
  constexpr QChar popDirectionalFormatting( u'\u202C' );
  Q_ASSERT( !title.isEmpty() );
  bool const titleMatchesUrl = title.front() == leftToRightEmbedding && title.back() == popDirectionalFormatting
                               && QStringView{ title.constData() + 1, title.size() - 2 } == url;
  if( titleMatchesUrl )
    return;
#endif

  emit titleChanged( this, title );
}

void ArticleView::handleUrlChanged( QUrl const & url )
{
#ifndef USE_QTWEBKIT
  isBlankPagePresentInWebHistory = isBlankPagePresentInWebHistory
          || QUrlQuery{ url }.queryItemValue( QStringLiteral( "blank" ) ) == QLatin1String( "1" );
#endif

  QIcon icon;

  unsigned group = getGroup( url );

  if ( group )
  {
    // Find the group's instance corresponding to the fragment value
    for( unsigned x = 0; x < groups.size(); ++x )
      if ( groups[ x ].id == group )
      {
        // Found it

        icon = groups[ x ].makeIcon();
        break;
      }
  }

  emit iconChanged( this, icon );
}

void ArticleView::runJavaScript( TargetFrame targetFrame, QString const & scriptSource )
{
#ifdef USE_QTWEBKIT
  QWebPage * const page = ui.definition->page();
  Q_ASSERT( page );
  QWebFrame * const frame = targetFrame == MainFrame ? page->mainFrame() : page->currentFrame();
  Q_ASSERT( frame );
  frame->evaluateJavaScript( scriptSource );
#else
  // The Qt WebEngine API allows to run JavaScript only on the main frame of a page.
  // Hopefully the target frame does not matter in the Qt WebEngine version.
  Q_UNUSED( targetFrame )
  ui.definition->page()->runJavaScript( scriptSource, QWebEngineScript::MainWorld );
#endif
}

void ArticleView::clearPageSelection()
{
  runJavaScript( CurrentFrame, "window.getSelection().removeAllRanges();" );
}

unsigned ArticleView::getGroup( QUrl const & url )
{
  if ( url.scheme() == "gdlookup" && Qt4x5::Url::hasQueryItem( url, "group" ) )
    return Qt4x5::Url::queryItemValue( url, "group" ).toUInt();

  return 0;
}

QStringList ArticleView::getArticleList() const
{
  return articleList;
}

QString ArticleView::getActiveArticleId()
{
  if ( !isScrollTo( currentArticle ) )
    return QString(); // Incorrect id

  return dictionaryIdFromScrollTo( currentArticle );
}

void ArticleView::jumpToDictionary( QString const & id, bool force )
{
  QString targetArticle = scrollToFromDictionaryId( id );

  // jump only if neceessary, or when forced
  if ( force || targetArticle != currentArticle )
  {
    setCurrentArticle( targetArticle, true );
  }
}

bool ArticleView::setCurrentArticle( QString const & id, bool moveToIt )
{
  if ( !isScrollTo( id ) )
  {
    gdWarning( "Attempt to set invalid current article ID: %s", id.toUtf8().constData() );
    return false;
  }

  if ( !ui.definition->isVisible() )
    return false; // No action on background page, scrollIntoView there don't work

  if( !articleList.contains( dictionaryIdFromScrollTo( id ) ) )
    return false;

#ifdef USE_QTWEBKIT
  runJavaScript( MainFrame,
    QString( "gdOnCppActiveArticleChanged('%1', %2);" ).arg( id, javaScriptBool( moveToIt ) ) );
#else
  currentArticleTimestamp = QDateTime::currentDateTimeUtc();
  ui.definition->page()->runJavaScript( QLatin1String( "gdOnCppActiveArticleChanged('%1', %2, '%3', '%4');" )
                                        .arg( id, javaScriptBool( moveToIt ), javaScriptDateString( pageTimestamp ),
                                              javaScriptDateString( currentArticleTimestamp ) ) );
#endif

  setValidCurrentArticleNoJs( id );

  return true;
}

void ArticleView::setValidCurrentArticleNoJs( QString const & id )
{
  if( currentArticle == id )
    return; // nothing to do
  currentArticle = id;
  emit activeArticleChanged( this, dictionaryIdFromScrollTo( id ) );
}

void ArticleView::selectCurrentArticle()
{
  runJavaScript( MainFrame, "gdSelectCurrentArticle();" );
}

#ifdef USE_QTWEBKIT
bool ArticleView::isFramedArticle( QString const & ca )
{
  if ( ca.isEmpty() )
    return false;

  return ui.definition->page()->mainFrame()->
               evaluateJavaScript( QString( "!!document.getElementById('gdexpandframe-%1');" )
                                          .arg( dictionaryIdFromScrollTo( ca ) ) ).toBool();
}
#endif

bool ArticleView::isExternalLink( QUrl const & url )
{
  return url.scheme() == "http" || url.scheme() == "https" ||
         url.scheme() == "ftp" || url.scheme() == "mailto" ||
         url.scheme() == "file";
}

#ifdef USE_QTWEBKIT
void ArticleView::tryMangleWebsiteClickedUrl( QUrl & url, Contexts & contexts )
{
  // Don't try mangling audio urls, even if they are from the framed websites

  if( ( url.scheme() == "http" || url.scheme() == "https" )
      && ! Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    // Maybe a link inside a website was clicked?

    if( isFramedArticle( currentArticle ) )
    {
      QVariant result = evaluateJavaScriptVariableSafe( ui.definition->page()->currentFrame(), "gdLastUrlText" );

      if ( result.type() == QVariant::String )
      {
        // Looks this way
        contexts[ dictionaryIdFromScrollTo( currentArticle ) ] = QString::fromLatin1( url.toEncoded() );

        QUrl target;

        QString queryWord = result.toString();

        // Empty requests are treated as no request, so we work this around by
        // adding a space.
        if ( queryWord.isEmpty() )
          queryWord = " ";

        target.setScheme( "gdlookup" );
        target.setHost( "localhost" );
        target.setPath( "/" + queryWord );

        url = target;
      }
    }
  }
}

void ArticleView::updateCurrentArticleFromCurrentFrame( QWebFrame * frame )
{
  if ( !frame )
    frame = ui.definition->page()->currentFrame();

  for( ; frame; frame = frame->parentFrame() )
  {
    QString frameName = frame->frameName();

    if ( frameName.startsWith( "gdexpandframe-" ) )
    {
      QString newCurrent = scrollToFromDictionaryId( frameName.mid( 14 ) );

      if( currentArticle != newCurrent )
        setCurrentArticle( newCurrent, false );

      break;
    }
  }
}
#endif // USE_QTWEBKIT

#ifndef USE_QTWEBKIT
void ArticleView::updateSourceCodeOfInjectedScript( QString const & name, QString const & sourceCode )
{
  auto & scripts = ui.definition->page()->scripts();

  auto script = scripts.findScript( name );
  Q_ASSERT( !script.isNull() );

  scripts.remove( script );
  script.setSourceCode( sourceCode );
  scripts.insert( script );
}

void ArticleView::updateInjectedPageReloadingScript( bool scrollToCurrentArticle )
{
  updateSourceCodeOfInjectedScript( pageReloadingScriptName(),
                                    pageReloadingScriptSourceCode( currentArticle, scrollToCurrentArticle ) );
}

void ArticleView::updateInjectedSelectWordBySingleClickScript( bool selectWordBySingleClick )
{
  QString const assignmentScript = selectWordBySingleClickAssignmentScript( selectWordBySingleClick );
  ui.definition->page()->runJavaScript( assignmentScript );
  updateSourceCodeOfInjectedScript( selectWordBySingleClickScriptName(),
                                    variableDeclarationFromAssignmentScript( assignmentScript ) );
}
#endif

void ArticleView::saveHistoryUserData()
{
  // JavaScript code implements this feature in the Qt WebEngine version.
#ifdef USE_QTWEBKIT
  QMap< QString, QVariant > userData = ui.definition->history()->
                                       currentItem().userData().toMap();

  // Save current article, which can be empty

  userData[ "currentArticle" ] = currentArticle;

  // We also save window position. We restore it when the page is fully loaded,
  // when any hidden frames are expanded.

  userData[ "sx" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollX;" ).toDouble();
  userData[ "sy" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollY;" ).toDouble();

  ui.definition->history()->currentItem().setUserData( userData );
#endif // USE_QTWEBKIT
}

void ArticleView::load( QUrl const & url )
{
#ifndef USE_QTWEBKIT
  // Translating the current word again looks like reloading the page to JavaScript code.
  // Update the script to prevent setting a wrong current article and unwanted scrolling.
  // After such "reloading" the web engine restores the window position automatically.
  updateInjectedPageReloadingScript( false );
#endif

  saveHistoryUserData();
  ui.definition->load( url );
}

void ArticleView::cleanupTemp()
{
  QSet< QString >::iterator it = desktopOpenedTempFiles.begin();
  while( it != desktopOpenedTempFiles.end() )
  {
    if( QFile::remove( *it ) )
      it = desktopOpenedTempFiles.erase( it );
    else
      ++it;
  }
}

bool ArticleView::handleF3( QObject * /*obj*/, QEvent * ev )
{
  if ( ev->type() == QEvent::ShortcutOverride
       || ev->type() == QEvent::KeyPress )
  {
    QKeyEvent * ke = static_cast<QKeyEvent *>( ev );
    if ( ke->key() == Qt::Key_F3 && isSearchOpened() ) {
      if ( !ke->modifiers() )
      {
        if( ev->type() == QEvent::KeyPress )
          on_searchNext_clicked();

        ev->accept();
        return true;
      }

      if ( ke->modifiers() == Qt::ShiftModifier )
      {
        if( ev->type() == QEvent::KeyPress )
          on_searchPrevious_clicked();

        ev->accept();
        return true;
      }
    }
    if ( ke->key() == Qt::Key_F3 && ftsSearchIsOpened )
    {
      if ( !ke->modifiers() )
      {
        if( ev->type() == QEvent::KeyPress )
          on_ftsSearchNext_clicked();

        ev->accept();
        return true;
      }

      if ( ke->modifiers() == Qt::ShiftModifier )
      {
        if( ev->type() == QEvent::KeyPress )
          on_ftsSearchPrevious_clicked();

        ev->accept();
        return true;
      }
    }
  }

  return false;
}

bool ArticleView::eventFilter( QObject * obj, QEvent * ev )
{
#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
  if( ev->type() == QEvent::Gesture )
  {
    Gestures::GestureResult result;
    QPoint pt;

    bool handled = Gestures::handleGestureEvent( obj, ev, result, pt );

    if( handled )
    {
      if( result == Gestures::ZOOM_IN )
        zoomIn();
      else
      if( result == Gestures::ZOOM_OUT )
        zoomOut();
      else
      if( result == Gestures::SWIPE_LEFT )
        back();
      else
      if( result == Gestures::SWIPE_RIGHT )
        forward();
      else
      if( result == Gestures::SWIPE_UP || result == Gestures::SWIPE_DOWN )
      {
        int delta = result == Gestures::SWIPE_UP ? -120 : 120;
        QWidget *widget = static_cast< QWidget * >( obj );

        QWidget *child = widget->childAt( widget->mapFromGlobal( pt ) );
        if( child )
          widget = child;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 12, 0 )
        QWheelEvent whev( widget->mapFromGlobal( pt ), pt, QPoint(), QPoint( 0, delta ),
                          Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false );
#else
        QWheelEvent whev( widget->mapFromGlobal( pt ), pt, delta, Qt::NoButton, Qt::NoModifier );
#endif
        qApp->sendEvent( widget, &whev );
      }
    }

    return handled;
  }

  if( ev->type() == QEvent::MouseMove )
  {
    if( Gestures::isFewTouchPointsPresented() )
    {
      ev->accept();
      return true;
    }
  }
#endif

  if ( handleF3( obj, ev ) )
  {
    return true;
  }

#ifdef USE_QTWEBKIT
  if ( obj != ui.definition )
#else
  if( !ui.definition->isWatched( obj ) )
#endif
    return QFrame::eventFilter( obj, ev );

  switch( ev->type() )
  {
    case QEvent::MouseButtonPress:
    {
      QMouseEvent * event = static_cast< QMouseEvent * >( ev );

#ifndef USE_QTWEBKIT
      if( lastLeftMouseButtonPressEvent && event->button() == Qt::LeftButton
          // Don't store events synthesized by this class to prevent recursion.
          && event->source() != Qt::MouseEventSynthesizedByApplication )
      {
        // "Select word by single click" option is on. A double click must be generated to select the word
        // under cursor. But if this click is on a scrollbar, generating a double click prevents dragging it.
        // We cannot determine if the click is on a scrollbar here. Store the event away; generate a double
        // click once the JavaScript side confirms this is a click on the page proper, not on its scrollbar.
        lastLeftMouseButtonPressEvent->assign( obj, *event );
      }
#endif

      if ( event->button() == Qt::XButton1 ) {
        back();
        return true;
      }
      if ( event->button() == Qt::XButton2 ) {
        forward();
        return true;
      }

      break;
    }
#ifndef USE_QTWEBKIT
    case QEvent::MouseButtonDblClick:
    {
      auto * const event = static_cast< QMouseEvent * >( ev );
      if( lastLeftMouseButtonPressEvent && event->button() == Qt::LeftButton )
      {
        if( lastLeftMouseButtonPressEvent->isValid() )
        {
          // This means a double click on a scrollbar or a timing issue. In any case, now that a non-synthesized
          // double click has occurred, synthesizing another one makes no sense => invalidate the stored event.
          lastLeftMouseButtonPressEvent->invalidate();
        }
        else
        {
          // A synthesized double click has already selected a word after the first click.
          // The web page treats a second non-synthesized click as a separate first click.
          // Filter it out to prevent immediate deselection of the word.
          return true;
        }
      }
      break;
    }
    case QEvent::MouseButtonRelease:
      // In the Qt WebEngine version, ArticleWebPage::linkClicked() is emitted several milliseconds after the mouse
      // button release event, so the Qt WebKit version's ArticleWebView::isMidButtonPressed() approach does not work.
      // Resort to the following hack: on events that can activate a link, set isNavigationByMiddleMouseButton to true
      // in the case of the middle mouse button release, to false otherwise.
      isNavigationByMiddleMouseButton = static_cast< QMouseEvent * >( ev )->button() == Qt::MiddleButton;
      break;
#endif // USE_QTWEBKIT
    case QEvent::KeyPress:
    {
#ifndef USE_QTWEBKIT
      // Once a link is focused with [Shift+]Tab key presses, pressing a Return/Enter key activates it.
      // The last released mouse button must not affect whether the link is opened in a new tab then.
      isNavigationByMiddleMouseButton = false;
#endif

      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( keyEvent->key() == Qt::Key_Space ||
           keyEvent->key() == Qt::Key_Backspace ||
           keyEvent->key() == Qt::Key_Tab ||
           keyEvent->key() == Qt::Key_Backtab ||
           keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter )
        return false; // Those key have other uses than to start typing

      QString text = keyEvent->text();

      if ( text.size() )
      {
        emit typingEvent( text );
        return true;
      }

      break;
    }
    default:
      break;
  }

  return false;
}

QString ArticleView::getMutedForGroup( unsigned group )
{
  if ( dictionaryBarToggled && dictionaryBarToggled->isChecked() )
  {
    // Dictionary bar is active -- mute the muted dictionaries
    Instances::Group const * groupInstance = groups.findGroup( group );

    // Find muted dictionaries for current group
    Config::Group const * grp = cfg.getGroup( group );
    Config::MutedDictionaries const * mutedDictionaries;
    if( group == Instances::Group::AllGroupId )
      mutedDictionaries = popupView ? &cfg.popupMutedDictionaries : &cfg.mutedDictionaries;
    else
        mutedDictionaries = grp ? ( popupView ? &grp->popupMutedDictionaries : &grp->mutedDictionaries ) : 0;
    if( !mutedDictionaries )
      return QString();

    QStringList mutedDicts;

    if ( groupInstance )
    {
      for( unsigned x = 0; x < groupInstance->dictionaries.size(); ++x )
      {
        QString id = QString::fromStdString(
                       groupInstance->dictionaries[ x ]->getId() );

        if ( mutedDictionaries->contains( id ) )
          mutedDicts.append( id );
      }
    }

    if ( mutedDicts.size() )
      return mutedDicts.join( "," );
  }

  return QString();
}

void ArticleView::linkHovered ( const QString & link, const QString & , const QString & )
{
  QString msg;
  QUrl url(link);

  if ( url.scheme() == "bres" )
  {
    msg = tr( "Resource" );
  }
  else
  if ( url.scheme() == "gdau" || Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    msg = tr( "Audio" );
  }
  else
  if ( url.scheme() == "gdtts" )
  {
    msg = tr( "TTS Voice" );
  }
  else
  if ( url.scheme() == "gdpicture" )
  {
    msg = tr( "Picture" );
  }
  else
  if ( url.scheme() == "gdvideo" )
  {
    if ( url.path().isEmpty() )
    {
      msg = tr( "Video" );
    }
    else
    {
      QString path = url.path();
      if ( path.startsWith( '/' ) )
      {
        path = path.mid( 1 );
      }
      msg = tr( "Video: %1" ).arg( path );
    }
  }
  else
  if (url.scheme() == "gdlookup" || url.scheme().compare( "bword" ) == 0)
  {
    QString def = url.path();
    if (def.startsWith("/"))
    {
      def = def.mid( 1 );
    }

    if( Qt4x5::Url::hasQueryItem( url, "dict" ) )
    {
      // Link to other dictionary
      QString dictName( Qt4x5::Url::queryItemValue( url, "dict" ) );
      if( !dictName.isEmpty() )
        msg = tr( "Definition from dictionary \"%1\": %2" ).arg( dictName ).arg( def );
    }

    if( msg.isEmpty() )
    {
      if( def.isEmpty() && url.hasFragment() )
        msg = '#' + url.fragment(); // this must be a citation, footnote or backlink
      else
        msg = tr( "Definition: %1").arg( def );
    }
  }
  else
  {
    msg = link;
  }

  emit statusBarMessage( msg );
}

#ifdef USE_QTWEBKIT
void ArticleView::attachToJavaScript()
{
  ui.definition->page()->mainFrame()->addToJavaScriptWindowObject( "gdArticleView", jsProxy );
}
#endif

void ArticleView::linkClicked( QUrl const & url_ )
{
  Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();

  // Lock jump on links while Alt key is pressed
  if( kmod & Qt::AltModifier )
    return;

#ifdef USE_QTWEBKIT
  updateCurrentArticleFromCurrentFrame();
#endif

  QUrl url( url_ );
  Contexts contexts;

#ifdef USE_QTWEBKIT
  tryMangleWebsiteClickedUrl( url, contexts );
  bool const isNavigationByMiddleMouseButton = ui.definition->isMidButtonPressed();
#endif

  if ( !popupView &&
       ( isNavigationByMiddleMouseButton ||
         ( kmod & ( Qt::ControlModifier | Qt::ShiftModifier ) ) ) )
  {
    // The link was middle-button-clicked or Control/Shift is currently pressed => open the link in a new tab.
    emit openLinkInNewTab( url, ui.definition->url(), currentArticle, contexts );
  }
  else
    openLink( url, ui.definition->url(), currentArticle, contexts );
}

static QUrl replaceScrollToInLinkWithFragment( QUrl const & url, QString const & scrollTo )
{
  static QString const scrollToQueryKey = "scrollto";

  QUrl result = url;

#ifdef USE_QTWEBKIT
  // In the Qt WebKit version, the "scrollto" query item value has no effect in background tabs and
  // overrides the more precise and useful scrolling to the fragment ID in foreground tabs => remove it.
  // TODO (Qt WebKit): fix these issues and share code with the Qt WebEngine version in order to set correct current
  // article instead of keeping the first article active (a new Qt4x5::Url::replaceQueryItem() wrapper could be useful).
  Q_UNUSED( scrollTo )
  Qt4x5::Url::removeQueryItem( result, scrollToQueryKey );
#else
  QUrlQuery urlQuery( url );
  urlQuery.removeQueryItem( scrollToQueryKey );
  if( !scrollTo.isEmpty() )
    urlQuery.addQueryItem( scrollToQueryKey, scrollTo );
  result.setQuery( urlQuery );
#endif

  return result;
}

void ArticleView::openLinkWithFragment( QUrl const & url, QString const & scrollTo )
{
  Q_ASSERT( url.hasFragment() );

  // This must be fragment navigation on the same page, but possibly in a new tab. No need to save
  // history user data, because its only user - loadFinished() - is not called after navigating back
  // within the page; navigating back to the initial blank page in the new tab is not allowed.

  // Neither assigning to window.location in the current tab nor navigating from the initial blank page
  // in the new tab looks like reloading to JavaScript code => no need to update the page-reloading script.

#if QT_VERSION >= QT_VERSION_CHECK( 5, 2, 0 )
  if( url.matches( ui.definition->url(), QUrl::RemoveFragment ) )
#else
  QUrl currentUrlWithEqualFragment = ui.definition->url();
  currentUrlWithEqualFragment.setFragment( url.fragment() );
  if( url == currentUrlWithEqualFragment )
#endif
  {
    // This is a fragment navigation on the same page and not in a new tab.
    // The regular loading of the url is slower and causes other issues => assign url to window.location.
    runJavaScript( MainFrame, QString( "window.location = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    return;
  }

  // url's target is a different page (is that possible?) or this link is being opened in a new tab.
  // Load the url regularly, because assigning it to window.location fails in the Qt WebEngine
  // version - the new tab's page remains blank and the following error message is printed:
  // JS error: Not allowed to load local resource: <url> (at :1)
  ui.definition->load( replaceScrollToInLinkWithFragment( url, scrollTo ) );
}

void ArticleView::openLink( QUrl const & url, QUrl const & ref,
                            QString const & scrollTo,
                            Contexts const & contexts_ )
{
  qDebug() << "clicked" << url;

  Contexts contexts( contexts_ );

  if( url.scheme().compare( "gdpicture" ) == 0 )
    load( url );
  else
  if ( url.scheme().compare( "bword" ) == 0 )
  {
    if( Qt4x5::Url::hasQueryItem( ref, "dictionaries" ) )
    {
      QStringList dictsList = Qt4x5::Url::queryItemValue( ref, "dictionaries" )
                                          .split( ",", Qt4x5::skipEmptyParts() );

      showDefinition( url.path(), dictsList, QRegExp(), getGroup( ref ), false );
    }
    else
      showDefinition( url.path(),
                      getGroup( ref ), scrollTo, contexts );
  }
  else
  if ( url.scheme() == "gdlookup" ) // Plain html links inherit gdlookup scheme
  {
    if ( url.hasFragment() )
      openLinkWithFragment( url, scrollTo );
    else
    {
      if( Qt4x5::Url::hasQueryItem( ref, "dictionaries" ) )
      {
        // Specific dictionary group from full-text search
        QStringList dictsList = Qt4x5::Url::queryItemValue( ref, "dictionaries" )
                                            .split( ",", Qt4x5::skipEmptyParts() );

        showDefinition( url.path().mid( 1 ), dictsList, QRegExp(), getGroup( ref ), false );
        return;
      }

      QString newScrollTo( scrollTo );
      if( Qt4x5::Url::hasQueryItem( url, "dict" ) )
      {
        // Link to other dictionary
        QString dictName( Qt4x5::Url::queryItemValue( url, "dict" ) );
        for( unsigned i = 0; i < allDictionaries.size(); i++ )
        {
          if( dictName.compare( QString::fromUtf8( allDictionaries[ i ]->getName().c_str() ) ) == 0 )
          {
            newScrollTo = scrollToFromDictionaryId( QString::fromUtf8( allDictionaries[ i ]->getId().c_str() ) );
            break;
          }
        }
      }

      if( Qt4x5::Url::hasQueryItem( url, "gdanchor" ) )
        contexts[ "gdanchor" ] = Qt4x5::Url::queryItemValue( url, "gdanchor" );

      showDefinition( url.path().mid( 1 ),
                      getGroup( ref ), newScrollTo, contexts );
    }
  }
  else
  if ( url.scheme() == "bres" || url.scheme() == "gdau" || url.scheme() == "gdvideo" ||
       Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    // Download it

    // Clear any pending ones

    resourceDownloadRequests.clear();

    resourceDownloadUrl = url;

    if ( Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
    {
      sptr< Dictionary::DataRequest > req =
        new Dictionary::WebMultimediaDownload( url, articleNetMgr );

      resourceDownloadRequests.push_back( req );

      connect( req.get(), SIGNAL( finished() ),
               this, SLOT( resourceDownloadFinished() ) );
    }
    else
    if ( url.scheme() == "gdau" && url.host() == "search" )
    {
      // Since searches should be limited to current group, we just do them
      // here ourselves since otherwise we'd need to pass group id to netmgr
      // and it should've been having knowledge of the current groups, too.

      unsigned currentGroup = getGroup( ref );

      std::vector< sptr< Dictionary::Class > > const * activeDicts = 0;

      if ( groups.size() )
      {
        for( unsigned x = 0; x < groups.size(); ++x )
          if ( groups[ x ].id == currentGroup )
          {
            activeDicts = &( groups[ x ].dictionaries );
            break;
          }
      }
      else
        activeDicts = &allDictionaries;

      if ( activeDicts )
      {
        unsigned preferred = UINT_MAX;
        if( url.hasFragment() )
        {
          // Find sound in the preferred dictionary
          QString preferredName = Qt4x5::Url::fragment( url );
          try
          {
            for( unsigned x = 0; x < activeDicts->size(); ++x )
            {
              if( preferredName.compare( QString::fromUtf8( (*activeDicts)[ x ]->getName().c_str() ) ) == 0 )
              {
                preferred = x;
                sptr< Dictionary::DataRequest > req =
                  (*activeDicts)[ x ]->getResource(
                    url.path().mid( 1 ).toUtf8().data() );

                resourceDownloadRequests.push_back( req );

                if ( !req->isFinished() )
                {
                  // Queued loading
                  connect( req.get(), SIGNAL( finished() ),
                           this, SLOT( resourceDownloadFinished() ) );
                }
                else
                {
                  // Immediate loading
                  if( req->dataSize() > 0 )
                  {
                    // Resource already found, stop next search
                    resourceDownloadFinished();
                    return;
                  }
                }
                break;
              }
            }
          }
          catch( std::exception & e )
          {
            emit statusBarMessage(
                  tr( "ERROR: %1" ).arg( e.what() ),
                  10000, QPixmap( ":/icons/error.png" ) );
          }
        }
        for( unsigned x = 0; x < activeDicts->size(); ++x )
        {
          try
          {
            if( x == preferred )
              continue;

            sptr< Dictionary::DataRequest > req =
              (*activeDicts)[ x ]->getResource(
                url.path().mid( 1 ).toUtf8().data() );

            resourceDownloadRequests.push_back( req );

            if ( !req->isFinished() )
            {
              // Queued loading
              connect( req.get(), SIGNAL( finished() ),
                       this, SLOT( resourceDownloadFinished() ) );
            }
            else
            {
              // Immediate loading
              if( req->dataSize() > 0 )
              {
                // Resource already found, stop next search
                break;
              }
            }
          }
          catch( std::exception & e )
          {
            emit statusBarMessage(
                  tr( "ERROR: %1" ).arg( e.what() ),
                  10000, QPixmap( ":/icons/error.png" ) );
          }
        }
      }
    }
    else
    {
      // Normal resource download
      QString contentType;

      sptr< Dictionary::DataRequest > req =
        articleNetMgr.getResource( url, contentType );

      if ( !req.get() )
      {
        // Request failed, fail
      }
      else
      if ( req->isFinished() && req->dataSize() >= 0 )
      {
        // Have data ready, handle it
        resourceDownloadRequests.push_back( req );
        resourceDownloadFinished();

        return;
      }
      else
      if ( !req->isFinished() )
      {
        // Queue to be handled when done

        resourceDownloadRequests.push_back( req );

        connect( req.get(), SIGNAL( finished() ),
                 this, SLOT( resourceDownloadFinished() ) );
      }
    }

    if ( resourceDownloadRequests.empty() ) // No requests were queued
    {
      QMessageBox::critical( this, "GoldenDict", tr( "The referenced resource doesn't exist." ) );
      return;
    }
    else
      resourceDownloadFinished(); // Check any requests finished already
  }
  else
  if ( url.scheme() == "gdprg" )
  {
    // Program. Run it.
    QString id( url.host() );

    for( Config::Programs::const_iterator i = cfg.programs.begin();
         i != cfg.programs.end(); ++i )
    {
      if ( i->id == id )
      {
        // Found the corresponding program.
        Programs::RunInstance * req = new Programs::RunInstance;

        connect( req, SIGNAL(finished(QByteArray,QString)),
                 req, SLOT( deleteLater() ) );

        QString error;

        // Delete the request if it fails to start
        if ( !req->start( *i, url.path().mid( 1 ), error ) )
        {
          delete req;

          QMessageBox::critical( this, "GoldenDict",
                                 error );
        }

        return;
      }
    }

    // Still here? No such program exists.
    QMessageBox::critical( this, "GoldenDict",
                           tr( "The referenced audio program doesn't exist." ) );
  }
  else
  if ( url.scheme() == "gdtts" )
  {
// TODO: Port TTS
#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
    // Text to speech
    QString md5Id = Qt4x5::Url::queryItemValue( url, "engine" );
    QString text( url.path().mid( 1 ) );

    for ( Config::VoiceEngines::const_iterator i = cfg.voiceEngines.begin();
          i != cfg.voiceEngines.end(); ++i )
    {
      QString itemMd5Id = QString( QCryptographicHash::hash(
                                     i->id.toUtf8(),
                                     QCryptographicHash::Md5 ).toHex() );

      if ( itemMd5Id == md5Id )
      {
        SpeechClient * speechClient = new SpeechClient( *i, this );
        connect( speechClient, SIGNAL( finished() ), speechClient, SLOT( deleteLater() ) );
        speechClient->tell( text );
        break;
      }
    }
#endif
  }
  else
  if ( isExternalLink( url ) )
  {
    // Use the system handler for the conventional external links
    QDesktopServices::openUrl( url );
  }
}

void ArticleView::saveResource( QUrl const & url, ResourceToSaveHandler & handler )
{
  return saveResource( url, ui.definition->url(), &handler );
}

void ArticleView::saveResource( QUrl const & url, QUrl const & ref, ResourceToSaveHandler * handler )
{
  Q_ASSERT( handler );

  sptr< Dictionary::DataRequest > req;

  if( url.scheme() == "bres" || url.scheme() == "gico" || url.scheme() == "gdau" || url.scheme() == "gdvideo" )
  {
    if ( url.host() == "search" )
    {
      // Since searches should be limited to current group, we just do them
      // here ourselves since otherwise we'd need to pass group id to netmgr
      // and it should've been having knowledge of the current groups, too.

      unsigned currentGroup = getGroup( ref );

      std::vector< sptr< Dictionary::Class > > const * activeDicts = 0;

      if ( groups.size() )
      {
        for( unsigned x = 0; x < groups.size(); ++x )
          if ( groups[ x ].id == currentGroup )
          {
            activeDicts = &( groups[ x ].dictionaries );
            break;
          }
      }
      else
        activeDicts = &allDictionaries;

      if ( activeDicts )
      {
        unsigned preferred = UINT_MAX;
        if( url.hasFragment() && url.scheme() == "gdau" )
        {
          // Find sound in the preferred dictionary
          QString preferredName = Qt4x5::Url::fragment( url );
          for( unsigned x = 0; x < activeDicts->size(); ++x )
          {
            try
            {
              if( preferredName.compare( QString::fromUtf8( (*activeDicts)[ x ]->getName().c_str() ) ) == 0 )
              {
                preferred = x;
                sptr< Dictionary::DataRequest > req =
                  (*activeDicts)[ x ]->getResource(
                    url.path().mid( 1 ).toUtf8().data() );

                handler->addRequest( req );

                if( req->isFinished() && req->dataSize() > 0 )
                {
                  handler->downloadFinished();
                  return;
                }
                break;
              }
            }
            catch( std::exception & e )
            {
              gdWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
                         (*activeDicts)[ x ]->getName().c_str() );
            }
          }
        }
        for( unsigned x = 0; x < activeDicts->size(); ++x )
        {
          try
          {
            if( x == preferred )
              continue;

            req = (*activeDicts)[ x ]->getResource(
                    Qt4x5::Url::path( url ).mid( 1 ).toUtf8().data() );

            handler->addRequest( req );

            if( req->isFinished() && req->dataSize() > 0 )
            {
              // Resource already found, stop next search
              break;
            }
          }
          catch( std::exception & e )
          {
            gdWarning( "getResource request error (%s) in \"%s\"\n", e.what(),
                       (*activeDicts)[ x ]->getName().c_str() );
          }
        }
      }
    }
    else
    {
      // Normal resource download
      QString contentType;
      req = articleNetMgr.getResource( url, contentType );

      if( req.get() )
      {
        handler->addRequest( req );
      }
    }
  }
  else
  {
    req = new Dictionary::WebMultimediaDownload( url, articleNetMgr );

    handler->addRequest( req );
  }

  if ( handler->isEmpty() ) // No requests were queued
  {
    emit statusBarMessage(
          tr( "ERROR: %1" ).arg( tr( "The referenced resource doesn't exist." ) ),
          10000, QPixmap( ":/icons/error.png" ) );
  }

  // Check already finished downloads
  handler->downloadFinished();

  return;
}

void ArticleView::updateMutedContents()
{
  QUrl currentUrl = ui.definition->url();

  if ( currentUrl.scheme() != "gdlookup" )
    return; // Weird url -- do nothing

  unsigned group = getGroup( currentUrl );

  if ( !group )
    return; // No group in url -- do nothing

  QString mutedDicts = getMutedForGroup( group );

  if ( Qt4x5::Url::queryItemValue( currentUrl, "muted" ) != mutedDicts )
  {
    // The list has changed -- update the url

    Qt4x5::Url::removeQueryItem( currentUrl, "muted" );

    if ( mutedDicts.size() )
    Qt4x5::Url::addQueryItem( currentUrl, "muted", mutedDicts );

    load( currentUrl );

    //QApplication::setOverrideCursor( Qt::WaitCursor );
    ui.definition->setCursor( Qt::WaitCursor );
  }
}

bool ArticleView::canGoBack() const
{
#ifndef USE_QTWEBKIT
  // Don't allow navigating back to page 0 if it is the initial blank page.
  if( isBlankPagePresentInWebHistory )
    return ui.definition->history()->currentItemIndex() > 1;
#endif
  return ui.definition->history()->canGoBack();
}

bool ArticleView::canGoForward() const
{
  return ui.definition->history()->canGoForward();
}

void ArticleView::setSelectionBySingleClick( bool set )
{
#ifdef USE_QTWEBKIT
  ui.definition->setSelectionBySingleClick( set );
#else
  if( set == static_cast< bool >( lastLeftMouseButtonPressEvent ) )
    return; // nothing changed

  if( set )
    lastLeftMouseButtonPressEvent.reset( new MouseEventInfo{} );
  else
    lastLeftMouseButtonPressEvent.reset();

  updateInjectedSelectWordBySingleClickScript( set );
#endif
}

void ArticleView::back()
{
  if ( canGoBack() )
  {
    saveHistoryUserData();
    ui.definition->back();
  }
}

void ArticleView::forward()
{
  saveHistoryUserData();
  ui.definition->forward();
}

void ArticleView::reload()
{
  // Reloading occurs in response to changes that may affect content height, so restoring the
  // current window position can cause uncontrolled jumps. Scrolling to the current article
  // (i.e. jumping to he top of it) is simple, reliable and predictable, if not ideal.
#ifdef USE_QTWEBKIT
  QMap< QString, QVariant > userData = ui.definition->history()->currentItem().userData().toMap();

  // Save current article, which can be empty
  userData[ "currentArticle" ] = currentArticle;

  // Remove saved window position to request scrolling to the current article.
  userData[ "sx" ].clear();
  userData[ "sy" ].clear();

  ui.definition->history()->currentItem().setUserData( userData );
#else
  // Scroll to the current article only if the "regexp" query item value is empty.
  // If it is not empty, highlightFTSResults() will most likely call
  // QWebEngineView::findText(), which will scroll to the first match instead.
  bool const scrollToCurrentArticle = QUrlQuery{ ui.definition->url() }
          .queryItemValue( QStringLiteral( "regexp" ) ).isEmpty();
  updateInjectedPageReloadingScript( scrollToCurrentArticle );
#endif // USE_QTWEBKIT

  ui.definition->reload();
}

bool ArticleView::hasSound() const
{
  return !firstAudioLink.isEmpty();
}

void ArticleView::playSound()
{
  // fallback to the first one
  QString const soundScript = audioLinks.value( currentArticle, firstAudioLink );
  if ( !soundScript.isEmpty() )
    openLink( QUrl::fromEncoded( soundScript.toUtf8() ), ui.definition->url() );
}

QString ArticleView::toHtml()
{
#ifdef USE_QTWEBKIT
  return ui.definition->page()->mainFrame()->toHtml();
#else
  // TODO (Qt WebEngine): port this function and its uses to asynchronous QWebEnginePage::toHtml().
  return QString();
#endif
}

QString ArticleView::getTitle()
{
#ifdef USE_QTWEBKIT
  return ui.definition->page()->mainFrame()->title();
#else
  return ui.definition->page()->title();
#endif
}

Config::InputPhrase ArticleView::getPhrase() const
{
  const QUrl url = ui.definition->url();
  return Config::InputPhrase( Qt4x5::Url::queryItemValue( url, "word" ),
                              Qt4x5::Url::queryItemValue( url, "punctuation_suffix" ) );
}

void ArticleView::print( QPrinter * printer ) const
{
#ifdef USE_QTWEBKIT
  ui.definition->print( printer );
#else
  // TODO (Qt WebEngine): port this function and its uses to QWebEnginePage::print(). From the documentation:
  // "It is the users responsibility to ensure the printer remains valid until resultCallback has been called." =>
  // consider capturing sptr< QPrinter > MainWindow::printer by value and calling sptr::reset() in the resultCallback.
  Q_UNUSED( printer )
#endif
}

void ArticleView::contextMenuRequested( QPoint const & pos )
{
  // Is that a link? Is there a selection?

#ifdef USE_QTWEBKIT
  QWebHitTestResult r = ui.definition->page()->mainFrame()->
                          hitTestContent( pos );

  updateCurrentArticleFromCurrentFrame( r.frame() );
#else
  auto const & r = ui.definition->page()->contextMenuData();
  Q_ASSERT( r.isValid() );
#endif

  QMenu menu( this );

  QAction * followLink = 0;
  QAction * followLinkExternal = 0;
  QAction * followLinkNewTab = 0;
  QAction * lookupSelection = 0;
  QAction * lookupSelectionGr = 0;
  QAction * lookupSelectionNewTab = 0;
  QAction * lookupSelectionNewTabGr = 0;
  QAction * maxDictionaryRefsAction = 0;
  QAction * addWordToHistoryAction = 0;
  QAction * addHeaderToHistoryAction = 0;
  QAction * sendWordToInputLineAction = 0;
  QAction * saveImageAction = 0;
  QAction * saveSoundAction = 0;

  QUrl targetUrl( r.linkUrl() );
  Contexts contexts;

#ifdef USE_QTWEBKIT
  tryMangleWebsiteClickedUrl( targetUrl, contexts );
#endif

  if ( !r.linkUrl().isEmpty() )
  {
    if ( !isExternalLink( targetUrl ) )
    {
      followLink = new QAction( tr( "&Open Link" ), &menu );
      menu.addAction( followLink );

      if ( !popupView )
      {
        followLinkNewTab = new QAction( QIcon( ":/icons/addtab.png" ),
                                        tr( "Open Link in New &Tab" ), &menu );
        menu.addAction( followLinkNewTab );
      }
    }

    if ( isExternalLink( r.linkUrl() ) )
    {
      followLinkExternal = new QAction( tr( "Open Link in &External Browser" ), &menu );
      menu.addAction( followLinkExternal );
      menu.addAction( ui.definition->pageAction( WebPage::CopyLinkToClipboard ) );
    }
  }

  QUrl imageUrl;
  if( !popupView )
  {
#ifdef USE_QTWEBKIT
    QWebElement const el = r.element();
    if( el.tagName().compare( "img", Qt::CaseInsensitive ) == 0 )
      imageUrl = QUrl::fromPercentEncoding( el.attribute( "src" ).toLatin1() );
#else
    if( r.mediaType() == QWebEngineContextMenuData::MediaTypeImage )
      imageUrl = r.mediaUrl();
#endif

    if( !imageUrl.isEmpty() )
    {
      menu.addAction( ui.definition->pageAction( WebPage::CopyImageToClipboard ) );
      saveImageAction = new QAction( tr( "Save &image..." ), &menu );
      menu.addAction( saveImageAction );
    }
  }

  if( !popupView && ( targetUrl.scheme() == "gdau"
                      || Dictionary::WebMultimediaDownload::isAudioUrl( targetUrl ) ) )
  {
    saveSoundAction = new QAction( tr( "Save s&ound..." ), &menu );
    menu.addAction( saveSoundAction );
  }

  QString selectedText = ui.definition->selectedText();
  QString text = selectedText.trimmed();

  if ( text.size() && text.size() < 60 )
  {
    // We don't prompt for selections larger or equal to 60 chars, since
    // it ruins the menu and it's hardly a single word anyway.

    if( text.isRightToLeft() )
    {
      text.insert( 0, (ushort)0x202E ); // RLE, Right-to-Left Embedding
      text.append( (ushort)0x202C ); // PDF, POP DIRECTIONAL FORMATTING
    }

    lookupSelection = new QAction( tr( "&Look up \"%1\"" ).
                                   arg( text ),
                                   &menu );
    menu.addAction( lookupSelection );

    if ( !popupView )
    {
      lookupSelectionNewTab = new QAction( QIcon( ":/icons/addtab.png" ),
                                           tr( "Look up \"%1\" in &New Tab" ).
                                           arg( text ),
                                           &menu );
      menu.addAction( lookupSelectionNewTab );

      sendWordToInputLineAction = new QAction( tr( "Send \"%1\" to input line" ).
                                               arg( text ),
                                               &menu );
      menu.addAction( sendWordToInputLineAction );
    }

    addWordToHistoryAction = new QAction( tr( "&Add \"%1\" to history" ).
                                          arg( text ),
                                          &menu );
    menu.addAction( addWordToHistoryAction );

    Instances::Group const * altGroup =
      ( groupComboBox && groupComboBox->getCurrentGroup() !=  getGroup( ui.definition->url() )  ) ?
        groups.findGroup( groupComboBox->getCurrentGroup() ) : 0;

    if ( altGroup )
    {
      QIcon icon = altGroup->icon.size() ? QIcon( ":/flags/" + altGroup->icon ) :
                   QIcon();

      lookupSelectionGr = new QAction( icon, tr( "Look up \"%1\" in %2" ).
                                       arg( text ).
                                       arg( altGroup->name ), &menu );
      menu.addAction( lookupSelectionGr );

      if ( !popupView )
      {
        lookupSelectionNewTabGr = new QAction( QIcon( ":/icons/addtab.png" ),
                                               tr( "Look up \"%1\" in %2 in &New Tab" ).
                                               arg( text ).
                                               arg( altGroup->name ), &menu );
        menu.addAction( lookupSelectionNewTabGr );
      }
    }
  }

  if( text.isEmpty() && !cfg.preferences.storeHistory)
  {
    QString txt = ui.definition->title();
    if( txt.size() > 60 )
      txt = txt.left( 60 ) + "...";

    addHeaderToHistoryAction = new QAction( tr( "&Add \"%1\" to history" ).
                                            arg( txt ),
                                            &menu );
    menu.addAction( addHeaderToHistoryAction );
  }

  if ( selectedText.size() )
  {
    menu.addAction( ui.definition->pageAction( WebPage::Copy ) );
    menu.addAction( &copyAsTextAction );
  }
  else
  {
    menu.addAction( &selectCurrentArticleAction );
    menu.addAction( ui.definition->pageAction( WebPage::SelectAll ) );
  }

  map< QAction *, QString > tableOfContents;

  // Add table of contents
  if ( !menu.isEmpty() && !articleList.empty() )
    menu.addSeparator();

  unsigned refsAdded = 0;
  bool maxDictionaryRefsReached = false;

  for( QStringList::const_iterator i = articleList.constBegin(); i != articleList.constEnd();
       ++i, ++refsAdded )
  {
    // Find this dictionary

    for( unsigned x = allDictionaries.size(); x--; )
    {
      if ( allDictionaries[ x ]->getId() == i->toUtf8().data() )
      {
        QAction * action = 0;
        if ( refsAdded == cfg.preferences.maxDictionaryRefsInContextMenu )
        {
          // Enough! Or the menu would become too large.
          maxDictionaryRefsAction = new QAction( ".........", &menu );
          action = maxDictionaryRefsAction;
          maxDictionaryRefsReached = true;
        }
        else
        {
          action = new QAction(
                  allDictionaries[ x ]->getIcon(),
                  QString::fromUtf8( allDictionaries[ x ]->getName().c_str() ),
                  &menu );
          // Force icons in menu on all platforms,
          // since without them it will be much harder
          // to find things.
          action->setIconVisibleInMenu( true );
        }
        menu.addAction( action );

        tableOfContents[ action ] = *i;

        break;
      }
    }
    if( maxDictionaryRefsReached )
      break;
  }

  menu.addSeparator();
  menu.addAction( &inspectAction );

  if ( !menu.isEmpty() )
  {
    connect( this, SIGNAL( closePopupMenu() ), &menu, SLOT( close() ) );
    QAction * result = menu.exec( ui.definition->mapToGlobal( pos ) );

    if ( !result )
      return;

    if ( result == followLink )
      openLink( targetUrl, ui.definition->url(), currentArticle, contexts );
    else
    if ( result == followLinkExternal )
      QDesktopServices::openUrl( r.linkUrl() );
    else
    if ( result == lookupSelection )
      showDefinition( selectedText, getGroup( ui.definition->url() ), currentArticle );
    else
    if ( result == lookupSelectionGr && groupComboBox )
      showDefinition( selectedText, groupComboBox->getCurrentGroup(), QString() );
    else
    if ( result == addWordToHistoryAction )
      emit forceAddWordToHistory( selectedText );
    if ( result == addHeaderToHistoryAction )
      emit forceAddWordToHistory( ui.definition->title() );
    else
    if( result == sendWordToInputLineAction )
      emit sendWordToInputLine( selectedText );
    else
    if ( !popupView && result == followLinkNewTab )
      emit openLinkInNewTab( targetUrl, ui.definition->url(), currentArticle, contexts );
    else
    if ( !popupView && result == lookupSelectionNewTab )
      emit showDefinitionInNewTab( Config::InputPhrase::fromPhrase( selectedText ), getGroup( ui.definition->url() ),
                                   currentArticle, Contexts() );
    else
    if ( !popupView && result == lookupSelectionNewTabGr && groupComboBox )
      emit showDefinitionInNewTab( Config::InputPhrase::fromPhrase( selectedText ), groupComboBox->getCurrentGroup(),
                                   QString(), Contexts() );
    else
    if( result == saveImageAction || result == saveSoundAction )
    {
#if QT_VERSION >= 0x040600
      QUrl url = ( result == saveImageAction ) ? imageUrl : targetUrl;
      QString savePath;
      QString fileName;

      if ( cfg.resourceSavePath.isEmpty() )
        savePath = QDir::homePath();
      else
      {
        savePath = QDir::fromNativeSeparators( cfg.resourceSavePath );
        if ( !QDir( savePath ).exists() )
          savePath = QDir::homePath();
      }

      QString name = Qt4x5::Url::path( url ).section( '/', -1 );

      if ( result == saveSoundAction )
      {
        // Audio data
        if ( name.indexOf( '.' ) < 0 )
          name += ".wav";

        fileName = savePath + "/" + name;
        fileName = QFileDialog::getSaveFileName( parentWidget(), tr( "Save sound" ),
                                                 fileName,
                                                 tr( "Sound files (*.wav *.ogg *.oga *.mp3 *.mp4 *.aac *.flac *.mid *.wv *.ape);;All files (*.*)" ) );
      }
      else
      {
        // Image data

        // Check for babylon image name
        if ( name[ 0 ] == '\x1E' )
          name.remove( 0, 1 );
        if ( name.length() && name[ name.length() - 1 ] == '\x1F' )
          name.chop( 1 );

        fileName = savePath + "/" + name;
        fileName = QFileDialog::getSaveFileName( parentWidget(), tr( "Save image" ),
                                                 fileName,
                                                 tr( "Image files (*.bmp *.jpg *.png *.tif);;All files (*.*)" ) );
      }

      if ( !fileName.isEmpty() )
      {
        QFileInfo fileInfo( fileName );
        emit storeResourceSavePath( QDir::toNativeSeparators( fileInfo.absoluteDir().absolutePath() ) );

        ResourceToSaveHandler * const handler = new ResourceToSaveHandler( this, fileName );
        saveResource( url, ui.definition->url(), handler );
      }
#endif
    }
    else
    {
      if ( !popupView && result == maxDictionaryRefsAction )
        emit showDictsPane();

      // Match against table of contents
      QString id = tableOfContents[ result ];

      if ( id.size() )
        setCurrentArticle( scrollToFromDictionaryId( id ), true );
    }
  }
#if 0
  DPRINTF( "%s\n", r.linkUrl().isEmpty() ? "null" : "not null" );

  DPRINTF( "url = %s\n", r.linkUrl().toString().toLocal8Bit().data() );
  DPRINTF( "title = %s\n", r.title().toLocal8Bit().data() );
#endif
}

void ArticleView::resourceDownloadFinished()
{
  if ( resourceDownloadRequests.empty() )
    return; // Stray signal

  // Find any finished resources
  for( list< sptr< Dictionary::DataRequest > >::iterator i =
       resourceDownloadRequests.begin(); i != resourceDownloadRequests.end(); )
  {
    if ( (*i)->isFinished() )
    {
      if ( (*i)->dataSize() >= 0 )
      {
        // Ok, got one finished, all others are irrelevant now

        vector< char > const & data = (*i)->getFullData();

        if ( resourceDownloadUrl.scheme() == "gdau" ||
             Dictionary::WebMultimediaDownload::isAudioUrl( resourceDownloadUrl ) )
        {
          // Audio data
          connect( audioPlayer.data(), SIGNAL( error( QString ) ), this, SLOT( audioPlayerError( QString ) ), Qt::UniqueConnection );
          QString errorMessage = audioPlayer->play( data.data(), data.size() );
          if( !errorMessage.isEmpty() )
            QMessageBox::critical( this, "GoldenDict", tr( "Failed to play sound file: %1" ).arg( errorMessage ) );
        }
        else
        {
          // Create a temporary file
          // Remove the ones previously used, if any
          cleanupTemp();
          QString fileName;

          {
            QTemporaryFile tmp(
              QDir::temp().filePath( "XXXXXX-" + resourceDownloadUrl.path().section( '/', -1 ) ), this );

            if ( !tmp.open() || (size_t) tmp.write( &data.front(), data.size() ) != data.size() )
            {
              QMessageBox::critical( this, "GoldenDict", tr( "Failed to create temporary file." ) );
              return;
            }

            tmp.setAutoRemove( false );

            desktopOpenedTempFiles.insert( fileName = tmp.fileName() );
          }

          if ( !QDesktopServices::openUrl( QUrl::fromLocalFile( fileName ) ) )
            QMessageBox::critical( this, "GoldenDict",
                                   tr( "Failed to auto-open resource file, try opening manually: %1." ).arg( fileName ) );
        }

        // Ok, whatever it was, it's finished. Remove this and any other
        // requests and finish.

        resourceDownloadRequests.clear();

        return;
      }
      else
      {
        // This one had no data. Erase it.
        resourceDownloadRequests.erase( i++ );
      }
    }
    else // Unfinished, wait.
      break;
  }

  if ( resourceDownloadRequests.empty() )
  {
    emit statusBarMessage(
          tr( "WARNING: %1" ).arg( tr( "The referenced resource failed to download." ) ),
          10000, QPixmap( ":/icons/error.png" ) );
  }
}

void ArticleView::audioPlayerError( QString const & message )
{
  emit statusBarMessage( tr( "WARNING: Audio Player: %1" ).arg( message ),
                         10000, QPixmap( ":/icons/error.png" ) );
}

void ArticleView::pasteTriggered()
{
  Config::InputPhrase phrase = cfg.preferences.sanitizeInputPhrase( QApplication::clipboard()->text() );

  if ( phrase.isValid() )
  {
    unsigned groupId = getGroup( ui.definition->url() );
    if ( groupId == 0 )
    {
      // We couldn't figure out the group out of the URL,
      // so let's try the currently selected group.
      groupId = groupComboBox->getCurrentGroup();
    }
    showDefinition( phrase, groupId, currentArticle );
  }
}

void ArticleView::moveOneArticleUp()
{
  if( !currentArticle.isEmpty() )
  {
    int idx = articleList.indexOf( dictionaryIdFromScrollTo( currentArticle ) );

    if ( idx != -1 )
    {
      --idx;

      if ( idx < 0 )
        idx = articleList.size() - 1;

      setCurrentArticle( scrollToFromDictionaryId( articleList.at( idx ) ), true );
    }
  }
}

void ArticleView::moveOneArticleDown()
{
  if( !currentArticle.isEmpty() )
  {
    int idx = articleList.indexOf( dictionaryIdFromScrollTo( currentArticle ) );

    if ( idx != -1 )
    {
      idx = ( idx + 1 ) % articleList.size();

      setCurrentArticle( scrollToFromDictionaryId( articleList.at( idx ) ), true );
    }
  }
}

void ArticleView::openSearch()
{
  if( !isVisible() )
    return;

  if( ftsSearchIsOpened )
    closeSearch();

  if ( !searchIsOpened )
  {
    ui.searchFrame->show();
    ui.searchText->setText( getTitle() );
    searchIsOpened = true;
  }

  ui.searchText->setFocus();
  ui.searchText->selectAll();

  // Clear any current selection
  if ( ui.definition->selectedText().size() )
    clearPageSelection();

  // Don't display obsolete status from the previous search.
  ui.searchStatusLabel->clear();
  if ( ui.searchText->property( "noResults" ).toBool() )
  {
    ui.searchText->setProperty( "noResults", false );

    // Reload stylesheet
    reloadStyleSheet();
  }
}

void ArticleView::on_searchPrevious_clicked()
{
  if ( searchIsOpened )
    performFindOperation( false, true );
}

void ArticleView::on_searchNext_clicked()
{
  if ( searchIsOpened )
    performFindOperation( false, false );
}

void ArticleView::on_searchText_textEdited()
{
  performFindOperation( true, false );
}

void ArticleView::on_searchText_returnPressed()
{
  on_searchNext_clicked();
}

void ArticleView::on_searchCloseButton_clicked()
{
  closeSearch();
}

void ArticleView::on_searchCaseSensitive_clicked()
{
  performFindOperation( true, false );
}

void ArticleView::on_highlightAllButton_clicked()
{
  performFindOperation( false, false, true );
}

#ifdef USE_QTWEBKIT
void ArticleView::onJsPageInitStarted()
#else
void ArticleView::onJsPageInitStarted( QStringList const & loadedArticles, QStringList const & loadedAudioLinks,
                                       int activeArticleIndex, bool hasPageInitFinished,
                                       QDateTime const & pageTimestamp_ )
#endif
{
  // When JavaScript code initialization starts, the previous page is definitely gone.
  // Clear the data associated with it and prepare to receive the current page's data.

  articleList.clear();
  audioLinks.clear();
  firstAudioLink.clear();
  currentArticle.clear();

  emit canGoBackForwardChanged( this );
  emit pageUnloaded( this );

#ifndef USE_QTWEBKIT
  // This is the first message from JavaScript on this page.
  // Initialize our stored timestamps to the page timestamp received from JavaScript.
  currentArticleTimestamp = pageTimestamp = pageTimestamp_;

  if( loadedArticles.size() == loadedAudioLinks.size() )
  {
    for( int i = 0; i != loadedArticles.size(); ++i )
      onJsArticleLoadedNoTimestamps( loadedArticles.at( i ), loadedAudioLinks.at( i ), i == activeArticleIndex );
  }
  else
    gdWarning( "Loaded item list sizes don't match: %d != %d", loadedArticles.size(), loadedAudioLinks.size() );

  if( hasPageInitFinished )
    onJsPageInitFinished();
#endif
}

#ifndef USE_QTWEBKIT
void ArticleView::onJsPageInitFinished()
{
  if( currentArticle.isEmpty() && !articleList.empty() )
  {
    // Deferred JavaScript code silently activates the first article in this case. Do the same here.
    setValidCurrentArticleNoJs( scrollToFromDictionaryId( articleList.constFirst() ) );
  }

  // Slots connected to ArticleView::pageLoaded signal, which is emitted from ArticleView::loadFinished(), may
  // call ArticleView::playSound(), which requires up-to-date values of the current article and the audio links.
  // So ArticleView::loadFinished() must not be invoked before this moment, even if QWebEngineView::loadFinished
  // is emitted before gdArticleView becomes available (rarely, but happens). Calling it here works well.
  loadFinished( true );
}

void ArticleView::onJsFirstLeftButtonMouseDown()
{
  if( !lastLeftMouseButtonPressEvent || !lastLeftMouseButtonPressEvent->isValid() )
    return; // "Select word by single click" option is off or this is a synthesized mousedown event => nothing to do.

  // The JavaScript mousedown event that invoked this function must represent the same user action as the stored
  // lastLeftMouseButtonPressEvent. Generate a double click with the stored event's data.

  // When "Select word by single click" option is on and more than one word is selected, clicking on one of the selected
  // words reduces the selection to this single word in the Qt WebKit version. In the Qt WebEngine version clicking on
  // one of the selected words has no effect. So the user has to click outside a selection to modify it. If the entire
  // page is selected (e.g. after the Select All action is triggered), the user can resort to a middle mouse button
  // click or press the left mouse button, move the cursor, then release the button.
  // A consequence of this unchanging selection: when "Double-click translates the word clicked" option is on, double
  // clicking on a selected word translates the entire selection, which can be considered a feature as it allows
  // translating a phrase, not just a single word, using only the left mouse button.
  // This selection issue can we worked around by sending an additional mouse release event, but then links are
  // activated on left mouse button press rather than release, which is a worse bug.

  // Even though a non-synthesized first MouseButtonPress event occurred just a few milliseconds ago, the web page
  // treats a synthesized MouseButtonPress or MouseButtonDblClick event as a separate first click. Synthesize two
  // identical events to generate a double click and select the word under cursor.
  for( int i = 0; i < 2; ++i )
    lastLeftMouseButtonPressEvent->synthesize( QEvent::MouseButtonPress, Qt::LeftButton );

  // This stored event has served its purpose. Prevent recursion by invalidating it.
  lastLeftMouseButtonPressEvent->invalidate();
}

void ArticleView::onJsDoubleClicked( QString const & imageUrl )
{
  if( !imageUrl.isEmpty() )
  {
    downloadImage( QUrl{ imageUrl } );
    return;
  }

  if( cfg.preferences.doubleClickTranslates )
  {
    // Sanitize selected text before translating, because multiple words and punctuation
    // marks can be selected during a prolonged double-click in the Qt WebEngine version.
    auto const phrase = cfg.preferences.sanitizeInputPhrase( ui.definition->selectedText() );
    if( phrase.isValid() )
      translatePossiblyInNewTab( phrase );
  }
}
#endif // USE_QTWEBKIT

void ArticleView::onJsArticleLoaded( QString const & id, QString const & audioLink, bool isActive )
{
#ifndef USE_QTWEBKIT
  // When JavaScript does not send the current article timestamp, it is implicitly equal to pageTimestamp.
  // If currentArticleTimestamp != pageTimestamp, then our current article value is fresher => keep it.
  if( isActive && currentArticleTimestamp != pageTimestamp )
    isActive = false;
#endif

  onJsArticleLoadedNoTimestamps( id, audioLink, isActive );
}

void ArticleView::onJsArticleLoadedNoTimestamps( QString const & id, QString const & audioLink, bool isActive )
{
  if( !isScrollTo( id ) )
  {
    gdWarning( "Invalid article ID received from JavaScript: %s", id.toUtf8().constData() );
    return;
  }

  articleList.push_back( dictionaryIdFromScrollTo( id ) );
  if( !audioLink.isEmpty() )
  {
    audioLinks.insert( id, audioLink );
    if( firstAudioLink.isEmpty() )
      firstAudioLink = audioLink;
  }
  if( isActive )
    currentArticle = id;

  emit articleLoaded( this, articleList.back(), isActive );
}

#ifdef USE_QTWEBKIT
void ArticleView::onJsActiveArticleChanged(QString const & id)
#else
void ArticleView::onJsActiveArticleChanged( QString const & id, QDateTime const & currentArticleTimestamp_ )
#endif
{
  if ( !isScrollTo( id ) )
  {
    gdWarning( "Invalid active article ID received from JavaScript: %s", id.toUtf8().constData() );
    return;
  }

#ifndef USE_QTWEBKIT
  // Compare the received and stored current article timestamps using operator< here and using
  // operator<= in the JavaScript code to ensure that the current article values are always consistent,
  // even if the user manages to activate different articles in JavaScript and C++ at the same time.
  if( currentArticleTimestamp_ < currentArticleTimestamp )
    return; // Our current article value is fresher => keep it.
  currentArticleTimestamp = currentArticleTimestamp_;
#endif

  setValidCurrentArticleNoJs( id );
}

void ArticleView::onJsLocationHashChanged()
{
  emit canGoBackForwardChanged( this ); // Fragment navigation on the same page adds a web history entry.
}

#ifdef USE_QTWEBKIT
void ArticleView::doubleClicked( QPoint pos )
{
  QWebHitTestResult r = ui.definition->page()->mainFrame()->hitTestContent( pos );
  QWebElement el = r.element();
  if( el.tagName().compare( "img", Qt::CaseInsensitive ) == 0 )
  {
    // Double click on image; download it and transfer to external program
    QUrl const imageUrl = QUrl::fromPercentEncoding( el.attribute( "src" ).toLatin1() );
    if( !imageUrl.isEmpty() )
      downloadImage( imageUrl );
    return;
  }

  // We might want to initiate translation of the selected word
  if( cfg.preferences.doubleClickTranslates )
  {
    QString const selectedText = ui.definition->selectedText();
    // Do some checks to make sure there's a sensible selection indeed
    if( !Folding::applyWhitespaceOnly( gd::toWString( selectedText ) ).empty() && selectedText.size() < 60 )
      translatePossiblyInNewTab( Config::InputPhrase::fromPhrase( selectedText ) );
  }
}
#endif // USE_QTWEBKIT

void ArticleView::downloadImage( QUrl const & imageUrl )
{
  // Clear any pending ones
  resourceDownloadRequests.clear();

  resourceDownloadUrl = imageUrl;
  sptr< Dictionary::DataRequest > req;

  if ( imageUrl.scheme() == "http" || imageUrl.scheme() == "https" || imageUrl.scheme() == "ftp" )
  {
    // Web resource
    req = new Dictionary::WebMultimediaDownload( imageUrl, articleNetMgr );
  }
  else
  if ( imageUrl.scheme() == "bres" || imageUrl.scheme() == "gdpicture" )
  {
    // Local resource
    QString contentType;
    req = articleNetMgr.getResource( imageUrl, contentType );
  }
  else
  {
    // Unsupported scheme
    gdWarning( "Unsupported url scheme \"%s\" to download image\n", imageUrl.scheme().toUtf8().data() );
    return;
  }

  if ( !req.get() )
  {
    // Request failed, fail
    gdWarning( "Can't create request to download image \"%s\"\n", imageUrl.toString().toUtf8().data() );
    return;
  }

  if ( req->isFinished() && req->dataSize() >= 0 )
  {
    // Have data ready, handle it
    resourceDownloadRequests.push_back( req );
    resourceDownloadFinished();
    return;
  }
  else
  if ( !req->isFinished() )
  {
    // Queue to be handled when done
    resourceDownloadRequests.push_back( req );
    connect( req.get(), SIGNAL( finished() ), this, SLOT( resourceDownloadFinished() ) );
  }
  if ( resourceDownloadRequests.empty() ) // No requests were queued
  {
    gdWarning( "The referenced resource \"%s\" doesn't exist\n", imageUrl.toString().toUtf8().data() ) ;
    return;
  }
  else
    resourceDownloadFinished(); // Check any requests finished already
}

void ArticleView::translatePossiblyInNewTab( Config::InputPhrase const & phrase )
{
  Q_ASSERT( phrase.isValid() );

  Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();
  if (kmod & (Qt::ControlModifier | Qt::ShiftModifier))
  { // open in new tab
    // TODO: emitting this signal has no effect in the scan popup's view.
    //       Should showDefinition() be called if popupView is true?
    emit showDefinitionInNewTab( phrase, getGroup( ui.definition->url() ),
                                 currentArticle, Contexts() );
  }
  else
  {
    QUrl const & ref = ui.definition->url();

    if( Qt4x5::Url::hasQueryItem( ref, "dictionaries" ) )
    {
      QStringList dictsList = Qt4x5::Url::queryItemValue(ref, "dictionaries" )
                                          .split( ",", Qt4x5::skipEmptyParts() );
      showDefinition( phrase, dictsList, QRegExp(), getGroup( ref ), false );
    }
    else
      showDefinition( phrase, getGroup( ref ), currentArticle );
  }
}


void ArticleView::performFindOperation( bool restart, bool backwards, bool checkHighlight )
{
  QString text = ui.searchText->text();

  if ( restart || checkHighlight )
  {
#ifdef USE_QTWEBKIT
    if( restart ) {
      // Anyone knows how we reset the search position?
      // For now we resort to this hack:
      if ( ui.definition->selectedText().size() )
        clearPageSelection();
    }

    QWebPage::FindFlags f = caseSensitivityFindFlags( ui.searchCaseSensitive->isChecked() );
    f |= QWebPage::HighlightAllOccurrences;

    ui.definition->findText( "", f );

    if( ui.highlightAllButton->isChecked() )
      ui.definition->findText( text, f );

    if( checkHighlight )
      return;
#else
    Q_ASSERT_X( !checkHighlight, Q_FUNC_INFO,
                "Toggling Web Engine page highlighting is not and cannot be implemented." );
    // Skip UI status update after the special empty-string search to prevent a flashing
    // of no-results style and status text. When text is empty though, this special
    // empty-string search is the only one, so the UI status has to be updated then.
    if( !text.isEmpty() )
      skipNextFindTextUiStatusUpdate = true;
    // Searching for an empty string is useful in 3 ways:
    // 1) clear the search highlight when case sensitivity is toggled;
    // 2) update the search status text;
    // 3) search from the beginning of the page rather than from the current match position.
    ui.definition->findText( QString{}, QWebEnginePage::FindFlags{} );
    // Clearing the page selection is also necessary to search from the beginning of
    // the page rather than from the current match position. When text is empty, this
    // call is also useful: clears the selection of the last active findText() match.
    clearPageSelection();
#endif // USE_QTWEBKIT
  }

  WebPage::FindFlags f = caseSensitivityFindFlags( ui.searchCaseSensitive->isChecked() );
  if ( backwards )
    f |= WebPage::FindBackward;

#ifdef USE_QTWEBKIT
  bool const noResults = !text.isEmpty() && !ui.definition->findText( text, f );
  updateSearchNoResultsProperty( noResults );
#else
  if( !text.isEmpty() )
    ui.definition->findText( text, f );
  // findTextFinished() will update the search status text and noResults property asynchronously.
#endif
}

#ifndef USE_QTWEBKIT
namespace {
struct FindTextUiStatus
{
  QString statusText;
  bool noResults;
};
FindTextUiStatus computeFindTextUiStatus( QString const & searchedText,
                                          QWebEngineFindTextResult const & result )
{
  if( searchedText.isEmpty() )
    return { QString(), false }; // An empty text is never found. Don't warn about it.

  auto const matchCount = result.numberOfMatches();
  if( matchCount == 0 )
    return { searchStatusMessageNoMatches(), true };

  return { searchStatusMessage( result.activeMatch(), matchCount ), false };
}

} // unnamed namespace

void ArticleView::findTextFinished( QWebEngineFindTextResult const & result )
{
  if( !searchIsOpened || skipNextFindTextUiStatusUpdate )
  {
    skipNextFindTextUiStatusUpdate = false;
    return;
  }
  auto const uiStatus = computeFindTextUiStatus( ui.searchText->text(), result );
  ui.searchStatusLabel->setText( uiStatus.statusText );
  updateSearchNoResultsProperty( uiStatus.noResults );
}
#endif // USE_QTWEBKIT

void ArticleView::updateSearchNoResultsProperty( bool noResults )
{
  if( ui.searchText->property( "noResults" ).toBool() != noResults )
  {
    ui.searchText->setProperty( "noResults", noResults );
    reloadStyleSheet();
  }
}

void ArticleView::reloadStyleSheet()
{
  for( QWidget * w = parentWidget(); w; w = w->parentWidget() )
  {
    if ( w->styleSheet().size() )
    {
      w->setStyleSheet( w->styleSheet() );
      break;
    }
  }
}

// TODO (Qt WebEngine): If the search is open when a new translation is requested, navigating back to the page
// where the search was open restores wrong vertical scroll position. Consider either not closing the search
// automatically, or closing it at a different time, or remembering it in web history and restoring,
// or remembering the vertical scroll position in web history and restoring it as the Qt WebKit version does.
// Note that the web engine's automatic vertical scroll position restoration is unreliable: some positions are
// stable and get restored correctly, others shift slightly when restored, even if the search is not involved.
bool ArticleView::closeSearch()
{
  if ( searchIsOpened )
  {
    // Give focus to the view to enable keyboard navigation on the page.
    // Transferring the focus before hiding the search frame works around the highlighting of and especially
    // the scrolling to the first link on the page when searchFrame has focus in the Qt WebEngine version.
    ui.definition->setFocus();

    ui.searchFrame->hide();
    searchIsOpened = false;

    return true;
  }
  else
  if( ftsSearchIsOpened )
  {
    allMatches.clear();
    uniqueMatches.clear();
    ftsPosition = 0;
    ftsSearchIsOpened = false;

    ui.ftsSearchFrame->hide();
    ui.definition->setFocus();

    WebPage::FindFlags flags;
#ifdef USE_QTWEBKIT
    flags |= QWebPage::HighlightAllOccurrences;
#endif
    ui.definition->findText( "", flags );

    return true;
  }
  else
    return false;
}

bool ArticleView::isSearchOpened()
{
  return searchIsOpened;
}

void ArticleView::showEvent( QShowEvent * ev )
{
  QFrame::showEvent( ev );

  if ( !searchIsOpened )
    ui.searchFrame->hide();

  if( !ftsSearchIsOpened )
    ui.ftsSearchFrame->hide();
}

void ArticleView::receiveExpandOptionalParts( bool expand )
{
  if( expandOptionalParts != expand )
    switchExpandOptionalParts();
}

void ArticleView::switchExpandOptionalParts()
{
  expandOptionalParts = !expandOptionalParts;
  emit setExpandMode( expandOptionalParts );
  reload();
}

void ArticleView::copyAsText()
{
  QString text = ui.definition->selectedText();
  if( !text.isEmpty() )
    QApplication::clipboard()->setText( text );
}

void ArticleView::inspect()
{
  ui.definition->triggerPageAction( WebPage::InspectElement );
}

void ArticleView::highlightFTSResults()
{
  closeSearch();

  const QUrl & url = ui.definition->url();

  QString regString = Qt4x5::Url::queryItemValue( url, "regexp" );
  if( regString.isEmpty() )
    return;
  const bool ignoreDiacritics = Qt4x5::Url::hasQueryItem( url, "ignore_diacritics" );
  if( ignoreDiacritics )
    regString = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( regString ) ) );
  else
    regString = regString.remove( AccentMarkHandler::accentMark() );

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  QRegularExpression regexp;
  if( Qt4x5::Url::hasQueryItem( url, "wildcards" ) )
    regexp.setPattern( wildcardsToRegexp( regString ) );
  else
    regexp.setPattern( regString );

  QRegularExpression::PatternOptions patternOptions = QRegularExpression::DotMatchesEverythingOption
                                                      | QRegularExpression::UseUnicodePropertiesOption
                                                      | QRegularExpression::MultilineOption
                                                      | QRegularExpression::InvertedGreedinessOption;
  if( !Qt4x5::Url::hasQueryItem( url, "matchcase" ) )
    patternOptions |= QRegularExpression::CaseInsensitiveOption;
  regexp.setPatternOptions( patternOptions );

  if( regexp.pattern().isEmpty() || !regexp.isValid() )
    return;
#else
  QRegExp regexp( regString,
                  Qt4x5::Url::hasQueryItem( url, "matchcase" ) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                  Qt4x5::Url::hasQueryItem( url, "wildcards" ) ? QRegExp::WildcardUnix : QRegExp::RegExp2 );


  if( regexp.pattern().isEmpty() )
    return;

  regexp.setMinimal( true );
#endif

#ifdef USE_QTWEBKIT
  // Clear any current selection. QWebView::selectedText() is not empty here if the user full-text-searches
  // for a common word, picks a result with many large articles in the Full-text search dialog, then quickly
  // selects text in the first article while the page is still loading.
  // At this point QWebEngineView::selectedText() is equal to the last highlighted word if the user selects
  // a second result in the Full-text search dialog without clicking on the article view after the previous
  // FTS result was displayed there. This existing selection is not a problem in the Qt WebEngine version =>
  // don't waste CPU time and risk bugs by clearing it.
  if ( ui.definition->selectedText().size() )
    clearPageSelection();

  QString pageText = ui.definition->page()->currentFrame()->toPlainText();
  highlightFTSResults( regexp, ignoreDiacritics, pageText );
#else
  ui.definition->page()->toPlainText( [ this, url, regexp, ignoreDiacritics ]( QString const & pageText) {
    if( pageText.isEmpty() )
      return; // Most likely this callback is being called during page destruction. Return now to prevent a crash.
    if( ui.definition->url() != url )
      return; // Anoter URL has been loaded before this callback got invoked => the highlighting is obsolete.
    highlightFTSResults( regexp, ignoreDiacritics, pageText );
  } );
#endif
}

// TODO (Qt WebEngine): there are multiple issues with the FTS highlighting and result navigation:
// 1. Only the words equal to the last found result are highlighted, not all FTS matches as in the Qt WebKit version.
// 2. QWebEngineView::findText() never finds the end-of-line character. Maybe some other characters too.
//    This breaks FTS result navigation on a page when such characters are matched in the search.
// Fixing or working around these QWebEngineView::findText() limitations may prove more difficult than
// implementing finding and highlighting regular expressions on a page via JavaScript.
// There seems to be no way to overcome these issues even using the Qt WebEngine's upstream Blink API:
// https://chromium.googlesource.com/chromium/src/+/HEAD/third_party/blink/public/mojom/frame/find_in_page.mojom
// One more issue affects the Qt WebKit version too: when the regexp matches at word boundaries (in the "Whole words"
// or the RegExp mode), QWeb[Engine]View::findText() may find extra results, because the plain-text search cannot be
// limited by boundaries. These extra results can mess up the FTS result navigation if not all elements of allMatches
// are equal (some matches could be skipped).
// So reimplementing this functionality in JavaScript could improve the Qt WebKit version too.
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
void ArticleView::highlightFTSResults( QRegularExpression const & regexp,
#else
void ArticleView::highlightFTSResults( QRegExp const & regexp,
#endif
                                       bool ignoreDiacritics, QString const & pageText )
{
  sptr< AccentMarkHandler > const marksHandler = ignoreDiacritics ?
                                                 new DiacriticsHandler : new AccentMarkHandler;
  marksHandler->setText( pageText );

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
  QRegularExpressionMatchIterator it = regexp.globalMatch( marksHandler->normalizedText() );
  while( it.hasNext() )
  {
    QRegularExpressionMatch match = it.next();

    // Mirror pos and matched length to original string
    int pos = match.capturedStart();
    int spos = marksHandler->mirrorPosition( pos );
    int matched = marksHandler->mirrorPosition( pos + match.capturedLength() ) - spos;

    // Add mark pos (if presented)
    while( spos + matched < pageText.length()
           && pageText[ spos + matched ].category() == QChar::Mark_NonSpacing )
      matched++;

    if( matched > FTS::MaxMatchLengthForHighlightResults )
    {
      gdWarning( "ArticleView::highlightFTSResults(): Too long match - skipped (matched length %i, allowed %i)",
                 match.capturedLength(), FTS::MaxMatchLengthForHighlightResults );
    }
    else
      allMatches.append( pageText.mid( spos, matched ) );
  }
#else
  int pos = 0;

  while( pos >= 0 )
  {
    pos = regexp.indexIn( marksHandler->normalizedText(), pos );
    if( pos >= 0 )
    {
      // Mirror pos and matched length to original string
      int spos = marksHandler->mirrorPosition( pos );
      int matched = marksHandler->mirrorPosition( pos + regexp.matchedLength() ) - spos;

      // Add mark pos (if presented)
      while( spos + matched < pageText.length()
             && pageText[ spos + matched ].category() == QChar::Mark_NonSpacing )
        matched++;

      if( matched > FTS::MaxMatchLengthForHighlightResults )
      {
        gdWarning( "ArticleView::highlightFTSResults(): Too long match - skipped (matched length %i, allowed %i)",
                   regexp.matchedLength(), FTS::MaxMatchLengthForHighlightResults );
      }
      else
        allMatches.append( pageText.mid( spos, matched ) );

      pos += regexp.matchedLength();
    }
  }
#endif

  ftsSearchMatchCase = Qt4x5::Url::hasQueryItem( ui.definition->url(), "matchcase" );

  WebPage::FindFlags const flags = caseSensitivityFindFlags( ftsSearchMatchCase );

  if( allMatches.isEmpty() )
    ui.ftsSearchStatusLabel->setText( searchStatusMessageNoMatches() );
  else
  {
#ifdef USE_QTWEBKIT
    highlightAllFtsOccurences( flags );
    if( ui.definition->findText( allMatches.at( 0 ), flags ) )
    {
        ui.definition->page()->currentFrame()->
               evaluateJavaScript( QString( "%1=window.getSelection().getRangeAt(0);_=0;" )
                                   .arg( rangeVarName ) );
    }
#else
    ui.definition->findText( allMatches.constFirst(), flags );
#endif
    Q_ASSERT( ftsPosition == 0 );
    ui.ftsSearchStatusLabel->setText( searchStatusMessage( 1, allMatches.size() ) );
  }

  ui.ftsSearchFrame->show();
  ui.ftsSearchPrevious->setEnabled( false );
  ui.ftsSearchNext->setEnabled( allMatches.size()>1 );

  ftsSearchIsOpened = true;
}

#ifdef USE_QTWEBKIT
void ArticleView::highlightAllFtsOccurences( QWebPage::FindFlags flags )
{
  flags |= QWebPage::HighlightAllOccurrences;

  // Usually allMatches contains mostly duplicates. Thus searching for each element of
  // allMatches to highlight them takes a long time => collect unique elements into a
  // set and search for them instead.
  // Don't use QList::toSet() or QSet's range constructor because they reserve space
  // for QList::size() elements, whereas the final QSet size is likely 1 or 2.
  QSet< QString > uniqueMatches;
  for( int x = 0; x < allMatches.size(); ++x )
  {
    QString const & match = allMatches.at( x );
    // Consider words that differ only in case equal if the search is case-insensitive.
    uniqueMatches.insert( ftsSearchMatchCase ? match : match.toLower() );
  }

  for( QSet< QString >::const_iterator it = uniqueMatches.constBegin(); it != uniqueMatches.constEnd(); ++it )
    ui.definition->findText( *it, flags );
}
#endif // USE_QTWEBKIT

void ArticleView::performFtsFindOperation( bool backwards )
{
  if( !ftsSearchIsOpened )
    return;

  if( allMatches.isEmpty() )
  {
    ui.ftsSearchStatusLabel->setText( searchStatusMessageNoMatches() );
    ui.ftsSearchNext->setEnabled( false );
    ui.ftsSearchPrevious->setEnabled( false );
    return;
  }

#ifndef USE_QTWEBKIT
  // QWebEngineView::findText() always wraps around document, but it does not always scroll to the
  // wrapped-around first or last result (bug?) => "disable" the wraparound by returning early here.
  if( ( backwards && ftsPosition == 0 ) || ( !backwards && ftsPosition == allMatches.size() - 1 ) )
    return;
#endif

  WebPage::FindFlags flags = caseSensitivityFindFlags( ftsSearchMatchCase );

#ifdef USE_QTWEBKIT
  // Restore saved highlighted selection
  ui.definition->page()->currentFrame()->
         evaluateJavaScript( QString( "var sel=window.getSelection();sel.removeAllRanges();sel.addRange(%1);_=0;" )
                             .arg( rangeVarName ) );

  bool res;
  if( backwards )
  {
    if( ftsPosition > 0 )
    {
      res = ui.definition->findText( allMatches.at( ftsPosition - 1 ),
                                     flags | QWebPage::FindBackward );
      ftsPosition -= 1;
    }
    else
      res = ui.definition->findText( allMatches.at( ftsPosition ),
                                     flags | QWebPage::FindBackward );

    ui.ftsSearchPrevious->setEnabled( res );
    if( !ui.ftsSearchNext->isEnabled() )
      ui.ftsSearchNext->setEnabled( res );
  }
  else
  {
    if( ftsPosition < allMatches.size() - 1 )
    {
      res = ui.definition->findText( allMatches.at( ftsPosition + 1 ), flags );
      ftsPosition += 1;
    }
    else
      res = ui.definition->findText( allMatches.at( ftsPosition ), flags );

    ui.ftsSearchNext->setEnabled( res );
    if( !ui.ftsSearchPrevious->isEnabled() )
      ui.ftsSearchPrevious->setEnabled( res );
  }

  // Store new highlighted selection
  ui.definition->page()->currentFrame()->
         evaluateJavaScript( QString( "%1=window.getSelection().getRangeAt(0);_=0;" )
                             .arg( rangeVarName ) );
#else
  // Clearing selection here ensures that the search continues from the last match,
  // no matter what the user clicked or selected since the last FTS find operation.
  clearPageSelection();

  if( backwards )
  {
    --ftsPosition;
    flags |= QWebEnginePage::FindBackward;
  }
  else
    ++ftsPosition;

  ui.definition->findText( allMatches.at( ftsPosition ), flags );

  ui.ftsSearchNext->setEnabled( ftsPosition != allMatches.size() - 1 );
  ui.ftsSearchPrevious->setEnabled( ftsPosition != 0 );
#endif // USE_QTWEBKIT

  ui.ftsSearchStatusLabel->setText( searchStatusMessage( ftsPosition + 1, allMatches.size() ) );
}

void ArticleView::on_ftsSearchPrevious_clicked()
{
  performFtsFindOperation( true );
}

void ArticleView::on_ftsSearchNext_clicked()
{
  performFtsFindOperation( false );
}

#ifdef USE_QTWEBKIT
#ifdef Q_OS_WIN32

void ArticleView::readTag( const QString & from, QString & to, int & count )
{
    QChar ch, prev_ch;
    bool inQuote = false, inDoublequote = false;

    to.append( ch = prev_ch = from[ count++ ] );
    while( count < from.size() )
    {
        ch = from[ count ];
        if( ch == '>' && !( inQuote || inDoublequote ) )
        {
            to.append( ch );
            break;
        }
        if( ch == '\'' )
            inQuote = !inQuote;
        if( ch == '\"' )
            inDoublequote = !inDoublequote;
        to.append( prev_ch = ch );
        count++;
    }
}

QString ArticleView::insertSpans( QString const & html )
{
    QChar ch;
    QString newContent;
    bool inSpan = false, escaped = false;

    /// Enclose every word in string (exclude tags) with <span></span>

    for( int i = 0; i < html.size(); i++ )
    {
        ch = html[ i ];
        if( ch == '&' )
        {
            escaped = true;
            if( inSpan )
            {
                newContent.append( "</span>" );
                inSpan = false;
            }
            newContent.append( ch );
            continue;
        }

        if( ch == '<' ) // Skip tag
        {
            escaped = false;
            if( inSpan )
            {
                newContent.append( "</span>" );
                inSpan = false;
            }
            readTag( html, newContent, i );
            continue;
        }

        if( escaped )
        {
            if( ch == ';' )
                escaped = false;
            newContent.append( ch );
            continue;
        }

        if( !inSpan && ( ch.isLetterOrNumber() || ch.isLowSurrogate() ) )
        {
            newContent.append( "<span>");
            inSpan = true;
        }

        if( inSpan && !( ch.isLetterOrNumber() || ch.isLowSurrogate() ) )
        {
            newContent.append( "</span>");
            inSpan = false;
        }

        if( ch.isLowSurrogate() )
        {
            newContent.append( ch );
            ch = html[ ++i ];
        }

        newContent.append( ch );
        if( ch == '-' && !( html[ i + 1 ] == ' ' || ( i > 0 && html[ i - 1 ] == ' ' ) ) )
            newContent.append( "<span style=\"font-size:0pt\"> </span>" );
    }
    if( inSpan )
        newContent.append( "</span>" );
    return newContent;
}

QString ArticleView::checkElement( QWebElement & elem, QPoint const & pt )
{
    /// Search for lower-level matching element

    QWebElement parentElem = elem;
    QWebElement childElem = elem.firstChild();
    while( !childElem.isNull() )
    {
      if( childElem.geometry().contains( pt ) )
      {
        parentElem = childElem;
        childElem = parentElem.firstChild();
        continue;
      }
      childElem = childElem.nextSibling();
    }

    return parentElem.toPlainText();
}

QString ArticleView::wordAtPoint( int x, int y )
{
  QString word;

  if( popupView )
    return word;

  QPoint pos = mapFromGlobal( QPoint( x, y ) );
  QWebFrame *frame = ui.definition->page()->frameAt( pos );
  if( !frame )
    return word;

  QPoint posWithScroll = pos + frame->scrollPosition();

  /// Find target HTML element

  QWebHitTestResult result = frame->hitTestContent( pos );
  QWebElement baseElem = result.enclosingBlockElement();

  if( baseElem.tagName().compare( "BODY" ) == 0 ||      /// Assume empty field position
      baseElem.tagName().compare( "HTML" ) == 0 ||
      baseElem.tagName().compare( "HEAD" ) == 0 )
    return word;

  /// Save selection position

  baseElem.evaluateJavaScript( "var __gd_sel=window.getSelection();"
                               "if(__gd_sel && __gd_sel.rangeCount>0) {"
                                 "__gd_SelRange=__gd_sel.getRangeAt(0);"
                                 "if(__gd_SelRange.collapsed) __gd_sel.removeAllRanges();"
                                 "else {"
                                   "__gd_StartTree=[]; __gd_EndTree=[];"
                                   "var __gd_baseRange=document.createRange();"
                                   "__gd_baseRange.selectNode(this);"
                                   "if(__gd_baseRange.comparePoint(__gd_SelRange.startContainer,0)==0) {"
                                     "__gd_StartOffset=__gd_SelRange.startOffset;"
                                     "var __gd_child=__gd_SelRange.startContainer;"
                                     "var __gd_parent='';"
                                     "if(__gd_child==this) __gd_StartTree.push(-1);"
                                     "else while(__gd_parent!=this) {"
                                       "var n=0; __gd_parent=__gd_child.parentNode;"
                                       "var __gd_el=__gd_parent.firstChild;"
                                       "while(__gd_el!=__gd_child) { n++; __gd_el=__gd_el.nextSibling; }"
                                       "__gd_StartTree.push(n);"
                                       "__gd_child=__gd_parent;"
                                     "}"
                                   "}"
                                   "if(__gd_baseRange.comparePoint(__gd_SelRange.endContainer,0)==0) {"
                                     "__gd_EndOffset=__gd_SelRange.endOffset;"
                                     "var __gd_child=__gd_SelRange.endContainer;"
                                     "var __gd_parent='';"
                                     "if(__gd_child==this) __gd_EndTree.push(-1);"
                                     "else while(__gd_parent!=this) {"
                                       "var n=0; __gd_parent=__gd_child.parentNode;"
                                       "var __gd_el=__gd_parent.firstChild;"
                                       "while(__gd_el!=__gd_child) { n++; __gd_el=__gd_el.nextSibling; }"
                                       "__gd_EndTree.push(n);"
                                       "__gd_child=__gd_parent;"
                                     "}"
                                   "}"
                                 "}"
                               "}"
                               );

  /// Enclose every word be <span> </span>

  QString content = baseElem.toInnerXml();
  QString newContent = insertSpans( content );

  /// Set new code and re-render it to fill geometry

  QImage img( baseElem.geometry().width(), baseElem.geometry().height(), QImage::Format_Mono );
  img.fill( 0 );
  QPainter painter( & img );

  baseElem.setInnerXml( newContent );
  baseElem.render( &painter );

  /// Search in all child elements and check it

  QWebElementCollection elemCollection = baseElem.findAll( "*" );
  foreach ( QWebElement elem, elemCollection )
  {
      if( elem.geometry().contains( posWithScroll ) )
          word = checkElement( elem, posWithScroll );
      if( !word.isEmpty() )
          break;
  }

  /// Restore old content
  baseElem.setInnerXml( content );

  /// Restore selection

  baseElem.evaluateJavaScript( "var flag=0;"
                               "if(__gd_StartTree && __gd_StartTree.length) {"
                                 "var __gd_el=this;"
                                 "while(__gd_StartTree.length) {"
                                   "__gd_el=__gd_el.firstChild;"
                                   "var n=__gd_StartTree.pop();"
                                   "if(n<0) __gd_el=this;"
                                   "else for(var i=0;i<n;i++) __gd_el=__gd_el.nextSibling;"
                                 "}"
                                 "__gd_SelRange.setStart(__gd_el, __gd_StartOffset);"
                                 "__gd_StartTree.splice(0,__gd_StartTree.length);"
                                 "flag+=1;"
                               "}"
                               "if(__gd_EndTree && __gd_EndTree.length) {"
                                 "var __gd_el=this;"
                                 "while(__gd_EndTree.length) {"
                                   "__gd_el=__gd_el.firstChild;"
                                   "var n=__gd_EndTree.pop();"
                                   "if(n<0) __gd_el=this;"
                                   "else for(var i=0;i<n;i++) __gd_el=__gd_el.nextSibling;"
                                 "}"
                                 "__gd_SelRange.setEnd(__gd_el, __gd_EndOffset);"
                                 "__gd_EndTree.splice(0,__gd_EndTree.length);"
                                 "flag+=1;"
                               "}"
                               "if(flag>0) {"
                                 "var __gd_sel=window.getSelection();"
                                 "__gd_sel.removeAllRanges();"
                                 "__gd_sel.addRange(__gd_SelRange);"
                               "}"
                               );

  return word;
}

#endif // Q_OS_WIN32
#endif // USE_QTWEBKIT

ResourceToSaveHandler::ResourceToSaveHandler(ArticleView * view, QString const & fileName ) :
  QObject( view ),
  fileName( fileName ),
  alreadyDone( false )
{
  connect( this, SIGNAL( statusBarMessage( QString, int, QPixmap ) ),
           view, SIGNAL( statusBarMessage( QString, int, QPixmap ) ) );
}

void ResourceToSaveHandler::addRequest( sptr<Dictionary::DataRequest> req )
{
  if( !alreadyDone )
  {
    downloadRequests.push_back( req );

    connect( req.get(), SIGNAL( finished() ),
             this, SLOT( downloadFinished() ) );
  }
}

void ResourceToSaveHandler::downloadFinished()
{
  if ( downloadRequests.empty() )
    return; // Stray signal

  // Find any finished resources
  for( list< sptr< Dictionary::DataRequest > >::iterator i =
       downloadRequests.begin(); i != downloadRequests.end(); )
  {
    if ( (*i)->isFinished() )
    {
      if ( (*i)->dataSize() >= 0 && !alreadyDone )
      {
        QByteArray resourceData;
        vector< char > const & data = (*i)->getFullData();
        resourceData = QByteArray( data.data(), data.size() );

        emit downloaded( fileName, &resourceData );

        // Write data to file

        if ( !fileName.isEmpty() )
        {
          QFileInfo fileInfo( fileName );
          QDir().mkpath( fileInfo.absoluteDir().absolutePath() );

          QFile file( fileName );
          if ( file.open( QFile::WriteOnly ) )
          {
            file.write( resourceData.data(), resourceData.size() );
            file.close();
          }

          if ( file.error() )
          {
            emit statusBarMessage(
                  tr( "ERROR: %1" ).arg( tr( "Resource saving error: " ) + file.errorString() ),
                  10000, QPixmap( ":/icons/error.png" ) );
          }
        }
        alreadyDone = true;

        // Clear other requests

        downloadRequests.clear();
        break;
      }
      else
      {
        // This one had no data. Erase it.
        downloadRequests.erase( i++ );
      }
    }
    else // Unfinished, wait.
      break;
  }

  if ( downloadRequests.empty() )
  {
    if( !alreadyDone )
    {
      emit statusBarMessage(
            tr( "WARNING: %1" ).arg( tr( "The referenced resource failed to download." ) ),
            10000, QPixmap( ":/icons/error.png" ) );
    }
    emit done();
    deleteLater();
  }
}

#include "articleview.moc"
