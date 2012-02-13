#include <string.h>

#include "decompress.hh"
#include "zlib.h"
#include "bzlib.h"

string decompressZlib( char * bufptr, unsigned length )
{
z_stream zs;
char buf[2048];
string str;
int res;
    memset( &zs, 0, sizeof(zs) );
    zs.next_in = (Bytef *)bufptr;
    zs.avail_in = length;
    while( 1 )
    {
        res = inflateInit( &zs );
        if( res != Z_OK )
            break;
        while( res != Z_STREAM_END )
        {
            memset( buf, 0, sizeof(buf) );
            zs.next_out = (Bytef *)buf;
            zs.avail_out = 2047;
            res = inflate( &zs, Z_SYNC_FLUSH );
            str += buf;
            if( res != Z_OK && res != Z_STREAM_END )
                 break;
        }
        break;
    }
    inflateEnd( &zs );
    if( res != Z_STREAM_END )
        str.clear();
    return str;
}

string decompressBzip2( char * bufptr, unsigned length )
{
bz_stream zs;
char buf[2048];
string str;
int res;
    memset( &zs, 0, sizeof(zs) );
    zs.next_in = bufptr;
    zs.avail_in = length;
    zs.total_in_lo32 = length;
    while( 1 )
    {
        res = BZ2_bzDecompressInit( &zs, 0, 0 );
        if( res != BZ_OK )
            break;
        while( res != BZ_STREAM_END )
        {
            memset( buf, 0, sizeof(buf) );
            zs.next_out = buf;
            zs.avail_out = 2047;
            zs.total_out_lo32 = length;
            res = BZ2_bzDecompress( &zs );
            str += buf;
            if( res != BZ_OK && res != BZ_STREAM_END )
                break;
        }
        break;
    }
    BZ2_bzDecompressEnd( &zs );
    if( res != BZ_STREAM_END )
        str.clear();
    return str;
}

