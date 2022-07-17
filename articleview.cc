/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articleview.hh"
#include <map>
#include <QMessageBox>
#include <QWebHitTestResult>
#include <QMenu>
#include <QDesktopServices>
#include <QWebHistory>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileDialog>
#include "articlewebpage.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include "webmultimediadownload.hh"
#include "programs.hh"
#include "gddebug.hh"
#include <QDebug>
#include <QCryptographicHash>
#include "gestures.hh"
#include "fulltextsearch.hh"

#if QT_VERSION >= 0x040600
#include <QWebElement>
#include <QWebElementCollection>
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
  void onPageJsReady( QVariantMap const & audioLinks, QString const & activeArticleId )
  { articleView.onPageJsReady( audioLinks, activeArticleId ); }

  void onJsActiveArticleChanged( QString const & id )
  { articleView.onJsActiveArticleChanged( id ); }

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

static QVariant evaluateJavaScriptVariableSafe( QWebFrame * frame, const QString & variable )
{
  return frame->evaluateJavaScript(
        QString( "( typeof( %1 ) !== 'undefined' && %1 !== undefined ) ? %1 : null;" )
        .arg( variable ) );
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

QWebPage::FindFlags caseSensitivityFindFlags( bool matchCase )
{
  return matchCase ? QWebPage::FindCaseSensitively : QWebPage::FindFlags();
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

} // unnamed namespace

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
  ui.definition->setUp( const_cast< Config::Class * >( &cfg ) );

  goBackAction.setShortcut( QKeySequence( "Alt+Left" ) );
  ui.definition->addAction( &goBackAction );
  connect( &goBackAction, SIGNAL( triggered() ),
           this, SLOT( back() ) );

  goForwardAction.setShortcut( QKeySequence( "Alt+Right" ) );
  ui.definition->addAction( &goForwardAction );
  connect( &goForwardAction, SIGNAL( triggered() ),
           this, SLOT( forward() ) );

  QAction * const copyAction = ui.definition->pageAction( QWebPage::Copy );
  copyAction->setShortcut( QKeySequence::Copy );
  ui.definition->addAction( copyAction );

  QAction * selectAll = ui.definition->pageAction( QWebPage::SelectAll );
  selectAll->setShortcut( QKeySequence::SelectAll );
  selectAll->setShortcutContext( Qt::WidgetWithChildrenShortcut );
  ui.definition->addAction( selectAll );

  ui.definition->setContextMenuPolicy( Qt::CustomContextMenu );

  ui.definition->page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );

  ui.definition->page()->setNetworkAccessManager( &articleNetMgr );

  connect( ui.definition, SIGNAL( loadFinished( bool ) ),
           this, SLOT( loadFinished( bool ) ) );

  attachToJavaScript();
  connect( ui.definition->page()->mainFrame(), SIGNAL( javaScriptWindowObjectCleared() ),
           this, SLOT( attachToJavaScript() ) );

  connect( ui.definition, SIGNAL( titleChanged( QString const & ) ),
           this, SLOT( handleTitleChanged( QString const & ) ) );

  connect( ui.definition, SIGNAL( urlChanged( QUrl const & ) ),
           this, SLOT( handleUrlChanged( QUrl const & ) ) );

  connect( ui.definition, SIGNAL( customContextMenuRequested( QPoint const & ) ),
           this, SLOT( contextMenuRequested( QPoint const & ) ) );

  connect( webPage, SIGNAL( linkClicked( QUrl const & ) ),
           this, SLOT( linkClicked( QUrl const & ) ) );

  connect( ui.definition->page(), SIGNAL( linkHovered ( const QString &, const QString &, const QString & ) ),
           this, SLOT( linkHovered ( const QString &, const QString &, const QString & ) ) );

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
  connect( &inspectAction, SIGNAL( triggered() ), this, SLOT( inspect() ) );

  ui.definition->installEventFilter( this );
  ui.searchFrame->installEventFilter( this );
  ui.ftsSearchFrame->installEventFilter( this );

  QWebSettings * settings = ui.definition->page()->settings();
  settings->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  settings->setAttribute( QWebSettings::LocalContentCanAccessFileUrls, true );

  // Load the default blank page instantly, so there would be no flicker.

  QString contentType;
  QUrl blankPage( "gdlookup://localhost?blank=1" );

  sptr< Dictionary::DataRequest > r = articleNetMgr.getResource( blankPage,
                                                                 contentType );

  ui.definition->setHtml( QString::fromUtf8( &( r->getFullData().front() ),
                                             r->getFullData().size() ),
                          blankPage );

  expandOptionalParts = cfg.preferences.alwaysExpandOptionalParts;

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
  ui.definition->grabGesture( Gestures::GDPinchGestureType );
  ui.definition->grabGesture( Gestures::GDSwipeGestureType );
