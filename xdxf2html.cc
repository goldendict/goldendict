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
#include <QRegExp>
#include <QDebug>

namespace Xdxf2Html {

static void fixLink( QDomElement & el, string const & dictId, const char *attrName )
{
  QUrl url;
  url.setScheme( "bres" );
  url.setHost( QString::fromStdString(dictId) );
  url.setPath( el.attribute(attrName) );

  el.setAttribute( attrName, url.toEncoded().data() );
}

// converting a number into roman representation
string convertToRoman( int input, int lower_case )
{
    string romanvalue = "";
    if( input >= 4000 )
    {
        int x = ( input - input % 4000 ) / 1000;
        romanvalue = "(" + convertToRoman( x, lower_case ) + ")" ;
        input %= 4000;
    }

    const string roman[26] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I",
                               "m", "cm", "d", "cd", "c", "xc", "l", "xl", "x", "ix", "v", "iv", "i"};
    const int decimal[13] =  {1000,  900, 500,  400, 100,   90,  50,   40,  10,    9,   5,    4,   1};

    for( int i = 0; i < 13; i++ )
    {
      while( input >= decimal[ i ] )
      {
        input -= decimal[ i ];
        if ( lower_case == 1 )
          romanvalue += roman[ i + 13 ];
        else
          romanvalue += roman[ i ];
      }
    }
    return romanvalue;
}

