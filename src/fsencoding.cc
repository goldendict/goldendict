/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fsencoding.hh"
#include <QString>
#include <QDir>
#include <vector>
#include <libgen.h>

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
  std::vector< char > tmp( str.size() + 1 );

  memcpy( &tmp.front(), str.c_str(), tmp.size() );

  return ::dirname( &tmp.front() );
}

}
