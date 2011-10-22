/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FSENCODING_HH_INCLUDED__
#define __FSENCODING_HH_INCLUDED__

#include "wstring.hh"
#include <QString>

/// Utilities to convert a wide string or an utf8 string to the local 8bit
/// encoding of the file system, and to do other manipulations on the file
/// names.
namespace FsEncoding {

using std::string;
using gd::wstring;

/// Encodes the given wide string to the utf8 encoding.
string encode( wstring const & );

/// Encodes the given string in utf8 to the system 8bit encoding.
string encode( string const & );

/// Encodes the QString to the utf8/local 8-bit encoding.
string encode( QString const & );

/// Decodes the given utf8-encoded string to a wide string.
wstring decode( string const & str );

/// Decodes the given utf8/local 8-bit string to a QString.
QString decode( const char *str );

/// Returns the filesystem separator (/ on Unix and clones, \ on Windows).
char separator();

/// Returns the directory part of the given filename.
string dirname( string const & );

/// Returns the name part of the given filename.
string basename( string const & );

}

#endif
