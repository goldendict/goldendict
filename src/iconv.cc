/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "iconv.hh"
#include <vector>
#include <errno.h>
#include <string.h>

char const * const Iconv::Wchar_t = "WCHAR_T";
char const * const Iconv::Utf16Le = "UTF-16LE";
char const * const Iconv::Utf8 = "UTF-8";

Iconv::Iconv( char const * to, char const * from ) throw( exCantInit ):
  state( iconv_open( to, from ) )
{
  if ( state == (iconv_t) -1 )
    throw exCantInit( strerror( errno ) );
}

void Iconv::reinit( char const * to, char const * from ) throw( exCantInit )
{
  iconv_close( state );

  state = iconv_open( to, from );

  if ( state == (iconv_t) -1 )
    throw exCantInit( strerror( errno ) );
}

Iconv::~Iconv() throw()
{
  iconv_close( state );
}

Iconv::Result Iconv::convert( void const * & inBuf, size_t  & inBytesLeft,
                              void * & outBuf, size_t & outBytesLeft )
  throw( exIncorrectSeq, exOther )
{
  size_t result = iconv( state,
                         (char **)&inBuf, &inBytesLeft,
                         (char **)&outBuf, &outBytesLeft );

  if ( result == (size_t) -1 )
  {
    switch( errno )
    {
      case EILSEQ:
        throw exIncorrectSeq();
      case EINVAL:
        return NeedMoreIn;
      case E2BIG:
        return NeedMoreOut;
      default:
        throw exOther( strerror( errno ) );
    }
  }

  return Success;
}

std::wstring Iconv::toWstring( char const * fromEncoding, void const * fromData,
                               size_t dataSize )
  throw( exCantInit, exIncorrectSeq, exPrematureEnd, exOther )
{
  /// Special-case the dataSize == 0 to avoid any kind of iconv-specific
  /// behaviour in that regard.

  if ( !dataSize )
    return std::wstring();

  Iconv ic( Wchar_t, fromEncoding );

  /// This size is usually enough, but may be enlarged during the conversion
  std::vector< wchar_t > outBuf( dataSize );

  void * outBufPtr = &outBuf.front();

  size_t outBufLeft = outBuf.size() * sizeof( wchar_t );

  for( ; ; )
  {
    switch( ic.convert( fromData, dataSize, outBufPtr, outBufLeft ) )
    {
      case Success:
        return std::wstring( &outBuf.front(),
                             outBuf.size() - outBufLeft / sizeof( wchar_t ) );
      case NeedMoreIn:
        throw exPrematureEnd();
      case NeedMoreOut:
      {
        // Grow the buffer and retry
        // The pointer may get invalidated so we save the diff and restore it
        size_t offset = (wchar_t *)outBufPtr - &outBuf.front();
        outBuf.resize( outBuf.size() + 256 );
        outBufPtr = &outBuf.front() + offset;
        outBufLeft += 256;
      }
    }
  }
}

std::string Iconv::toUtf8( char const * fromEncoding, void const * fromData,
                           size_t dataSize )
  throw( exCantInit, exIncorrectSeq, exPrematureEnd, exOther )
{
  // Similar to toWstring

  if ( !dataSize )
    return std::string();

  Iconv ic( Utf8, fromEncoding );

  std::vector< char > outBuf( dataSize );

  void * outBufPtr = &outBuf.front();

  size_t outBufLeft = outBuf.size();

  for( ; ; )
  {
    switch( ic.convert( fromData, dataSize, outBufPtr, outBufLeft ) )
    {
      case Success:
        return std::string( &outBuf.front(),
                            outBuf.size() - outBufLeft );
      case NeedMoreIn:
        throw exPrematureEnd();
      case NeedMoreOut:
      {
        // Grow the buffer and retry
        // The pointer may get invalidated so we save the diff and restore it
        size_t offset = (char *)outBufPtr - &outBuf.front();
        outBuf.resize( outBuf.size() + 256 );
        outBufPtr = &outBuf.front() + offset;
        outBufLeft += 256;
      }
    }
  }
}

