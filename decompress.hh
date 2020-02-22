#ifndef __DECOMPRESS_HH_INCLUDED__
#define __DECOMPRESS_HH_INCLUDED__

#include <QByteArray>
#include <string>

using std::string;

QByteArray zlibDecompress( const char * bufptr, unsigned length );

string decompressZlib( const char * bufptr, unsigned length );

string decompressBzip2( const char * bufptr, unsigned length );

#if defined(GD_ZIM_SUPPORT) || defined(GD_SLOB_SUPPORT)

string decompressLzma2( const char * bufptr, unsigned length,
                        bool raw_decoder = false );

#endif

#endif // DECOMPRESS_HH
