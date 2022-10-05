/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "article_maker.hh"
#include "config.hh"
#include "htmlescape.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include <limits.h>
#include <QFileInfo>
#include <QUrl>
#include <QTextDocumentFragment>
#include "folding.hh"
#include "langcoder.hh"
#include "gddebug.hh"
#include "qt4x5.hh"

#include <algorithm>

#ifndef USE_QTWEBKIT
#include <QColor>
#include <QFile>

#include <regex>
#endif

using std::vector;
using std::string;
using gd::wstring;
using std::set;
using std::list;

namespace {

void appendScripts( string & result )
{
  // Use the qrcx scheme for JavaScript files enabled in the Qt WebKit version. The qrc scheme causes the warning
  // "QIODevice::read (QNetworkReplyFileImpl): device not open" at GoldenDict start, because
  // AllowFrameReply::baseReply is not open when AllowFrameReply::readDataFromBase() is invoked.

  result +=
  // *blocking.js scripts block HTML parser, which is acceptable here,
  // because the scripts are local, instantly available and fast.
#ifdef USE_QTWEBKIT
  // Evaluate webkit_blocking.js now to call gdArticleView.onJsPageInitStarted() ASAP.
            "<script src='qrcx://localhost/scripts/webkit_blocking.js'></script>"
#else
  // Create QWebChannel now to make gdArticleView available ASAP.
            "<script src='qrc:///qtwebchannel/qwebchannel.js'></script>"
            "<script src='qrc:///scripts/webengine_blocking.js'></script>"
#endif
  // Start reading the deferred scripts early so that they are ready when needed.
            "<script defer src='qrcx://localhost/scripts/deferred.js'></script>"
#ifndef USE_QTWEBKIT
  // Load webengine_deferred.js in the end because it calls gdArticleView.onJsPageInitFinished().
            "<script defer src='qrc:///scripts/webengine_deferred.js'></script>"
#endif
            "<script>"
            "const gdExpandArticleTitle = \"";
  result += ArticleMaker::tr( "Expand article" ).toUtf8().constData();
  result += "\";\n"
            "const gdCollapseArticleTitle = \"";
  result += ArticleMaker::tr( "Collapse article" ).toUtf8().constData();
  result += "\";\n"
            "</script>"
            "<script src='qrcx://localhost/scripts/blocking.js'></script>"
            "\n";
}

class CssAppender
{
public:
  /// @param needPageBackgroundColor_ whether findPageBackgroundColor() will be called.
  explicit CssAppender( string & result_, bool needPageBackgroundColor_ ):
    result( result_ ), needPageBackgroundColor( needPageBackgroundColor_ ), isPrintMedia( false )
  {}

  /// Style sheets appended after a call to this function apply only while printing.
  /// @note: style sheets with media="print" are appended after uncoditional style sheets,
  ///        because print-only CSS needs higher priority to override the style used for printing.
  void startPrintMedia()
  { isPrintMedia = true; }

  void appendFile( QString const & fileName )
  {
    if( !QFileInfo( fileName ).isFile() )
      return;

#ifndef USE_QTWEBKIT
    // We are not looking for printed background color.
    if( needPageBackgroundColor && !isPrintMedia )
      cssFiles.push_back( fileName );
#endif

    result += "<link href=\"";
    result += Html::escape( localFileNameToHtml( fileName ) );
    result += "\" rel=\"stylesheet\" media=\"";
    result += isPrintMedia ? "print" : "all";
    result += "\" />\n";
  }

#ifndef USE_QTWEBKIT
  // TODO (Qt WebEngine): on my GNU/Linux system a call to findPageBackgroundColor() takes about 15 ms and slows down
  // GoldenDict start by about 15*2=30 ms when the Default display style is configured. On the bright side, when some
  // other built-in display style is configured, a call to findPageBackgroundColor() takes up to 1 ms (with one
  // exception: Lingoes-Blue about 2.5 ms). The significant slowdown suggests reconsidering this implementation. Perhaps
  // introduce into built-in CSS files a special comment that specifies the background color or states that the CSS file
  // does not affect the background color, and parse it much faster. For example, /*gdPageBackgroundColor:#abcdef;*/
  // The comment's format requirements should be strict for fast and easy parsing w/o regex: the only allowed variation
  // is replacing the #abcdef substring with another color name parsable by QColor::setNamedColor() or with an empty
  // string to request parsing the previous CSS file. A downside of this optimization is that such a comment would have
  // to be added into all user-defined styles. The downside can be mitigated by falling back to the current slow regex
  // implementation if the comment is not found. But with a silent fallback like this the users would never update their
  // styles, and suffer suboptimal performance. A way to annoy users into adding the comment into their styles is to
  // print a warning if the comment is not found. The warning message should include the CSS file name and the comment
  // format specification.
  // An alternative optimization is to assume that the <body> background color is specified at most once in each CSS
  // file and return the first regex match instead of the last. If the <body> background color is specified close to the
  // top of CSS files, which is indeed the case for built-in styles, this could speed up the search dramatically. But
  // returning the first match would be even less accurate than the current algorithm; would be much slower than the
  // strictly formatted comment approach (especially if the style sheets become referenced rather than embedded and the
  // files could be read line by line in case of the plain text search); would retain a hidden performance trap when the
  // <body> background color is specified at the bottom of a large file, or not specified at all.

