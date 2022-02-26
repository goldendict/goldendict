/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "iconv.hh"
#include <vector>
#include <errno.h>
#include <string.h>
#include <QDebug>
#include "wstring_qt.hh"

#ifdef __WIN32
char const * const Iconv::GdWchar = "UTF-32LE";
#else
char const * const Iconv::GdWchar = "UTF-32LE";
#endif

char const * const Iconv::Utf16Le = "UTF-16LE";
char const * const Iconv::Utf8 = "UTF-8";

using gd::wchar;

Iconv::Iconv( char const * to, char const * from )
{
    codec = QTextCodec::codecForName(from);
}

Iconv::~Iconv()
{
  
}

QString Iconv::convert(void const* & inBuf, size_t& inBytesLeft)
{
    return codec->toUnicode(static_cast<const char*>(inBuf), inBytesLeft);
 
}

gd::wstring Iconv::toWstring( char const * fromEncoding, void const * fromData,
                              size_t dataSize )
  
{
  /// Special-case the dataSize == 0 to avoid any kind of iconv-specific
  /// behaviour in that regard.

  if ( !dataSize )
    return gd::wstring();

  Iconv ic( GdWchar, fromEncoding );

  /// This size is usually enough, but may be enlarged during the conversion
  std::vector< wchar > outBuf( dataSize );

  QString outStr = ic.convert(fromData, dataSize);
  return gd::toWString(outStr);
}

std::string Iconv::toUtf8( char const * fromEncoding, void const * fromData,
                           size_t dataSize )
  
{
  // Similar to toWstring

  if ( !dataSize )
    return std::string();

  Iconv ic( Utf8, fromEncoding );

  std::vector< char > outBuf( dataSize );

  QString outStr = ic.convert(fromData, dataSize);
  return gd::toStdString(outStr);
}

