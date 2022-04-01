/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ICONV_HH_INCLUDED__
#define __ICONV_HH_INCLUDED__

#include <QTextCodec>

#include "wstring.hh"
#include "ex.hh"

/// A wrapper for the iconv() character set conversion functions
class Iconv
{
  //iconv_t state;
    QTextCodec* codec;

public:

  DEF_EX( Ex, "Iconv exception", std::exception )

  // Some predefined character sets' names

  static char const * const GdWchar;
  static char const * const Utf16Le;
  static char const * const Utf8;

  Iconv( char const * to, char const * from );

  ~Iconv();

  QString convert(void const* & inBuf, size_t& inBytesLeft);

  // Converts a given block of data from the given encoding to a wide string.
  static gd::wstring toWstring( char const * fromEncoding, void const * fromData,
                                 size_t dataSize );

  // Converts a given block of data from the given encoding to an utf8-encoded
  // string.
  static std::string toUtf8( char const * fromEncoding, void const * fromData,
                             size_t dataSize );
private:

  // Copying/assigning not supported
  Iconv( Iconv const & );
  Iconv & operator = ( Iconv const & );
};

#endif

