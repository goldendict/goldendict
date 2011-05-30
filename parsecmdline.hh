#ifndef PARSECMDLINE_HH
#define PARSECMDLINE_HH

#include <QStringList>

/// Given a command line (name of the executable with optional arguments),
/// separates-out the name and all the arguments into a list. Supports quotes
/// and double-quotes.
QStringList parseCommandLine( QString const & );

#endif // PARSECMDLINE_HH
