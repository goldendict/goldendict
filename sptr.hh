/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __SPTR_HH_INCLUDED__
#define __SPTR_HH_INCLUDED__

// using std::shared_ptr


template< class T >
class sptr: public std::shared_ptr< T >
{
public:

  sptr() {}

  sptr( T * p ): std::shared_ptr< T >( p ) {}
  
  // TT is meant to be a derivative of T
  template< class TT >
  sptr( sptr< TT > const & other ): std::shared_ptr< T >( other ) {}
};

#endif

