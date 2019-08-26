/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef TERMINATION_HH
#define TERMINATION_HH

#include <QFile>

extern QFile * logFilePtr;

// Installs the termination handler which attempts to pop Qt's dialog showing
// the exception and backtrace, and then aborts.
void installTerminationHandler();

#endif // TERMINATION_HH
