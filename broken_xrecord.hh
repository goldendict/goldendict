/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __BROKEN_XRECORD_HH_INCLUDED__
#define __BROKEN_XRECORD_HH_INCLUDED__

/// Returns true if the RECORD extension is likely to be broken. Under Windows
/// it always returns false.
/// This function is to be removed once the RECORD extension is working again
/// on all the major distributions.
bool isRECORDBroken();

#endif
