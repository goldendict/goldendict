/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WSTRING_HH_INCLUDED__
#define __WSTRING_HH_INCLUDED__

#include <string>

/// While most systems feature a 4-byte wchar_t and an UCS-4 Unicode
/// characters representation for it, Windows uses 2-byte wchar_t and an UTF-16
/// encoding. The use of UTF-16 on Windows is most probably a homeage to an
/// ancient history dating back to when there was nothing but a BMP, and
/// all Unicode chars were 2 bytes long. After the Unicode got expanded past
/// two-byte representation, the guys at Microsoft had probably decided that
/// the least painful way to go is to just switch to UTF-16. Or so's the theory.
///
/// Now, the UTF family is an encoding, made for transit purposes -- is not a
/// representation. While it's good for passthrough, it's not directly
/// applicable for manipulation on Unicode symbols. It must be decoded first to
/// a normal UCS. Think like this: UTF to UCS is something like Base64 to ASCII.
///
/// The distinction between Microsoft platform and all other ones is that while
/// the latters are stuck in an 8-bit era and use UTF-8 to pass unicode around
/// through its venerable interfaces, the former one is stuck in a 16-bit era,
/// and uses UTF-16 instead. Neither solution allows for direct processing of
/// the symbols in those strings without decoding them first. And the 16-bit
/// solution is even more ugly than the 8-bit one, because it doesn't have a
/// benefit of ASCII compatibility, having a much more useless UCS-2
/// compatibility instead. It's stuck in the middle of nowhere, really.
///
/// The question is, what are we going to do with all this? When we do Unicode
/// processing in GoldenDict, we want to use real Unicode characters, not some
/// UTF-16 encoded ones. To that end, we have two options under Windows: first,
/// use QString, and second, use basic_string< unsigned int >.
/// While we use QStrings for the GUI and other non-critical code, there is a
/// serious doubt on the efficiency of QStrings for bulk text processing. And
/// since a lot of code uses wstring already, it would be much easier to convert
/// it to use basic_string< unsigned int > instead, since it shares the same
/// template, and therefore the interface too, with wstring. That's why we
/// introduce our own gd::wstring and gd::wchar types here. On all systems but
/// Windows, they are equivalent to std::wstring and wchar_t. On Windows, they
/// are basic_string< unsigned int > and unsigned int.
///
///
/// Now we have a better built-in type as char32_t and std::u32string

namespace gd
{
   typedef char32_t wchar;
   typedef std::u32string wstring;
}

#endif
