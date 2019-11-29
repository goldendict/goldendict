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
#include "fsencoding.hh"
#include "zipfile.hh"

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

bool tryPossibleZipName( std::string const & name, std::string & copyTo )
{
  if ( ZipFile::SplitZipFile( FsEncoding::decode( name.c_str() ) ).exists() )
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
#if defined(__WIN64) || defined(_MSC_VER)
  struct _stat64 buf;
#else
  struct _stat buf;
#endif
  wchar_t wname[16384];
  MultiByteToWideChar( CP_UTF8, 0, filename, -1, wname, 16384 );
#if defined(__WIN64) || defined(_MSC_VER)
  return _wstat64( wname, &buf ) == 0;
#else
  return _wstat( wname, &buf ) == 0;
#endif
#else
  struct stat buf;

  // EOVERFLOW rationale: if the file is too large, it still does exist
  return stat( filename, &buf ) == 0 || errno == EOVERFLOW;
#endif
}

void Class::open( char const * filename, char const * mode ) THROW_SPEC( exCantOpen )
{
  QFile::OpenMode openMode = QIODevice::Text;
  const char * pch = mode;
  while( *pch )
  {
    switch( *pch )
    {
      case 'r': openMode |= QIODevice::ReadOnly;
                break;
      case 'w': openMode |= QIODevice::WriteOnly;
                break;
      case '+': openMode &= ~( QIODevice::ReadOnly | QIODevice::WriteOnly );
                openMode |= QIODevice::ReadWrite;
                break;
      case 'a': openMode |= QIODevice::Append;
                break;
      case 'b': openMode &= ~QIODevice::Text;
                break;
      default:  break;
    }
    ++pch;
  }

  f.setFileName( FsEncoding::decode( filename ) );

  if ( !f.open( openMode ) )
    throw exCantOpen( std::string( filename ) + ": " + f.errorString().toUtf8().data() );
}

Class::Class( char const * filename, char const * mode ) THROW_SPEC( exCantOpen ):
  writeBuffer( 0 )
{
  open( filename, mode );
}

Class::Class( std::string const & filename, char const * mode )
  THROW_SPEC( exCantOpen ): writeBuffer( 0 )
{
  open( filename.c_str(), mode );
}

void Class::read( void * buf, qint64 size ) THROW_SPEC( exReadError, exWriteError )
{
  if ( !size )
    return;

  if ( writeBuffer )
    flushWriteBuffer();

  qint64 result = f.read( reinterpret_cast<char *>( buf ), size );

  if ( result != size )
    throw exReadError();
}

size_t Class::readRecords( void * buf, qint64 size, size_t count ) THROW_SPEC( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  qint64 result = f.read( reinterpret_cast<char *>( buf ), size * count );
  return result < 0 ? result : result / size;
}

void Class::write( void const * buf, qint64 size ) THROW_SPEC( exWriteError, exAllocation )
{
  if ( !size )
    return;

  if ( size >= WriteBufferSize )
  {
    // If the write is large, there's not much point in buffering
    flushWriteBuffer();

    qint64 result = f.write( reinterpret_cast<char const *>( buf ), size );

    if ( result != size )
      throw exWriteError();

    return;
  }

  if ( !writeBuffer )
  {
    // Allocate the writing buffer since we don't have any yet
    writeBuffer = new char[ WriteBufferSize ];
    if( !writeBuffer )
      throw exAllocation();
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

size_t Class::writeRecords( void const * buf, qint64 size, size_t count )
  THROW_SPEC( exWriteError )
{
  flushWriteBuffer();

  qint64 result = f.write( reinterpret_cast<const char *>( buf ), size * count );
  return result < 0 ? result : result / size;
}

char * Class::gets( char * s, int size, bool stripNl )
  THROW_SPEC( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  qint64 len = f.readLine( s, size );
  char * result = len > 0 ? s : NULL;

  if ( result && stripNl )
  {
    
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

std::string Class::gets( bool stripNl ) THROW_SPEC( exReadError, exWriteError )
{
  char buf[ 1024 ];

  if ( !gets( buf, sizeof( buf ), stripNl ) )
    throw exReadError();

  return std::string( buf );
}

void Class::seek( long offset ) THROW_SPEC( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if ( !f.seek( offset ) )
    throw exSeekError();
}

void Class::seekCur( long offset ) THROW_SPEC( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if( !f.seek( f.pos() + offset ) )
    throw exSeekError();
}

void Class::seekEnd( long offset ) THROW_SPEC( exSeekError, exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  if( !f.seek( f.size() + offset ) )
    throw exSeekError();
}

void Class::rewind() THROW_SPEC( exSeekError, exWriteError )
{
  seek( 0 );
}

size_t Class::tell() THROW_SPEC( exSeekError )
{
  qint64 result = f.pos();

  if ( result == -1 )
    throw exSeekError();

  if ( writeBuffer )
    result += ( WriteBufferSize - writeBufferLeft );

  return ( size_t ) result;
}

bool Class::eof() THROW_SPEC( exWriteError )
{
  if ( writeBuffer )
    flushWriteBuffer();

  return f.atEnd();
}

QFile & Class::file() THROW_SPEC( exWriteError )
{
  flushWriteBuffer();

  return f;
}

void Class::close() THROW_SPEC( exWriteError )
{
  releaseWriteBuffer();
  f.close();
}

Class::~Class() throw()
{
  if ( f.isOpen() )
  {
    try
    {
      releaseWriteBuffer();
    }
    catch( exWriteError & )
    {
    }
    f.close();
  }
}

void Class::flushWriteBuffer() THROW_SPEC( exWriteError )
{
  if ( writeBuffer && writeBufferLeft != WriteBufferSize )
  {
    qint64 result = f.write( writeBuffer, WriteBufferSize - writeBufferLeft );

    if ( result != WriteBufferSize - writeBufferLeft )
      throw exWriteError();

    writeBufferLeft = WriteBufferSize;
  }
}

void Class::releaseWriteBuffer() THROW_SPEC( exWriteError )
{
  flushWriteBuffer();

  if ( writeBuffer )
  {
    delete [] writeBuffer;

    writeBuffer = 0;
  }
}


}
