/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fsencoding.hh"
#include <QString>
#include <QDir>
#include <vector>

namespace FsEncoding {

string encode( wstring const & str )
{
  return string( QString::fromStdWString( str ).toLocal8Bit().data() );
}

string encode( string const & str )
{
  return string( QString::fromUtf8( str.c_str() ).toLocal8Bit().data() );
}

char separator()
{
  return QDir::separator().toAscii();
}

string dirname( string const & str )
{
  size_t x = str.rfind( separator() );

  if ( x == string::npos )
    return string( "." );

  return string( str, 0, x );
}

string basename( string const & str )
{
  size_t x = str.rfind( separator() );

  if ( x == string::npos )
    return str;

  return string( str, x + 1 );
}

}