  /// @return The page background color or an empty string if
  ///         the background color could not be found in the style sheets.
  /// @warning This function parses the style sheets that have been appended to @a result. Therefore it
  ///          must be called after appending all style sheets that may override the page background color.
  string findPageBackgroundColor() const
  {
    Q_ASSERT( needPageBackgroundColor );

    string backgroundColor;

    // Iterate in the reverse order because the CSS code lower in the page overrides.
    std::find_if( cssFiles.crbegin(), cssFiles.crend(), [ &backgroundColor ]( QString const & fileName ) {
      QFile cssFile( fileName );
      if( !cssFile.open( QFile::ReadOnly ) )
      {
        gdWarning( "Couldn't open CSS file \"%s\" for reading: %s (%d)", qUtf8Printable( fileName ),
                   qUtf8Printable( cssFile.errorString() ), static_cast< int >( cssFile.error() ) );
        return false;
      }

      auto const cssCode = cssFile.readAll();
      backgroundColor = findBodyBackgroundColor( cssCode.constData(), cssCode.constData() + cssCode.size() );
      return !backgroundColor.empty();
    } );

    return backgroundColor;
  }
#endif // USE_QTWEBKIT

private:
  static std::string localFileNameToHtml( QString const & fileName )
  {
    string result;
    if( fileName.startsWith( QLatin1String( ":/" ) ) )
    {
      // Replace the local file resource prefix ":/" with "qrcx://localhost/" for the web page.
      result = "qrcx://localhost/";
      result += fileName.toUtf8().constData() + 2;
    }
    else
    {
      // fileName must be a local filesystem path => convert it into a file URL.
#ifdef Q_OS_WIN32
      result = "file:///";
#else
      result = "file://";
#endif
      result += fileName.toUtf8().constData();
    }
    return result;
  }

  string & result;
  bool const needPageBackgroundColor;
  bool isPrintMedia;

#ifndef USE_QTWEBKIT
  vector< QString > cssFiles;

  /// Finds the background color of the <body> element in [@p first, @p last).
  /// @return the background color or an empty string if it could not be found.
  static string findBodyBackgroundColor( char const * first, char const * last )
  {
    // This regular expression is simple and efficient. But the result is not always accurate:
    // 1. The first word after "background:" is considered to be the color, even though this first word could
    //    be something else, e.g. "border-box".
    // 2. The code inside CSS comments (/*comment*/) is matched too, not skipped.
    // Built-in and user-defined style sheets must take this simplified matching into account.
    // On the bright side, the user can easily override the background color matched here by adding a comment
    // like /* body{ background:#abcdef } */ at the end of the user-defined article-style.css file.
    static std::regex const backgroundRegex( R"(\bbody\s*\{[^}]*\bbackground(?:|-color)\s*:\s*([^\s;}]+))",
                      // CSS code is case-insensitive => regex::icase.
                      // The regex object is reused (static) and the CSS code can be large => regex::optimize.
                                             std::regex::icase | std::regex::optimize );

    // Iterate over all matches and return the last one, because the CSS code lower in the page overrides.
    string::const_iterator::difference_type position = -1, length;
    for( std::cregex_iterator it( first, last, backgroundRegex ), end; it != end; ++it )
    {
      Q_ASSERT( it->size() == 2 );

      position = it->position( 1 );
      Q_ASSERT( position >= 0 );

      length = it->length( 1 );
      Q_ASSERT( length > 0 );
      Q_ASSERT( first + position + length <= last );
    }

    if( position == -1 )
      return {};
    return string( first + position, first + position + length );
  }
#endif
};

} // unnamed namespace

ArticleMaker::ArticleMaker( vector< sptr< Dictionary::Class > > const & dictionaries_,
                            vector< Instances::Group > const & groups_,
                            QString const & displayStyle_,
                            QString const & addonStyle_):
  dictionaries( dictionaries_ ),
  groups( groups_ ),
  displayStyle( displayStyle_ ),
  addonStyle( addonStyle_ ),
  needExpandOptionalParts( true )
, collapseBigArticles( true )
, articleLimitSize( 500 )
{
}

void ArticleMaker::setDisplayStyle( QString const & st, QString const & adst )
{
  displayStyle = st;
  addonStyle = adst;
}

#ifndef USE_QTWEBKIT
QColor ArticleMaker::colorFromString( string const & pageBackgroundColor )
{
  if( pageBackgroundColor.empty() )
  {
    Q_ASSERT_X( false, Q_FUNC_INFO, "The default built-in style sheet :/article-style.css is unconditionally appended "
                                    "and specifies a valid page background color, which is parsed correctly." );
    gdWarning( "Couldn't find the page background color in the article style sheets." );
    return QColor();
  }

  QColor color( QString::fromUtf8( pageBackgroundColor.c_str() ) );
  if( !color.isValid() )
  {
    gdWarning( "Found invalid page background color in the article style sheets: \"%s\"", pageBackgroundColor.c_str() );
    return QColor();
  }

  GD_DPRINTF( "Found page background color in the article style sheets: \"%s\" = %s\n", pageBackgroundColor.c_str(),
              // Print the result of QColor's nontrivial parsing of pageBackgroundColor string.
              // Print the alpha component only if the color is not fully opaque.
              qPrintable( color.name( color.alpha() == 255 ? QColor::HexRgb : QColor::HexArgb ) ) );
  return color;
}
#endif