string convert( string const & in, DICT_TYPE type, map < string, string > const * pAbrv,
                Dictionary::Class *dictPtr, bool isLogicalFormat, unsigned revisionNumber )
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
        if( !isLogicalFormat )
          inConverted.append( "<br/>" );
        break;

      case '\r':
        break;

      case ' ':
        if ( afterEol )
        {
          if( !isLogicalFormat )
            inConverted.append( "&nbsp;" ); 
          break;
        }
        // Fall-through

      default:
        inConverted.push_back( *i );
        afterEol = false;
    }
  }

  // Strip "<nu />" tags - QDomDocument don't handle it correctly
  string::size_type n;
  while( ( n = inConverted.find( "<nu />" ) ) != string::npos )
      inConverted.erase( n, 6 );

  // We build a dom representation of the given xml, then do some transforms
  QDomDocument dd;

  QString errorStr;
  int errorLine, errorColumn;

  string in_data;
  if( type == XDXF )
  {
      in_data = "<div class=\"xdxf\"";
      if( dictPtr->isToLanguageRTL() )
        in_data += " dir=\"rtl\"";
      in_data += ">";
  }
  else
      in_data = "<div class=\"sdct_x\">";
  in_data += inConverted + "</div>";

  if( !dd.setContent( QByteArray( in_data.c_str() ), false, &errorStr, &errorLine, &errorColumn  ) )
  {
    qWarning( "Xdxf2html error, xml parse failed: %s at %d,%d\n", errorStr.toLocal8Bit().constData(),  errorLine,  errorColumn );
    qWarning( "The input was: %s\n", in.c_str() );

    return in;
  }

  QDomNodeList nodes = dd.elementsByTagName( "ex" ); // Example

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_ex" );
    else
      el.setAttribute( "class", "xdxf_ex_old" );
  }
  
  nodes = dd.elementsByTagName( "mrkd" ); // marked out words in tranlations/examples of usage

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_ex_markd" );
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
        if( dictPtr->isFromLanguageRTL() != dictPtr->isToLanguageRTL() )
          el.setAttribute( "dir", dictPtr->isFromLanguageRTL() ? "rtl" : "ltr" );
    }
  }
  
  // processing of nested <def>s
  if( isLogicalFormat ) // in articles with visual format <def> tags do not effect the formatting.
  {
    nodes = dd.elementsByTagName( "def" );
    
    // this is a logical type of XDXF, so we need to render proper numbering
    // we will do it this way:
    
    // 1. we compute the maximum nesting depth of the article
    int maxNestingDepth = 1; // maximum nesting depth of the article
    for( int i = 0; i < nodes.size(); i++ )
    {
      QDomElement el = nodes.at( i ).toElement();
      QDomElement nestingNode = el;
      int nestingCount = 0;
      while ( nestingNode.parentNode().toElement().tagName() == "def" )
      {
        nestingCount++;
        nestingNode = nestingNode.parentNode().toElement();
      }
      if ( nestingCount > maxNestingDepth )
        maxNestingDepth = nestingCount;
    }
    // 2. in this loop we go layer-by-layer through all <def> and insert proper numbers according to its structure
    for( int j = maxNestingDepth; j > 0; j-- ) // j symbolizes special depth to be processed at this iteration
    {
      int siblingCount = 0; // this  that counts the number of among all siblings of this depth
      QString numberText = ""; // the number to be inserted into the beginning of <def> (I,II,IV,1,2,3,a),b),c)...)
      for( int i = 0; i < nodes.size(); i++ )
      {
        QDomElement el = nodes.at( i ).toElement();
        QDomElement nestingNode = el;
        // computing the depth @nestingDepth of a current node @el
        int nestingDepth = 0;
        while( nestingNode.parentNode().toElement().tagName() == "def" )
        {
          nestingDepth++;
          nestingNode=nestingNode.parentNode().toElement();
        } 
        // we process nodes on of current depth @j
        // we do this in order not to break the numbering at this depth level
        if (nestingDepth == j)
        {
          siblingCount++;
          if( maxNestingDepth == 1 )
          {
            numberText = numberText.setNum( siblingCount ) + ". ";
          }
          else if( maxNestingDepth == 2 )
          {
            if( nestingDepth == 1 )
              numberText = numberText.setNum( siblingCount ) + ". ";
            if( nestingDepth == 2 )
              numberText = numberText.setNum( siblingCount ) + ") ";
          }
          else
          {
            if( nestingDepth == 1 )
              numberText = QString::fromStdString( convertToRoman(siblingCount,0) + ". " );
            if( nestingDepth == 2 )
              numberText = numberText.setNum( siblingCount ) + ". ";
            if( nestingDepth == 3 )
              numberText = numberText.setNum( siblingCount ) + ") ";
            if( nestingDepth == 4 )
              numberText = QString::fromStdString( convertToRoman(siblingCount,1) + ") " );
          }
          QDomElement numberNode = dd.createElement( "span" );
          numberNode.setAttribute( "class", "xdxf_num" );
          QDomText text_num = dd.createTextNode( numberText );
          numberNode.appendChild( text_num );
          el.insertBefore( numberNode, el.firstChild() );
          
          if ( el.hasAttribute( "cmt" ) )
          {
            QDomElement cmtNode = dd.createElement( "span" );
            cmtNode.setAttribute( "class", "xdxf_co" );
            QDomText text_num = dd.createTextNode( el.attribute( "cmt" ) );
            cmtNode.appendChild( text_num );
            el.insertAfter( cmtNode, el.firstChild() );
          }
        }
        else if( nestingDepth < j ) // if it goes one level up @siblingCount needs to be reset
          siblingCount = 0;
      }
    }
    // we finally change all <def> tags into 'xdxf_def' <span>s
    while( nodes.size() )
    {
      QDomElement el = nodes.at( 0 ).toElement();
      el.setTagName( "span" );
      el.setAttribute( "class", "xdxf_def" );
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
    if ( el.hasAttribute( "idref" ) )
    {
      // todo implement support for referencing only specific parts of the article
      el.setAttribute( "href", QString( "bword:" ) + el.text() + "#" + el.attribute( "idref" ));
    }
    if ( el.hasAttribute( "kcmt" ) )
    {
      QDomText kcmtText = dd.createTextNode( " " + el.attribute( "kcmt" ) );
      el.parentNode().insertAfter( kcmtText, el );
    }
  }

  nodes = dd.elementsByTagName( "iref" ); // Reference to internet site

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "a" );
    el.setAttribute( "href", el.text() );
  }

  // Abbreviations
  if( revisionNumber < 29 )
    nodes = dd.elementsByTagName( "abr" );
  else
    nodes = dd.elementsByTagName( "abbr" );

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    el.setAttribute( "class", "xdxf_abbr" );
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
            // Replace all spaces with non-breakable ones, since that's how Lingvo shows tooltips
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

    el.setTagName( "span" );

    if ( el.hasAttribute( "c" ) )
    {
      el.setAttribute( "style", "color:" + el.attribute( "c" ) );
      el.removeAttribute( "c" );
    }
    else
      el.setAttribute( "style", "color:blue" );
  }

  nodes = dd.elementsByTagName( "co" ); // Editorial comment

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_co" );
    else
      el.setAttribute( "class", "xdxf_co_old" );
  }

  /* grammar information */
  nodes = dd.elementsByTagName( "gr" ); // proper grammar tag
  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_gr" );
    else
      el.setAttribute( "class", "xdxf_gr_old" );
  }
  nodes = dd.elementsByTagName( "pos" ); // deprecated grammar tag
  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_gr" );
    else
      el.setAttribute( "class", "xdxf_gr_old" );
  }
  nodes = dd.elementsByTagName( "tense" ); // deprecated grammar tag
  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_gr" );
    else
      el.setAttribute( "class", "xdxf_gr_old" );
  }
  /* end of grammar generation */
  
  nodes = dd.elementsByTagName( "tr" ); // Transcription

  while( nodes.size() )
  {
    QDomElement el = nodes.at( 0 ).toElement();

    el.setTagName( "span" );
    if( isLogicalFormat )
      el.setAttribute( "class", "xdxf_tr" );
    else
      el.setAttribute( "class", "xdxf_tr_old" );
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
            el_script.setAttribute( "type", "text/javascript" );
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

  return dd.toString().remove('\n').remove( QRegExp( "<(b|i)/>" ) ).toUtf8().data();
}

}

