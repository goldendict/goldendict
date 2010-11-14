/* This file is (c) 2008-2011 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SPTR_HH_INCLUDED__
#define __SPTR_HH_INCLUDED__

// A generic non-intrusive smart-pointer template. We could use boost::, tr1::
// or whatever, but since there's no standard solution yet, it isn't worth
// the dependency given the simplicity of the template.

template< class T >
class sptr_base
{
  template< class TT > friend class sptr_base;

  T * p;
  unsigned * count;


  void increment()
  {
    if ( count )
      ++*count;
  }

public:

  sptr_base(): p( 0 ), count( 0 ) {}

  sptr_base( T * p_ ): p( p_ ), count( p ? new unsigned( 1 ) : 0 )
  {
  }

  sptr_base( sptr_base< T > const & other ): p( other.p ), count( other.count )
  { increment(); }

  // TT is meant to be a derivative of T
  template< class TT >
  sptr_base( sptr_base< TT > const & other ): p( ( T * ) other.p ),
    count( other.count )
  { increment(); }

  void reset()
  {
    if ( count )
    {
      if ( ! -- *count )
      {
        delete count;

        count = 0;

        if ( p )
        {
          T * p_ = p;
  
          p = 0;
  
          delete p_;
        }
      }
      else
      {
        p = 0;
        count = 0;
      }
    }
  }

  unsigned use_count() const
  { return count; }

  sptr_base & operator = ( sptr_base const & other )
  { if ( &other != this ) { reset(); p = other.p; count = other.count; increment(); }
    return * this; }

  bool operator ! ( void ) const
  { return !p; }

  bool operator == ( sptr_base const & other ) const
  { return p == other.p; }

  bool operator != ( sptr_base const & other ) const
  { return p != other.p; }

  ~sptr_base()
  { reset(); }

protected:

  T * get_base( void ) const
  { return p; }
};

template< class T >
class sptr: public sptr_base< T >
{
public:

  sptr() {}

  sptr( T * p ): sptr_base< T >( p ) {}
  
  // TT is meant to be a derivative of T
  template< class TT >
  sptr( sptr< TT > const & other ): sptr_base< T >( other ) {}
    
  // Retrieval

  T * get( void ) const
    { return sptr_base< T > :: get_base(); }

  T * operator -> ( void ) const
    { return get(); }

  T & operator * ( void ) const
    { return * get(); }

  // Check

  operator bool( void ) const
    { return get();  }

  bool operator ! ( void ) const
    { return !get(); }
};

template< class T >
class const_sptr: public sptr_base< T >
{
public:

  const_sptr() {}

  const_sptr( T * p_ ): sptr_base< T >( p_ ) {}
  
  const_sptr( sptr< T > const & other ): sptr_base< T >( other ) {}
  
  // TT is meant to be a derivative of T
  template< class TT >
  const_sptr( sptr_base< TT > const & other ): sptr_base< T >( other ) {}
    
  // Retrieval

  T const * get( void ) const
    { return sptr_base< T > :: get_base(); }

  T const * operator -> ( void ) const
    { return get(); }

  T const & operator * ( void ) const
    { return * get(); }
};


#endif

