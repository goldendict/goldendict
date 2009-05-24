/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __LANGUAGE_HH_INCLUDED__
#define __LANGUAGE_HH_INCLUDED__

#include <QString>

/// Language-specific stuff - codes, names, ids etc.
namespace Language {

/// This identifies any language uniquely within the program. It also has
/// two special meanings - Unknown and Any.
typedef quint32 Id;

enum
{
  /// Value for Id which signifies that the language is unknown or unspecified.
  Unknown = 0,
  /// Value for Id which signifies that the language can be any, or all of them.
  Any =  0xFFffFFff
};

/// Returns name in English for the given Id. If the Id is incorrect, or has
/// Unknown/Any values, returns empty string.
QString englishNameForId( Id );

/// Returns name for the given Id, translated to the current UI language.
/// If the Id is incorrect, or has Unknown/Any values, returns empty string.
QString localizedNameForId( Id );

// All other functions are to be used from LangCoder, which is supposed to
// be migrated here over time.

}

#endif
