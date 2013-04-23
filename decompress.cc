#include "decompress.hh"
#include "zlib.h"
#include "bzlib.h"

#define CHUNK_SIZE 2048

QByteArray zlibDecompress( const char * bufptr, unsigned length )
{
z_stream zs;
char buf[CHUNK_SIZE];
QByteArray str;
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
            zs.next_out = (Bytef *)buf;
            zs.avail_out = CHUNK_SIZE;
            res = inflate( &zs, Z_SYNC_FLUSH );
            str.append( buf, CHUNK_SIZE - zs.avail_out );
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

string decompressZlib( const char * bufptr, unsigned length )
{
  QByteArray b = zlibDecompress( bufptr, length );
  return string( b.constData(), b.size() );
}

string decompressBzip2( const char * bufptr, unsigned length )
{
bz_stream zs;
char buf[CHUNK_SIZE];
string str;
int res;
    memset( &zs, 0, sizeof(zs) );
    zs.next_in = (char *)bufptr;
    zs.avail_in = length;
    zs.total_in_lo32 = length;
    while( 1 )
    {
        res = BZ2_bzDecompressInit( &zs, 0, 0 );
        if( res != BZ_OK )
            break;
        while( res != BZ_STREAM_END )
        {
            zs.next_out = buf;
            zs.avail_out = CHUNK_SIZE;
            zs.total_out_lo32 = length;
            res = BZ2_bzDecompress( &zs );
            str.append( buf, CHUNK_SIZE - zs.avail_out );
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

