/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FSENCODING_HH_INCLUDED__
#define __FSENCODING_HH_INCLUDED__

#include <string>

/// Utilities to convert a wide string or an utf8 string to the local 8bit
/// encoding of the file system, and to do other manipulations on the file
/// names.
namespace FsEncoding {

using std::string;
using std::wstring;

/// Encodes the given wide string to the system 8bit encoding.
string encode( wstring const & );

/// Encodes the given string in utf8 to the system 8bit encoding.
string encode( string const & );

/// Returns the filesystem separator (/ on Unix and clones, \ on Windows).
char separator();

/// Returns the directory part of the given filename.
string dirname( string const & );

}

#endif
