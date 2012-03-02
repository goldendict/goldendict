#ifndef __DECOMPRESS_HH_INCLUDED__
#define __DECOMPRESS_HH_INCLUDED__

#include <string>

using std::string;

string decompressZlib( char * bufptr, unsigned length );

string decompressBzip2( char * bufptr, unsigned length );

#endif // DECOMPRESS_HH
