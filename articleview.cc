/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articleview.hh"
#include "QtCore/qvariant.h"
#include "folding.hh"
#include "fulltextsearch.hh"
#include "gddebug.hh"
#include "gestures.hh"
#include "programs.hh"
#include "utils.hh"
#include "webmultimediadownload.hh"
#include "weburlrequestinterceptor.h"
#include "wildcard.hh"
#include "wstring_qt.hh"
#include <QClipboard>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QRegularExpression>
#include <QWebChannel>
#include <QWebEngineHistory>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineSettings>
#include <assert.h>
#include <map>
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <QWebEngineContextMenuData>
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QRegExp>
#include <QWebEngineContextMenuRequest>
#include <QWebEngineFindTextResult>
#endif
#ifdef Q_OS_WIN32
#include <windows.h>
#include <QPainter>
#endif

#include <QBuffer>

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
#include "speechclient.hh"
#endif

#include "globalbroadcaster.h"
using std::map;
using std::list;

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
      if( *nextChar >= 0x10000U )
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

void ArticleView::emitJavascriptFinished(){
    emit notifyJavascriptFinished();
}

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

} // unnamed namespace

QString ArticleView::scrollToFromDictionaryId( QString const & dictionaryId )
{
  Q_ASSERT( !isScrollTo( dictionaryId ) );
  return scrollToPrefix + dictionaryId;
}

ArticleView::ArticleView( QWidget * parent, ArticleNetworkAccessManager & nm, AudioPlayerPtr const & audioPlayer_,
                          std::vector< sptr< Dictionary::Class > > const & allDictionaries_,
                          Instances::Groups const & groups_, bool popupView_, Config::Class const & cfg_,
                          QAction & openSearchAction_, QAction * dictionaryBarToggled_,
                          GroupComboBox const * groupComboBox_ ) :
  QFrame( parent ),
  articleNetMgr( nm ),
  audioPlayer( audioPlayer_ ),
  allDictionaries( allDictionaries_ ),
  groups( groups_ ),
  popupView( popupView_ ),
  cfg( cfg_ ),
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

  ui.definition->setUp( const_cast< Config::Class * >( &cfg ) );

  goBackAction.setShortcut( QKeySequence( "Alt+Left" ) );
  ui.definition->addAction( &goBackAction );
  connect( &goBackAction, SIGNAL( triggered() ),
           this, SLOT( back() ) );

  goForwardAction.setShortcut( QKeySequence( "Alt+Right" ) );
  ui.definition->addAction( &goForwardAction );
  connect( &goForwardAction, SIGNAL( triggered() ),
           this, SLOT( forward() ) );

  ui.definition->pageAction( QWebEnginePage::Copy )->setShortcut( QKeySequence::Copy );
  ui.definition->addAction( ui.definition->pageAction( QWebEnginePage::Copy ) );

  QAction * selectAll = ui.definition->pageAction( QWebEnginePage::SelectAll );
  selectAll->setShortcut( QKeySequence::SelectAll );
  selectAll->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  ui.definition->addAction( selectAll );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

  connect(ui.definition, SIGNAL(loadFinished(bool)), this,
          SLOT(loadFinished(bool)));

  connect(ui.definition, SIGNAL(loadProgress(int)), this,
          SLOT(loadProgress(int)));
  connect( ui.definition, SIGNAL( linkClicked( QUrl ) ), this, SLOT( linkClicked( QUrl ) ) );

  connect( ui.definition->page(), SIGNAL( titleChanged( QString  ) ),
           this, SLOT( handleTitleChanged( QString  ) ) );

  connect( ui.definition->page(), SIGNAL( urlChanged(QUrl) ),
           this, SLOT( handleUrlChanged(QUrl) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( ui.definition->page(), SIGNAL( linkHovered ( const QString &) ),
           this, SLOT( linkHovered ( const QString & ) ) );

  connect( ui.definition, SIGNAL( doubleClicked( QPoint ) ),this,SLOT( doubleClicked( QPoint ) ) );

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


  connect(&inspectAction, &QAction::triggered, this, &ArticleView::inspectElement);

  ui.definition->installEventFilter( this );
  ui.searchFrame->installEventFilter( this );
  ui.ftsSearchFrame->installEventFilter( this );

  QWebEngineSettings * settings = ui.definition->settings();
  settings->setUnknownUrlSchemePolicy(QWebEngineSettings::UnknownUrlSchemePolicy::DisallowUnknownUrlSchemes);
#if( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) )
  settings->defaultSettings()->setAttribute( QWebEngineSettings::LocalContentCanAccessRemoteUrls, true );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::LocalContentCanAccessFileUrls, true );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::ErrorPageEnabled, false );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::PluginsEnabled, cfg.preferences.enableWebPlugins );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::PlaybackRequiresUserGesture, false );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::JavascriptCanAccessClipboard, true );
  settings->defaultSettings()->setAttribute( QWebEngineSettings::PrintElementBackgrounds, false );
#else
  settings->setAttribute( QWebEngineSettings::LocalContentCanAccessRemoteUrls, true );
  settings->setAttribute( QWebEngineSettings::LocalContentCanAccessFileUrls, true );
  settings->setAttribute( QWebEngineSettings::ErrorPageEnabled, false );
  settings->setAttribute( QWebEngineSettings::PluginsEnabled, cfg.preferences.enableWebPlugins );
  settings->setAttribute( QWebEngineSettings::PlaybackRequiresUserGesture, false );
  settings->setAttribute( QWebEngineSettings::JavascriptCanAccessClipboard, true );
  settings->setAttribute( QWebEngineSettings::PrintElementBackgrounds, false );