void ArticleMaker::appendCss( string & result, bool expandOptionalParts, QColor * pageBackgroundColor ) const
{
  CssAppender cssAppender( result, static_cast< bool >( pageBackgroundColor ) );

  cssAppender.appendFile( ":/article-style.css" );
  if( !displayStyle.isEmpty() )
    cssAppender.appendFile( QString( ":/article-style-st-%1.css" ).arg( displayStyle ) );
  cssAppender.appendFile( Config::getUserCssFileName() );
  if( !addonStyle.isEmpty() )
    cssAppender.appendFile( Config::getStylesDir() + addonStyle + QDir::separator() + "article-style.css" );

  // Turn on/off expanding of article optional parts
  if( expandOptionalParts )
    cssAppender.appendFile( ":/article-style-expand-optional-parts.css" );

  cssAppender.startPrintMedia();

  cssAppender.appendFile( ":/article-style-print.css" );
  cssAppender.appendFile( Config::getUserCssPrintFileName() );
  if( !addonStyle.isEmpty() )
    cssAppender.appendFile( Config::getStylesDir() + addonStyle + QDir::separator() + "article-style-print.css" );

#ifdef USE_QTWEBKIT
  Q_ASSERT( !pageBackgroundColor );
#else
  if( pageBackgroundColor )
    *pageBackgroundColor = colorFromString( cssAppender.findPageBackgroundColor() );
#endif
}

std::string ArticleMaker::makeHtmlHeader( QString const & word,
                                          QString const & icon,
                                          bool expandOptionalParts,
                                          QColor * pageBackgroundColor ) const
{
  string result =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
    "<html><head>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

  appendScripts( result );
  appendCss( result, expandOptionalParts, pageBackgroundColor );

  result += "<title>" + Html::escape( Utf8::encode( gd::toWString( word ) ) ) + "</title>";

  // This doesn't seem to be much of influence right now, but we'll keep
  // it anyway.
  if ( icon.size() )
    result += "<link rel=\"icon\" type=\"image/png\" href=\"qrcx://localhost/flags/" + Html::escape( icon.toUtf8().data() ) + "\" />\n";

  result += "</head><body"
#ifndef USE_QTWEBKIT
  // Qt WebEngine API does not provide a way to check whether a mouse click occurs on a page
  // proper or on its scrollbar. We are only interested in clicks on the page contents
  // within <body>. Listen to such mouse events and send messages from JavaScript to C++.
            " onMouseDown='gdBodyMouseDown(event);'"
            " onMouseUp='gdBodyMouseUp(event);'"
#endif
            ">";

  return result;
}

std::string ArticleMaker::makeNotFoundBody( QString const & word,
                                            QString const & group )
{
  string result( "<div class=\"gdnotfound\"><p>" );

  QString str( word );
  if( str.isRightToLeft() )
  {
    str.insert( 0, (ushort)0x202E ); // RLE, Right-to-Left Embedding
    str.append( (ushort)0x202C ); // PDF, POP DIRECTIONAL FORMATTING
  }

  if ( word.size() )
    result += tr( "No translation for <b>%1</b> was found in group <b>%2</b>." ).
              arg( QString::fromUtf8( Html::escape( str.toUtf8().data() ).c_str() ) ).
              arg( QString::fromUtf8( Html::escape( group.toUtf8().data() ).c_str() ) ).
                toUtf8().data();
  else
    result += tr( "No translation was found in group <b>%1</b>." ).
              arg( QString::fromUtf8( Html::escape( group.toUtf8().data() ).c_str() ) ).
                toUtf8().data();

  result += "</p></div>";

  return result;
}

