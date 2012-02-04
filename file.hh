/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FILE_HH_INCLUDED__
#define __FILE_HH_INCLUDED__

#include <cstdio>
#include <string>
#include <vector>
#include "ex.hh"

/// A simple wrapper over FILE * operations with added write-buffering,
/// used for non-Qt parts of code.
/// It is possible to ifdef implementation details for some platforms.
namespace File {

DEF_EX( Ex, "File exception", std::exception )
DEF_EX_STR( exCantOpen, "Can't open", Ex )
DEF_EX( exReadError, "Error reading from the file", Ex )
DEF_EX( exWriteError, "Error writing to the file", Ex )
DEF_EX( exSeekError, "File seek error", Ex )

/// Checks if the file exists or not.

bool tryPossibleName( std::string const & name, std::string & copyTo );

void loadFromFile( std::string const & n, std::vector< char > & data );

bool exists( char const * filename ) throw();

inline bool exists( std::string const & filename ) throw()
{ return exists( filename.c_str() ); }

class Class
{
  FILE * f;
  char * writeBuffer;
  size_t writeBufferLeft;

  void open( char const * filename, char const * mode ) throw( exCantOpen );

public:

  Class( char const * filename, char const * mode ) throw( exCantOpen );

  Class( std::string const & filename, char const * mode ) throw( exCantOpen );

  /// Reads the number of bytes to the buffer, throws an error if it
  /// failed to fill the whole buffer (short read, i/o error etc).
  void read( void * buf, size_t size ) throw( exReadError, exWriteError );

  template< typename T >
  void read( T & value ) throw( exReadError, exWriteError )
  { read( &value, sizeof( value ) ); }

  template< typename T >
  T read() throw( exReadError, exWriteError )
  { T value; read( value ); return value; }

  /// Attempts reading at most 'count' records sized 'size'. Returns
  /// the number of records it managed to read, up to 'count'.
  size_t readRecords( void * buf, size_t size, size_t count ) throw( exWriteError );

  /// Writes the number of bytes from the buffer, throws an error if it
  /// failed to write the whole buffer (short write, i/o error etc).
  /// This function employs write buffering, and as such, writes may not
  /// end up on disk immediately, or a short write may occur later
  /// than it really did. If you don't want write buffering, use
  /// writeRecords() function instead.
  void write( void const * buf, size_t size ) throw( exWriteError );

  template< typename T >
  void write( T const & value ) throw( exWriteError )
  { write( &value, sizeof( value ) ); }

  /// Attempts writing at most 'count' records sized 'size'. Returns
  /// the number of records it managed to write, up to 'count'.
  /// This function does not employ buffering, but flushes the buffer if it
  /// was used before.
  size_t writeRecords( void const * buf, size_t size, size_t count )
    throw( exWriteError );

  /// Reads a string from the file. Unlike the normal fgets(), this one
  /// can strip the trailing newline character, if this was requested.
  /// Returns either s or 0 if no characters were read.
  char * gets( char * s, int size, bool stripNl = false ) throw( exWriteError );

  /// Like the above, but uses its own local internal buffer (1024 bytes
  /// currently), and strips newlines by default.
  std::string gets( bool stripNl = true ) throw( exReadError, exWriteError );

  /// Seeks in the file, relative to its beginning.
  void seek( long offset ) throw( exSeekError, exWriteError );
  /// Seeks in the file, relative to the current position.
  void seekCur( long offset ) throw( exSeekError, exWriteError );
  /// Seeks in the file, relative to the end of file.
  void seekEnd( long offset = 0 ) throw( exSeekError, exWriteError );

  /// Seeks to the beginning of file
  void rewind() throw( exSeekError, exWriteError );

  /// Tells the current position within the file, relative to its beginning.
  size_t tell() throw( exSeekError );

  /// Returns true if end-of-file condition is set.
  bool eof() throw( exWriteError );

  /// Returns the underlying FILE * record, so other operations can be
  /// performed on it.
  FILE * file() throw( exWriteError );

  /// Releases the file handle out of the control of the class. No further
  /// operations are valid. The file will not be closed on destruction.
  FILE * release() throw( exWriteError );

  /// Closes the file. No further operations are valid.
  void close() throw( exWriteError );

  ~Class() throw();

private:

  void flushWriteBuffer() throw( exWriteError );
  void releaseWriteBuffer() throw( exWriteError );
};

}

#endif