#endif

  expandOptionalParts = cfg.preferences.alwaysExpandOptionalParts;

  ui.definition->grabGesture( Gestures::GDPinchGestureType );
  ui.definition->grabGesture( Gestures::GDSwipeGestureType );

  // Variable name for store current selection range
  rangeVarName = QString( "sr_%1" ).arg( QString::number( (quint64)this, 16 ) );

  connect(GlobalBroadcaster::instance(), SIGNAL( dictionaryChanges(ActiveDictIds)), this,
          SLOT(setActiveDictIds(ActiveDictIds)));

  channel = new QWebChannel(ui.definition->page());
  agent = new ArticleViewAgent(this);
  attachWebChannelToHtml();
  ankiConnector = new AnkiConnector( this, cfg );
  connect( ankiConnector,
           &AnkiConnector::errorText,
           this,
           [ this ]( QString const & errorText ) { emit statusBarMessage( errorText ); } );
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
  //channel->deregisterObject(this);
  ui.definition->ungrabGesture( Gestures::GDPinchGestureType );
  ui.definition->ungrabGesture( Gestures::GDSwipeGestureType );
}

void ArticleView::showDefinition( Config::InputPhrase const & phrase, unsigned group,
                                  QString const & scrollTo,
                                  Contexts const & contexts_ )
{
  currentWord = phrase.phrase.trimmed();
  currentActiveDictIds.clear();
  // first, let's stop the player
  audioPlayer->stop();

  QUrl req;
  Contexts contexts( contexts_ );

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  Utils::Url::addQueryItem( req, "word", phrase.phrase );
  if ( !phrase.punctuationSuffix.isEmpty() )
    Utils::Url::addQueryItem( req, "punctuation_suffix", phrase.punctuationSuffix );
  Utils::Url::addQueryItem( req, "group", QString::number( group ) );
  if( cfg.preferences.ignoreDiacritics )
    Utils::Url::addQueryItem( req, "ignore_diacritics", "1" );

  if ( scrollTo.size() )
    Utils::Url::addQueryItem( req, "scrollto", scrollTo );

  Contexts::Iterator pos = contexts.find( "gdanchor" );
  if( pos != contexts.end() )
  {
    Utils::Url::addQueryItem( req, "gdanchor", contexts[ "gdanchor" ] );
    contexts.erase( pos );
  }

  if ( contexts.size() )
  {
    QBuffer buf;

    buf.open( QIODevice::WriteOnly );

    QDataStream stream( &buf );

    stream << contexts;

    buf.close();

    Utils::Url::addQueryItem( req,  "contexts", QString::fromLatin1( buf.buffer().toBase64() ) );
  }

  QString mutedDicts = getMutedForGroup( group );

  if ( mutedDicts.size() )
    Utils::Url::addQueryItem( req,  "muted", mutedDicts );

  // Update both histories (pages history and headwords history)
  saveHistoryUserData();
  emit sendWordToHistory( phrase.phrase );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

  ui.definition->load( req );

  //QApplication::setOverrideCursor( Qt::WaitCursor );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::showDefinition( QString const & word, unsigned group,
                                  QString const & scrollTo,
                                  Contexts const & contexts_ )
{
  showDefinition( Config::InputPhrase::fromPhrase( word ), group, scrollTo, contexts_ );
}

