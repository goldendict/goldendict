/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "article_maker.hh"
#include "config.hh"
#include "htmlescape.hh"
#include "utf8.hh"
#include "dictlock.hh"
#include <QFile>
#include <set>


using std::vector;
using std::string;
using std::wstring;
using std::set;

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

  result += "<title>" + Html::escape( Utf8::encode( word.toStdWString() ) ) + "</title>";

  // This doesn't seem to be much of influence right now, but we'll keep
  // it anyway.
  if ( icon.size() )
    result += "<link rel=\"icon\" type=\"image/png\" href=\"qrcx://localhost/flags/" + Html::escape( icon.toUtf8().data() ) + "\" />\n";

  result += "</head><body>";

  return result;
}

string ArticleMaker::makeDefinitionFor( QString const & inWord,
                                        QString const & group ) const
{
  printf( "group = %ls\n", group.toStdWString().c_str() );

  wstring word = inWord.trimmed().toStdWString();

  if ( group == "internal:about" )
  {
    // This is a special group containing internal welcome/help pages
    string result = makeHtmlHeader( inWord, QString() );
    
    if ( inWord == "Welcome!" )
    {
      result += tr(
"<h3 align=\"center\">Welcome to <b>GoldenDict</b>!</h3>"
"<p>To start working with the program, first add some directory paths where to search "
"for the dictionary files at <b>Edit|Sources</b>. Note that the each subdirectory is to be added separately. "
"After that, you can organize all the dictionaries found into groups "
"in <b>Edit|Groups</b>."
"<p>You can also check out the available program preferences at <b>Edit|Preferences</b>. "
"All settings there have tooltips, be sure to read them if you are in doubt about anything."
"<p>And then you're reading to look up your words! You can do that in this window "
"by using a pane to the left, or you can <a href=\"Working with popup\">look up words from other active applications</a>. "
"<p>Should you need further help, have any questions, "
"suggestions or just wonder what the others think, you are welcome at the program's <a href=\"http://goldendict.berlios.de/forum/\">forum</a>."
"<p>You can also contact the author directly by writing an <a href=\"mailto: Konstantin Isakov <ikm@users.berlios.de>\">e-mail</a>."
"<p>Check program's <a href=\"http://goldendict.berlios.de/\">website</a> for the updates. "
"<p>(c) 2008-2009 Konstantin Isakov. Licensed under GPLv3 or later."
        
        ).toUtf8().data();
    }
    else
    if ( inWord == "Working with popup" )
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
      return makeNotFoundTextFor( inWord,  group );
    }
    
    result += "</body></html>";

    return result;
  }
  
  // Find the given group

  Instances::Group const * activeGroup = 0;

  for( unsigned x = 0; x < groups.size(); ++x )
    if ( groups[ x ].name == group )
    {
      activeGroup = &groups[ x ];
      break;
    }

  // If we've found a group, use its dictionaries; otherwise, use the global
  // heap.
  std::vector< sptr< Dictionary::Class > > const & activeDicts =
    activeGroup ? activeGroup->dictionaries : dictionaries;

  string result = makeHtmlHeader( inWord.trimmed(),
                                  activeGroup && activeGroup->icon.size() ?
                                    activeGroup->icon : QString() );

  DictLock _;

  // Accumulate main forms

  vector< wstring > alts;

  {
    set< wstring > altsSet;

    for( unsigned x = 0; x < activeDicts.size(); ++x )
    {
      vector< wstring > found = activeDicts[ x ]->findHeadwordsForSynonym( word );

      altsSet.insert( found.begin(), found.end() );
    }

    alts.insert( alts.begin(), altsSet.begin(), altsSet.end() );
  }

  for( unsigned x = 0; x < alts.size(); ++x )
  {
    printf( "Alt: %ls\n", alts[ x ].c_str() );
  }

  for( unsigned x = 0; x < activeDicts.size(); ++x )
  {
    try
    {
      string body = activeDicts[ x ]->getArticle( word, alts );

      printf( "From %s: %s\n", activeDicts[ x ]->getName().c_str(), body.c_str() );

      result += string( "<div class=\"gddictname\">" ) +
        tr( "From " ).toUtf8().data() +
        Html::escape( activeDicts[ x ]->getName() ) + "</div>" + body;
    }
    catch( Dictionary::exNoSuchWord & )
    {
      continue;
    }
  }

  result += "</body></html>";

  return result;
}

string ArticleMaker::makeNotFoundTextFor( QString const & word,
                                          QString const & group ) const
{
  return makeHtmlHeader( word, QString() ) +
    "<div class=\"gdnotfound\"><p>" +
      tr( "No translation for <b>%1</b> was found in group <b>%2</b>." ).
        arg( QString::fromUtf8( Html::escape( word.toUtf8().data() ).c_str() ) ).
        arg( QString::fromUtf8(Html::escape( group.toUtf8().data() ).c_str() ) ).
          toUtf8().data()
        +"</p></div>"
         "</body></html>";
}
