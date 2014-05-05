#ifndef __DICTSERVER_HH__INCLUDED__
#define __DICTSERVER_HH__INCLUDED__

#include "dictionary.hh"
#include "config.hh"

/// Support for servers supporting DICT protocol.
namespace DictServer {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Config::DictServers const & servers )
    throw( std::exception );

}

#endif // __DICTSERVER_HH__INCLUDED__
