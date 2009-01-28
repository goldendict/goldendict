/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "article_maker.hh"
#include "config.hh"
#include "htmlescape.hh"
#include "utf8.hh"
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

string ArticleMaker::makeDefinitionFor( QString const & inWord,
                                        QString const & group ) const
{
  printf( "group = %ls\n", group.toStdWString().c_str() );

  wstring word = inWord.trimmed().toStdWString();

  string result =
    "<html><head>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

  QFile cssFile( Config::getUserCssFileName() );

  if ( cssFile.open( QFile::ReadOnly ) )
  {
    result += "<style type=\"text/css\">\n";
    result += cssFile.readAll().data();
    result += "</style>\n";
  }

  result += "<title>" + Html::escape( Utf8::encode( word ) ) + "</title>";

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

  if ( activeGroup && activeGroup->icon.size() )
  {
    // This doesn't seem to be much of influence right now, but we'll keep
    // it anyway.
    result += "<link rel=\"icon\" type=\"image/png\" href=\"qrcx://localhost/flags/" + Html::escape( activeGroup->icon.toUtf8().data() ) + "\" />\n";
  }

  result += "</head><body>";

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

      result += "<div class=\"gddictname\">From " + Html::escape( activeDicts[ x ]->getName() ) + "</div>" + body;
    }
    catch( Dictionary::exNoSuchWord & )
    {
      continue;
    }
  }

  result += "</body></html>";

  return result;
}




