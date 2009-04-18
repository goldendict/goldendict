/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ICONV_HH_INCLUDED__
#define __ICONV_HH_INCLUDED__

#include <iconv.h>
#include "wstring.hh"
#include "ex.hh"

/// A wrapper for the iconv() character set conversion functions
class Iconv
{
  iconv_t state;

public:

  DEF_EX( Ex, "Iconv exception", std::exception )
  DEF_EX_STR( exCantInit, "Can't initialize iconv conversion:", Ex )
  DEF_EX( exIncorrectSeq, "Invalid character sequence encountered during character convesion", Ex )
  DEF_EX( exPrematureEnd, "Character sequence ended prematurely during character conversion", Ex )
  DEF_EX_STR( exOther, "An error has occured during character conversion:", Ex )

  // Some predefined character sets' names

  static char const * const GdWchar;
  static char const * const Utf16Le;
  static char const * const Utf8;

  Iconv( char const * to, char const * from ) throw( exCantInit );

  // Changes to another pair of encodings. All the internal state is reset.
  void reinit( char const * to, char const * from ) throw( exCantInit );

  ~Iconv() throw();

  enum Result
  {
    Success, // All the data was successfully converted
    NeedMoreIn, // Input has an incomplete multibyte character at its end
    NeedMoreOut // The output buffer can't hold the result
  };

  Result convert( void const * & inBuf, size_t & inBytesLeft,
                  void * & outBuf, size_t & outBytesLeft ) throw( exIncorrectSeq,
                                                                  exOther );

  // Converts a given block of data from the given encoding to a wide string.
  static gd::wstring toWstring( char const * fromEncoding, void const * fromData,
                                 size_t dataSize )
    throw( exCantInit, exIncorrectSeq, exPrematureEnd, exOther );

  // Converts a given block of data from the given encoding to an utf8-encoded
  // string.
  static std::string toUtf8( char const * fromEncoding, void const * fromData,
                             size_t dataSize )
    throw( exCantInit, exIncorrectSeq, exPrematureEnd, exOther );

private:

  // Copying/assigning not supported
  Iconv( Iconv const & );
  Iconv & operator = ( Iconv const & );
};

#endif

