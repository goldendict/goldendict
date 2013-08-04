/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "utf8.hh"
#include <vector>

namespace Utf8 {

size_t encode( wchar const * in, size_t inSize, char * out_ )
{
  unsigned char * out = (unsigned char *) out_;

  while( inSize-- )
  {
    if ( *in < 0x80 )
      *out++ = *in++;
    else
    if ( *in < 0x800 )
    {
      *out++ = 0xC0 | ( *in >> 6 );
      *out++ = 0x80 | ( *in++ & 0x3F );
    }
    else
    if ( *in < 0x10000 )
    {
      *out++ = 0xE0 | ( *in >> 12 );
      *out++ = 0x80 | ( ( *in >> 6 ) & 0x3F );
      *out++ = 0x80 | ( *in++ & 0x3F );
    }
    else
    {
      *out++ = 0xF0 | ( *in >> 18 );
      *out++ = 0x80 | ( ( *in >> 12 ) & 0x3F );
      *out++ = 0x80 | ( ( *in >> 6 ) & 0x3F );
      *out++ = 0x80 | ( *in++ & 0x3F );
    }
  }

  return out - (unsigned char *) out_;
}

long decode( char const * in_, size_t inSize, wchar * out_ )
{
  unsigned char const * in = (unsigned char const *) in_;
  wchar * out = out_;

  while( inSize-- )
  {
    wchar result;

    if ( *in & 0x80 )
    {
      if ( *in & 0x40 )
      {
        if ( *in & 0x20 )
        {
          if ( *in & 0x10 )
          {
            // Four-byte sequence
            if ( *in & 8 )
              // This can't be
              return -1;

            if ( inSize < 3 )
              return -1;

            inSize -= 3;

            result = ( (wchar )*in++ & 7 ) << 18;

            if ( ( *in & 0xC0 ) != 0x80 )
              return -1;
            result |= ( (wchar)*in++ & 0x3F ) << 12;

            if ( ( *in & 0xC0 ) != 0x80 )
              return -1;
            result |= ( (wchar)*in++ & 0x3F ) << 6;

            if ( ( *in & 0xC0 ) != 0x80 )
              return -1;
            result |= (wchar)*in++ & 0x3F;
          }
          else
          {
            // Three-byte sequence

            if ( inSize < 2 )
              return -1;

            inSize -= 2;

            result = ( (wchar )*in++ & 0xF ) << 12;

            if ( ( *in & 0xC0 ) != 0x80 )
              return -1;
            result |= ( (wchar)*in++ & 0x3F ) << 6;

            if ( ( *in & 0xC0 ) != 0x80 )
              return -1;
            result |= (wchar)*in++ & 0x3F;
          }
        }
        else
        {
          // Two-byte sequence
          if ( !inSize )
            return -1;

          --inSize;

          result = ( (wchar )*in++ & 0x1F ) << 6;

          if ( ( *in & 0xC0 ) != 0x80 )
            return -1;
          result |= (wchar)*in++ & 0x3F;
        }
      }
      else
      {
        // This char is from the middle of encoding, it can't be leading
        return -1;
      }
    }
    else
      // One-byte encoding
      result = *in++;

    *out++ = result;
  }

  return out - out_;
}

string encode( wstring const & in ) throw()
{
  std::vector< char > buffer( in.size() * 4 );

  return string( &buffer.front(),
                 encode( in.data(), in.size(), &buffer.front() ) );
}

wstring decode( string const & in ) throw( exCantDecode )
{
  
  if ( in.size() == 0 )
    return wstring();

  std::vector< wchar > buffer( in.size() );

  long result = decode( in.data(),  in.size(), &buffer.front() );

  if ( result < 0 )
    throw exCantDecode( in );

  return wstring( &buffer.front(), result );
}

}
