/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "xdxf2html.hh"
#include <QtXml>
#include "dprintf.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include "folding.hh"

namespace Xdxf2Html {

string convert( string const & in, DICT_TYPE type, map < string, string > const * pAbrv )
{
  DPRINTF( "Source>>>>>>>>>>: %s\n\n\n", in.c_str() );

  // Convert spaces after each end of line to &nbsp;s, and then each end of
  // line to a <br>

  string inConverted;

  inConverted.reserve( in.size() );

  bool afterEol = false;

  for( string::const_iterator i = in.begin(), j = in.end(); i != j; ++i )
  {
    switch( *i )
    {
      case '\n':
        afterEol = true;
        inConverted.append( "<br/>" );
      break;

      case ' ':
        if ( afterEol )
        {
          inConverted.append( "&nbsp;" );
          break;
        }
        // Fall-through

      default:
        inConverted.push_back( *i );
        afterEol = false;
    }
  }

  // We build a dom representation of the given xml, then do some transforms
  QDomDocument dd;

  QString errorStr;
  int errorLine, errorColumn;

  if ( !dd.setContent( QByteArray( ( "<div class=\"sdct_x\">" + inConverted + "</div>" ).c_str() ), false, &errorStr, &errorLine, &errorColumn  ) )
  {
    FDPRINTF( stderr, "Xdxf2html error, xml parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
    FDPRINTF( stderr, "The input was: %s\n", in.c_str() );

    return in;
  }

  QDomNodeList nodes = dd.elementsByTagName( "ex" ); // Example

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_ex" );
  }

  nodes = dd.elementsByTagName( "k" ); // Key

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    if( type == STARDICT )
    {
        el.setTagName( "span" );
        el.setAttribute( "class", "xdxf_k" );
    }
    else
    {
        el.setTagName( "div" );
        el.setAttribute( "class", "xdxf_headwords" );
    }
  }

  nodes = dd.elementsByTagName( "opt" ); // Optional headword part

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_opt" );
  }

  nodes = dd.elementsByTagName( "kref" ); // Reference to another word

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "a" );
    el.setAttribute( "href", QString( "bword:" ) + el.text() );
    el.setAttribute( "class", "xdxf_kref" );
  }

  nodes = dd.elementsByTagName( "iref" ); // Reference to internet site

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "a" );
    el.setAttribute( "href", el.text() );
  }

  nodes = dd.elementsByTagName( "abr" ); // Abbreviation

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_abr" );
    if( type == XDXF && pAbrv != NULL )
    {
        string val = Utf8::encode( Folding::trimWhitespace( gd::toWString( el.text() ) ) );

        // If we have such a key, display a title

        map< string, string >::const_iterator i = pAbrv->find( val );

        if ( i != pAbrv->end() )
        {
          string title;

          if ( Utf8::decode( i->second ).size() < 70 )
          {
            // Replace all spaces with non-breakable ones, since that's how
            // Lingvo shows tooltips
            title.reserve( i->second.size() );

            for( char const * c = i->second.c_str(); *c; ++c )
              if ( *c == ' ' || *c == '\t' )
              {
                // u00A0 in utf8
                title.push_back( 0xC2 );
                title.push_back( 0xA0 );
              }
              else
                title.push_back( *c );
          }
          else
            title = i->second;
          el.setAttribute( "title", title.c_str() );
        }
    }
  }

  nodes = dd.elementsByTagName( "dtrn" ); // Direct translation

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_dtrn" );
  }

  nodes = dd.elementsByTagName( "c" ); // Color

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "font" );
    el.setAttribute( "class", "xdxf_c" );

    if ( el.hasAttribute( "c" ) )
    {
      el.setAttribute( "color", el.attribute( "c" ) );
      el.removeAttribute( "c" );
    }
  }

  nodes = dd.elementsByTagName( "co" ); // Editorial comment

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_co" );
  }

  nodes = dd.elementsByTagName( "tr" ); // Transcription

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_tr" );
  }

  nodes = dd.elementsByTagName( "rref" ); // Resource reference

  // We don't really know how to handle this at the moment, so we'll just
  // convert it to a span and leave it as is for now.
  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_rref" );
  }

  return dd.toByteArray().data();
}

}

