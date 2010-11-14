/* This file is (c) 2008-2011 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef TERMINATION_HH
#define TERMINATION_HH

// Installs the termination handler which attempts to pop Qt's dialog showing
// the exception and backtrace, and then aborts.
void installTerminationHandler();

#endif // TERMINATION_HH
