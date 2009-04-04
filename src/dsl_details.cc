/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dsl_details.hh"
#include <wctype.h>
#include <stdio.h>

namespace Dsl {
namespace Details {

using std::wstring;
using std::list;

#ifdef __WIN32

// wcscasecmp() function is a GNU extension, we need to reimplement it
// for non-GNU systems.

int wcscasecmp( const wchar_t *s1, const wchar_t *s2 )
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

wstring ArticleDom::Node::renderAsText() const
{
  if ( !isTag )
    return text;

  wstring result;

  for( list< Node >::const_iterator i = begin(); i != end(); ++i )
    result += i->renderAsText();

  return result;
}

// Returns true if src == 'm' and dest is 'mX', where X is a digit
static inline bool checkM( wstring const & dest, wstring const & src )
{
  return ( src == L"m" && dest.size() == 2 &&
    dest[ 0 ] == L'm' && iswdigit( dest[ 1 ] ) );
}

ArticleDom::ArticleDom( wstring const & str ):
  root( Node::Tag(), L"", L"" ), stringPos( str.c_str() )
{
  list< Node * > stack; // Currently opened tags

  Node * textNode = 0; // A leaf node which currently accumulates text.

  try
  {
    for( ;; )
    {
      nextChar();
  
      if ( ch == L'[' && !escaped )
      {
        // Beginning of a tag.
        do
        {
          nextChar();
        } while( iswblank( ch ) );

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

        while( ( ch != L']' || escaped ) && !iswblank( ch ) )
        {
          name.push_back( ch );
          nextChar();
        }

        while( iswblank( ch ) )
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

        if ( !isClosing )
        {
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
        }
        else
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

              if ( stack.back()->empty() )
              {
                // Empty nodes are deleted since they're no use

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
          {
            fprintf( stderr, "Warning: no corresponding opening tag for closing tag \"/%ls\" found.\n",
                     name.c_str() );
          }
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
          } while( iswblank( ch ) );

          wstring linkTo;

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
              }
            }
            else
              linkTo.push_back( ch );
          }

          // Add the corresponding node

          if ( textNode )
          {
            // Close the currently opened text node
            stack.pop_back();
            textNode = 0;
          }

          Node link( Node::Tag(), L"ref", L"" );
          link.push_back( Node( Node::Text(), linkTo ) );

          if ( stack.empty() )
            root.push_back( link );
          else
            stack.back()->push_back( link );

          continue;
        }
      } // if ( ch == '<' )

      // If we're here, we've got a normal symbol, to be saved as text.

      // If there's currently no text node, open one
      if ( !textNode )
      {
        Node text( Node::Text(), L"" );

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

      textNode->text.push_back( ch );
    } // for( ; ; )
  }
  catch( eot )
  {
  }

  if ( textNode )
    stack.pop_back();

  if ( stack.size() )
    fprintf( stderr, "Warning: %u tags were unclosed.\n", stack.size() );
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
    escaped = false;
}


/////////////// DslScanner

DslScanner::DslScanner( string const & fileName ) throw( Ex, Iconv::Ex ):
  encoding( Windows1252 ), iconv( encoding ), readBufferPtr( readBuffer ),
  readBufferLeft( 0 )
{
  // Since .dz is backwards-compatible with .gz, we use gz- functions to
  // read it -- they are much nicer than the dict_data- ones.
  f = gzopen( fileName.c_str(), "rb");

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

    if ( !str.compare( 0, 5, L"#NAME", 5 ) )
      isName = true;
    else
    if ( str.compare( 0, 17, L"#SOURCE_CODE_PAGE", 17 ) )
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
    else
    {
      // The encoding
      if ( !needExactEncoding )
      {
        // We don't need that!
        fprintf( stderr, "Warning: encoding was specified in a Unicode file, ignoring.\n" );
      }
      else
      if ( !wcscasecmp( arg.c_str(), L"Latin" ) )
        encoding = Windows1252;
      else
      if ( !wcscasecmp( arg.c_str(), L"Cyrillic" ) )
        encoding = Windows1251;
      else
      if ( !wcscasecmp( arg.c_str(), L"EasternEuropean" ) )
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

  wchar_t * outPtr = &wcharBuffer.front();

  for( ; ; )
  {
    // Check that we have bytes to read
    if ( readBufferLeft < 4 ) // To convert one char, we need at most 4 bytes
    {
      if ( gzeof( f ) )
      {
        if ( !readBufferLeft )
          return false;
      }
      else
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
    size_t outBytesLeft = sizeof( wchar_t );

    Iconv::Result r =
      iconv.convert( (void const *&)readBufferPtr, readBufferLeft,
                     (void *&)outPtr, outBytesLeft );

    if ( r == Iconv::NeedMoreOut && outBytesLeft == sizeof( wchar_t ) )
    {
      // Seems to be a surrogate pair with a 16-bit target wchar

      outBytesLeft *= 2;
      r = iconv.convert( (void const *&)readBufferPtr, readBufferLeft,
                     (void *&)outPtr, outBytesLeft );
      --leftInOut; // Complements the next decremention
    }

    if ( ( r != Iconv::Success && r != Iconv::NeedMoreOut ) || outBytesLeft )
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

      return true;
    }
  }
}


/////////////// DslScanner

DslIconv::DslIconv( DslEncoding e ) throw( Iconv::Ex ):
  Iconv( Iconv::Wchar_t, getEncodingNameFor( e ) )
{
}

void DslIconv::reinit( DslEncoding e ) throw( Iconv::Ex )
{
  Iconv::reinit( Iconv::Wchar_t, getEncodingNameFor( e ) );
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
    wchar_t ch = str[ x ];

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
        fprintf( stderr, "Warning: an unmatched closing brace was encountered.\n" );
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
    fprintf( stderr, "Warning: unclosed brace(s) encountered.\n" );
    str.erase( startPos );
  }
}

void expandOptionalParts( wstring & str, list< wstring > & result,
                          size_t x )
{
  for( ; x < str.size(); )
  {
    wchar_t ch = str[ x ];

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
          wchar_t ch = str[ y ];
  
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
    
                expandOptionalParts( removed, result, x );
              }
  
              break;
            }
          }
        }
  
        if ( refCount && x != str.size() - 1 )
        {
          // Closing paren not found? Chop it.

          wstring removed( str, 0, x );
  
          result.push_back( removed );
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

  result.push_back( str );
}

void expandTildes( wstring & str, wstring const & tildeReplacement )
{
  for( size_t x = 0; x < str.size(); )
    if ( str[ x ] == L'\\' )
      x+=2;
    else
    if ( str[ x ] == L'~' )
    {
      str.replace( x, 1, tildeReplacement );
      x += tildeReplacement.size();
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

}
}
