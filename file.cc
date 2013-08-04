/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "file.hh"

#include <cstring>
#include <cerrno>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifdef __WIN32
#include <windows.h>
#endif

#include "ufile.hh"

namespace File {

enum
{
  // We employ a writing buffer to considerably speed up file operations when
  // they consists of many small writes. The default size for the buffer is 64k
  WriteBufferSize = 65536
};

bool tryPossibleName( std::string const & name, std::string & copyTo )
{
  if ( File::exists( name ) )
  {
    copyTo = name;
    return true;
  }
  else
    return false;
}

void loadFromFile( std::string const & n, std::vector< char > & data )
{
  File::Class f( n, "rb" );

  f.seekEnd();

  data.resize( f.tell() );

  f.rewind();

  f.read( &data.front(), data.size() );
}

bool exists( char const * filename ) throw()
{
#ifdef __WIN32
  struct _stat buf;
  wchar_t wname[16384];
  MultiByteToWideChar( CP_UTF8, 0, filename, -1, wname, 16384 );
  return _wstat( wname, &buf ) == 0;
#else
  struct stat buf;

  // EOVERFLOW rationale: if the file is too large, it still does exist
  return stat( filename, &buf ) == 0 || errno == EOVERFLOW;
#endif
}

void Class::open( char const * filename, char const * mode ) throw( exCantOpen )
{
  f = gd_fopen( filename, mode );

  if ( !f )
    throw exCantOpen( std::string( filename ) + ": " + strerror( errno ) );
}

Class::Class( char const * filename, char const * mode ) throw( exCantOpen ):
  writeBuffer( 0 )
{
  open( filename, mode );
}

Class::Class( std::string const & filename, char const * mode )
  throw( exCantOpen ): writeBuffer( 0 )
{
  open( filename.c_str(), mode );
}

void Class::read( void * buf, size_t size ) throw( exReadError, exWriteError )
{
  if ( !size )
    return;

  if ( writeBuffer )
    flushWriteBuffer();

  size_t result = fread( buf, size, 1, f );

  if ( result != 1 )
    throw exReadError();
}

size_t Class::readRecords( void * buf, size_t size, size_t count ) throw( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  return fread( buf, size, count, f );
}

void Class::write( void const * buf, size_t size ) throw( exWriteError )
{
  if ( !size )
    return;

  if ( size >= WriteBufferSize )
  {
    // If the write is large, there's not much point in buffering
    flushWriteBuffer();

    size_t result = fwrite( buf, size, 1, f );

    if ( result != 1 )
      throw exWriteError();

    return;
  }

  if ( !writeBuffer )
  {
    // Allocate the writing buffer since we don't have any yet
    writeBuffer = new char[ WriteBufferSize ];
    writeBufferLeft = WriteBufferSize;
  }

  size_t toAdd = size < writeBufferLeft ? size : writeBufferLeft;

  memcpy( writeBuffer + ( WriteBufferSize - writeBufferLeft ),
          buf, toAdd );

  size -= toAdd;
  writeBufferLeft -= toAdd;

  if ( !writeBufferLeft ) // Out of buffer? Flush it.
  {
    flushWriteBuffer();

    if ( size ) // Something's still left? Add to buffer.
    {
      memcpy( writeBuffer, (char const *)buf + toAdd, size );
      writeBufferLeft -= size;
    }
  }
}

size_t Class::writeRecords( void const * buf, size_t size, size_t count )
  throw( exWriteError )
{
  flushWriteBuffer();

  return fwrite( buf, size, count, f );
}

char * Class::gets( char * s, int size, bool stripNl )
  throw( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  char * result = fgets( s, size, f );

  if ( result && stripNl )
  {
    size_t len = strlen( result );
    
    char * last = result + len;

    while( len-- )
    {
      --last;

      if ( *last == '\n' || *last == '\r' )
        *last = 0;
      else
        break;
    }
  }

  return result;
}

std::string Class::gets( bool stripNl ) throw( exReadError, exWriteError )
{
  char buf[ 1024 ];

  if ( !gets( buf, sizeof( buf ), stripNl ) )
    throw exReadError();

  return std::string( buf );
}

void Class::seek( long offset ) throw( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if ( fseek( f, offset, SEEK_SET ) != 0 )
    throw exSeekError();
}

void Class::seekCur( long offset ) throw( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if ( fseek( f, offset, SEEK_CUR ) != 0 )
    throw exSeekError();
}

void Class::seekEnd( long offset ) throw( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if ( fseek( f, offset, SEEK_END ) != 0 )
    throw exSeekError();
}

void Class::rewind() throw( exSeekError, exWriteError )
{
  seek( 0 );
}

size_t Class::tell() throw( exSeekError )
{
  long result = ftell( f );

  if ( result == -1 )
    throw exSeekError();

  if ( writeBuffer )
    result += ( WriteBufferSize - writeBufferLeft );

  return ( size_t ) result;
}

bool Class::eof() throw( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  return feof( f ) != 0;
}

FILE * Class::file() throw( exWriteError )
{
  flushWriteBuffer();

  return f;
}

FILE * Class::release() throw( exWriteError )
{
  releaseWriteBuffer();

  FILE * c = f;

  f = 0;

  return c;
}

void Class::close() throw( exWriteError )
{
  fclose( release() );
}

Class::~Class() throw()
{
  if ( f )
  {
    try
    {
      releaseWriteBuffer();
    }
    catch( exWriteError & )
    {
    }
    fclose( f );
  }
}

void Class::flushWriteBuffer() throw( exWriteError )
{
  if ( writeBuffer && writeBufferLeft != WriteBufferSize )
  {
    size_t result = fwrite( writeBuffer, WriteBufferSize - writeBufferLeft, 1, f );

    if ( result != 1 )
      throw exWriteError();

    writeBufferLeft = WriteBufferSize;
  }
}

void Class::releaseWriteBuffer() throw( exWriteError )
{
  flushWriteBuffer();

  if ( writeBuffer )
  {
    delete [] writeBuffer;

    writeBuffer = 0;
  }
}


}
