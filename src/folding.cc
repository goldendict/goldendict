/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "folding.hh"

namespace Folding {

namespace
{
  #include "inc_case_folding.hh"
  #include "inc_diacritic_folding.hh"
}

wstring apply( wstring const & in )
{
  // First, strip diacritics

  wstring withoutDiacritics;

  withoutDiacritics.reserve( in.size() );

  wchar_t const * nextChar = in.data();

  size_t consumed;

  for( size_t left = in.size(); left; )
  {
    withoutDiacritics.push_back( foldDiacritic( nextChar, left, consumed ) );

    nextChar += consumed;
    left -= consumed;
  }

  // Now, fold the case

  wstring caseFolded;

  caseFolded.reserve( withoutDiacritics.size() * foldCaseMaxOut );

  nextChar = withoutDiacritics.data();

  wchar_t buf[ foldCaseMaxOut ];

  for( size_t left = withoutDiacritics.size(); left--; )
    caseFolded.append( buf, foldCase(  *nextChar++, buf ) );

  return caseFolded;
}

wstring applySimpleCaseOnly( wstring const & in )
{
  wchar_t const * nextChar = in.data();

  wstring out;

  out.reserve( in.size() );

  for( size_t left = in.size(); left--; )
    out.push_back( foldCaseSimple( *nextChar++ ) );

  return out;
}

}
