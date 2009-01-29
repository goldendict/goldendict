/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FOLDING_HH_INCLUDED__
#define __FOLDING_HH_INCLUDED__

#include <string>

/// Folding provides means to translate several possible ways to write a
/// symbol into one. This facilitates searching. Here we currently perform
/// full case folding (everything gets translated to lowercase, ligatures
/// and complex letters are decomposed) and diacritics folding (all diacritic
/// marks get removed). These transforms are done according to the Unicode
/// standard and/or drafts. The exact algorithms, lists and types of folding
/// performed might get changed in the future -- in this case, the Version
/// field will be bumped up.

namespace Folding {

using std::wstring;

/// The algorithm's version.
enum
{
  Version = 1
};

/// Applies the folding algorithm to each character in the given string,
/// making another one as a result.
wstring apply( wstring const & );

/// Applies only simple case folding algorithm. Since many dictionaries have
/// different case style, we interpret words differing only by case as synonyms.
wstring applySimpleCaseOnly( wstring const & );

/// Same as apply( wstring ), but without any heap operations, therefore
/// preferable when there're many strings to process. Returns -1 if the
/// operation succeded, or otherwise the minimum value of outSize required
/// to succeed.
/// Currently commented out, consider implementing it in case indices'
/// generation would be too slow.
//ssize_t apply( wchar_t const * in, wchar_t * out, size_t outSize );

}

#endif