sptr< Dictionary::DataRequest > ArticleMaker::makeDefinitionFor(
  Config::InputPhrase const & phrase, unsigned groupId,
  QMap< QString, QString > const & contexts,
  QSet< QString > const & mutedDicts,
  QStringList const & dictIDs , bool ignoreDiacritics ) const
{
  if( !dictIDs.isEmpty() )
  {
    QStringList ids = dictIDs;
    std::vector< sptr< Dictionary::Class > > ftsDicts;

    // Find dictionaries by ID's
    for( unsigned x = 0; x < dictionaries.size(); x++ )
    {
      for( QStringList::Iterator it = ids.begin(); it != ids.end(); ++it )
      {
        if( *it == QString::fromStdString( dictionaries[ x ]->getId() ) )
        {
          ftsDicts.push_back( dictionaries[ x ] );
          ids.erase( it );
          break;
        }
      }
      if( ids.isEmpty() )
        break;
    }

    string header = makeHtmlHeader( phrase.phrase, QString(), true );

    return new ArticleRequest( phrase, "",
                               contexts, ftsDicts, header,
                               -1, true );
  }

  if ( groupId == Instances::Group::HelpGroupId )
  {
    // This is a special group containing internal welcome/help pages
    string result = makeHtmlHeader( phrase.phrase, QString(), needExpandOptionalParts );

    if ( phrase.phrase == tr( "Welcome!" ) )
    {
      result += tr(
"<h3 align=\"center\">Welcome to <b>GoldenDict</b>!</h3>"
"<p>To start working with the program, first visit <b>Edit|Dictionaries</b> to add some directory paths where to search "
"for the dictionary files, set up various Wikipedia sites or other sources, adjust dictionary order or create dictionary groups."
"<p>And then you're ready to look up your words! You can do that in this window "
"by using a pane to the left, or you can <a href=\"Working with popup\">look up words from other active applications</a>. "
"<p>To customize program, check out the available preferences at <b>Edit|Preferences</b>. "
"All settings there have tooltips, be sure to read them if you are in doubt about anything."
"<p>Should you need further help, have any questions, "
"suggestions or just wonder what the others think, you are welcome at the program's <a href=\"http://goldendict.org/forum/\">forum</a>."
"<p>Check program's <a href=\"http://goldendict.org/\">website</a> for the updates. "
"<p>(c) 2008-2013 Konstantin Isakov. Licensed under GPLv3 or later."

        ).toUtf8().data();
    }
    else
    if ( phrase.phrase == tr( "Working with popup" ) )
    {
      result += ( tr( "<h3 align=\"center\">Working with the popup</h3>"

"To look up words from other active applications, you would need to first activate the <i>\"Scan popup functionality\"</i> in <b>Preferences</b>, "
"and then enable it at any time either by triggering the 'Popup' icon above, or "
"by clicking the tray icon down below with your right mouse button and choosing so in the menu you've popped. " ) +

#ifdef Q_OS_WIN32
  tr( "Then just stop the cursor over the word you want to look up in another application, "
       "and a window would pop up which would describe it to you." )
#else
  tr( "Then just select any word you want to look up in another application by your mouse "
      "(double-click it or swipe it with mouse with the button pressed), "
      "and a window would pop up which would describe the word to you." )
#endif
  ).toUtf8().data();
    }
    else
    {
      // Not found
      return makeNotFoundTextFor( phrase.phrase, "help" );
    }

    result += "</body></html>";

    sptr< Dictionary::DataRequestInstant > r = new Dictionary::DataRequestInstant( true );

    r->getData().resize( result.size() );
    memcpy( &( r->getData().front() ), result.data(), result.size() );

    return r;
  }

  // Find the given group

  Instances::Group const * activeGroup = 0;

  for( unsigned x = 0; x < groups.size(); ++x )
    if ( groups[ x ].id == groupId )
    {
      activeGroup = &groups[ x ];
      break;
    }

  // If we've found a group, use its dictionaries; otherwise, use the global
  // heap.
  std::vector< sptr< Dictionary::Class > > const & activeDicts =
    activeGroup ? activeGroup->dictionaries : dictionaries;

  string header = makeHtmlHeader( phrase.phrase,
                                  activeGroup && activeGroup->icon.size() ?
                                    activeGroup->icon : QString(),
                                  needExpandOptionalParts );

  if ( mutedDicts.size() )
  {
    std::vector< sptr< Dictionary::Class > > unmutedDicts;

    unmutedDicts.reserve( activeDicts.size() );

    for( unsigned x = 0; x < activeDicts.size(); ++x )
      if ( !mutedDicts.contains(
              QString::fromStdString( activeDicts[ x ]->getId() ) ) )
        unmutedDicts.push_back( activeDicts[ x ] );

    return new ArticleRequest( phrase, activeGroup ? activeGroup->name : "",
                               contexts, unmutedDicts, header,
                               collapseBigArticles ? articleLimitSize : -1,
                               needExpandOptionalParts, ignoreDiacritics );
  }
  else
    return new ArticleRequest( phrase, activeGroup ? activeGroup->name : "",
                               contexts, activeDicts, header,
                               collapseBigArticles ? articleLimitSize : -1,
                               needExpandOptionalParts, ignoreDiacritics );
}

sptr< Dictionary::DataRequest > ArticleMaker::makeNotFoundTextFor(
  QString const & word, QString const & group ) const
{
  string result = makeHtmlHeader( word, QString(), true ) + makeNotFoundBody( word, group ) +
    "</body></html>";

  sptr< Dictionary::DataRequestInstant > r = new Dictionary::DataRequestInstant( true );

  r->getData().resize( result.size() );
  memcpy( &( r->getData().front() ), result.data(), result.size() );

  return r;
}

string ArticleMaker::makeBlankPageHtmlCode( QColor * pageBackgroundColor ) const
{
  return makeHtmlHeader( tr( "(untitled)" ), QString(), true, pageBackgroundColor ) +
    "</body></html>";
}

sptr< Dictionary::DataRequest > ArticleMaker::makeBlankPage() const
{
  string const result = makeBlankPageHtmlCode();

  sptr< Dictionary::DataRequestInstant > r =
      new Dictionary::DataRequestInstant( true );

  r->getData().resize( result.size() );
  memcpy( &( r->getData().front() ), result.data(), result.size() );

  return r;
}

sptr< Dictionary::DataRequest > ArticleMaker::makePicturePage( string const & url ) const
{
  string result = makeHtmlHeader( tr( "(picture)" ), QString(), true )
                  + "<a href=\"javascript: history.back();\">"
                  + "<img src=\"" + url + "\" /></a>"
                  + "</body></html>";

  sptr< Dictionary::DataRequestInstant > r =
      new Dictionary::DataRequestInstant( true );

  r->getData().resize( result.size() );
  memcpy( &( r->getData().front() ), result.data(), result.size() );

  return r;
}

