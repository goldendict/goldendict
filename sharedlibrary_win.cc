#include "sharedlibrary.hh"
#include "dprintf.hh"

#include <windows.h>
#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif
#include <stdio.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


class Utf16FromUtf8
{
  vector< wchar_t > buffer;

public:
  Utf16FromUtf8( const char * src )
    : buffer( 1, 0 )
  {
    int srcSize = strlen( src );
    uint32_t size = MultiByteToWideChar( CP_UTF8, 0, src, srcSize, 0, 0 ) + 1;

    if ( size == 0 )
      return;

    buffer.resize( size );
    MultiByteToWideChar( CP_UTF8, 0, src, srcSize, &buffer.front(), size );
    buffer[size - 1] = 0;
  }

  inline operator const wchar_t * () const
  {
    return ptr();
  }

  inline const wchar_t * ptr() const
  {
    return &buffer.front();
  }
};

class Utf8FromUtf16
{
  vector< char > buffer;

public:
  Utf8FromUtf16( const wchar_t * src )
    : buffer( 1, 0 )
  {
    int srcSize = wcslen( src );
    uint32_t size = WideCharToMultiByte( CP_UTF8, 0, src, srcSize, 0, 0 , NULL, NULL ) + 1;

    if ( size == 0 )
      return;

    buffer.resize( size );
    WideCharToMultiByte( CP_UTF8, 0, src, srcSize, &buffer.front(), size, NULL, NULL );
    buffer[size - 1] = 0;
  }

  inline operator const char * () const
  {
    return ptr();
  }

  inline const char * ptr() const
  {
    return &buffer.front();
  }
};

static string getLastWinErrorString()
{
  const int bufferSize = 256;
  wchar_t buffer[256];
  uint32_t lastError = GetLastError();
  if ( lastError == 0 )
    return "";

  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, lastError,
                 MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                 buffer, bufferSize - 1, NULL );
  return string( Utf8FromUtf16( buffer ) );
}

struct SharedLibrary::Impl
{
  Impl( const char * filename_ ) :
    filename( filename_ ),
    module( NULL )
  {
    module = LoadLibraryW( Utf16FromUtf8( filename_ ) );
    if ( !module )
    {
      errString = getLastWinErrorString();
      FDPRINTF( stderr, "Load libary \"%s\" failed, error: %s\n", filename_,
                errString.c_str() );
    }
  }

  ~Impl()
  {
    if ( module )
      FreeLibrary( module );
  }

  inline bool loaded()
  {
    return !!module;
  }

  inline void * getSymbol( const char * symbol )
  {
    if ( !loaded() || !symbol )
      return NULL;

    void * ret = ( void * )GetProcAddress( module, symbol );
    if ( !ret )
    {
      errString = getLastWinErrorString();
      FDPRINTF( stderr, "Get symbol \"%s\" from libary \"%s\" failed, error: %s\n", symbol,
                filename.c_str(), errString.c_str() );
    }
    return ret;
  }

  inline const char * errorString()
  {
    return errString.c_str();
  }

  string filename;
  HMODULE module;
  string errString;
};

SharedLibrary::SharedLibrary() : impl( NULL )
{

}

SharedLibrary::SharedLibrary( const char * filename, unsigned /*flags*/ ) : impl( new Impl( filename ) )
{
}

SharedLibrary::~SharedLibrary()
{
  unload();
}

bool SharedLibrary::load( const char * filename, unsigned /*flags*/ )
{
  unload();
  impl = new Impl( filename );
  return loaded();
}

void SharedLibrary::unload()
{
  if ( !impl )
    return;
  delete impl;
  impl = NULL;
}

bool SharedLibrary::loaded() const
{
  if ( !impl )
    return false;
  return impl->loaded();
}

void * SharedLibrary::handle() const
{
  return static_cast< void * >( impl->module );
}

void * SharedLibrary::getSymbol( const char * symbol ) const
{
  if ( !impl )
    return NULL;
  return impl->getSymbol( symbol );
}

const char * SharedLibrary::errorString() const
{
  if ( !impl )
    return "";
  return impl->errorString();
}
