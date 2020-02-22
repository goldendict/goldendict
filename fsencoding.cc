/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fsencoding.hh"
#include "wstring_qt.hh"
#include <QString>
#include <QDir>
#include <vector>

namespace FsEncoding {

string encode( wstring const & str )
{
#ifdef __WIN32
    return string( gd::toQString( str ).toUtf8().data() );
#else
    return string( gd::toQString( str ).toLocal8Bit().data() );
#endif
}

string encode( string const & str )
{
#ifdef __WIN32
    return string( str );
#else
    return string( QString::fromUtf8( str.c_str() ).toLocal8Bit().data() );
#endif
}

string encode( QString const & str )
{
#ifdef __WIN32
    return string( str.toUtf8().data() );
#else
    return string( str.toLocal8Bit().data() );
#endif
}

wstring decode( string const & str )
{
#ifdef __WIN32
    return gd::toWString( QString::fromUtf8( str.c_str() ) );
#else
    return gd::toWString( QString::fromLocal8Bit( str.c_str() ) );
#endif
}

QString decode( const char *str )
{
#ifdef __WIN32
    return QString::fromUtf8( str );
#else
    return QString::fromLocal8Bit( str );
#endif
}

char separator()
{
    return QDir::separator().toLatin1();
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