void ArticleMaker::setExpandOptionalParts( bool expand )
{
  needExpandOptionalParts = expand;
}

void ArticleMaker::setCollapseParameters( bool autoCollapse, int articleSize )
{
  collapseBigArticles = autoCollapse;
  articleLimitSize = articleSize;
}


bool ArticleMaker::adjustFilePath( QString & fileName )
{
  QFileInfo info( fileName );
  if( !info.isFile() )
  {
    QString dir = Config::getConfigDir();
    dir.chop( 1 );
    info.setFile( dir + fileName);
    if( info.isFile() )
    {
      fileName = info.canonicalFilePath();
      return true;
    }
  }
  return false;
}

//////// ArticleRequest

ArticleRequest::ArticleRequest(
  Config::InputPhrase const & phrase, QString const & group_,
  QMap< QString, QString > const & contexts_,
  vector< sptr< Dictionary::Class > > const & activeDicts_,
  string const & header,
  int sizeLimit, bool needExpandOptionalParts_, bool ignoreDiacritics_ ):
    word( phrase.phrase ), group( group_ ), contexts( contexts_ ),
    activeDicts( activeDicts_ ),
    altsDone( false ), bodyDone( false ), foundAnyDefinitions( false ),
    closePrevSpan( false )
,   articleSizeLimit( sizeLimit )
,   needExpandOptionalParts( needExpandOptionalParts_ )
,   ignoreDiacritics( ignoreDiacritics_ )
{
  if ( !phrase.punctuationSuffix.isEmpty() )
    alts.insert( gd::toWString( phrase.phraseWithSuffix() ) );

  // No need to lock dataMutex on construction

  hasAnyData = true;

  data.resize( header.size() );
  memcpy( &data.front(), header.data(), header.size() );

  // Accumulate main forms

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    sptr< Dictionary::WordSearchRequest > s = activeDicts[ x ]->findHeadwordsForSynonym( gd::toWString( word ) );

    connect( s.get(), SIGNAL( finished() ),
             this, SLOT( altSearchFinished() ), Qt::QueuedConnection );

    altSearches.push_back( s );
  }

  altSearchFinished(); // Handle any ones which have already finished
}

void ArticleRequest::altSearchFinished()
{
  if ( altsDone )
    return;

  // Check every request for finishing
  for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
         altSearches.begin(); i != altSearches.end(); )
  {
    if ( (*i)->isFinished() )
    {
      // This one's finished
      for( size_t count = (*i)->matchesCount(), x = 0; x < count; ++x )
        alts.insert( (**i)[ x ].word );

      altSearches.erase( i++ );
    }
    else
      ++i;
  }

  if ( altSearches.empty() )
  {
#ifdef QT_DEBUG
    qDebug( "alts finished\n" );
#endif

    // They all've finished! Now we can look up bodies

    altsDone = true; // So any pending signals in queued mode won't mess us up

    vector< wstring > altsVector( alts.begin(), alts.end() );

#ifdef QT_DEBUG
    for( unsigned x = 0; x < altsVector.size(); ++x )
    {
      qDebug() << "Alt:" << gd::toQString( altsVector[ x ] );
    }
#endif

    wstring wordStd = gd::toWString( word );

    if( activeDicts.size() <= 1 )
      articleSizeLimit = -1; // Don't collapse article if only one dictionary presented

    for( unsigned x = 0; x < activeDicts.size(); ++x )
    {
      try
      {
        sptr< Dictionary::DataRequest > r =
          activeDicts[ x ]->getArticle( wordStd, altsVector,
                                        gd::toWString( contexts.value( QString::fromStdString( activeDicts[ x ]->getId() ) ) ),
                                        ignoreDiacritics );

        connect( r.get(), SIGNAL( finished() ),
                 this, SLOT( bodyFinished() ), Qt::QueuedConnection );

        bodyRequests.push_back( r );
      }
      catch( std::exception & e )
      {
        gdWarning( "getArticle request error (%s) in \"%s\"\n",
                   e.what(), activeDicts[ x ]->getName().c_str() );
      }
    }

    bodyFinished(); // Handle any ones which have already finished
  }
}

int ArticleRequest::findEndOfCloseDiv( const QString &str, int pos )
{
  for( ; ; )
  {
    int n1 = str.indexOf( "</div>", pos );
    if( n1 <= 0 )
      return n1;

    int n2 = str.indexOf( "<div ", pos );
    if( n2 <= 0 || n2 > n1 )
      return n1 + 6;

    pos = findEndOfCloseDiv( str, n2 + 1 );
    if( pos <= 0 )
      return pos;
  }
}

static void appendGdMakeArticleActiveOn( string & result, char const * jsEvent, string const & dictionaryId )
{
  result += " on";
  result += jsEvent;
  result += "=\"gdMakeArticleActive('";
  result += dictionaryId;
  result += "');\"";
}

