/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "chunkedstorage.hh"
#include <zlib.h>
#include <string.h>
#include <QDataStream>
#include <QScopeGuard>

namespace ChunkedStorage {

enum
{
  ChunkMaxSize = 65536 // Can't be more since it would overflow the address
};

Writer::Writer( File::Class & f ):
  file( f ), chunkStarted( false ), bufferUsed( 0 )
{
  // Create a sratchpad at the beginning of file. We use it to write chunk
  // table if it would fit, in order to save some seek times.

  char zero[ 4096 ];

  memset( zero, 0, sizeof( zero ) );

  scratchPadOffset = file.tell();
  scratchPadSize = sizeof( zero );

  file.write( zero, sizeof( zero ) );
}

uint32_t Writer::startNewBlock()
{
  if ( bufferUsed >= ChunkMaxSize )
  {
    // Need to flush first.
    saveCurrentChunk();
  }

  chunkStarted = true;

  // The address is comprised of the offset within the chunk (in lower
  // 16 bits, always fits there since ChunkMaxSize-1 does) and the
  // number of the chunk, which is therefore limited to be 65535 max.
  return bufferUsed | ( (uint32_t)offsets.size() << 16 );
}

void Writer::addToBlock( void const * data, size_t size )
{
  if ( !size )
    return;

  if ( buffer.size() - bufferUsed < size )
    buffer.resize( bufferUsed + size );

  memcpy( &buffer.front() + bufferUsed, data, size );

  bufferUsed += size;

  chunkStarted = false;
}

void Writer::saveCurrentChunk()
{
  size_t maxCompressedSize = compressBound( bufferUsed );

  if ( bufferCompressed.size() < maxCompressedSize )
    bufferCompressed.resize( maxCompressedSize );

  unsigned long compressedSize = bufferCompressed.size();

  if ( compress( &bufferCompressed.front(), &compressedSize,
                 &buffer.front(), bufferUsed ) != Z_OK )
    throw exFailedToCompressChunk();

  offsets.push_back( file.tell() );

  file.write( (uint32_t) bufferUsed );
  file.write( (uint32_t) compressedSize );
  file.write( &bufferCompressed.front(), compressedSize );

  bufferUsed = 0;

  chunkStarted = false;
}

uint32_t Writer::finish()
{
  if ( bufferUsed || chunkStarted )
    saveCurrentChunk();

  bool useScratchPad = false;
  uint32_t savedOffset = 0;

  if ( scratchPadSize >= offsets.size() * sizeof( uint32_t ) + sizeof( uint32_t ) )
  {
    useScratchPad = true;
    savedOffset = file.tell();
    file.seek( scratchPadOffset );
  }

  uint32_t offset = file.tell();

  file.write( (uint32_t) offsets.size() );

  if ( offsets.size() )
    file.write( &offsets.front(), offsets.size() * sizeof( uint32_t ) );

  if ( useScratchPad )
    file.seek( savedOffset );

  offsets.clear();
  chunkStarted = false;

  return offset;
}

Reader::Reader( File::Class & f, uint32_t offset ): file( f )
{
  file.seek( offset );

  uint32_t size =  file.read< uint32_t >();
  if ( size == 0 )
    return;
  offsets.resize( size );
  file.read( &offsets.front(), offsets.size() * sizeof( uint32_t ) );
}

char * Reader::getBlock( uint32_t address, vector< char > & chunk )
{
  size_t chunkIdx = address >> 16;

  if ( chunkIdx >= offsets.size() )
    throw exAddressOutOfRange();

  // Read and decompress the chunk
  {
    // file.seek( offsets[ chunkIdx ] );
    QMutexLocker _( &file.lock );
    auto bytes = file.map( offsets[ chunkIdx ], 8 );
    if( bytes == nullptr )
      throw mapFailed();
    auto qBytes = QByteArray::fromRawData( reinterpret_cast< char * >(bytes), 8 );
    QDataStream in( qBytes );
    in.setByteOrder( QDataStream::LittleEndian );

    uint32_t uncompressedSize;
    uint32_t compressedSize;

    in >> uncompressedSize >> compressedSize;

    file.unmap( bytes );
    chunk.resize( uncompressedSize );

    // vector< unsigned char > compressedData( compressedSize );
    auto chunkDataBytes = file.map( offsets[ chunkIdx ] + 8, compressedSize );
    if( chunkDataBytes == nullptr )
      throw mapFailed();
    // file.read( &compressedData.front(), compressedData.size() );
    auto autoUnmap = qScopeGuard(
      [ & ] {
        file.unmap( chunkDataBytes );
      } );
    Q_UNUSED( autoUnmap )

    unsigned long decompressedLength = chunk.size();

    if( uncompress( (unsigned char *)&chunk.front(), &decompressedLength, chunkDataBytes, compressedSize ) != Z_OK
        || decompressedLength != chunk.size() )
    {
      throw exFailedToDecompressChunk();
    }

  }

  size_t offsetInChunk = address & 0xffFF;

  if ( offsetInChunk > chunk.size() ) // It can be equal to for 0-sized blocks
    throw exAddressOutOfRange();

  return &chunk.front() + offsetInChunk;
}

}
