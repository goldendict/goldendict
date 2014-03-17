/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FOLDING_HH_INCLUDED__
#define __FOLDING_HH_INCLUDED__

#include "wstring.hh"

/// Folding provides means to translate several possible ways to write a
/// symbol into one. This facilitates searching. Here we currently perform
/// full case folding (everything gets translated to lowercase, ligatures
/// and complex letters are decomposed), diacritics folding (all diacritic
/// marks get removed) and whitespace/punctuation marks removal. These
/// transforms are done according to the Unicode standard and/or drafts. The
/// exact algorithms, lists and types of folding performed might get changed
/// in the future -- in this case, the Version field will be bumped up.

namespace Folding {

using gd::wstring;
using gd::wchar;

/// The algorithm's version.
enum
{
  Version = 5
};

/// Applies the folding algorithm to each character in the given string,
/// making another one as a result.
wstring apply( wstring const &, bool preserveWildcards = false );

/// Applies only simple case folding algorithm. Since many dictionaries have
/// different case style, we interpret words differing only by case as synonyms.
wstring applySimpleCaseOnly( wstring const & );

/// Applies only full case folding algorithm. This includes simple case, but also
/// decomposing ligatures and complex letters.
wstring applyFullCaseOnly( wstring const & );

/// Applies only diacritics folding algorithm.
wstring applyDiacriticsOnly( wstring const & );

/// Applies only punctuation folding algorithm.
wstring applyPunctOnly( wstring const & );

/// Applies only whitespace folding algorithm.
wstring applyWhitespaceOnly( wstring const & );

/// Applies only whitespace&punctuation folding algorithm.
wstring applyWhitespaceAndPunctOnly( wstring const & );

/// Returns true if the given character is any form of whitespace, false
/// otherwise. Whitespace corresponds to Zl/Zp/Zs Unicode classes, and also
/// includes \n, \r and \t.
bool isWhitespace( wchar ch );

/// Returns true if the given character is any form of punctuation, false
/// otherwise. Punctuation corresponds to Pc/Pd/Pe/Pf/Pi/Po/Ps classes.
bool isPunct( wchar ch );

/// Removes any whitespace or punctuation from the beginning and the end of
/// the word.
wstring trimWhitespaceOrPunct( wstring const & );

/// Removes any whitespace from the beginning and the end of
/// the word.
wstring trimWhitespace( wstring const & );

/// Turns any sequences of consecutive whitespace into a single basic space.
void normalizeWhitespace( wstring & );

/// Same as apply( wstring ), but without any heap operations, therefore
/// preferable when there're many strings to process. Returns -1 if the
/// operation succeded, or otherwise the minimum value of outSize required
/// to succeed.
/// Currently commented out, consider implementing it in case indices'
/// generation would be too slow.
//ssize_t apply( wchar const * in, wchar * out, size_t outSize );

/// Unescape all wildcard symbols (for exast search)
wstring unescapeWildcardSymbols( wstring const & );

}

#endif

