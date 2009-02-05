/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "htmlescape.hh"

namespace Html {

string escape( string const & str )
{
  string result( str );

  for( size_t x = result.size(); x--; )
    switch ( result[ x ] )
    {
      case '&':
        result.erase( x, 1 );
        result.insert( x, "&amp;" );
      break;

      case '<':
        result.erase( x, 1 );
        result.insert( x, "&lt;" );
      break;

      case '>':
        result.erase( x, 1 );
        result.insert( x, "&gt;" );
      break;

      case '"':
        result.erase( x, 1 );
        result.insert( x, "&quot;" );
      break;

      default:
      break;
    }

  return result;
}

}
