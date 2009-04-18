/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "article_maker.hh"
#include "config.hh"
#include "htmlescape.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include <limits.h>
#include <QFile>

using std::vector;
using std::string;
using gd::wstring;
using std::set;
using std::list;

ArticleMaker::ArticleMaker( vector< sptr< Dictionary::Class > > const & dictionaries_,
                            vector< Instances::Group > const & groups_ ):
  dictionaries( dictionaries_ ),
  groups( groups_ )
{
}

std::string ArticleMaker::makeHtmlHeader( QString const & word,
                                          QString const & icon )
{
  string result =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
    "<html><head>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

  // Add a css stylesheet
  
  QFile builtInCssFile( ":/article-style.css" );
  builtInCssFile.open( QFile::ReadOnly );
  QByteArray css = builtInCssFile.readAll();
  
  QFile cssFile( Config::getUserCssFileName() );

  if ( cssFile.open( QFile::ReadOnly ) )
    css += cssFile.readAll();
  
  result += "<style type=\"text/css\">\n";
  result += css.data();
  result += "</style>\n";

  result += "<title>" + Html::escape( Utf8::encode( gd::toWString( word ) ) ) + "</title>";

  // This doesn't seem to be much of influence right now, but we'll keep
  // it anyway.
  if ( icon.size() )
    result += "<link rel=\"icon\" type=\"image/png\" href=\"qrcx://localhost/flags/" + Html::escape( icon.toUtf8().data() ) + "\" />\n";

  result += "</head><body>";

  return result;
}

std::string ArticleMaker::makeNotFoundBody( QString const & word, QString const & group )
{

  return string( "<div class=\"gdnotfound\"><p>" ) +
      tr( "No translation for <b>%1</b> was found in group <b>%2</b>." ).
        arg( QString::fromUtf8( Html::escape( word.toUtf8().data() ).c_str() ) ).
        arg( QString::fromUtf8( Html::escape( group.toUtf8().data() ).c_str() ) ).
          toUtf8().data()
        +"</p></div>";
}

sptr< Dictionary::DataRequest > ArticleMaker::makeDefinitionFor(
  QString const & inWord, unsigned groupId ) const
{
  if ( groupId == UINT_MAX )
  {
    // This is a special group containing internal welcome/help pages
    string result = makeHtmlHeader( inWord, QString() );
    
    if ( inWord == tr( "Welcome!" ) )
    {
      result += tr(
"<h3 align=\"center\">Welcome to <b>GoldenDict</b>!</h3>"
"<p>To start working with the program, first visit <b>Edit|Sources</b> to add some directory paths where to search "
"for the dictionary files, and/or set up various Wikipedia sources. "
"After that, you can optionally organize all the dictionaries found into groups "
"in <b>Edit|Groups</b>."
"<p>You can also check out the available program preferences at <b>Edit|Preferences</b>. "
"All settings there have tooltips, be sure to read them if you are in doubt about anything."
"<p>And then you're ready to look up your words! You can do that in this window "
"by using a pane to the left, or you can <a href=\"Working with popup\">look up words from other active applications</a>. "
"<p>Should you need further help, have any questions, "
"suggestions or just wonder what the others think, you are welcome at the program's <a href=\"http://goldendict.berlios.de/forum/\">forum</a>."
"<p>You can also contact the author directly by writing an <a href=\"mailto: Konstantin Isakov <ikm@users.berlios.de>\">e-mail</a>."
"<p>Check program's <a href=\"http://goldendict.berlios.de/\">website</a> for the updates. "
"<p>(c) 2008-2009 Konstantin Isakov. Licensed under GPLv3 or later."
        
        ).toUtf8().data();
    }
    else
    if ( inWord == tr( "Working with popup" ) )
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
      return makeNotFoundTextFor( inWord, "help" );
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

  string header = makeHtmlHeader( inWord.trimmed(),
                                  activeGroup && activeGroup->icon.size() ?
                                    activeGroup->icon : QString() );

  return new ArticleRequest( inWord.trimmed(), activeGroup ? activeGroup->name : "", activeDicts, header );
}

sptr< Dictionary::DataRequest > ArticleMaker::makeNotFoundTextFor(
  QString const & word, QString const & group ) const
{
  string result = makeHtmlHeader( word, QString() ) + makeNotFoundBody( word, group ) +
    "</body></html>";

  sptr< Dictionary::DataRequestInstant > r = new Dictionary::DataRequestInstant( true );

  r->getData().resize( result.size() );
  memcpy( &( r->getData().front() ), result.data(), result.size() );

  return r;
}

//////// ArticleRequest

ArticleRequest::ArticleRequest(
  QString const & word_, QString const & group_,
  vector< sptr< Dictionary::Class > > const & activeDicts_,
  string const & header ):
    word( word_ ), group( group_ ), activeDicts( activeDicts_ ),
    altsDone( false ), bodyDone( false ), foundAnyDefinitions( false ),
    closePrevSpan( false )
{
  // No need to lock dataMutex on construction

  hasAnyData = true;

  data.resize( header.size() );
  memcpy( &data.front(), header.data(), header.size() );

  // Accumulate main forms

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    sptr< Dictionary::WordSearchRequest > s = activeDicts[ x ]->findHeadwordsForSynonym( gd::toWString( word ) );

    connect( s.get(), SIGNAL( finished() ),
             this, SLOT( altSearchFinished() ) );

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
    printf( "alts finished\n" );
    
    // They all've finished! Now we can look up bodies

    altsDone = true; // So any pending signals in queued mode won't mess us up

    vector< wstring > altsVector( alts.begin(), alts.end() );
    
    for( unsigned x = 0; x < altsVector.size(); ++x )
    {
      printf( "Alt: %ls\n", altsVector[ x ].c_str() );
    }

    wstring wordStd = gd::toWString( word );

    for( unsigned x = 0; x < activeDicts.size(); ++x )
    {
      sptr< Dictionary::DataRequest > r =
        activeDicts[ x ]->getArticle( wordStd, altsVector );

      connect( r.get(), SIGNAL( finished() ),
               this, SLOT( bodyFinished() ) );

      bodyRequests.push_back( r );
    }

    bodyFinished(); // Handle any ones which have already finished
  }
}