#endif

  // Variable name for store current selection range
  rangeVarName = QString( "sr_%1" ).arg( QString::number( (quint64)this, 16 ) );
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

void ArticleView::showDefinition( QString const & word, QStringList const & dictIDs,
                                  QRegExp const & searchRegExp, unsigned group,
                                  bool ignoreDiacritics )
{
  if( dictIDs.isEmpty() )
    return;

  // first, let's stop the player
  audioPlayer->stop();

  QUrl req;

  req.setScheme( "gdlookup" );
  req.setHost( "localhost" );
  Qt4x5::Url::addQueryItem( req, "word", word );
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
  emit sendWordToHistory( word );

  // Any search opened is probably irrelevant now
  closeSearch();

  // Clear highlight all button selection
  ui.highlightAllButton->setChecked(false);

  emit setExpandMode( expandOptionalParts );

  load( req );

  //QApplication::setOverrideCursor( Qt::WaitCursor );
  ui.definition->setCursor( Qt::WaitCursor );
}

void ArticleView::showAnticipation()
{
  ui.definition->setHtml( "" );
  ui.definition->setCursor( Qt::WaitCursor );
  //QApplication::setOverrideCursor( Qt::WaitCursor );
}

void ArticleView::loadFinished( bool )
{
  QUrl url = ui.definition->url();

  // See if we have any iframes in need of expansion

  QList< QWebFrame * > frames = ui.definition->page()->mainFrame()->childFrames();

  bool wereFrames = false;

  for( QList< QWebFrame * >::iterator i = frames.begin(); i != frames.end(); ++i )
  {
    if ( (*i)->frameName().startsWith( "gdexpandframe-" ) )
    {
      //DPRINTF( "Name: %s\n", (*i)->frameName().toUtf8().data() );
      //DPRINTF( "Size: %d\n", (*i)->contentsSize().height() );
      //DPRINTF( ">>>>>>>>Height = %s\n", (*i)->evaluateJavaScript( "document.body.offsetHeight;" ).toString().toUtf8().data() );

      // Set the height
      ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "document.getElementById('%1').height = %2;" ).
        arg( (*i)->frameName() ).
        arg( (*i)->contentsSize().height() ) );

      // Show it
      ui.definition->page()->mainFrame()->evaluateJavaScript( QString( "document.getElementById('%1').style.display = 'block';" ).
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

    qApp->sendEvent( ui.definition, &ev );
  }

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
    QString const scrollTo = Qt4x5::Url::queryItemValue( url, "scrollto" );
    if( isScrollTo( scrollTo ) )
    {
      // There is no active article saved in history, but we have it as a parameter.
      // setCurrentArticle will save it and scroll there.
      setCurrentArticle( scrollTo, true );
    }
  }

  ui.definition->unsetCursor();
  //QApplication::restoreOverrideCursor();

