/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __EX_HH_INCLUDED__
#define __EX_HH_INCLUDED__

#include <string>

/// A way to declare an exception class fast
/// Do like this:
/// DEF_EX( exErrorInFoo, "An error in foo encountered", std::exception )
/// DEF_EX( exFooNotFound, "Foo was not found", exErrorInFoo )

#define DEF_EX( exName, exDescription, exParent ) \
class exName: public exParent { \
public: \
virtual const char * what() const throw() { return (exDescription); } \
virtual ~exName() throw() {} };

/// Same as DEF_EX, but takes a runtime string argument, which gets concatenated
/// with the description.
/// 
///   DEF_EX_STR( exCantOpen, "can't open file", std::exception )
///   ...
///   throw exCantOpen( "example.txt" );
///
///   what() would return "can't open file example.txt"

#define DEF_EX_STR( exName, exDescription, exParent ) \
class exName: public exParent { \
std::string value; \
public: \
  exName( std::string const & value_ ): value( std::string( exDescription ) + " " + value_ ) {} \
virtual const char * what() const throw() { return value.c_str(); } \
virtual ~exName() throw() {} };

#endif