void ArticleRequest::bodyFinished()
{
  if ( bodyDone )
    return;

  printf( "some body finished\n" );
  
  bool wasUpdated = false;
  
  while ( bodyRequests.size() )
  {
    // Since requests should go in order, check the first one first
    if ( bodyRequests.front()->isFinished() )
    {
      // Good

      printf( "one finished.\n" );

      Dictionary::DataRequest & req = *bodyRequests.front();

      QString errorString = req.getErrorString();

      if ( req.dataSize() >= 0 || errorString.size() )
      {
        string dictId = activeDicts[ activeDicts.size() - bodyRequests.size() ]->getId();
        
        string head;

        if ( closePrevSpan )
        {
          head += "</span>";
          closePrevSpan = false;
        }

        string jsVal = Html::escapeForJavaScript( dictId );
        head += "<script language=\"JavaScript\">var gdArticleContents; "
          "if ( !gdArticleContents ) gdArticleContents = \"" + jsVal +" \"; "
          "else gdArticleContents += \"" + jsVal + " \";</script>";
        
        head += "<span id=\"gdfrom-" + Html::escape( dictId ) + "\">";

        closePrevSpan = true;
        
        head += string( "<div class=\"gddictname\">" ) +
          Html::escape(
            tr( "From %1" ).arg( QString::fromUtf8( activeDicts[ activeDicts.size() - bodyRequests.size() ]->getName().c_str() ) ).toUtf8().data() )
           + "</div>";

        if ( errorString.size() )
        {
          head += "<div class=\"gderrordesc\">" +
            Html::escape( tr( "Query error: %1" ).arg( errorString ).toUtf8().data() )
          + "</div>";
        }

        Mutex::Lock _( dataMutex );
  
        size_t offset = data.size();
        
        data.resize( data.size() + head.size() + ( req.dataSize() > 0 ? req.dataSize() : 0 ) );
  
        memcpy( &data.front() + offset, head.data(), head.size() );

        if ( req.dataSize() > 0 )
          bodyRequests.front()->getDataSlice( 0, req.dataSize(),
                                              &data.front() + offset + head.size() );

        wasUpdated = true;

        foundAnyDefinitions = true;
      }
      printf( "erasing..\n" );
      bodyRequests.pop_front();
      printf( "erase done..\n" );
    }
    else
    {
        printf( "one not finished.\n" );
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
        footer += "</span>";
        closePrevSpan = false;
      }
      
      if ( !foundAnyDefinitions )
      {
        // No definitions were ever found, say so to the user.
        footer += ArticleMaker::makeNotFoundBody( word, group );

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

  if ( sr.size() )
  {
    footer += "<div class=\"gdstemmedsuggestion\"><span class=\"gdstemmedsuggestion_head\">" +
      Html::escape( tr( "Close words: " ).toUtf8().data() ) +
      "</span><span class=\"gdstemmedsuggestion_body\">";

    for( unsigned x = 0; x < sr.size(); ++x )
    {
      string escapedResult = Html::escape( sr[ x ].first.toUtf8().data() );
      footer += "<a href=\"bword://" + escapedResult + "\">" + escapedResult +"</a>";

      if ( x != sr.size() - 1 )
      {
        footer += ", ";
      }
    }

    footer += "</span></div>";
  }

  footer += "</body></html>";

  {
    Mutex::Lock _( dataMutex );
  
    size_t offset = data.size();
  
    data.resize( data.size() + footer.size() );
  
    memcpy( &data.front() + offset, footer.data(), footer.size() );
  }

  finish();
}

