#include "decompress.hh"
#include "zlib.h"
#include "bzlib.h"

#ifdef MAKE_ZIM_SUPPORT
#include "lzma.h"
#endif

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

#ifdef MAKE_ZIM_SUPPORT

string decompressLzma2( const char * bufptr, unsigned length,
                        bool raw_decoder )
{
string str;
lzma_ret res;
char buf[CHUNK_SIZE];

  lzma_stream strm = LZMA_STREAM_INIT;
  strm.next_in = reinterpret_cast< const uint8_t * >( bufptr );
  strm.avail_in = length;

  lzma_options_lzma opt;
  lzma_filter filters[ 2 ];

  if( raw_decoder )
  {
    lzma_lzma_preset(&opt, LZMA_PRESET_DEFAULT);

    filters[ 0 ].id = LZMA_FILTER_LZMA2;
    filters[ 0 ].options = &opt;
    filters[ 1 ].id = LZMA_VLI_UNKNOWN;
  }

  while( 1 )
  {
    if( raw_decoder )
      res = lzma_raw_decoder( &strm, filters );
    else
      res = lzma_stream_decoder( &strm, UINT64_MAX, 0 );

    if( res != LZMA_OK )
      break;

    while ( res != LZMA_STREAM_END )
    {
      strm.next_out = reinterpret_cast< uint8_t * >( buf );
      strm.avail_out = CHUNK_SIZE;
      res = lzma_code( &strm, LZMA_RUN );
      str.append( buf, CHUNK_SIZE - strm.avail_out );
      if( res != LZMA_OK && res != LZMA_STREAM_END )
        break;
    }
    lzma_end( &strm );
    if( res != LZMA_STREAM_END )
      str.clear();

    break;
  }
  return str;
}

#endif
