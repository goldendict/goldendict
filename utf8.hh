/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <cstdio>
#include <string>
#include "ex.hh"
#include "wstring.hh"

/// A simple UTF-8 encoder/decoder. Some dictionary backends only require
/// utf8, so we have this separately, removing the iconv dependency for them.
/// Besides, utf8 is quite ubiquitous now, and its use is spreaded over many
/// places.
namespace Utf8 {

using std::string;
using gd::wstring;
using gd::wchar;

DEF_EX_STR( exCantDecode, "Can't decode the given string from Utf8:", std::exception )

/// Encodes the given UCS-4 into UTF-8. The inSize specifies the number
/// of wide characters the 'in' pointer points to. The 'out' buffer must be
/// at least inSize * 4 bytes long. The function returns the number of chars
/// stored in the 'out' buffer. The result is not 0-terminated.
size_t encode( wchar const * in, size_t inSize, char * out );
/// Decodes the given UTF-8 into UCS-32. The inSize specifies the number
/// of bytes the 'in' pointer points to. The 'out' buffer must be at least
/// inSize wide characters long. If the given UTF-8 is invalid, the decode
/// function returns -1, otherwise it returns the number of wide characters
/// stored in the 'out' buffer. The result is not 0-terminated.
long decode( char const * in, size_t inSize, wchar * out );

/// Versions for non time-critical code.
string encode( wstring const & ) throw();
wstring decode( string const & ) throw( exCantDecode );

/// Since the standard isspace() is locale-specific, we need something
/// that would never mess up our utf8 input. The stock one worked fine under
/// Linux but was messing up strings under Windows.
bool isspace( int c );

}
