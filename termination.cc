/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "termination.hh"
#include <exception>
#include <typeinfo>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

#include <string>

#ifndef __WIN32
#include <execinfo.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <QtCore>

using std::string;

static void termHandler()
{
  if( logFile.isOpen() )
    logFile.close();

  std::string message( "GoldenDict has crashed with an unexpected exception\n\n" );

  int status;
  char * function = 0;
  size_t functionLength = 0;

#ifdef _MSC_VER
  std::type_info * ti = 0;
#else
  std::type_info * ti = __cxxabiv1::__cxa_current_exception_type();
#endif

  if ( ti )
  {
    char const * name = ti->name();

#ifdef _MSC_VER
    char * ret = 0;
    // avoid 'unused' warnings
    (void) status;
    (void) functionLength;
#else
    char * ret = abi::__cxa_demangle( name, function, &functionLength, &status );
#endif

    if ( ret )
    {
      function = ret;
      name = function;
    }

    message += "Exception: ";
    message += name;
    message += '\n';

    try
    {
      throw;
    }
    catch( std::exception & e )
    {
      message += "Message: ";
      
      message += e.what();
      
      message += '\n';
    }
    catch( ... )
    {
    }
  }
  else
  {
    message += "terminate() called without active exception\n";
  }

#ifndef __WIN32

  message += "\nBacktrace:\n";

  const size_t maxDepth = 200;
  size_t stackDepth;
  void * stackAddrs[ maxDepth ];
  char ** stackStrings;

  stackDepth = backtrace( stackAddrs, maxDepth );
  stackStrings = backtrace_symbols( stackAddrs, stackDepth );

  for (size_t i = 1; i < stackDepth; i++)
  {
    char * begin = 0, * end = 0;

    for (char *j = stackStrings[i]; *j; ++j)
    {
      if (*j == '(')
        begin = j + 1;
      else if ( begin && ( *j == '+' || *j == ')' ) )
      {
        end = j;
        break;
      }
    }

    string line;

    if ( end )
    {
      char endSymbol = *end;

      *end = 0;

      char * ret = abi::__cxa_demangle( begin, function, &functionLength, &status );

      *end = endSymbol;

      if ( ret )
      {
        function = ret;

        line = string( stackStrings[ i ], begin );
        line += function;
        line += string( end );
      }
    }

    message += "  ";
    message += line.size() ? line.c_str() : stackStrings[ i ];
    message += '\n';
  }

#endif

  QTemporaryFile file;

  if ( file.open() )
  {
    file.setAutoRemove( false );
    file.write( message.data(), message.size() );

    QStringList args;

    args << "--show-error-file";
    args << file.fileName();

    file.close();

    QProcess::execute( QCoreApplication::applicationFilePath(), args );

    abort();
  }
}

void installTerminationHandler()
{
  std::set_terminate( termHandler );
}
