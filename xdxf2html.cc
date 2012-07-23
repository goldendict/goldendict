/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "xdxf2html.hh"
#include <QtXml>
#include "dprintf.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include "folding.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "file.hh"
#include "filetype.hh"
#include "htmlescape.hh"

namespace Xdxf2Html {

static void fixLink( QDomElement & el, string const & dictId, const char *attrName )
{
  QUrl url;
  url.setScheme( "bres" );
  url.setHost( QString::fromStdString(dictId) );
  url.setPath( el.attribute(attrName) );

  el.setAttribute( attrName, url.toEncoded().data() );
}

string convert( string const & in, DICT_TYPE type, map < string, string > const * pAbrv, Dictionary::Class *dictPtr )
{
//  DPRINTF( "Source>>>>>>>>>>: %s\n\n\n", in.c_str() );

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

  string in_data;
  if( type == XDXF )
      in_data = "<div class=\"xdxf\">";
  else
      in_data = "<div class=\"sdct_x\">";
  in_data += inConverted + "</div>";

  if ( !dd.setContent( QByteArray( in_data.c_str() ), false, &errorStr, &errorLine, &errorColumn  ) )
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
          el.setAttribute( "title", gd::toQString( Utf8::decode( title ) ) );
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

  // Ensure that ArticleNetworkAccessManager can deal with XDXF images.
  // We modify the URL by using the dictionary ID as the hostname.
  // This is necessary to determine from which dictionary a requested
  // image originates.
  nodes = dd.elementsByTagName( "img" );

  for( int i = 0; i < nodes.size(); i++ )
  {
    QDomElement el = nodes.at( i ).toElement();

    if ( el.hasAttribute( "src" ) )
    {
      fixLink( el, dictPtr->getId(), "src" );
    }

    if ( el.hasAttribute( "losrc" ) )
    {
      fixLink( el, dictPtr->getId(), "losrc" );
    }

    if ( el.hasAttribute( "hisrc" ) )
    {
      fixLink( el, dictPtr->getId(), "hisrc" );
    }
  }

  nodes = dd.elementsByTagName( "rref" ); // Resource reference

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

//    if( type == XDXF && dictPtr != NULL && !el.hasAttribute( "start" ) )
    if( dictPtr != NULL && !el.hasAttribute( "start" ) )
    {
        string filename = Utf8::encode( gd::toWString( el.text() ) );

        if ( Filetype::isNameOfPicture( filename ) )
        {
          QUrl url;
          url.setScheme( "bres" );
          url.setHost( QString::fromUtf8( dictPtr->getId().c_str() ) );
          url.setPath( QString::fromUtf8( filename.c_str() ) );

          QDomElement newEl = dd.createElement( "img" );
          newEl.setAttribute( "src", url.toEncoded().data() );
          newEl.setAttribute( "alt", Html::escape( filename ).c_str() );

          QDomNode parent = el.parentNode();
          if( !parent.isNull() )
          {
            parent.replaceChild( newEl, el );
            continue;
          }
        }
        else if( Filetype::isNameOfSound( filename ) )
        {
          QUrl url;
          url.setScheme( "gdau" );
          url.setHost( QString::fromUtf8( dictPtr->getId().c_str() ) );
          url.setPath( QString::fromUtf8( filename.c_str() ) );

          QDomElement el_script = dd.createElement( "script" );
          QDomNode parent = el.parentNode();
          if( !parent.isNull() )
          {
            el_script.setAttribute( "language", "JavaScript" );
            parent.replaceChild( el_script, el );

            QDomText el_txt = dd.createTextNode( makeAudioLinkScript( string( "\"" ) + url.toEncoded().data() + "\"",
                                                                      dictPtr->getId() ).c_str() );
            el_script.appendChild( el_txt );

            QDomElement el_span = dd.createElement( "span" );
            el_span.setAttribute( "class", "xdxf_wav" );
            parent.insertAfter( el_span, el_script );

            QDomElement el_a = dd.createElement( "a" );
            el_a.setAttribute( "href", url.toEncoded().data() );
            el_span.appendChild( el_a );

            QDomElement el_img = dd.createElement( "img");
            el_img.setAttribute( "src", "qrcx://localhost/icons/playsound.png" );
            el_img.setAttribute( "border", "0" );
            el_img.setAttribute( "align", "absmiddle" );
            el_img.setAttribute( "alt", "Play" );
            el_a.appendChild( el_img );

            continue;
          }
        }
    }

    // We don't really know how to handle this at the moment, so we'll just
    // convert it to a span and leave it as is for now.

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_rref" );
  }

//  DPRINTF( "Result>>>>>>>>>>: %s\n\n\n", dd.toByteArray().data() );

  return dd.toByteArray().data();
}

}

