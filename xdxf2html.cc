/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "xdxf2html.hh"
#include <QtXml>

namespace Xdxf2Html {

string convert( string const & in )
{
  printf( "Source>>>>>>>>>>: %s\n\n\n", in.c_str() );

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
    fprintf( stderr, "Xdxf2html error, xml parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
    fprintf( stderr, "The input was: %s\n", in.c_str() );

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

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_k" );
  }


  nodes = dd.elementsByTagName( "kref" ); // Reference to another word

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "a" );
    el.setAttribute( "href", QString( "bword:" ) + el.text() );
    el.setAttribute( "class", "xdxf_kref" );
  }

  nodes = dd.elementsByTagName( "abr" ); // Abbreviation

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_abr" );
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