void ArticleView::showDefinition( QString const & word, QStringList const & dictIDs,
                                  QRegExp const & searchRegExp, unsigned group,
                                  bool ignoreDiacritics )
{
  if( dictIDs.isEmpty() )
    return;
  currentWord = word.trimmed();
  // first, let's stop the player
  audioPlayer->stop();

  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  Utils::Url::addQueryItem( req, "word", word );
  Utils::Url::addQueryItem( req, "dictionaries", dictIDs.join( ",") );
  Utils::Url::addQueryItem( req, "regexp", searchRegExp.pattern() );
  if( searchRegExp.caseSensitivity() == Qt::CaseSensitive )
    Utils::Url::addQueryItem( req, "matchcase", "1" );
  if( searchRegExp.patternSyntax() == QRegExp::WildcardUnix )
    Utils::Url::addQueryItem( req, "wildcards", "1" );
  Utils::Url::addQueryItem( req, "group", QString::number( group ) );
  if( ignoreDiacritics )
    Utils::Url::addQueryItem( req, "ignore_diacritics", "1" );

  // Update both histories (pages history and headwords history)
  saveHistoryUserData();
  emit sendWordToHistory( word );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

  ui.definition->load( req );

  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::sendToAnki(QString const & word, QString const & text ){
  ankiConnector->sendToAnki(word,text);
}

void ArticleView::showAnticipation()
{
  ui.definition->setHtml( "" );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::inspectElement()
{
  emit inspectSignal( ui.definition );
}

void ArticleView::loadFinished( bool result )
{
  setZoomFactor( cfg.preferences.zoomFactor );
  QUrl url = ui.definition->url();
  qDebug() << "article view loaded url:" << url.url().left( 200 );

  if( cfg.preferences.autoScrollToTargetArticle )
  {
    QString const scrollTo = Utils::Url::queryItemValue( url, "scrollto" );
    if( isScrollTo( scrollTo ) )
    {
      setCurrentArticle( scrollTo, true );
    }
  }

  ui.definition->unsetCursor();

  // Expand collapsed article if only one loaded
  ui.definition->page()->runJavaScript( QString( "gdCheckArticlesNumber();" ) );

  // Jump to current article after page reloading
  if( !articleToJump.isEmpty() )
  {
    setCurrentArticle( articleToJump, true );
    articleToJump.clear();
  }

  if( !Utils::Url::queryItemValue( url, "gdanchor" ).isEmpty() )
  {
    QString anchor = QUrl::fromPercentEncoding( Utils::Url::encodedQueryItemValue( url, "gdanchor" ) );

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

      int end = originalAnchor.indexOf('_');
      QString hash=originalAnchor.left(end);
      url.clear();
      url.setFragment(hash);
      ui.definition->page()->runJavaScript(
          QString("window.location.hash = \"%1\"").arg(QString::fromUtf8(url.toEncoded())));
      
    }
    else
    {
      url.clear();
      url.setFragment( anchor );
      ui.definition->page()->runJavaScript(
         QString( "window.location.hash = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
  }

  //the click audio url such as gdau://xxxx ,webview also emit a pageLoaded signal but with the result is false.need future investigation.
  //the audio link click ,no need to emit pageLoaded signal
  if(result){
    emit pageLoaded( this );
  }
  if( Utils::Url::hasQueryItem( ui.definition->url(), "regexp" ) )
    highlightFTSResults();
}

void ArticleView::loadProgress(int ){
    setZoomFactor(cfg.preferences.zoomFactor);
}

void ArticleView::handleTitleChanged( QString const & title )
{
  if( !title.isEmpty() ) // Qt 5.x WebKit raise signal titleChanges(QString()) while navigation within page
    emit titleChanged( this, title );
}

void ArticleView::handleUrlChanged( QUrl const & url )
{
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

unsigned ArticleView::getGroup( QUrl const & url )
{
  if ( url.scheme() == "gdlookup" && Utils::Url::hasQueryItem( url, "group" ) )
    return Utils::Url::queryItemValue( url, "group" ).toUInt();

  return 0;
}

QStringList ArticleView::getArticlesList()
{
  return currentActiveDictIds;
}

QString ArticleView::getActiveArticleId()
{
    return activeDictId;
}

void ArticleView::setActiveArticleId(QString const & dictId){
    this->activeDictId=dictId;
}

QString ArticleView::getCurrentArticle()
{
  QString dictId=getActiveArticleId();
  return scrollToFromDictionaryId( dictId );
}

void ArticleView::jumpToDictionary( QString const & id, bool force )
{
  QString targetArticle = scrollToFromDictionaryId( id );

  // jump only if neceessary, or when forced
  if ( force || targetArticle != getCurrentArticle() )
  {
    setCurrentArticle( targetArticle, true );
  }
}

bool ArticleView::setCurrentArticle( QString const & id, bool moveToIt )
{
  if ( !isScrollTo( id ) )
    return false; // Incorrect id

  if ( !ui.definition->isVisible() )
    return false; // No action on background page, scrollIntoView there don't work

  if(moveToIt){
    QString dictId = id.mid( 7 );
    if( dictId.isEmpty() )
      return false;
    QString script = QString( "var elem=document.getElementById('%1'); "
                              "if(elem!=undefined){elem.scrollIntoView(true);} gdMakeArticleActive('%2',true);" )
                       .arg( id, dictId );
    onJsActiveArticleChanged( id );
    ui.definition->page()->runJavaScript( script );
    setActiveArticleId( dictId );
  }
  return true;
}

void ArticleView::selectCurrentArticle()
{
  ui.definition->page()->runJavaScript(
        QString( "gdSelectArticle( '%1' );var elem=document.getElementById('%2'); if(elem!=undefined){elem.scrollIntoView(true);}" ).arg( getActiveArticleId() ,getCurrentArticle()) );
}

void ArticleView::isFramedArticle( QString const & ca, const std::function< void( bool ) > & callback )
{
  if( ca.isEmpty() )
    callback( false );

  ui.definition->page()->runJavaScript( QString( "!!document.getElementById('gdexpandframe-%1');" ).arg( ca.mid( 7 ) ),
                                        [ callback ]( const QVariant & res ) { callback( res.toBool() ); } );
}

bool ArticleView::isExternalLink( QUrl const & url )
{
  return Utils::isExternalLink(url);
}

void ArticleView::tryMangleWebsiteClickedUrl( QUrl & url, Contexts & contexts )
{
  // Don't try mangling audio urls, even if they are from the framed websites

  if( ( url.scheme() == "http" || url.scheme() == "https" ) && !Dictionary::WebMultimediaDownload::isAudioUrl( url ) )
  {
    // Maybe a link inside a website was clicked?

    QString ca = getCurrentArticle();
    isFramedArticle( ca,
                     [ &url, &contexts, &ca ]( bool framed )
                     {
                       if( framed )
                       {
                         // QVariant result = runJavaScriptSync( ui.definition->page(), "gdLastUrlText" );
                         QVariant result;

                         if( result.type() == QVariant::String )
                         {
                           // Looks this way
                           contexts[ dictionaryIdFromScrollTo( ca ) ] = QString::fromLatin1( url.toEncoded() );

                           QUrl target;

                           QString queryWord = result.toString();

                           // Empty requests are treated as no request, so we work this around by
                           // adding a space.
                           if( queryWord.isEmpty() )
                             queryWord = " ";

                           target.setScheme( "gdlookup" );
                           target.setHost( "localhost" );
                           target.setPath( "/" + queryWord );

                           url = target;
                         }
                       }
                     } );
  }
}

void ArticleView::updateCurrentArticleFromCurrentFrame( QWebEnginePage * frame ,QPoint * point)
{

}

void ArticleView::saveHistoryUserData()
{
  ui.definition->setProperty("sx", ui.definition->page()->scrollPosition().x());
  ui.definition->setProperty("sy", ui.definition->page()->scrollPosition().y());
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
#ifdef Q_OS_MAC

  if( ev->type() == QEvent::NativeGesture )
  {
    qDebug() << "it's a Native Gesture!";
    // handle Qt::ZoomNativeGesture Qt::SmartZoomNativeGesture here
    // ignore swipe left/right.
    // QWebEngine can handle Qt::SmartZoomNativeGesture.
  }

#else
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

        QPoint angleDelta(0, delta);
        QPoint pixelDetal;
        QWidget *child = widget->childAt( widget->mapFromGlobal( pt ) );
        if( child )
        {
          QWheelEvent whev( child->mapFromGlobal( pt ), pt,  pixelDetal,angleDelta, Qt::NoButton, Qt::NoModifier,Qt::NoScrollPhase,false);
          qApp->sendEvent( child, &whev );
        }
        else
        {
          QWheelEvent whev( widget->mapFromGlobal( pt ), pt,pixelDetal, angleDelta,Qt::NoButton, Qt::NoModifier,Qt::NoScrollPhase,false );
          qApp->sendEvent( widget, &whev );
        }
      }
    }

    return handled;
  }
#endif

  if( ev->type() == QEvent::MouseMove )
  {
    if( Gestures::isFewTouchPointsPresented() )
    {
      ev->accept();
      return true;
    }
  }

  if ( handleF3( obj, ev ) )
  {
    return true;
  }

  if ( obj == ui.definition )
  {
    if ( ev->type() == QEvent::MouseButtonPress ) {
      QMouseEvent * event = static_cast< QMouseEvent * >( ev );
      if ( event->button() == Qt::XButton1 ) {
        back();
        return true;
      }
      if ( event->button() == Qt::XButton2 ) {
        forward();
        return true;
      }
    }
    else
    if ( ev->type() == QEvent::KeyPress || ev->type ()==QEvent::ShortcutOverride)
    {
      QKeyEvent * keyEvent = static_cast< QKeyEvent * >( ev );

      if ( keyEvent->modifiers() &
           ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) )
        return false; // A non-typing modifier is pressed

      if ( Utils::ignoreKeyEvent(keyEvent)||
           keyEvent->key() == Qt::Key_Return ||
           keyEvent->key() == Qt::Key_Enter )
        return false; // Those key have other uses than to start typing

      QString text = keyEvent->text();

      if ( text.size() )
      {
        emit typingEvent( text );
        return true;
      }
    }
    else if( ev->type() == QEvent::Wheel )
    {
      QWheelEvent * pe = static_cast< QWheelEvent * >( ev );
      if( pe->modifiers().testFlag( Qt::ControlModifier ) )
      {
        if( pe->angleDelta().y() > 0 )
        {
          zoomIn();
        }
        else
        {
          zoomOut();
        }
      }
    }
  }
  else
    return QFrame::eventFilter( obj, ev );

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

QStringList ArticleView::getMutedDictionaries(unsigned group) {
  if (dictionaryBarToggled && dictionaryBarToggled->isChecked()) {
    // Dictionary bar is active -- mute the muted dictionaries
    Instances::Group const *groupInstance = groups.findGroup(group);

    // Find muted dictionaries for current group
    Config::Group const *grp = cfg.getGroup(group);
    Config::MutedDictionaries const *mutedDictionaries;
    if (group == Instances::Group::AllGroupId)
      mutedDictionaries = popupView ? &cfg.popupMutedDictionaries : &cfg.mutedDictionaries;
    else
      mutedDictionaries = grp ? (popupView ? &grp->popupMutedDictionaries : &grp->mutedDictionaries) : 0;
    if (!mutedDictionaries)
      return QStringList();

    QStringList mutedDicts;

    if (groupInstance) {
      for (unsigned x = 0; x < groupInstance->dictionaries.size(); ++x) {
        QString id = QString::fromStdString(groupInstance->dictionaries[x]->getId());

        if (mutedDictionaries->contains(id))
          mutedDicts.append(id);
      }
    }

    return mutedDicts;
  }

  return QStringList();
}

void ArticleView::linkHovered ( const QString & link )
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

    if( Utils::Url::hasQueryItem( url, "dict" ) )
    {
      // Link to other dictionary
      QString dictName( Utils::Url::queryItemValue( url, "dict" ) );
      if( !dictName.isEmpty() )
        msg = tr( "Definition from dictionary \"%1\": %2" ).arg( dictName , def );
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

void ArticleView::attachWebChannelToHtml() {
  // set the web channel to be used by the page
  // see http://doc.qt.io/qt-5/qwebenginepage.html#setWebChannel
  ui.definition->page()->setWebChannel(channel, QWebEngineScript::MainWorld);

  // register QObjects to be exposed to JavaScript
  channel->registerObject(QStringLiteral("articleview"), agent);
}

void ArticleView::linkClicked( QUrl const & url_ )
{
  Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();

  // Lock jump on links while Alt key is pressed
  if( kmod & Qt::AltModifier )
    return;

  updateCurrentArticleFromCurrentFrame();

  QUrl url( url_ );
  Contexts contexts;

  tryMangleWebsiteClickedUrl( url, contexts );

  if ( !popupView &&
       ( ui.definition->isMidButtonPressed() ||
         ( kmod & ( Qt::ControlModifier | Qt::ShiftModifier ) ) ) )
  {
    // Mid button or Control/Shift is currently pressed - open the link in new tab
    ui.definition->resetMidButtonPressed();
    emit openLinkInNewTab( url, ui.definition->url(), getCurrentArticle(), contexts );
  }
  else
    openLink( url, ui.definition->url(), getCurrentArticle(), contexts );
}

void ArticleView::linkClickedInHtml( QUrl const & url_ )
{
  emit ui.definition->linkClickedInHtml(url_);
  if(!url_.isEmpty())
  {
    linkClicked( url_ );
  }
}
void ArticleView::openLink( QUrl const & url, QUrl const & ref,
                            QString const & scrollTo,
                            Contexts const & contexts_ )
{
  audioPlayer->stop();
  qDebug() << "open link url:" << url;

  Contexts contexts( contexts_ );

  if( url.scheme().compare( "gdpicture" ) == 0 )
    ui.definition->load( url );
  else
  if ( url.scheme().compare( "bword" ) == 0 )
  {
    if( Utils::Url::hasQueryItem( ref, "dictionaries" ) )
    {
      QStringList dictsList = Utils::Url::queryItemValue( ref, "dictionaries" )
                                          .split( ",", Qt::SkipEmptyParts );

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
    {
      ui.definition->page()->runJavaScript(
        QString( "window.location = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
    else
    {
      if( Utils::Url::hasQueryItem( ref, "dictionaries" ) )
      {
        // Specific dictionary group from full-text search
        QStringList dictsList = Utils::Url::queryItemValue( ref, "dictionaries" )
                                            .split( ",", Qt::SkipEmptyParts );

        showDefinition( url.path().mid( 1 ), dictsList, QRegExp(), getGroup( ref ), false );
        return;
      }

      QString word;

      if( Utils::Url::hasQueryItem( url, "word" ) )
      {
          word=Utils::Url::queryItemValue (url,"word");
      }
      else{
          word=url.path ().mid (1);
      }

      QString newScrollTo( scrollTo );
      if( Utils::Url::hasQueryItem( url, "dict" ) )
      {
        // Link to other dictionary
        QString dictName( Utils::Url::queryItemValue( url, "dict" ) );
        for( unsigned i = 0; i < allDictionaries.size(); i++ )
        {
          if( dictName.compare( QString::fromUtf8( allDictionaries[ i ]->getName().c_str() ) ) == 0 )
          {
            newScrollTo = scrollToFromDictionaryId( QString::fromUtf8( allDictionaries[ i ]->getId().c_str() ) );
            break;
          }
        }
      }

      if( Utils::Url::hasQueryItem( url, "gdanchor" ) )
        contexts[ "gdanchor" ] = Utils::Url::queryItemValue( url, "gdanchor" );

      showDefinition( word,
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
          QString preferredName = Utils::Url::fragment( url );
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
                tr("ERROR: %1").arg(e.what()),
                10000, QPixmap(":/icons/error.svg"));
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
                tr("ERROR: %1").arg(e.what()),
                10000, QPixmap(":/icons/error.svg"));
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
    QString md5Id = Utils::Url::queryItemValue( url, "engine" );
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

ResourceToSaveHandler * ArticleView::saveResource( const QUrl & url, const QString & fileName )
{
  return saveResource( url, ui.definition->url(), fileName );
}

ResourceToSaveHandler * ArticleView::saveResource( const QUrl & url, const QUrl & ref, const QString & fileName )
{
  ResourceToSaveHandler * handler = new ResourceToSaveHandler( this, fileName );
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
          QString preferredName = Utils::Url::fragment( url );
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
                  return handler;
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
                    Utils::Url::path( url ).mid( 1 ).toUtf8().data() );

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
        tr("ERROR: %1").arg(tr("The referenced resource doesn't exist.")),
        10000, QPixmap(":/icons/error.svg"));
  }

  // Check already finished downloads
  handler->downloadFinished();

  return handler;
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

  if ( Utils::Url::queryItemValue( currentUrl, "muted" ) != mutedDicts )
  {
    // The list has changed -- update the url

    Utils::Url::removeQueryItem( currentUrl, "muted" );

    if ( mutedDicts.size() )
    Utils::Url::addQueryItem( currentUrl, "muted", mutedDicts );

    saveHistoryUserData();

    ui.definition->load( currentUrl );

    //QApplication::setOverrideCursor( Qt::WaitCursor );
    ui.definition->setCursor( Qt::WaitCursor );
  }
}

bool ArticleView::canGoBack()
{
  // First entry in a history is always an empty page,
  // so we skip it.
  return ui.definition->history()->currentItemIndex() > 1;
}

bool ArticleView::canGoForward()
{
  return ui.definition->history()->canGoForward();
}

void ArticleView::setSelectionBySingleClick( bool set )
{
  ui.definition->setSelectionBySingleClick( set );
}

void ArticleView::back()
{
  // Don't allow navigating back to page 0, which is usually the initial
  // empty page
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

void ArticleView::hasSound( const std::function< void( bool ) > & callback )
{
  ui.definition->page()->runJavaScript( "gdAudioLinks.first",
                                        [ callback ]( const QVariant & v )
                                        {
                                          bool has = false;
                                          if( v.type() == QVariant::String )
                                            has = !v.toString().isEmpty();
                                          callback( has );
                                        } );
}

//use webengine javascript to playsound
void ArticleView::playSound()
{
  QString variable = " (function(){  var link=gdAudioLinks[gdAudioLinks.current];           "
    "   if(link==undefined){           "
    "       link=gdAudioLinks.first;           "
    "   }          "
    "    return link;})();         ";

  ui.definition->page()->runJavaScript(variable,[this](const QVariant & result){
      if (result.type() == QVariant::String) {
          QString soundScript = result.toString();
          if (!soundScript.isEmpty())
            openLink(QUrl::fromEncoded(soundScript.toUtf8()), ui.definition->url());
      }
  });
}

// use eventloop to turn the async callback to sync execution.
void ArticleView::toHtml( const std::function< void( QString & ) > & callback )
{
  ui.definition->page()->toHtml(
    [ = ]( const QString & content )
    {
      QString html = content;
      callback( html );
    } );
}

void ArticleView::setHtml(const QString& content,const QUrl& baseUrl){
    ui.definition->page()->setHtml(content,baseUrl);
}

void ArticleView::setContent(const QByteArray &data, const QString &mimeType, const QUrl &baseUrl ){
    ui.definition->page()->setContent(data,mimeType,baseUrl);
}

QString ArticleView::getTitle()
{
  return ui.definition->page()->title();
}

Config::InputPhrase ArticleView::getPhrase() const
{
  const QUrl url = ui.definition->url();
  return Config::InputPhrase( Utils::Url::queryItemValue( url, "word" ),
                              Utils::Url::queryItemValue( url, "punctuation_suffix" ) );
}

void ArticleView::print( QPrinter * printer ) const
{
  QEventLoop loop;
  bool result;
  auto printPreview = [ & ]( bool success )
  {
    result = success;
    loop.quit();
  };
#if( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) )
  ui.definition->page()->print( printer, std::move( printPreview ) );
#else
  connect( ui.definition, &QWebEngineView::printFinished, &loop, std::move( printPreview ) );
  ui.definition->print( printer );
#endif
  loop.exec();
  if( !result )
  {
    qDebug() << "print failed";
  }
}

void ArticleView::contextMenuRequested( QPoint const & pos )
{
  // Is that a link? Is there a selection?
  QWebEnginePage* r=ui.definition->page();
  updateCurrentArticleFromCurrentFrame(ui.definition->page(), const_cast<QPoint *>(& pos));

  QMenu menu( this );

  QAction * followLink = 0;
  QAction * followLinkExternal = 0;
  QAction * followLinkNewTab = 0;
  QAction * lookupSelection = 0;
  QAction * sendToAnkiAction = 0 ;
  QAction * lookupSelectionGr = 0;
  QAction * lookupSelectionNewTab = 0;
  QAction * lookupSelectionNewTabGr = 0;
  QAction * maxDictionaryRefsAction = 0;
  QAction * addWordToHistoryAction = 0;
  QAction * addHeaderToHistoryAction = 0;
  QAction * sendWordToInputLineAction = 0;
  QAction * saveImageAction = 0;
  QAction * saveSoundAction           = 0;

#if( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) )
  const QWebEngineContextMenuData * menuData = &(r->contextMenuData());
#else
  QWebEngineContextMenuRequest * menuData = ui.definition->lastContextMenuRequest();
#endif
  QUrl targetUrl(menuData->linkUrl());
  Contexts contexts;

  tryMangleWebsiteClickedUrl( targetUrl, contexts );

  if ( !targetUrl.isEmpty() )
  {
    if ( !isExternalLink( targetUrl ) )
    {
      followLink = new QAction( tr( "&Open Link" ), &menu );
      menu.addAction( followLink );

      if ( !popupView )
      {
        followLinkNewTab = new QAction( QIcon( ":/icons/addtab.svg" ),
                                        tr( "Open Link in New &Tab" ), &menu );
        menu.addAction( followLinkNewTab );
      }
    }

    if ( isExternalLink( targetUrl ) )
    {
      followLinkExternal = new QAction( tr( "Open Link in &External Browser" ), &menu );
      menu.addAction( followLinkExternal );
      menu.addAction( ui.definition->pageAction( QWebEnginePage::CopyLinkToClipboard ) );
    }
  }

  QUrl imageUrl;
#if( QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 ) )
  if( !popupView && menuData->mediaType ()==QWebEngineContextMenuData::MediaTypeImage)
#else
  if( !popupView && menuData->mediaType ()==QWebEngineContextMenuRequest::MediaType::MediaTypeImage)
#endif
  {
    imageUrl = menuData->mediaUrl ();
      if( !imageUrl.isEmpty() )
      {
          menu.addAction( ui.definition->pageAction( QWebEnginePage::CopyImageToClipboard ) );
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
      lookupSelectionNewTab = new QAction( QIcon( ":/icons/addtab.svg" ),
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
        lookupSelectionNewTabGr = new QAction( QIcon( ":/icons/addtab.svg" ),
                                               tr( "Look up \"%1\" in %2 in &New Tab" ).
                                               arg( text ).
                                               arg( altGroup->name ), &menu );
        menu.addAction( lookupSelectionNewTabGr );
      }
    }
  }

  // add anki menu
  if( !text.isEmpty() && cfg.preferences.ankiConnectServer.enabled )
  {
    QString txt      = ui.definition->title();
    sendToAnkiAction = new QAction( tr( "&Send \"%1\" to anki with selected text." ).arg( txt ), &menu );
    menu.addAction( sendToAnkiAction );
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
    menu.addAction( ui.definition->pageAction( QWebEnginePage::Copy ) );
    menu.addAction( &copyAsTextAction );
  }
  else
  {
    menu.addAction( &selectCurrentArticleAction );
    menu.addAction( ui.definition->pageAction( QWebEnginePage::SelectAll ) );
  }

  map< QAction *, QString > tableOfContents;

  // Add table of contents
  QStringList ids = getArticlesList();

  if ( !menu.isEmpty() && ids.size() )
    menu.addSeparator();

  unsigned refsAdded = 0;
  bool maxDictionaryRefsReached = false;

  for( QStringList::const_iterator i = ids.constBegin(); i != ids.constEnd();
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
      openLink( targetUrl, ui.definition->url(), getCurrentArticle(), contexts );
    else
    if ( result == followLinkExternal )
      QDesktopServices::openUrl( targetUrl );
    else
    if ( result == lookupSelection )
      showDefinition( selectedText, getGroup( ui.definition->url() ), getCurrentArticle() );
    else if( result == sendToAnkiAction )
    {
      sendToAnki( ui.definition->title(), ui.definition->selectedText() );
    }
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
      emit openLinkInNewTab( targetUrl, ui.definition->url(), getCurrentArticle(), contexts );
    else
    if ( !popupView && result == lookupSelectionNewTab )
      emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                   getCurrentArticle(), Contexts() );
    else
    if ( !popupView && result == lookupSelectionNewTabGr && groupComboBox )
      emit showDefinitionInNewTab( selectedText, groupComboBox->getCurrentGroup(),
                                   QString(), Contexts() );
    else
    if( result == saveImageAction || result == saveSoundAction )
    {
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

      QString name = Utils::Url::path( url ).section( '/', -1 );

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
        saveResource( url, ui.definition->url(), fileName );
      }
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

  qDebug()<< "title = "<< r->title();

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
        tr("WARNING: %1").arg(tr("The referenced resource failed to download.")),
        10000, QPixmap(":/icons/error.svg"));
  }
}

void ArticleView::audioPlayerError( QString const & message )
{
  emit statusBarMessage(tr("WARNING: Audio Player: %1").arg(message),
                        10000, QPixmap(":/icons/error.svg"));
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
    showDefinition( phrase, groupId, getCurrentArticle() );
  }
}

void ArticleView::moveOneArticleUp()
{
  QString current = getCurrentArticle();

  if ( current.size() )
  {
    QStringList lst = getArticlesList();

    int idx = lst.indexOf( dictionaryIdFromScrollTo( current ) );

    if ( idx != -1 )
    {
      --idx;

      if ( idx < 0 )
        idx = lst.size() - 1;

      setCurrentArticle( scrollToFromDictionaryId( lst[ idx ] ), true );
    }
  }
}

void ArticleView::moveOneArticleDown()
{
  QString current = getCurrentArticle();
  QString currentDictId = dictionaryIdFromScrollTo( current );
  QStringList lst       = getArticlesList();
  // if current article is empty .use the first as default.
  if( currentDictId.isEmpty() && !lst.isEmpty() )
  {
    currentDictId = lst[ 0 ];
  }

  int idx = lst.indexOf( currentDictId );

  if( idx != -1 )
  {
    idx = ( idx + 1 ) % lst.size();

    setCurrentArticle( scrollToFromDictionaryId( lst[ idx ] ), true );
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
  {
    ui.definition->page()->
           runJavaScript( "window.getSelection().removeAllRanges();_=0;" );
  }

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
  performFindOperation( false, false, true );
}

void ArticleView::on_highlightAllButton_clicked()
{
  performFindOperation( false, false, true );
}

//the id start with "gdform-"
void ArticleView::onJsActiveArticleChanged(QString const & id)
{
  if ( !isScrollTo( id ) )
    return; // Incorrect id

  QString dictId = dictionaryIdFromScrollTo( id );
  setActiveArticleId( dictId );
  emit activeArticleChanged( this, dictId );
}

void ArticleView::doubleClicked( QPoint pos )
{
  // We might want to initiate translation of the selected word
  audioPlayer->stop();
  if ( cfg.preferences.doubleClickTranslates )
  {
    QString selectedText = ui.definition->selectedText();

    // ignore empty word;
    if( selectedText.isEmpty() )
      return;

    emit sendWordToInputLine( selectedText );
    // Do some checks to make sure there's a sensible selection indeed
    if ( Folding::applyWhitespaceOnly( gd::toWString( selectedText ) ).size() &&
         selectedText.size() < 60 )
    {
      // Initiate translation
      Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();
      if (kmod & (Qt::ControlModifier | Qt::ShiftModifier))
      { // open in new tab
        emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                     getCurrentArticle(), Contexts() );
      }
      else
      {
        QUrl const & ref = ui.definition->url();

        if( Utils::Url::hasQueryItem( ref, "dictionaries" ) )
        {
          QStringList dictsList = Utils::Url::queryItemValue(ref, "dictionaries" )
                                              .split( ",", Qt::SkipEmptyParts );
          showDefinition( selectedText, dictsList, QRegExp(), getGroup( ref ), false );
        }
        else
          showDefinition( selectedText, getGroup( ref ), getCurrentArticle() );
      }
    }
  }
}


void ArticleView::performFindOperation( bool restart, bool backwards, bool checkHighlight )
{
  QString text = ui.searchText->text();

  if ( restart || checkHighlight )
  {
    if( restart ) {
      // Anyone knows how we reset the search position?
      // For now we resort to this hack:
      if ( ui.definition->selectedText().size() )
      {
        ui.definition->page()->
               runJavaScript( "window.getSelection().removeAllRanges();_=0;" );
      }
    }

    QWebEnginePage::FindFlags f( 0 );

    if ( ui.searchCaseSensitive->isChecked() )
      f |= QWebEnginePage::FindCaseSensitively;

    ui.definition->findText( "", f );

    if( ui.highlightAllButton->isChecked() )
      ui.definition->findText( text, f );

    if( checkHighlight )
      return;
  }

  QWebEnginePage::FindFlags f( 0 );

  if ( ui.searchCaseSensitive->isChecked() )
    f |= QWebEnginePage::FindCaseSensitively;

  if ( backwards )
    f |= QWebEnginePage::FindBackward;

  bool setMark = text.size() && !findText(text, f);

  if ( ui.searchText->property( "noResults" ).toBool() != setMark )
  {
    ui.searchText->setProperty( "noResults", setMark );

    // Reload stylesheet
    reloadStyleSheet();
  }
}

bool ArticleView::findText(QString& text, const QWebEnginePage::FindFlags& f)
{
  bool r;
  // turn async to sync invoke.
  QSharedPointer<QEventLoop> loop = QSharedPointer<QEventLoop>(new QEventLoop());
  QTimer::singleShot(1000, loop.data(), &QEventLoop::quit);
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
  ui.definition->findText(text, f, [&](const QWebEngineFindTextResult& result)
                           {
                             if(loop->isRunning()){
                               r = result.numberOfMatches()>0;
                               loop->quit();
                             } });
#else
  ui.definition->findText(text, f, [&](bool result)
                           {
                             if(loop->isRunning()){
                               r = result;
                               loop->quit();
                             } });
#endif


  loop->exec();
  return r;
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


bool ArticleView::closeSearch()
{
  if ( searchIsOpened )
  {
    ui.searchFrame->hide();
    ui.definition->setFocus();
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

    QWebEnginePage::FindFlags flags ( 0 );

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
  {
    int n = getArticlesList().indexOf( getActiveArticleId() );
    if( n > 0 )
       articleToJump = getCurrentArticle();

    emit setExpandMode( expand );
    expandOptionalParts = expand;
    reload();
  }
}

void ArticleView::switchExpandOptionalParts()
{
  expandOptionalParts = !expandOptionalParts;

  int n = getArticlesList().indexOf( getActiveArticleId() );
  if( n > 0 )
    articleToJump = getCurrentArticle();

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
  ui.definition->triggerPageAction( QWebEnginePage::InspectElement );
}

void ArticleView::highlightFTSResults()
{
  closeSearch();

  // Clear any current selection
  if( ui.definition->selectedText().size() )
  {
    ui.definition->page()->runJavaScript( "window.getSelection().removeAllRanges();_=0;" );
  }

  ui.definition->page()->toPlainText(
    [ & ]( const QString pageText )
    {
      const QUrl & url = ui.definition->url();

      bool ignoreDiacritics = Utils::Url::hasQueryItem( url, "ignore_diacritics" );

      QString regString = Utils::Url::queryItemValue( url, "regexp" );
      if( ignoreDiacritics )
        regString = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( regString ) ) );
      else
        regString = regString.remove( AccentMarkHandler::accentMark() );

      QRegularExpression regexp;
      if( Utils::Url::hasQueryItem( url, "wildcards" ) )
        regexp.setPattern( wildcardsToRegexp( regString ) );
      else
        regexp.setPattern( regString );

      QRegularExpression::PatternOptions patternOptions =
        QRegularExpression::DotMatchesEverythingOption | QRegularExpression::UseUnicodePropertiesOption |
        QRegularExpression::MultilineOption | QRegularExpression::InvertedGreedinessOption;
      if( !Utils::Url::hasQueryItem( url, "matchcase" ) )
        patternOptions |= QRegularExpression::CaseInsensitiveOption;
      regexp.setPatternOptions( patternOptions );

      if( regexp.pattern().isEmpty() || !regexp.isValid() )
        return;
      sptr< AccentMarkHandler > marksHandler = ignoreDiacritics ? new DiacriticsHandler : new AccentMarkHandler;

      marksHandler->setText( pageText );

      QRegularExpressionMatchIterator it = regexp.globalMatch( marksHandler->normalizedText() );
      while( it.hasNext() )
      {
        QRegularExpressionMatch match = it.next();

        // Mirror pos and matched length to original string
        int pos     = match.capturedStart();
        int spos    = marksHandler->mirrorPosition( pos );
        int matched = marksHandler->mirrorPosition( pos + match.capturedLength() ) - spos;

        // Add mark pos (if presented)
        while( spos + matched < pageText.length() && pageText[ spos + matched ].category() == QChar::Mark_NonSpacing )
          matched++;

        if( matched > FTS::MaxMatchLengthForHighlightResults )
        {
          gdWarning( "ArticleView::highlightFTSResults(): Too long match - skipped (matched length %i, allowed %i)",
                     match.capturedLength(),
                     FTS::MaxMatchLengthForHighlightResults );
        }
        else
          allMatches.append( pageText.mid( spos, matched ) );
      }

      ftsSearchMatchCase = Utils::Url::hasQueryItem( url, "matchcase" );

      QWebEnginePage::FindFlags flags( QWebEnginePage::FindBackward );

      if( ftsSearchMatchCase )
        flags |= QWebEnginePage::FindCaseSensitively;

      if( !allMatches.isEmpty() )
      {
        highlightAllFtsOccurences( flags );
        ui.definition->findText( allMatches.at( 0 ), flags );
        // if( ui.definition->findText( allMatches.at( 0 ), flags ) )
        {
          ui.definition->page()->runJavaScript(
            QString( "%1=window.getSelection().getRangeAt(0);_=0;" ).arg( rangeVarName ) );
        }
      }

      ui.ftsSearchFrame->show();
      ui.ftsSearchPrevious->setEnabled( false );
      ui.ftsSearchNext->setEnabled( allMatches.size() > 1 );

      ftsSearchIsOpened = true;
    } );
}

void ArticleView::highlightAllFtsOccurences( QWebEnginePage::FindFlags flags )
{
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

void ArticleView::setActiveDictIds(ActiveDictIds ad) {
  // ignore all other signals.
  qDebug() << "receive dicts, current word:" << currentWord << ad.word << ":" << ad.dictIds;
  if (ad.word == currentWord) {
    qDebug() << "receive dicts, current word accept:" << currentWord;
    currentActiveDictIds << ad.dictIds;
    currentActiveDictIds.removeDuplicates();
    emit updateFoundInDictsList();
  }
}

//todo ,futher refinement?
void ArticleView::performFtsFindOperation( bool backwards )
{
  if( !ftsSearchIsOpened )
    return;

  if( allMatches.isEmpty() )
  {
    ui.ftsSearchNext->setEnabled( false );
    ui.ftsSearchPrevious->setEnabled( false );
    return;
  }

  QWebEnginePage::FindFlags flags( 0 );

  if( ftsSearchMatchCase )
    flags |= QWebEnginePage::FindCaseSensitively;


  // Restore saved highlighted selection
  ui.definition->page()->
         runJavaScript( QString( "var sel=window.getSelection();sel.removeAllRanges();sel.addRange(%1);_=0;" )
                             .arg( rangeVarName ) );

  if (backwards) {
      if (ftsPosition > 0) {
          ftsPosition -= 1;
      }
#if( QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 ) )
      ui.definition->findText( allMatches.at( ftsPosition ),
                               flags | QWebEnginePage::FindBackward,
                               [ this ]( const QWebEngineFindTextResult & result )
                               {
                                 if( result.numberOfMatches ()== 0 )
                                   return;
                                 ui.ftsSearchPrevious->setEnabled(true);
                                 if (!ui.ftsSearchNext->isEnabled())
                                   ui.ftsSearchNext->setEnabled(true);
                               });
#else
      ui.definition->findText(allMatches.at(ftsPosition),
                              flags | QWebEnginePage::FindBackward,
                              [this](bool res) {
                                  ui.ftsSearchPrevious->setEnabled(res);
                                  if (!ui.ftsSearchNext->isEnabled())
                                      ui.ftsSearchNext->setEnabled(res);
                              });
#endif
  } else {
      if (ftsPosition < allMatches.size() - 1) {
          ftsPosition += 1;
      }
#if( QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 ) )
      ui.definition->findText(allMatches.at(ftsPosition), flags, [this](const QWebEngineFindTextResult & result ) {
        if( result.numberOfMatches() == 0 )
          return;
        ui.ftsSearchNext->setEnabled(true);
        if (!ui.ftsSearchPrevious->isEnabled())
          ui.ftsSearchPrevious->setEnabled(true);
      });
  }
#else

      ui.definition->findText(allMatches.at(ftsPosition), flags, [this](bool res) {
          ui.ftsSearchNext->setEnabled(res);
          if (!ui.ftsSearchPrevious->isEnabled())
              ui.ftsSearchPrevious->setEnabled(res);
      });
  }
#endif
  // Store new highlighted selection
  ui.definition->page()->
         runJavaScript( QString( "%1=window.getSelection().getRangeAt(0);_=0;" )
                             .arg( rangeVarName ) );
}

void ArticleView::on_ftsSearchPrevious_clicked()
{
  performFtsFindOperation( true );
}

void ArticleView::on_ftsSearchNext_clicked()
{
  performFtsFindOperation( false );
}

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
                tr("ERROR: %1").arg(tr("Resource saving error: ") + file.errorString()),
                10000, QPixmap(":/icons/error.svg"));
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
          tr("WARNING: %1").arg(tr("The referenced resource failed to download.")),
          10000, QPixmap(":/icons/error.svg"));
    }
    emit done();
    deleteLater();
  }
}

ArticleViewAgent::ArticleViewAgent( ArticleView * articleView ) : QObject( articleView ), articleView( articleView )
{
}

void ArticleViewAgent::onJsActiveArticleChanged( QString const & id )
{
  articleView->onJsActiveArticleChanged( id );
}

void ArticleViewAgent::linkClickedInHtml( QUrl const & url )
{
  articleView->linkClickedInHtml( url );
}
