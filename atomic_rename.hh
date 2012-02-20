/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __ATOMIC_RENAME_HH_INCLUDED__
#define __ATOMIC_RENAME_HH_INCLUDED__

#include <QString>

/// Performs an atomic rename of file, from oldBame to newName. If newName
/// exists, it gets overwritten. Names should feature Qt-style separators
/// (straight slashes). Returns true on success, false on failure.
bool renameAtomically( QString const & oldName, QString const & newName );

#endif
