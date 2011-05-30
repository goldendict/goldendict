#include "parsecmdline.hh"

QStringList parseCommandLine( QString const & commandLine )
{
  // Parse arguments. Handle quotes correctly.
  QStringList args;
  bool openQuote = false;
  bool possibleDoubleQuote = false;
  bool startNew = true;
  for( QString::const_iterator c = commandLine.begin(),
       e = commandLine.end(); c != e; )
  {
    if ( *c == '"' && !possibleDoubleQuote )
    {
      ++c;
      if ( !openQuote )
      {
        openQuote = true;
        if ( startNew )
        {
          args.push_back( QString() );
          startNew = false;
        }
      }
      else
        possibleDoubleQuote = true;
    }
    else
    if ( possibleDoubleQuote && *c != '"' )
    {
      openQuote = false;
      possibleDoubleQuote = false;
    }
    else
    if ( *c == ' ' && !openQuote )
    {
      ++c;
      startNew = true;
    }
    else
    {
      if ( startNew )
      {
        args.push_back( QString() );
        startNew = false;
      }
      args.last().push_back( *c++ );
      possibleDoubleQuote = false;
    }
  }

  return args;
}
