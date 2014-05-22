/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "epwing_charmap.hh"

namespace Epwing {

EpwingCharmap & EpwingCharmap::instance()
{
  static EpwingCharmap ec;
  return ec;
}

QByteArray EpwingCharmap::mapToUtf8( QString const & code )
{
  if( charMap.contains( code ) )
    return QString( charMap[ code ] ).toUtf8();

  return QByteArray();
}

void EpwingCharmap::addEntry( QString const & code, QChar ch )
{
  charMap[ code ] = ch;
}

EpwingCharmap::EpwingCharmap()
{
  addEntry( "na121", 0x00E1 );
  addEntry( "na122", 0x00E0 );
  addEntry( "na12e", 0x00E9 );
  addEntry( "na12f", 0x00E8 );
  addEntry( "na134", 0x00ED );
  addEntry( "na135", 0x00EC );
  addEntry( "na136", 0x00F3 );
  addEntry( "na137", 0x00F2 );
  addEntry( "na13e", 0x00FA );
  addEntry( "na144", 0x01FD );
  addEntry( "na149", 0x0067 );
  addEntry( "na157", 0x00E9 );
  addEntry( "na16a", 0x00C1 );
  addEntry( "na16b", 0x00C0 );
  addEntry( "na16d", 0x00C9 );
  addEntry( "na171", 0x00FD );
  addEntry( "na172", 0x1EF3 );
  addEntry( "na17d", 0x00DA );
  addEntry( "na235", 0x002F );
  addEntry( "na240", 0x0154 );
  addEntry( "na627", 0x00A7 );
  addEntry( "na637", 0x30FB );
  addEntry( "na646", 0x00C6 );
  addEntry( "na647", 0x00C7 );
  addEntry( "na649", 0x00C9 );
  addEntry( "na660", 0x00E0 );
  addEntry( "na62b", 0x00AB );
  addEntry( "na63b", 0x00BB );
  addEntry( "na664", 0x00E4 );
  addEntry( "na667", 0x00E7 );
  addEntry( "na668", 0x00E8 );
  addEntry( "na669", 0x00E9 );
  addEntry( "na66a", 0x00EA );
  addEntry( "na66b", 0x00EB );
  addEntry( "na671", 0x00F1 );
  addEntry( "na674", 0x00F4 );
  addEntry( "na676", 0x00F6 );
  addEntry( "na722", 0x0152 );
  addEntry( "na723", 0x0153 );
  addEntry( "na73e", 0x0101 );
  addEntry( "na74b", 0x0227 );
  addEntry( "na823", 0x2026 );
  addEntry( "wa424", 0x00E6 );
  addEntry( "wa460", 0x2460 );
  addEntry( "wa461", 0x2461 );
  addEntry( "wa462", 0x2462 );
  addEntry( "wa463", 0x2463 );
  addEntry( "wa464", 0x2464 );
}

} // namespace Epwing
