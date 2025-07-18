#ifndef COUNTRY_HH
#define COUNTRY_HH

#include <QString>

namespace Country {

/// Attempts converting the given country name, in english, to its iso-3166-1
/// 2-letter code. If fails, empty string is returned.
QString englishNametoIso2( QString const & );

}

#endif // COUNTRY_HH
