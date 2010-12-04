// This program generates uninstallation script automatically, using the
// output of the makensis command.
// Why is this not part of NSIS? Ask NSIS developers.

#include <QtCore>

int main( int argc, char *argv[] )
{
  QCoreApplication app( argc, argv );

  QTextStream stream( stdin );

  QRegExp setOutPath( "^SetOutPath: \"(.*)\"$" );
  QRegExp createDirectory( "^CreateDirectory: \"(.*)\"$" );
  QRegExp file( "^File: \"([^\"]*)\"(->\"([^\"]*)\")?.*$" );
  QRegExp createShortCut( "^CreateShortCut: \"([^\"]*)\"(->\"([^\"]*)\")?.*$" );

  QStringList log;

  QString currentOutPath;

  for( QString line; !( line = stream.readLine() ).isNull(); )
  {
    if ( setOutPath.exactMatch( line ) )
    {
      qDebug( "Setting out path to %s", qPrintable( setOutPath.cap( 1 ) ) );

      currentOutPath = setOutPath.cap( 1 );
    }
    else
    if ( createDirectory.exactMatch( line ) )
    {
      qDebug( "Creating directory %s", qPrintable( createDirectory.cap( 1 ) ) );

      log.append( "RMDir \"" + createDirectory.cap( 1 ) + "\"" );
    }
    else
    if ( file.exactMatch( line ) )
    {
      QString command( "Delete \"" );

      if ( file.cap( 3 ).isEmpty() )
      {
        // Using the current out path
        command += currentOutPath + "\\" + file.cap( 1 );
      }
      else
      {
        // Use the complete path available
        command += file.cap( 3 );
      }

      command += "\"";

      log.append( command );

      qDebug( "Writing file %s (%s)", qPrintable( file.cap( 1 ) ),
             qPrintable( file.cap( 3 ) ) );
    }
    else
    if ( createShortCut.exactMatch( line ) )
    {
      qDebug( "Creating shortcut %s", qPrintable( createShortCut.cap( 1 ) ) );

      log.append( "Delete \"" + createShortCut.cap( 1 ) + "\"" );
    }
  }

  // Ok, replay the log back

  for( int x = log.size(); x--; )
  {
    printf( "%s\n", qPrintable( log[ x ] ) );
  }

  return 0;
}