void ArticleRequest::bodyFinished()
{
  if ( bodyDone )
    return;

  GD_DPRINTF( "some body finished\n" );

  bool wasUpdated = false;

  while ( bodyRequests.size() )
  {
    // Since requests should go in order, check the first one first
    if ( bodyRequests.front()->isFinished() )
    {
      // Good

      GD_DPRINTF( "one finished.\n" );

      Dictionary::DataRequest & req = *bodyRequests.front();

      QString errorString = req.getErrorString();

      if ( req.dataSize() >= 0 || errorString.size() )
      {
        sptr< Dictionary::Class > const & activeDict =
            activeDicts[ activeDicts.size() - bodyRequests.size() ];

        string dictId = activeDict->getId();

        string head;

        string gdFrom = "gdfrom-" + Html::escape( dictId );

        if ( closePrevSpan )
        {
          head += "</div></div><div style=\"clear:both;\"></div><span class=\"gdarticleseparator\"></span>";
        }
        // else: this is the first article

        bool collapse = false;
        if( articleSizeLimit >= 0 )
        {
          try
          {
            Mutex::Lock _( dataMutex );
            QString text = QString::fromUtf8( req.getFullData().data(), req.getFullData().size() );

            if( !needExpandOptionalParts )
            {
              // Strip DSL optional parts
              int pos = 0;
              for( ; ; )
              {
                pos = text.indexOf( "<div class=\"dsl_opt\"" );
                if( pos > 0 )
                {
                  int endPos = findEndOfCloseDiv( text, pos + 1 );
                  if( endPos > pos)
                    text.remove( pos, endPos - pos );
                  else
                    break;
                }
                else
                  break;
              }
            }

            int size = QTextDocumentFragment::fromHtml( text ).toPlainText().length();
            if( size > articleSizeLimit )
              collapse = true;
          }
          catch(...)
          {
          }
        }

        string jsVal = Html::escapeForJavaScript( dictId );

        head += string( "<div class=\"gdarticle" ) +
#ifdef USE_QTWEBKIT
                // gdCurrentArticleLoaded() initializes " gdactivearticle" in the Qt WebEngine version.
                ( closePrevSpan ? "" : " gdactivearticle" ) +
#endif
                ( collapse ? " gdcollapsedarticle" : "" ) +
                "\" id=\"" + gdFrom + '"';

        // Make the article active on left, middle or right mouse button click.
        appendGdMakeArticleActiveOn( head, "click", jsVal );
        // A right mouse button click triggers only "contextmenu" JavaScript event.
        appendGdMakeArticleActiveOn( head, "contextmenu", jsVal );
        // In the Qt WebKit version both a left and a middle mouse button click triggers "click" JavaScript event.
        // In the Qt WebEngine version a left mouse button click triggers "click", a middle - "auxclick" event.
#ifndef USE_QTWEBKIT
        appendGdMakeArticleActiveOn( head, "auxclick", jsVal );
#endif

        head += '>';

        closePrevSpan = true;

        head += string( "<div class=\"gddictname\" onclick=\"gdExpandArticle(\'" ) + dictId + "\');"
          + ( collapse ? "\" style=\"cursor:pointer;" : "" )
          + "\" id=\"gddictname-" + Html::escape( dictId ) + "\""
          + ( collapse ? string( " title=\"" ) + tr( "Expand article" ).toUtf8().data() + "\"" : "" )
          + "><span class=\"gddicticon\"><img src=\"gico://" + Html::escape( dictId )
          + "/dicticon.png\"></span><span class=\"gdfromprefix\">"  +
          Html::escape( tr( "From " ).toUtf8().data() ) + "</span><span class=\"gddicttitle\">" +
          Html::escape( activeDict->getName().c_str() ) + "</span>"
          + "<span class=\"collapse_expand_area\"><img src=\"qrcx://localhost/icons/blank.png\" class=\""
          + ( collapse ? "gdexpandicon" : "gdcollapseicon" )
          + "\" id=\"expandicon-" + Html::escape( dictId ) + "\""
          + ( collapse ? "" : string( " title=\"" ) + tr( "Collapse article" ).toUtf8().data() + "\"" )
          + "></span>" + "</div>";

        head += "<div class=\"gddictnamebodyseparator\"></div>";

        head += "<div class=\"gdarticlebody gdlangfrom-";
        head += LangCoder::intToCode2( activeDict->getLangFrom() ).toLatin1().data();
        head += "\" lang=\"";
        head += LangCoder::intToCode2( activeDict->getLangTo() ).toLatin1().data();
        head += "\"";
        head += " style=\"display:";
        head += collapse ? "none" : "inline";
        head += string( "\" id=\"gdarticlefrom-" ) + Html::escape( dictId ) + "\">";

        if ( errorString.size() )
        {
          head += "<div class=\"gderrordesc\">" +
            Html::escape( tr( "Query error: %1" ).arg( errorString ).toUtf8().data() )
          + "</div>";
        }

        Mutex::Lock _( dataMutex );

        size_t offset = data.size();

        string const articleEnding = "<script>gdArticleLoaded(\"" + gdFrom + "\");</script>";

        data.resize( data.size() + head.size() + ( req.dataSize() > 0 ? req.dataSize() : 0 ) + articleEnding.size() );

        memcpy( &data.front() + offset, head.data(), head.size() );

        try
        {
          if ( req.dataSize() > 0 )
            bodyRequests.front()->getDataSlice( 0, req.dataSize(),
                                                &data.front() + offset + head.size() );
        }
        catch( std::exception & e )
        {
          gdWarning( "getDataSlice error: %s\n", e.what() );
        }

        std::copy( articleEnding.begin(), articleEnding.end(), data.end() - articleEnding.size() );

        wasUpdated = true;

        foundAnyDefinitions = true;
      }
      GD_DPRINTF( "erasing..\n" );
      bodyRequests.pop_front();
      GD_DPRINTF( "erase done..\n" );
    }
    else
    {
      GD_DPRINTF( "one not finished.\n" );
      break;
    }
  }

  if ( bodyRequests.empty() )
  {
    // No requests left, end the article

    bodyDone = true;

    {
      string footer;

      if ( closePrevSpan )
      {
        footer += "</div></div>";
        closePrevSpan = false;
      }

      if ( !foundAnyDefinitions )
      {
        // No definitions were ever found, say so to the user.

        // Larger words are usually whole sentences - don't clutter the output
        // with their full bodies.
        footer += ArticleMaker::makeNotFoundBody( word.size() < 40 ? word : "", group );

        // When there were no definitions, we run stemmed search.
        stemmedWordFinder = new WordFinder( this );

        connect( stemmedWordFinder.get(), SIGNAL( finished() ),
                 this, SLOT( stemmedSearchFinished() ), Qt::QueuedConnection );

        stemmedWordFinder->stemmedMatch( word, activeDicts );
      }
      else
      {
        footer += "</body></html>";
      }

      Mutex::Lock _( dataMutex );

      size_t offset = data.size();

      data.resize( data.size() + footer.size() );

      memcpy( &data.front() + offset, footer.data(), footer.size() );
    }

    if ( stemmedWordFinder.get() )
      update();
    else
      finish();
  }
  else
  if ( wasUpdated )
    update();
}