#if QT_VERSION >= QT_VERSION_CHECK(4, 6, 0)
  if( !Qt4x5::Url::queryItemValue( url, "gdanchor" ).isEmpty() )
  {
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

      QWebElementCollection coll = ui.definition->page()->mainFrame()->findAllElements( "a[name]" );
      coll += ui.definition->page()->mainFrame()->findAllElements( "a[id]" );

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
          ui.definition->page()->mainFrame()->evaluateJavaScript(
             QString( "window.location.hash = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );

          break;
        }
      }
    }
    else
    {
      url.clear();
      url.setFragment( anchor );
      ui.definition->page()->mainFrame()->evaluateJavaScript(
         QString( "window.location.hash = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
  }
#endif

  emit pageLoaded( this );

  if( Qt4x5::Url::hasQueryItem( ui.definition->url(), "regexp" ) )
    highlightFTSResults();
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

void ArticleView::clearPageSelection()
{
  ui.definition->page()->currentFrame()->evaluateJavaScript( "window.getSelection().removeAllRanges();" );
}

unsigned ArticleView::getGroup( QUrl const & url )
{
  if ( url.scheme() == "gdlookup" && Qt4x5::Url::hasQueryItem( url, "group" ) )
    return Qt4x5::Url::queryItemValue( url, "group" ).toUInt();

  return 0;
}

QStringList ArticleView::getArticlesList()
{
  return evaluateJavaScriptVariableSafe( ui.definition->page()->mainFrame(), "gdArticleContents" )
      .toString().trimmed().split( ' ', Qt4x5::skipEmptyParts() );
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

  if( !getArticlesList().contains( dictionaryIdFromScrollTo( id ) ) )
    return false;

  ui.definition->page()->mainFrame()->evaluateJavaScript(
    QString( "gdOnCppActiveArticleChanged('%1', %2);" ).arg( id, moveToIt ? "true" : "false" ) );
  onJsActiveArticleChanged( id );

  return true;
}

void ArticleView::selectCurrentArticle()
{
  QString const articleId = getActiveArticleId();
  if( articleId.isEmpty() )
  {
    // The gdSelectArticle( '' ) JavaScript function call fails with a TypeError and has
    // no visible effect. This must be a special page such as the initial Welcome! page or
    // a not-found page. Consider the entire page to be a single article and select it all.
    gdDebug( "No current article to select => select all" );
    ui.definition->pageAction( QWebPage::SelectAll )->trigger();
    return;
  }

  ui.definition->page()->mainFrame()->evaluateJavaScript(
        QString( "gdSelectArticle( '%1' );" ).arg( articleId ) );
}

bool ArticleView::isFramedArticle( QString const & ca )
{
  if ( ca.isEmpty() )
    return false;

  return ui.definition->page()->mainFrame()->
               evaluateJavaScript( QString( "!!document.getElementById('gdexpandframe-%1');" )
                                          .arg( dictionaryIdFromScrollTo( ca ) ) ).toBool();
}

bool ArticleView::isExternalLink( QUrl const & url )
{
  return url.scheme() == "http" || url.scheme() == "https" ||
         url.scheme() == "ftp" || url.scheme() == "mailto" ||
         url.scheme() == "file";
}

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

void ArticleView::saveHistoryUserData()
{
  QMap< QString, QVariant > userData = ui.definition->history()->
                                       currentItem().userData().toMap();

  // Save current article, which can be empty

  userData[ "currentArticle" ] = currentArticle;

  // We also save window position. We restore it when the page is fully loaded,
  // when any hidden frames are expanded.

  userData[ "sx" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollX;" ).toDouble();
  userData[ "sy" ] = ui.definition->page()->mainFrame()->evaluateJavaScript( "window.scrollY;" ).toDouble();

  ui.definition->history()->currentItem().setUserData( userData );
}

void ArticleView::load( QUrl const & url )
{
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
    if ( ev->type() == QEvent::KeyPress )
    {
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

void ArticleView::attachToJavaScript()
{
  ui.definition->page()->mainFrame()->addToJavaScriptWindowObject( "gdArticleView", jsProxy );
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
    emit openLinkInNewTab( url, ui.definition->url(), currentArticle, contexts );
  }
  else
    openLink( url, ui.definition->url(), currentArticle, contexts );
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
    {
      ui.definition->page()->mainFrame()->evaluateJavaScript(
        QString( "window.location = \"%1\"" ).arg( QString::fromUtf8( url.toEncoded() ) ) );
    }
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

bool ArticleView::canGoBack()
{
  return ui.definition->history()->canGoBack();
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
  QMap< QString, QVariant > userData = ui.definition->history()->currentItem().userData().toMap();

  // Save current article, which can be empty
  userData[ "currentArticle" ] = currentArticle;

  // Remove saved window position. Reloading occurs in response to changes that
  // may affect content height, so restoring the current window position can cause
  // uncontrolled jumps. Scrolling to the current article (i.e. jumping to the top
  // of it) is simple, reliable and predictable, if not ideal.
  userData[ "sx" ].clear();
  userData[ "sy" ].clear();

  ui.definition->history()->currentItem().setUserData( userData );

  ui.definition->reload();
}

bool ArticleView::hasSound() const
{
  return !audioLinks.value( "first" ).toString().isEmpty();
}

void ArticleView::playSound()
{
  QString soundScript = audioLinks.value( getActiveArticleId() ).toString();
  // fallback to the first one
  if ( soundScript.isEmpty() )
    soundScript = audioLinks.value( "first" ).toString();

  if ( !soundScript.isEmpty() )
    openLink( QUrl::fromEncoded( soundScript.toUtf8() ), ui.definition->url() );
}

QString ArticleView::toHtml()
{
  return ui.definition->page()->mainFrame()->toHtml();
}

QString ArticleView::getTitle()
{
  return ui.definition->page()->mainFrame()->title();
}

Config::InputPhrase ArticleView::getPhrase() const
{
  const QUrl url = ui.definition->url();
  return Config::InputPhrase( Qt4x5::Url::queryItemValue( url, "word" ),
                              Qt4x5::Url::queryItemValue( url, "punctuation_suffix" ) );
}

void ArticleView::print( QPrinter * printer ) const
{
  ui.definition->print( printer );
}

void ArticleView::contextMenuRequested( QPoint const & pos )
{
  // Is that a link? Is there a selection?

  QWebHitTestResult r = ui.definition->page()->mainFrame()->
                          hitTestContent( pos );

  updateCurrentArticleFromCurrentFrame( r.frame() );

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

  tryMangleWebsiteClickedUrl( targetUrl, contexts );

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
      menu.addAction( ui.definition->pageAction( QWebPage::CopyLinkToClipboard ) );
    }
  }

#if QT_VERSION >= 0x040600
  QWebElement el = r.element();
  QUrl imageUrl;
  if( !popupView && el.tagName().compare( "img", Qt::CaseInsensitive ) == 0 )
  {
    imageUrl = QUrl::fromPercentEncoding( el.attribute( "src" ).toLatin1() );
    if( !imageUrl.isEmpty() )
    {
      menu.addAction( ui.definition->pageAction( QWebPage::CopyImageToClipboard ) );
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
#endif

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
    menu.addAction( ui.definition->pageAction( QWebPage::Copy ) );
    menu.addAction( &copyAsTextAction );
  }
  else
  {
    menu.addAction( &selectCurrentArticleAction );
    menu.addAction( ui.definition->pageAction( QWebPage::SelectAll ) );
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
      emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                   currentArticle, Contexts() );
    else
    if ( !popupView && result == lookupSelectionNewTabGr && groupComboBox )
      emit showDefinitionInNewTab( selectedText, groupComboBox->getCurrentGroup(),
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
    QStringList lst = getArticlesList();

    int idx = lst.indexOf( dictionaryIdFromScrollTo( currentArticle ) );

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
  if( !currentArticle.isEmpty() )
  {
    QStringList lst = getArticlesList();

    int idx = lst.indexOf( dictionaryIdFromScrollTo( currentArticle ) );

    if ( idx != -1 )
    {
      idx = ( idx + 1 ) % lst.size();

      setCurrentArticle( scrollToFromDictionaryId( lst[ idx ] ), true );
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

void ArticleView::onPageJsReady( QVariantMap const & audioLinks_, QString const & activeArticleId )
{
  audioLinks = audioLinks_;
  currentArticle = activeArticleId;
}

void ArticleView::onJsActiveArticleChanged(QString const & id)
{
  if ( !isScrollTo( id ) )
  {
    gdWarning( "Invalid active article ID received from JavaScript: %s", id.toUtf8().constData() );
    return;
  }

  currentArticle = id;
  emit activeArticleChanged( this, dictionaryIdFromScrollTo( id ) );
}

void ArticleView::doubleClicked( QPoint pos )
{
#if QT_VERSION >= 0x040600
  QWebHitTestResult r = ui.definition->page()->mainFrame()->hitTestContent( pos );
  QWebElement el = r.element();
  QUrl imageUrl;
  if( el.tagName().compare( "img", Qt::CaseInsensitive ) == 0 )
  {
    // Double click on image; download it and transfer to external program

    imageUrl = QUrl::fromPercentEncoding( el.attribute( "src" ).toLatin1() );
    if( !imageUrl.isEmpty() )
    {
      // Download it

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
    return;
  }
#endif

  // We might want to initiate translation of the selected word

  if ( cfg.preferences.doubleClickTranslates )
  {
    QString selectedText = ui.definition->selectedText();

    // Do some checks to make sure there's a sensible selection indeed
    if ( Folding::applyWhitespaceOnly( gd::toWString( selectedText ) ).size() &&
         selectedText.size() < 60 )
    {
      // Initiate translation
      Qt::KeyboardModifiers kmod = QApplication::keyboardModifiers();
      if (kmod & (Qt::ControlModifier | Qt::ShiftModifier))
      { // open in new tab
        emit showDefinitionInNewTab( selectedText, getGroup( ui.definition->url() ),
                                     currentArticle, Contexts() );
      }
      else
      {
        QUrl const & ref = ui.definition->url();

        if( Qt4x5::Url::hasQueryItem( ref, "dictionaries" ) )
        {
          QStringList dictsList = Qt4x5::Url::queryItemValue(ref, "dictionaries" )
                                              .split( ",", Qt4x5::skipEmptyParts() );
          showDefinition( selectedText, dictsList, QRegExp(), getGroup( ref ), false );
        }
        else
          showDefinition( selectedText, getGroup( ref ), currentArticle );
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
        clearPageSelection();
    }

    QWebPage::FindFlags f = caseSensitivityFindFlags( ui.searchCaseSensitive->isChecked() );
    f |= QWebPage::HighlightAllOccurrences;

    ui.definition->findText( "", f );

    if( ui.highlightAllButton->isChecked() )
      ui.definition->findText( text, f );

    if( checkHighlight )
      return;
  }

  QWebPage::FindFlags f = caseSensitivityFindFlags( ui.searchCaseSensitive->isChecked() );
  if ( backwards )
    f |= QWebPage::FindBackward;

  bool setMark = text.size() && !ui.definition->findText( text, f );

  if ( ui.searchText->property( "noResults" ).toBool() != setMark )
  {
    ui.searchText->setProperty( "noResults", setMark );

    // Reload stylesheet
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

    QWebPage::FindFlags flags;

  #if QT_VERSION >= 0x040600
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
  ui.definition->triggerPageAction( QWebPage::InspectElement );
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

  sptr< AccentMarkHandler > marksHandler = ignoreDiacritics ?
                                           new DiacriticsHandler : new AccentMarkHandler;

  // Clear any current selection
  if ( ui.definition->selectedText().size() )
    clearPageSelection();

  QString pageText = ui.definition->page()->currentFrame()->toPlainText();
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

  ftsSearchMatchCase = Qt4x5::Url::hasQueryItem( url, "matchcase" );

  QWebPage::FindFlags const flags = caseSensitivityFindFlags( ftsSearchMatchCase );

  if( allMatches.isEmpty() )
    ui.ftsSearchStatusLabel->setText( searchStatusMessageNoMatches() );
  else
  {
    highlightAllFtsOccurences( flags );
    if( ui.definition->findText( allMatches.at( 0 ), flags ) )
    {
        ui.definition->page()->currentFrame()->
               evaluateJavaScript( QString( "%1=window.getSelection().getRangeAt(0);_=0;" )
                                   .arg( rangeVarName ) );
    }
    Q_ASSERT( ftsPosition == 0 );
    ui.ftsSearchStatusLabel->setText( searchStatusMessage( 1, allMatches.size() ) );
  }

  ui.ftsSearchFrame->show();
  ui.ftsSearchPrevious->setEnabled( false );
  ui.ftsSearchNext->setEnabled( allMatches.size()>1 );

  ftsSearchIsOpened = true;
}

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

  QWebPage::FindFlags flags = caseSensitivityFindFlags( ftsSearchMatchCase );

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

  ui.ftsSearchStatusLabel->setText( searchStatusMessage( ftsPosition + 1, allMatches.size() ) );

  // Store new highlighted selection
  ui.definition->page()->currentFrame()->
         evaluateJavaScript( QString( "%1=window.getSelection().getRangeAt(0);_=0;" )
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

#endif

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
