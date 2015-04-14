/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dsl_details.hh"

#include "folding.hh"
#include "langcoder.hh"
#include "gddebug.hh"
#include "ufile.hh"
#include "wstring_qt.hh"
#include "utf8.hh"

#include <stdio.h>
#include <wctype.h>

namespace Dsl {
namespace Details {

using gd::wstring;
using std::list;

#ifndef __linux__

// wcscasecmp() function is a GNU extension, we need to reimplement it
// for non-GNU systems.

int wcscasecmp( const wchar *s1, const wchar *s2 )
{
  for( ; ; ++s1, ++s2 )
  {
    if ( towlower( *s1 ) != towlower( *s2 ) )
      return towlower( *s1 ) > towlower( *s2 ) ? 1 : -1;

    if ( !*s1 )
      break;
  }

  return 0;
}

#endif

/////////////// ArticleDom

wstring ArticleDom::Node::renderAsText( bool stripTrsTag ) const
{
  if ( !isTag )
    return text;

  wstring result;

  for( list< Node >::const_iterator i = begin(); i != end(); ++i )
    if( !stripTrsTag || i->tagName != GD_NATIVE_TO_WS( L"!trs" ) )
      result += i->renderAsText( stripTrsTag );

  return result;
}

// Returns true if src == 'm' and dest is 'mX', where X is a digit
static inline bool checkM( wstring const & dest, wstring const & src )
{
  return ( src == GD_NATIVE_TO_WS( L"m" ) && dest.size() == 2 &&
    dest[ 0 ] == L'm' && iswdigit( dest[ 1 ] ) );
}

ArticleDom::ArticleDom( wstring const & str, string const & dictName,
                        wstring const & headword_):
  root( Node::Tag(), wstring(), wstring() ), stringPos( str.c_str() ),
  transcriptionCount( 0 ),
  dictionaryName( dictName ),
  headword( headword_ )
{
  list< Node * > stack; // Currently opened tags

  Node * textNode = 0; // A leaf node which currently accumulates text.

  try
  {
    for( ;; )
    {
      nextChar();

      if ( ch == L'@' && !escaped )
      {
        // Insided card
        wstring linkTo;
        nextChar();
        for( ; ; nextChar() )
        {
          if( ch == L'\n' )
            break;
          if( ch != L'\r' )
            linkTo.push_back( ch );
        }
        linkTo = Folding::trimWhitespace( linkTo );

        if( !linkTo.empty() )
        {
          list< wstring > allLinkEntries;
          expandOptionalParts( linkTo, &allLinkEntries );

          for( list< wstring >::iterator entry = allLinkEntries.begin();
               entry != allLinkEntries.end(); )
          {
            if ( !textNode )
            {
              Node text = Node( Node::Text(), wstring() );

              if ( stack.empty() )
              {
                root.push_back( text );
                stack.push_back( &root.back() );
              }
              else
              {
                stack.back()->push_back( text );
                stack.push_back( &stack.back()->back() );
              }

              textNode = stack.back();
            }
            textNode->text.push_back( L'-' );
            textNode->text.push_back( L' ' );

            // Close the currently opened text node
            stack.pop_back();
            textNode = 0;

            wstring linkText = Folding::trimWhitespace( *entry );
            processUnsortedParts( linkText, true );
            ArticleDom nodeDom( linkText, dictName, headword_ );

            Node link( Node::Tag(), GD_NATIVE_TO_WS( L"@" ), wstring() );
            for( Node::iterator n = nodeDom.root.begin(); n != nodeDom.root.end(); ++n )
              link.push_back( *n );

            ++entry;

            if ( stack.empty() )
            {
              root.push_back( link );
              if( entry != allLinkEntries.end() ) // Add line break before next entry
                root.push_back( Node( Node::Tag(), GD_NATIVE_TO_WS( L"br" ), wstring() ) );
            }
            else
            {
              stack.back()->push_back( link );
              if( entry != allLinkEntries.end() )
                stack.back()->push_back( Node( Node::Tag(), GD_NATIVE_TO_WS( L"br" ), wstring() ) );
            }
          }



          // Skip to next '@'

          while( !( ch == L'@' && !escaped ) )
            nextChar();

          stringPos--;
          ch = L'\n';
          escaped = false;
        }

      } // if ( ch == L'@' )

      if ( ch == L'[' && !escaped )
      {
        // Beginning of a tag.
        do
        {
          nextChar();
        } while( Folding::isWhitespace( ch ) );

        bool isClosing;

        if ( ch == L'/' && !escaped )
        {
          // A closing tag.
          isClosing = true;
          nextChar();
        }
        else
          isClosing = false;

        // Read tag's name
        wstring name;

        while( ( ch != L']' || escaped ) && !Folding::isWhitespace( ch ) )
        {
          name.push_back( ch );
          nextChar();
        }

        while( Folding::isWhitespace( ch ) )
          nextChar();

        // Read attrs

        wstring attrs;

        while( ch != L']' || escaped )
        {
          attrs.push_back( ch );
          nextChar();
        }

        // Add the tag, or close it

        if ( textNode )
        {
          // Close the currently opened text node
          stack.pop_back();
          textNode = 0;
        }

        // If the tag is [t], we update the transcriptionCount
        if ( name == GD_NATIVE_TO_WS( L"t" ) )
        {
          if ( isClosing )
          {
            if ( transcriptionCount )
              --transcriptionCount;
          }
          else
            ++transcriptionCount;
        }
        
        if ( !isClosing )
        {
          if ( name == GD_NATIVE_TO_WS( L"m" ) ||
               ( name.size() == 2 && name[ 0 ] == L'm' && iswdigit( name[ 1 ] ) ) )
          {
            // Opening an 'mX' or 'm' tag closes any previous 'm' tag
            closeTag( GD_NATIVE_TO_WS( L"m" ), stack, false );
          }
          openTag( name, attrs, stack );
          if ( name == GD_NATIVE_TO_WS( L"br" ) )
          {
            // [br] tag don't have closing tag
            closeTag( name, stack );
          }
        }
        else
        {
          closeTag( name, stack );
        } // if ( isClosing )
        continue;
      } // if ( ch == '[' )

      if ( ch == L'<' && !escaped )
      {
        // Special case: the <<name>> link

        nextChar();

        if ( ch != L'<' || escaped )
        {
          // Ok, it's not it.
          --stringPos;

          if ( escaped )
          {
            --stringPos;
            escaped = false;
          }
          ch = L'<';
        }
        else
        {
          // Get the link's body
          do
          {
            nextChar();
          } while( Folding::isWhitespace( ch ) );

          wstring linkTo, linkText;

          for( ; ; nextChar() )
          {
            // Is it the end?
            if ( ch == L'>' && !escaped )
            {
              nextChar();

              if ( ch == L'>' && !escaped )
                break;
              else
              {
                linkTo.push_back( L'>' );
                linkTo.push_back( ch );

                linkText.push_back( L'>' );
                if( escaped )
                  linkText.push_back( L'\\' );
                linkText.push_back( ch );
              }
            }
            else
            {
              linkTo.push_back( ch );

              if( escaped )
                linkText.push_back( L'\\' );
              linkText.push_back( ch );
            }
          }

          // Add the corresponding node

          if ( textNode )
          {
            // Close the currently opened text node
            stack.pop_back();
            textNode = 0;
          }

          linkText = Folding::trimWhitespace( linkText );
          processUnsortedParts( linkText, true );
          ArticleDom nodeDom( linkText, dictName, headword_ );

          Node link( Node::Tag(), GD_NATIVE_TO_WS( L"ref" ), wstring() );
          for( Node::iterator n = nodeDom.root.begin(); n != nodeDom.root.end(); ++n )
            link.push_back( *n );

          if ( stack.empty() )
            root.push_back( link );
          else
            stack.back()->push_back( link );

          continue;
        }
      } // if ( ch == '<' )

      if ( ch == L'{' && !escaped )
      {
        // Special case: {{comment}}

        nextChar();

        if ( ch != L'{' || escaped )
        {
          // Ok, it's not it.
          --stringPos;

          if ( escaped )
          {
            --stringPos;
            escaped = false;
          }
          ch = L'{';
        }
        else
        {
          // Skip the comment's body
          for( ; ; )
          {
            nextChar();

            // Is it the end?
            if ( ch == L'}' && !escaped )
            {
              nextChar();

              if ( ch == L'}' && !escaped )
                break;
            }
          }

          continue;
        }
      } // if ( ch == '{' )

      // If we're here, we've got a normal symbol, to be saved as text.

      // If there's currently no text node, open one
      if ( !textNode )
      {
        Node text = Node( Node::Text(), wstring() );

        if ( stack.empty() )
        {
          root.push_back( text );
          stack.push_back( &root.back() );
        }
        else
        {
          stack.back()->push_back( text );
          stack.push_back( &stack.back()->back() );
        }

        textNode = stack.back();
      }

      // If we're inside the transcription, do old-encoding conversion
      if ( transcriptionCount )
      {
        switch ( ch )
        {
          case 0x2021: ch = 0xE6; break;
          case 0x407: ch = 0x72; break;
          case 0xB0: ch = 0x6B; break;
          case 0x20AC: ch = 0x254; break;
          case 0x404: ch = 0x7A; break;
          case 0x40F: ch = 0x283; break;
          case 0xAB: ch = 0x74; break;
          case 0xAC: ch = 0x64; break;
          case 0x2020: ch = 0x259; break;
          case 0x490: ch = 0x6D; break;
          case 0xA7: ch = 0x66; break;
          case 0xAE: ch = 0x6C; break;
          case 0xB1: ch = 0x67; break;
          case 0x45E: ch = 0x65; break;
          case 0xAD: ch = 0x6E; break;
          case 0xA9: ch = 0x73; break;
          case 0xA6: ch = 0x77; break;
          case 0x2026: ch = 0x28C; break;
          case 0x452: ch = 0x76; break;
          case 0x408: ch = 0x70; break;
          case 0x40C: ch = 0x75; break;
          case 0x406: ch = 0x68; break;
          case 0xB5: ch = 0x61; break;
          case 0x491: ch = 0x25B; break;
          case 0x40A: ch = 0x14B; break;
          case 0x2030: ch = 0xF0; break;
          case 0x456: ch = 0x6A; break;
          case 0xA4: ch = 0x62; break;
          case 0x409: ch = 0x292; break;
          case 0x40E: ch = 0x69; break;
          //case 0x44D: ch = 0x131; break;
          case 0x40B: ch = 0x4E8; break;
          case 0xB6: ch = 0x28A; break;
          case 0x2018: ch = 0x251; break;
          case 0x457: ch = 0x265; break;
          case 0x458: ch = 0x153; break;
          case 0x405: textNode->text.push_back( 0x153 ); ch = 0x303; break;
          case 0x441: ch = 0x272; break;
          case 0x442: textNode->text.push_back( 0x254 ); ch = 0x303; break;
          case 0x443: ch = 0xF8; break;
          case 0x445: textNode->text.push_back(0x25B ); ch = 0x303; break;
          case 0x446: ch = 0xE7; break;
          case 0x44C: textNode->text.push_back( 0x251 ); ch = 0x303; break;
          case 0x44D: ch = 0x26A; break;
          case 0x44F: ch = 0x252; break;
          case 0x30: ch = 0x3B2; break;
          case 0x31: textNode->text.push_back( 0x65 ); ch = 0x303; break;
          case 0x32: ch = 0x25C; break;
          case 0x33: ch = 0x129; break;
          case 0x34: ch = 0xF5; break;
          case 0x36: ch = 0x28E; break;
          case 0x37: ch = 0x263; break;
          case 0x38: ch = 0x1DD; break;
          case 0x3A: ch = 0x2D0; break;
          case 0x27: ch = 0x2C8; break;
          case 0x455: ch = 0x1D0; break;
          case 0xB7: ch = 0xE3; break;

          case 0x00a0: ch = 0x02A7; break;
          //case 0x00b1: ch = 0x0261; break;
          case 0x0402: textNode->text.push_back( 0x0069 ); ch = L':'; break;
          case 0x0403: textNode->text.push_back( 0x0251 ); ch = L':'; break;
          //case 0x040b: ch = 0x03b8; break;
          //case 0x040e: ch = 0x026a; break;
          case 0x0428: ch = 0x0061; break;
          case 0x0453: textNode->text.push_back( 0x0075 ); ch = L':'; break;
          case 0x201a: ch = 0x0254; break;
          case 0x201e: ch = 0x0259; break;
          case 0x2039: textNode->text.push_back( 0x0064 ); ch = 0x0292; break;
        }
      }

      if ( escaped && ch == L' ' )
        ch = 0xA0; // Escaped spaces turn into non-breakable ones in Lingvo
            
      textNode->text.push_back( ch );
    } // for( ; ; )
  }
  catch( eot )
  {
  }

  if ( textNode )
    stack.pop_back();

  if ( stack.size() )
  {
    GD_FDPRINTF( stderr, "Warning: %u tags were unclosed.\n", (unsigned) stack.size() );
  }
}

void ArticleDom::openTag( wstring const & name,
                          wstring const & attrs,
                          list<Node *> &stack )
{
  list< Node > nodesToReopen;

  if( name == GD_NATIVE_TO_WS( L"m" ) || checkM( name, GD_NATIVE_TO_WS( L"m" ) ) )
  {
    // All tags above [m] tag will be closed and reopened after
    // to avoid break this tag by closing some other tag.

    while( stack.size() )
    {
      nodesToReopen.push_back( Node( Node::Tag(), stack.back()->tagName,
                                     stack.back()->tagAttrs ) );

      if ( stack.back()->empty() )
      {
        // Empty nodes are deleted since they're no use

        stack.pop_back();

        Node * parent = stack.size() ? stack.back() : &root;

        parent->pop_back();
      }
      else
        stack.pop_back();
    }
  }

  // Add tag

  Node node( Node::Tag(), name, attrs );

  if ( stack.empty() )
  {
    root.push_back( node );
    stack.push_back( &root.back() );
  }
  else
  {
    stack.back()->push_back( node );
    stack.push_back( &stack.back()->back() );
  }

  // Reopen tags if needed

  while( nodesToReopen.size() )
  {
    if ( stack.empty() )
    {
      root.push_back( nodesToReopen.back() );
      stack.push_back( &root.back() );
    }
    else
    {
      stack.back()->push_back( nodesToReopen.back() );
      stack.push_back( &stack.back()->back() );
    }

    nodesToReopen.pop_back();
  }

}

void ArticleDom::closeTag( wstring const & name,
                           list< Node * > & stack,
                           bool warn )
{
  // Find the tag which is to be closed

  list< Node * >::reverse_iterator n;

  for( n = stack.rbegin(); n != stack.rend(); ++n )
  {
    if ( (*n)->tagName == name || checkM( (*n)->tagName, name ) )
    {
      // Found it
      break;
    }
  }

  if ( n != stack.rend() )
  {
    // If there is a corresponding tag, close all tags above it,
    // then close the tag itself, then reopen all the tags which got
    // closed.

    list< Node > nodesToReopen;

    while( stack.size() )
    {
      bool found = stack.back()->tagName == name ||
                   checkM( stack.back()->tagName, name );

      if ( !found )
        nodesToReopen.push_back( Node( Node::Tag(), stack.back()->tagName,
                                       stack.back()->tagAttrs ) );

      if ( stack.back()->empty() && stack.back()->tagName != GD_NATIVE_TO_WS( L"br" ) )
      {
        // Empty nodes except [br] tag are deleted since they're no use

        stack.pop_back();

        Node * parent = stack.size() ? stack.back() : &root;

        parent->pop_back();
      }
      else
        stack.pop_back();

      if ( found )
        break;
    }

    while( nodesToReopen.size() )
    {
      if ( stack.empty() )
      {
        root.push_back( nodesToReopen.back() );
        stack.push_back( &root.back() );
      }
      else
      {
        stack.back()->push_back( nodesToReopen.back() );
        stack.push_back( &stack.back()->back() );
      }

      nodesToReopen.pop_back();
    }
  }
  else
  if ( warn )
  {
    if( !dictionaryName.empty() )
      gdWarning( "Warning: no corresponding opening tag for closing tag \"%s\" found in \"%s\", article \"%s\".",
                 gd::toQString( name ).toUtf8().data(), dictionaryName.c_str(),
                 gd::toQString( headword ).toUtf8().data() );
    else
      gdWarning( "Warning: no corresponding opening tag for closing tag \"%s\" found.",
                 gd::toQString( name ).toUtf8().data() );
  }
}

void ArticleDom::nextChar() throw( eot )
{
  if ( !*stringPos )
    throw eot();

  ch = *stringPos++;

  if ( ch == L'\\' )
  {
    if ( !*stringPos )
      throw eot();

    ch = *stringPos++;

    escaped = true;
  }
  else
  if ( ch == L'[' && *stringPos == L'[' )
  {
    ++stringPos;
    escaped = true;
  }
  else
  if ( ch == L']' && *stringPos == L']' )
  {
    ++stringPos;
    escaped = true;
  }
  else
    escaped = false;
}


/////////////// DslScanner

DslScanner::DslScanner( string const & fileName ) throw( Ex, Iconv::Ex ):
  encoding( Windows1252 ), iconv( encoding ), readBufferPtr( readBuffer ),
  readBufferLeft( 0 ), wcharBuffer( 64 ), linesRead( 0 )
{
  // Since .dz is backwards-compatible with .gz, we use gz- functions to
  // read it -- they are much nicer than the dict_data- ones.

  f = gd_gzopen( fileName.c_str() );
  if ( !f )
    throw exCantOpen( fileName );

  // Now try guessing the encoding by reading the first two bytes

  unsigned char firstBytes[ 2 ];

  if ( gzread( f, firstBytes, sizeof( firstBytes ) ) != sizeof( firstBytes ) )
  {
    // Apparently the file's too short
    gzclose( f );
    throw exMalformedDslFile( fileName );
  }

  bool needExactEncoding = false;


  // If the file begins with the dedicated Unicode marker, we just consume
  // it. If, on the other hand, it's not, we return the bytes back
  if ( firstBytes[ 0 ] == 0xFF && firstBytes[ 1 ] == 0xFE )
    encoding = Utf16LE;
  else
  if ( firstBytes[ 0 ] == 0xFE && firstBytes[ 1 ] == 0xFF )
    encoding = Utf16BE;
  else
  if ( firstBytes[ 0 ] == 0xEF && firstBytes[ 1 ] == 0xBB )
  {
    // Looks like Utf8, read one more byte
    if ( gzread( f, firstBytes, 1 ) != 1 || firstBytes[ 0 ] != 0xBF )
    {
      // Either the file's too short, or the BOM is weird
      gzclose( f );
      throw exMalformedDslFile( fileName );
    }
    
    encoding = Utf8;
  }
  else
  {
    if ( firstBytes[ 0 ] && !firstBytes[ 1 ] )
      encoding = Utf16LE;
    else
    if ( !firstBytes[ 0 ] && firstBytes[ 1 ] )
      encoding = Utf16BE;
    else
    {
      // Ok, this doesn't look like 16-bit Unicode. We will start with a
      // 8-bit encoding with an intent to find out the exact one from
      // the header.
      needExactEncoding = true;
      encoding = Windows1251;
    }

    if ( gzrewind( f ) )
    {
      gzclose( f );
      throw exCantOpen( fileName );
    }
  }

  iconv.reinit( encoding );

  // We now can use our own readNextLine() function

  wstring str;
  size_t offset;

  for( ; ; )
  {
    if ( !readNextLine( str, offset ) )
    {
      gzclose( f );
      throw exMalformedDslFile( fileName );
    }

    if ( str.empty() || str[ 0 ] != L'#' )
      break;

    bool isName = false;
    bool isLangFrom = false;
    bool isLangTo = false;

    if ( !str.compare( 0, 5, GD_NATIVE_TO_WS( L"#NAME" ), 5 ) )
      isName = true;
    else
    if ( !str.compare( 0, 15, GD_NATIVE_TO_WS( L"#INDEX_LANGUAGE" ), 15 ) )
      isLangFrom = true;
    else
    if ( !str.compare( 0, 18, GD_NATIVE_TO_WS( L"#CONTENTS_LANGUAGE" ), 18 ) )
      isLangTo = true;
    else
    if ( str.compare( 0, 17, GD_NATIVE_TO_WS( L"#SOURCE_CODE_PAGE" ), 17 ) )
      continue;

    // Locate the argument

    size_t beg = str.find_first_of( L'"' );

    if ( beg == wstring::npos )
      throw exMalformedDslFile( fileName );

    size_t end = str.find_last_of( L'"' );

    if ( end == beg )
      throw exMalformedDslFile( fileName );

    wstring arg( str, beg + 1, end - beg - 1 );

    if ( isName )
      dictionaryName = arg;
    else if ( isLangFrom )
      langFrom = arg;
    else if ( isLangTo )
      langTo = arg;
    else
    {
      // The encoding
      if ( !needExactEncoding )
      {
        // We don't need that!
        GD_FDPRINTF( stderr, "Warning: encoding was specified in a Unicode file, ignoring.\n" );
      }
      else
      if ( !wcscasecmp( arg.c_str(), GD_NATIVE_TO_WS( L"Latin" ) ) )
        encoding = Windows1252;
      else
      if ( !wcscasecmp( arg.c_str(), GD_NATIVE_TO_WS( L"Cyrillic" ) ) )
        encoding = Windows1251;
      else
      if ( !wcscasecmp( arg.c_str(), GD_NATIVE_TO_WS( L"EasternEuropean" ) ) )
        encoding = Windows1250;
      else
      {
        gzclose( f );
        throw exUnknownCodePage();
      }
    }
  }

  // The loop will always end up reading a line which was not a #-directive.
  // We need to rewind to that line so readNextLine() would return it again
  // next time it's called. To do that, we just use the slow gzseek() and
  // empty the read buffer.
  if( gzdirect( f ) )                    // Without this ZLib 1.2.7 gzread() return 0
    gzrewind( f );                       // after gzseek() call on uncompressed files
  gzseek( f, offset, SEEK_SET );
  readBufferPtr = readBuffer;
  readBufferLeft = 0;

  if ( needExactEncoding )
    iconv.reinit( encoding );
}

DslScanner::~DslScanner() throw()
{
  gzclose( f );
}

bool DslScanner::readNextLine( wstring & out, size_t & offset ) throw( Ex,
                                                                       Iconv::Ex )
{
  offset = (size_t)( gztell( f ) - readBufferLeft );

  // For now we just read one char at a time
  size_t readMultiple = distanceToBytes( 1 );

  size_t leftInOut = wcharBuffer.size();

  wchar * outPtr = &wcharBuffer.front();

  for( ; ; )
  {
    // Check that we have bytes to read
    if ( readBufferLeft < 4 ) // To convert one char, we need at most 4 bytes
    {
      if ( !gzeof( f ) )
      {
        // To avoid having to deal with ring logic, we move the remaining bytes
        // to the beginning
        memmove( readBuffer, readBufferPtr, readBufferLeft );

        // Read some more bytes to readBuffer
        int result = gzread( f, readBuffer + readBufferLeft,
                             sizeof( readBuffer ) - readBufferLeft );

        if ( result == -1 )
          throw exCantReadDslFile();

        readBufferPtr = readBuffer;
        readBufferLeft += (size_t) result;
      }
    }

    if ( readBufferLeft < readMultiple )
    {
      // No more data. Return what we've got so far, forget the last byte if
      // it was a 16-bit Unicode and a file had an odd number of bytes.
      readBufferLeft = 0;

      if ( outPtr != &wcharBuffer.front() )
      {
        // If there was a stray \r, remove it
        if ( outPtr[ -1 ] == L'\r' )
          --outPtr;

        out = wstring( &wcharBuffer.front(), outPtr - &wcharBuffer.front() );

        ++linesRead;

        return true;
      }
      else
        return false;
    }

    // Check that we have chars to write
    if ( leftInOut < 2 ) // With 16-bit wchars, 2 is needed for a surrogate pair
    {
      wcharBuffer.resize( wcharBuffer.size() + 64 );
      outPtr = &wcharBuffer.front() + wcharBuffer.size() - 64 - leftInOut;
      leftInOut += 64;
    }

    // Ok, now convert one char
    size_t outBytesLeft = sizeof( wchar );

    Iconv::Result r =
      iconv.convert( (void const *&)readBufferPtr, readBufferLeft,
                     (void *&)outPtr, outBytesLeft );

    if ( r == Iconv::NeedMoreOut && outBytesLeft == sizeof( wchar ) )
    {
      // Seems to be a surrogate pair with a 16-bit target wchar

      outBytesLeft *= 2;
      r = iconv.convert( (void const *&)readBufferPtr, readBufferLeft,
                     (void *&)outPtr, outBytesLeft );
      --leftInOut; // Complements the next decremention
    }

    if ( outBytesLeft )
      throw exEncodingError();

    --leftInOut;

    // Have we got \n?
    if ( outPtr[ -1 ] == L'\n' )
    {
      --outPtr;

      // Now kill a \r if there is one, and return the result.
      if ( outPtr != &wcharBuffer.front() && outPtr[ -1 ] == L'\r' )
          --outPtr;

      out = wstring( &wcharBuffer.front(), outPtr - &wcharBuffer.front() );

      ++linesRead;

      return true;
    }
  }
}

bool DslScanner::readNextLineWithoutComments( wstring & out, size_t & offset )
                 throw( Ex, Iconv::Ex )
{
  wstring str;
  bool commentToNextLine = false;
  size_t currentOffset;

  out.erase();
  offset = 0;

  do
  {
    bool b = readNextLine( str, currentOffset );

    if( offset == 0 )
      offset = currentOffset;

    if( !b )
      return false;

    stripComments( str, commentToNextLine);

    out += str;
  }
  while( commentToNextLine );

  return true;
}

/////////////// DslScanner

DslIconv::DslIconv( DslEncoding e ) throw( Iconv::Ex ):
  Iconv( Iconv::GdWchar, getEncodingNameFor( e ) )
{
}

void DslIconv::reinit( DslEncoding e ) throw( Iconv::Ex )
{
  Iconv::reinit( Iconv::GdWchar, getEncodingNameFor( e ) );
}

char const * DslIconv::getEncodingNameFor( DslEncoding e )
{
  switch( e )
  {
    case Utf16LE:
      return "UTF-16LE";
    case Utf16BE:
      return "UTF-16BE";
    case Windows1252:
      return "WINDOWS-1252";
    case Windows1251:
      return "WINDOWS-1251";
    case Details::Utf8:
      return "UTF-8";
    case Windows1250:
    default:
      return "WINDOWS-1250";
  }
}


void processUnsortedParts( wstring & str, bool strip )
{
  int refCount = 0;

  size_t startPos = 0;

  for( size_t x = 0; x < str.size(); )
  {
    wchar ch = str[ x ];

    if ( ch == L'\\' )
    {
      // Escape code
      x += 2;
      continue;
    }

    if ( ch == '{' )
    {
      ++refCount;

      if ( !strip )
      {
        // Just remove it and continue
        str.erase( x, 1 );
        continue;
      }
      else
      if ( refCount == 1 )
      {
        // First opening brace. Save this position, we will be erasing the
        // whole range when we encounter the last closing brace.
        startPos = x;
      }
    }
    else
    if ( ch == '}' )
    {
      --refCount;

      if ( refCount < 0 )
      {
        GD_FDPRINTF( stderr, "Warning: an unmatched closing brace was encountered.\n" );
        refCount = 0;
        // But we remove that thing either way
        str.erase( x, 1 );
        continue;
      }

      if ( !strip )
      {
        // Just remove it and continue
        str.erase( x, 1 );
        continue;
      }
      else
      if ( !refCount )
      {
        // The final closing brace -- we can erase the whole range now.
        str.erase( startPos, x - startPos + 1 );
        x = startPos;

        continue;
      }
    }

    ++x;
  }

  if ( strip && refCount )
  {
    GD_FDPRINTF( stderr, "Warning: unclosed brace(s) encountered.\n" );
    str.erase( startPos );
  }
}

void expandOptionalParts( wstring & str, list< wstring > * result,
                          size_t x, bool inside_recurse )
{
  list< wstring > expanded;
  list< wstring > * headwords;
  headwords = inside_recurse ? result : &expanded;

  for( ; x < str.size(); )
  {
    wchar ch = str[ x ];

    if ( ch == L'\\' )
    {
      // Escape code
      x += 2;
    }
    else
    if ( ch == L'(' )
    {
      // First, handle the case where this block is removed

      {
        int refCount = 1;

        for( size_t y = x + 1; y < str.size(); ++y )
        {
          wchar ch = str[ y ];

          if ( ch == L'\\' )
          {
            // Escape code
            ++y;
          }
          else
          if ( ch == L'(' )
            ++refCount;
          else
          if ( ch == L')' )
          {
            if ( !--refCount )
            {
              // Now that the closing parenthesis is found,
              // cut the whole thing out and be done.

              if ( y != x + 1 ) // Only do for non-empty cases
              {
                wstring removed( str, 0, x );
                removed.append( str, y + 1, str.size() - y - 1 );

                expandOptionalParts( removed, headwords, x, true );
              }

              break;
            }
          }
        }

        if ( refCount && x != str.size() - 1 )
        {
          // Closing paren not found? Chop it.

          wstring removed( str, 0, x );

          // Limit the amount of results to avoid excessive resource consumption
          if ( headwords->size() < 32 )
            headwords->push_back( removed );
          else
          {
            if( !inside_recurse )
              result->merge( expanded );
            return;
          }
        }
      }

      // Now, handling the case where it is kept -- we just erase
      // the paren and go on

      str.erase( x, 1 );
    }
    else
    if ( ch == L')' )
    {
      // Closing paren doesn't mean much -- just erase it
      str.erase( x, 1 );
    }
    else
      ++x;
  }

  // Limit the amount of results to avoid excessive resource consumption
  if ( headwords->size() < 32 )
    headwords->push_back( str );
  if( !inside_recurse )
    result->merge( expanded );
}

static const wstring openBraces( GD_NATIVE_TO_WS( L"{{" ) );
static const wstring closeBraces( GD_NATIVE_TO_WS( L"}}" ) );

void stripComments( wstring & str, bool & nextLine )
{
  string::size_type n = 0, n2 = 0;

  for( ; ; )
  {
    if( nextLine )
    {
      n = str.find( closeBraces, n2 );
      if( n == string::npos )
      {
        str.erase( n2, n );
        return;
      }
      str.erase( n2, n - n2 + 2 );
      nextLine = false;
    }

    n = str.find( openBraces, n2 );
    if( n == string::npos )
      return;
    nextLine = true;
    n2 = n;
  }
}

void expandTildes( wstring & str, wstring const & tildeReplacement )
{
  for( size_t x = 0; x < str.size(); )
    if ( str[ x ] == L'\\' )
      x+=2;
    else
    if ( str[ x ] == L'~' )
    {
      if( x > 0 && str[ x - 1 ] == '^' && ( x < 2 || str[ x - 2 ] != '\\' ) )
      {
        str.replace( x - 1, 2, tildeReplacement );
        str[ x - 1 ] = QChar( str[ x - 1 ] ).isUpper() ? QChar::toLower( (uint)str[ x - 1 ] )
                                                       : QChar::toUpper( (uint)str[ x - 1 ] );
        x = x - 1 + tildeReplacement.size();
      }
      else
      {
        str.replace( x, 1, tildeReplacement );
        x += tildeReplacement.size();
      }
    }
    else
      ++x;
}

void unescapeDsl( wstring & str )
{
  for( size_t x = 0; x < str.size(); ++x )
    if ( str[ x ] == L'\\' )
      str.erase( x, 1 ); // ++x would skip the next char without processing it
}

void normalizeHeadword( wstring & str )
{
  for( size_t x = str.size(); x-- > 1; ) // >1 -- Don't test the first char
  {
    if ( str[ x ] == L' ' )
    {
      size_t y;
      for( y = x; y && ( str[ y - 1 ] == L' ' ) ; --y );

      if ( y != x )
      {
        // Remove extra spaces

        str.erase( y, x - y );

        x = y;
      }
    }
  }
  if( !str.empty() && str[ str.size() - 1 ] == L' ' )
    str.erase( str.size() - 1, 1 );
  if( !str.empty() && str[ 0 ] == L' ' )
    str.erase( 0, 1 );
}

namespace
{
  void cutEnding( wstring & where, wstring const & ending )
  {
    if ( where.size() > ending.size() &&
         where.compare( where.size() - ending.size(),
                               ending.size(), ending ) == 0 )
      where.erase( where.size() - ending.size() );
  }
}

quint32 dslLanguageToId( wstring const & name )
{
  static wstring newSp( GD_NATIVE_TO_WS( L"newspelling" ) );
  static wstring st( GD_NATIVE_TO_WS( L"standard" ) );
  static wstring ms( GD_NATIVE_TO_WS( L"modernsort" ) );
  static wstring ts( GD_NATIVE_TO_WS( L"traditionalsort" ) );
  static wstring prc( GD_NATIVE_TO_WS( L"prc" ) );

  // Any of those endings are to be removed

  wstring nameStripped = Folding::apply( name );

  cutEnding( nameStripped, newSp );
  cutEnding( nameStripped, st );
  cutEnding( nameStripped, ms );
  cutEnding( nameStripped, ts );
  cutEnding( nameStripped, prc );

  return LangCoder::findIdForLanguage( nameStripped );
}

}
}