void ArticleRequest::stemmedSearchFinished()
{
  // Got stemmed matching results

  WordFinder::SearchResults sr = stemmedWordFinder->getResults();

  string footer;

  bool continueMatching = false;

  if ( sr.size() )
  {
    footer += "<div class=\"gdstemmedsuggestion\"><span class=\"gdstemmedsuggestion_head\">" +
      Html::escape( tr( "Close words: " ).toUtf8().data() ) +
      "</span><span class=\"gdstemmedsuggestion_body\">";

    for( unsigned x = 0; x < sr.size(); ++x )
    {
      footer += linkWord( sr[ x ].first );

      if ( x != sr.size() - 1 )
      {
        footer += ", ";
      }
    }

    footer += "</span></div>";
  }

  splittedWords = splitIntoWords( word );

  if ( splittedWords.first.size() > 1 ) // Contains more than one word
  {
    disconnect( stemmedWordFinder.get(), SIGNAL( finished() ),
                this, SLOT( stemmedSearchFinished() ) );

    connect( stemmedWordFinder.get(), SIGNAL( finished() ),
             this, SLOT( individualWordFinished() ), Qt::QueuedConnection );

    currentSplittedWordStart = -1;
    currentSplittedWordEnd = currentSplittedWordStart;

    firstCompoundWasFound = false;

    compoundSearchNextStep( false );

    continueMatching = true;
  }

  if ( !continueMatching )
    footer += "</body></html>";

  {
    Mutex::Lock _( dataMutex );

    size_t offset = data.size();

    data.resize( data.size() + footer.size() );

    memcpy( &data.front() + offset, footer.data(), footer.size() );
  }

  if ( continueMatching )
    update();
  else
    finish();
}

void ArticleRequest::compoundSearchNextStep( bool lastSearchSucceeded )
{
  if ( !lastSearchSucceeded )
  {
    // Last search was unsuccessful. First, emit what we had.

    string footer;

    if ( lastGoodCompoundResult.size() ) // We have something to append
    {
//      DPRINTF( "Appending\n" );

      if ( !firstCompoundWasFound )
      {
        // Append the beginning
        footer += "<div class=\"gdstemmedsuggestion\"><span class=\"gdstemmedsuggestion_head\">" +
          Html::escape( tr( "Compound expressions: " ).toUtf8().data() ) +
          "</span><span class=\"gdstemmedsuggestion_body\">";

        firstCompoundWasFound = true;
      }
      else
      {
        // Append the separator
        footer += " / ";
      }

      footer += linkWord( lastGoodCompoundResult );

      lastGoodCompoundResult.clear();
    }

    // Then, start a new search for the next word, if possible

    if ( currentSplittedWordStart >= splittedWords.first.size() - 2 )
    {
      // The last word was the last possible to start from

      if ( firstCompoundWasFound )
        footer += "</span>";

      // Now add links to all the individual words. They conclude the result.

      footer += "<div class=\"gdstemmedsuggestion\"><span class=\"gdstemmedsuggestion_head\">" +
        Html::escape( tr( "Individual words: " ).toUtf8().data() ) +
        "</span><span class=\"gdstemmedsuggestion_body\"";
      if( splittedWords.first[ 0 ].isRightToLeft() )
        footer += " dir=\"rtl\"";
      footer += ">";

      footer += escapeSpacing( splittedWords.second[ 0 ] );

      for( int x = 0; x < splittedWords.first.size(); ++x )
      {
        footer += linkWord( splittedWords.first[ x ] );
        footer += escapeSpacing( splittedWords.second[ x + 1 ] );
      }

      footer += "</span>";

      footer += "</body></html>";

      appendToData( footer );

      finish();

      return;
    }

    if ( footer.size() )
    {
      appendToData( footer );
      update();
    }

    // Advance to the next word and start from looking up two words
    ++currentSplittedWordStart;
    currentSplittedWordEnd = currentSplittedWordStart + 1;
  }
  else
  {
    // Last lookup succeeded -- see if we can try the larger sequence

    if ( currentSplittedWordEnd < splittedWords.first.size() - 1 )
    {
      // We can, indeed.
      ++currentSplittedWordEnd;
    }
    else
    {
      // We can't. Emit what we have and start over.

      ++currentSplittedWordEnd; // So we could use the same code for result
                                // emitting

      // Initiate new lookup
      compoundSearchNextStep( false );

      return;
    }
  }

  // Build the compound sequence

  currentSplittedWordCompound = makeSplittedWordCompound();

  // Look it up

//  DPRINTF( "Looking up %s\n", qPrintable( currentSplittedWordCompound ) );

  stemmedWordFinder->expressionMatch( currentSplittedWordCompound, activeDicts, 40, // Would one be enough? Leave 40 to be safe.
                                      Dictionary::SuitableForCompoundSearching );
}

