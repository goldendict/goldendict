// Copyright (C) 2007 Michael Niedermayer <michaelni@gmx.at>
// Copyright (C) 2013 James Almer <jamrial@gmail.com>
// Copyright (C) 2015 Zhe Wang <0x1998@gmail.com>
//
// Based on the RIPEMD-128 implementation from libavutil
//
// This program is a free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// You can get a copy of GNU General Public License along this program
// But you can always get it from http://www.gnu.org/licenses/gpl.txt
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

#include "ripemd.hh"

#include <limits>
#include <string.h>
#include <QtEndian>


static const quint32 KA[4] = {
    0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xa953fd4e
};

static const quint32 KB[4] = {
    0x50a28be6, 0x5c4dd124, 0x6d703ef3, 0x7a6d76e9
};

static const int ROTA[80] = {
    11, 14, 15, 12,  5,  8,  7 , 9, 11, 13, 14, 15,  6,  7,  9,  8,
     7 , 6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
    11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
    11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
     9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6
};

static const int ROTB[80] = {
     8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
     9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
     9,  7, 15, 11,  8,  6,  6, 14, 12, 13,  5, 14, 13, 13,  7,  5,
    15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
     8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11
};

static const int WA[80] = {
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
     7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
     3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
     1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
     4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13
};

static const int WB[80] = {
     5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
     6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
    15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
     8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
    12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11
};

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define ROUND128_0_TO_15(a,b,c,d,e,f,g,h)                               \
    a = rol(a + ((  b ^ c  ^ d)      + block[WA[n]]),         ROTA[n]); \
    e = rol(e + ((((f ^ g) & h) ^ g) + block[WB[n]] + KB[0]), ROTB[n]); \
    n++

#define ROUND128_16_TO_31(a,b,c,d,e,f,g,h)                              \
    a = rol(a + ((((c ^ d) & b) ^ d) + block[WA[n]] + KA[0]), ROTA[n]); \
    e = rol(e + (((~g | f) ^ h)      + block[WB[n]] + KB[1]), ROTB[n]); \
    n++

#define ROUND128_32_TO_47(a,b,c,d,e,f,g,h)                              \
    a = rol(a + (((~c | b) ^ d)      + block[WA[n]] + KA[1]), ROTA[n]); \
    e = rol(e + ((((g ^ h) & f) ^ h) + block[WB[n]] + KB[2]), ROTB[n]); \
    n++

#define ROUND128_48_TO_63(a,b,c,d,e,f,g,h)                              \
    a = rol(a + ((((b ^ c) & d) ^ c) + block[WA[n]] + KA[2]), ROTA[n]); \
    e = rol(e + ((  f ^ g  ^ h)      + block[WB[n]]),         ROTB[n]); \
    n++

#define R128_0                          \
    ROUND128_0_TO_15(a,b,c,d,e,f,g,h);  \
    ROUND128_0_TO_15(d,a,b,c,h,e,f,g);  \
    ROUND128_0_TO_15(c,d,a,b,g,h,e,f);  \
    ROUND128_0_TO_15(b,c,d,a,f,g,h,e)

#define R128_16                         \
    ROUND128_16_TO_31(a,b,c,d,e,f,g,h); \
    ROUND128_16_TO_31(d,a,b,c,h,e,f,g); \
    ROUND128_16_TO_31(c,d,a,b,g,h,e,f); \
    ROUND128_16_TO_31(b,c,d,a,f,g,h,e)

#define R128_32                         \
    ROUND128_32_TO_47(a,b,c,d,e,f,g,h); \
    ROUND128_32_TO_47(d,a,b,c,h,e,f,g); \
    ROUND128_32_TO_47(c,d,a,b,g,h,e,f); \
    ROUND128_32_TO_47(b,c,d,a,f,g,h,e)

#define R128_48                         \
    ROUND128_48_TO_63(a,b,c,d,e,f,g,h); \
    ROUND128_48_TO_63(d,a,b,c,h,e,f,g); \
    ROUND128_48_TO_63(c,d,a,b,g,h,e,f); \
    ROUND128_48_TO_63(b,c,d,a,f,g,h,e)


RIPEMD128::RIPEMD128()
  : count(0)
  , buffer()
  , state()
{
  state[0] = 0x67452301;
  state[1] = 0xEFCDAB89;
  state[2] = 0x98BADCFE;
  state[3] = 0x10325476;
}

void RIPEMD128::transform( const uchar buffer[64] )
{
    quint32 a, b, c, d, e, f, g, h;
    quint32 block[16];
    int n;

    a = e = state[0];
    b = f = state[1];
    c = g = state[2];
    d = h = state[3];

    for (n = 0; n < 16; n++)
      block[n] = qFromLittleEndian<quint32>( buffer + 4 * n );
    n = 0;

    R128_0; R128_0; R128_0; R128_0;

    R128_16; R128_16; R128_16; R128_16;

    R128_32; R128_32; R128_32; R128_32;

    R128_48; R128_48; R128_48; R128_48;

    h += c + state[1];
    state[1] = state[2] + d + e;
    state[2] = state[3] + a + f;
    state[3] = state[0] + b + g;
    state[0] = h;
}

void RIPEMD128::update( const uchar * data, size_t len )
{
  size_t i, j;

  j = count & 63;
  count += len;
  if ( ( j + len ) > 63 )
  {
    memcpy( &buffer[j], data, ( i = 64 - j ) );
    transform( buffer );
    for ( ; i + 63 < len; i += 64 )
      transform( &data[i] );
    j = 0;
  }
  else
  {
    i = 0;
  }
  memcpy( &buffer[j], &data[i], len - i );
}

void RIPEMD128::digest( uchar * digest )
{
  quint64 finalcount = qFromLittleEndian( count << 3 );
  update( (const uchar *) "\200", 1 );
  while ( ( count & 63 ) != 56 )
    update( ( const uchar * ) "", 1 );
  update( ( uchar * ) &finalcount, 8 ); /* Should cause a transform() */
  for ( int i = 0; i < 4; i++ )
    qToLittleEndian( state[i], digest + i*4 );
}
