#ifndef __SHAREDLIBRARY_HH_INCLUDED__
#define __SHLIBLOADER_HH_INCLUDED__

class SharedLibrary
{
public:

  enum Flags
  {
    LD_LAZY       = 1,
    LD_NOW        = 2,
    LD_LOCAL      = 0,
    LD_GLOBAL     = 0x100,

    LD_DEFAULTS   = LD_NOW | LD_GLOBAL
  };

  SharedLibrary();
  SharedLibrary( const char * filename, unsigned flags = LD_DEFAULTS );
  ~SharedLibrary();

  // load shared library.
  //  filename: name of the shared library, utf8 encoded.
  //  flags:
  bool load( const char * filename, unsigned flags = LD_DEFAULTS );
  void unload();
  bool loaded() const;
  void * handle() const;
  void * getSymbol( const char * symbol ) const;
  const char * errorString() const;

private:
  struct Impl;
  Impl * impl;
};

#endif // __SHAREDLIBRARY_HH_INCLUDED__
