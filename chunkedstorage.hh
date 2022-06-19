/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __CHUNKEDSTORAGE_HH_INCLUDED__
#define __CHUNKEDSTORAGE_HH_INCLUDED__

#include "ex.hh"
#include "file.hh"

#include <vector>
#include <stdint.h>

/// A chunked compression storage. We use this for articles' bodies. The idea
/// is to store data in a separately-compressed chunks, much like in dictzip,
/// but without any fancy gzip-compatibility or whatever. Another difference
/// is that any block of data saved is always contained within one chunk,
/// even if its size does exceed its maximum allowed size. This is very
/// handy since we're retrieving the data by the same blocks we used to save
/// it as, that' the only kind of seek we support, really.
namespace ChunkedStorage {

using std::vector;

DEF_EX( Ex, "Chunked storage exception", std::exception )
DEF_EX( exFailedToCompressChunk, "Failed to compress a chunk", Ex )
DEF_EX( exAddressOutOfRange, "The given chunked address is out of range", Ex )
DEF_EX( exFailedToDecompressChunk, "Failed to decompress a chunk", Ex )
DEF_EX( mapFailed, "Failed to map/unmap the file", Ex )

/// This class writes data blocks in chunks.
class Writer
{
  vector< uint32_t > offsets;
  File::Class & file;
  size_t scratchPadOffset, scratchPadSize;

public:

  Writer( File::Class & );

  /// Starts new block. Returns its address.
  uint32_t startNewBlock();

  /// Add data to the previously started block.
  void addToBlock( void const * data, size_t size );

  /// Finishes writing chunks and returns the offset to the chunk table which
  /// gets written at the moment of finishing.
  uint32_t finish();

private:

  /// Indicates that an address was allocated, which would mean the writeout
  /// of the pending chunk is required even if its size is zero.
  bool chunkStarted;

  // This buffer accumulates the chunk data until either enough data is
  // stored (>=ChunkMaxSize), or there's no more data left to store.
  vector< unsigned char > buffer;

  // Here we compress the chunk before writing it out to file.
  vector< unsigned char > bufferCompressed;

  // The amount of data stored in buffer so far. We keep it separate
  // from buffer.size() for performance reasons; the latter one only
  // grows, but never shrinks.
  size_t bufferUsed;

  void saveCurrentChunk();
};

/// This class reads data blocks previously written by Writer.
class Reader
{
  vector< uint32_t > offsets;
  File::Class & file;

public:
  /// Creates reader by giving it a file to read from and the offset returned
  /// by Writer::finish().
  Reader( File::Class &, uint32_t );

  /// Reads the block previously written by Writer, identified by its address.
  /// Uses the user-provided storage to load the entire chunk, and then to
  /// return a pointer to the requested block inside it.
  char * getBlock( uint32_t address, vector< char > & );
};

}

#endif