QString ArticleRequest::makeSplittedWordCompound()
{
  QString result;

  result.clear();

  for( int x = currentSplittedWordStart; x <= currentSplittedWordEnd; ++x )
  {
    result.append( splittedWords.first[ x ] );

    if ( x < currentSplittedWordEnd )
    {
      wstring ws( gd::toWString( splittedWords.second[ x + 1 ] ) );

      Folding::normalizeWhitespace( ws );

      result.append( gd::toQString( ws ) );
    }
  }

  return result;
}

void ArticleRequest::individualWordFinished()
{
  WordFinder::SearchResults const & results = stemmedWordFinder->getResults();

  if ( results.size() )
  {
    wstring source = Folding::applySimpleCaseOnly( gd::toWString( currentSplittedWordCompound ) );

    bool hadSomething = false;

    for( unsigned x = 0; x < results.size(); ++x )
    {
      if ( results[ x ].second )
      {
        // Spelling suggestion match found. No need to continue.
        hadSomething = true;
        lastGoodCompoundResult = currentSplittedWordCompound;
        break;
      }

      // Prefix match found. Check if the aliases are acceptable.

      wstring result( Folding::applySimpleCaseOnly( gd::toWString( results[ x ].first ) ) );

      if ( source.size() <= result.size() && result.compare( 0, source.size(), source ) == 0 )
      {
        // The resulting string begins with the source one

        hadSomething = true;

        if ( source.size() == result.size() )
        {
          // Got the match. No need to continue.
          lastGoodCompoundResult = currentSplittedWordCompound;
          break;
        }
      }
    }

    if ( hadSomething )
    {
      compoundSearchNextStep( true );
      return;
    }
  }

  compoundSearchNextStep( false );
}

void ArticleRequest::appendToData( std::string const & str )
{
  Mutex::Lock _( dataMutex );

  size_t offset = data.size();

  data.resize( data.size() + str.size() );

  memcpy( &data.front() + offset, str.data(), str.size() );

}

QPair< ArticleRequest::Words, ArticleRequest::Spacings > ArticleRequest::splitIntoWords( QString const & input )
{
  QPair< Words, Spacings > result;

  QChar const * ptr = input.data();

  for( ; ; )
  {
    QString spacing;

    for( ; ptr->unicode() && ( Folding::isPunct( ptr->unicode() ) || Folding::isWhitespace( ptr->unicode() ) ); ++ptr )
      spacing.append( *ptr );

    result.second.append( spacing );

    QString word;

    for( ; ptr->unicode() && !( Folding::isPunct( ptr->unicode() ) || Folding::isWhitespace( ptr->unicode() ) ); ++ptr )
      word.append( *ptr );

    if ( word.isEmpty() )
      break;

    result.first.append( word );
  }

  return result;
}

string ArticleRequest::linkWord( QString const & str )
{
  QUrl url;

  url.setScheme( "gdlookup" );
  url.setHost( "localhost" );
  url.setPath( Qt4x5::Url::ensureLeadingSlash( str ) );

  string escapedResult = Html::escape( str.toUtf8().data() );
  return string( "<a href=\"" ) + url.toEncoded().data() + "\">" + escapedResult +"</a>";
}

std::string ArticleRequest::escapeSpacing( QString const & str )
{
  QByteArray spacing = Html::escape( str.toUtf8().data() ).c_str();

  spacing.replace( "\n", "<br>" );

  return spacing.data();
}

void ArticleRequest::cancel()
{
    if( isFinished() )
        return;
    if( !altSearches.empty() )
    {
        for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
               altSearches.begin(); i != altSearches.end(); ++i )
        {
            (*i)->cancel();
        }
    }
    if( !bodyRequests.empty() )
    {
        for( list< sptr< Dictionary::DataRequest > >::iterator i =
               bodyRequests.begin(); i != bodyRequests.end(); ++i )
        {
            (*i)->cancel();
        }
    }
    if( stemmedWordFinder.get() ) stemmedWordFinder->cancel();
    finish();
}
