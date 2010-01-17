/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "termination.hh"
#include <exception>
#include <typeinfo>
#include <cxxabi.h>
#include <string>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <QtCore>

using std::string;

static void termHandler()
{
  std::string message( "GoldenDict has crashed with an unexpected exception\n\n" );

  int status;
  char * function = 0;
  size_t functionLength = 0;

  std::type_info * ti = __cxxabiv1::__cxa_current_exception_type();

  if ( ti )
  {
    char const * name = ti->name();

    char * ret = abi::__cxa_demangle( name, function, &functionLength, &status );

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
