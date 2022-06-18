/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FILE_HH_INCLUDED__
#define __FILE_HH_INCLUDED__

#include <cstdio>
#include <string>
#include <vector>
#include <QFile>
#include "ex.hh"
#include "mutex.hh"

/// A simple wrapper over FILE * operations with added write-buffering,
/// used for non-Qt parts of code.
/// It is possible to ifdef implementation details for some platforms.
namespace File {

DEF_EX( Ex, "File exception", std::exception )
DEF_EX_STR( exCantOpen, "Can't open", Ex )
DEF_EX( exReadError, "Error reading from the file", Ex )
DEF_EX( exWriteError, "Error writing to the file", Ex )
DEF_EX( exSeekError, "File seek error", Ex )
DEF_EX( exAllocation, "Memory allocation error", Ex )

/// Checks if the file exists or not.

bool tryPossibleName( std::string const & name, std::string & copyTo );

bool tryPossibleZipName( std::string const & name, std::string & copyTo );

void loadFromFile( std::string const & n, std::vector< char > & data );

bool exists( char const * filename ) noexcept;

inline bool exists( std::string const & filename ) noexcept
{ return exists( filename.c_str() ); }

class Class
{
  QFile f;
  char * writeBuffer;
  qint64 writeBufferLeft;

  void open( char const * filename, char const * mode ) ;

public:
  QMutex lock;

  Class( char const * filename, char const * mode ) ;

  Class( std::string const & filename, char const * mode ) ;

  /// Reads the number of bytes to the buffer, throws an error if it
  /// failed to fill the whole buffer (short read, i/o error etc).
  void read( void * buf, qint64 size ) ;

  template< typename T >
  void read( T & value ) 
  { read( &value, sizeof( value ) ); }

  template< typename T >
  T read() 
  { T value; read( value ); return value; }

  /// Attempts reading at most 'count' records sized 'size'. Returns
  /// the number of records it managed to read, up to 'count'.
  size_t readRecords( void * buf, qint64 size, size_t count ) ;

  /// Writes the number of bytes from the buffer, throws an error if it
  /// failed to write the whole buffer (short write, i/o error etc).
  /// This function employs write buffering, and as such, writes may not
  /// end up on disk immediately, or a short write may occur later
  /// than it really did. If you don't want write buffering, use
  /// writeRecords() function instead.
  void write( void const * buf, qint64 size ) ;

  template< typename T >
  void write( T const & value ) 
  { write( &value, sizeof( value ) ); }

  /// Attempts writing at most 'count' records sized 'size'. Returns
  /// the number of records it managed to write, up to 'count'.
  /// This function does not employ buffering, but flushes the buffer if it
  /// was used before.
  size_t writeRecords( void const * buf, qint64 size, size_t count )
    ;

  /// Reads a string from the file. Unlike the normal fgets(), this one
  /// can strip the trailing newline character, if this was requested.
  /// Returns either s or 0 if no characters were read.
  char * gets( char * s, int size, bool stripNl = false ) ;

  /// Like the above, but uses its own local internal buffer (1024 bytes
  /// currently), and strips newlines by default.
  std::string gets( bool stripNl = true ) ;

  /// Seeks in the file, relative to its beginning.
  void seek( qint64 offset ) ;
  uchar * map( qint64 offset, qint64 size );
  /// Seeks in the file, relative to the current position.
  void seekCur( qint64 offset ) ;
  /// Seeks in the file, relative to the end of file.
  void seekEnd( qint64 offset = 0 ) ;

  /// Seeks to the beginning of file
  void rewind() ;

  /// Tells the current position within the file, relative to its beginning.
  qint64 tell() ;

  /// Returns true if end-of-file condition is set.
  bool eof() ;

  /// Returns the underlying FILE * record, so other operations can be
  /// performed on it.
  QFile & file() ;

  /// Closes the file. No further operations are valid.
  void close() ;

  ~Class() noexcept;
  bool unmap( uchar * address );

private:

  void flushWriteBuffer() ;
  void releaseWriteBuffer() ;
};

}

#endif
