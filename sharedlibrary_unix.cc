#include "sharedlibrary.hh"
#include "dprintf.hh"

#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>

#include <string>
#include <vector>

using std::string;
using std::vector;


struct SharedLibrary::Impl
{
  Impl( const char * filename_, unsigned flags ) :
    filename( filename_ ),
    handle( NULL )
  {
    unsigned loaderFlags = 0;

    if ( flags & LD_LAZY )
      loaderFlags |= RTLD_LAZY;
    if ( flags & LD_NOW )
      loaderFlags |= RTLD_NOW;
    if ( flags & LD_GLOBAL )
      loaderFlags |= RTLD_GLOBAL;

    handle = dlopen( filename_, loaderFlags );

    if ( !handle )
    {
      errString = dlerror();
      FDPRINTF( stderr, "Load libary \"%s\" failed, error: %s\n", filename_,
                errString.c_str() );
    }
  }

  ~Impl()
  {
    if ( handle )
      dlclose( handle );
  }

  inline bool loaded()
  {
    return !!handle;
  }

  inline void * getSymbol( const char * symbol )
  {
    if ( !loaded() || !symbol )
      return NULL;

    void * ret = dlsym( handle, symbol );
    if ( !ret )
    {
      errString = dlerror();
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
  void * handle;
  string errString;
};

SharedLibrary::SharedLibrary() : impl( NULL )
{

}

SharedLibrary::SharedLibrary( const char * filename, unsigned flags ) :
  impl( new Impl( filename, flags ) )
{
}

SharedLibrary::~SharedLibrary()
{
  unload();
}

bool SharedLibrary::load( const char * filename, unsigned flags )
{
  unload();
  impl = new Impl( filename, flags );
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
  return impl->handle;
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
