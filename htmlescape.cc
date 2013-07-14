/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QString>
#include <QTextDocumentFragment>
#include "htmlescape.hh"

namespace Html {

string escape( string const & str )
{
  string result( str );

  for( size_t x = result.size(); x--; )
    switch ( result[ x ] )
    {
      case '&':
        result.erase( x, 1 );
        result.insert( x, "&amp;" );
      break;

      case '<':
        result.erase( x, 1 );
        result.insert( x, "&lt;" );
      break;

      case '>':
        result.erase( x, 1 );
        result.insert( x, "&gt;" );
      break;

      case '"':
        result.erase( x, 1 );
        result.insert( x, "&quot;" );
      break;

      default:
      break;
    }

  return result;
}

static void storeLineInDiv( string & result, string const & line, bool baseRightToLeft )
{
  result += "<div";
  if( unescape( QString::fromUtf8( line.c_str(), line.size() ) ).isRightToLeft() != baseRightToLeft )
  {
    result += " dir=\"";
    result += baseRightToLeft ? "ltr\"" : "rtl\"";
  }
  result += ">";
  result += line + "</div>";
}

string preformat(string const & str , bool baseRightToLeft )
{
  string escaped = escape( str ), result, line;

  line.reserve( escaped.size() );
  result.reserve( escaped.size() );

  bool leading = true;

  for( char const * nextChar = escaped.c_str(); *nextChar; ++nextChar )
  {
    if ( leading )
    {
      if ( *nextChar == ' ' )
      {
        line += "&nbsp;";
        continue;
      }
      else
      if ( *nextChar == '\t' )
      {
        line += "&nbsp;&nbsp;&nbsp;&nbsp;";
        continue;
      }
    }

    if ( *nextChar == '\n' )
    {
      storeLineInDiv( result, line, baseRightToLeft );
      line.clear();
      leading = true;
      continue;
    }

    if ( *nextChar == '\r' )
      continue; // Just skip all \r

    line.push_back( *nextChar );

    leading = false;
  }

  if( !line.empty() )
    storeLineInDiv( result, line, baseRightToLeft );

  return result;
}

string escapeForJavaScript( string const & str )
{
  string result( str );

  for( size_t x = result.size(); x--; )
    switch ( result[ x ] )
    {
      case '\\':
      case '"':
      case '\'':
        result.insert( x, 1, '\\' );
      break;

      case '\n':
        result.erase( x, 1 );
        result.insert( x, "\\n" );
      break;

      case '\r':
        result.erase( x, 1 );
        result.insert( x, "\\r" );
      break;

      case '\t':
        result.erase( x, 1 );
        result.insert( x, "\\t" );
      break;

      default:
      break;
    }

  return result;
}

QString unescape( QString const & str )
{
  // Does it contain HTML? If it does, we need to strip it
  if ( str.contains( '<' ) || str.contains( '&' ) )
    return QTextDocumentFragment::fromHtml( str ).toPlainText();
  return str;
}

string unescapeUtf8( const string &str )
{
  return string( unescape( QString::fromUtf8( str.c_str(), str.size() ) ).toUtf8().data() );
}

}
