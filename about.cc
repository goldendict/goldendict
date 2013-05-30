/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "about.hh"
#include <QtGui>

#include "qt4x5.hh"

About::About( QWidget * parent ): QDialog( parent )
{
  ui.setupUi( this );

  QFile versionFile( ":/version.txt" );

  QString version;

  if ( !versionFile.open( QFile::ReadOnly ) )
    version = tr( "[Unknown]" );
  else
    version = QString::fromLatin1( versionFile.readAll() ).trimmed();

#ifdef _M_X64
  version += " (64-bit)";
#endif

  ui.version->setText( version );

  QFile creditsFile( ":/CREDITS.txt" );

  if ( creditsFile.open( QFile::ReadOnly ) )
  {
    QStringList creditsList =
      QString::fromUtf8(
        creditsFile.readAll() ).split( '\n', QString::SkipEmptyParts );

    QString html = "<html><body style='color: black; background: #f4f4f4;'>";

    for( int x = 0; x < creditsList.size(); ++x )
    {
      QString str = creditsList[ x ];

      str.replace( "\\", "@" );

      str = Qt4x5::escape( str );

      int colon = str.indexOf( ":" );

      if ( colon != -1 )
      {
        QString name( str.left( colon ) );

        name.replace( ", ", "<br>" );

        str = "<font color='blue'>" + name + "</font><br>&nbsp;&nbsp;&nbsp;&nbsp;"
              + str.mid( colon + 1 );
      }

      html += str;
      html += "<br>";
    }

    html += "</body></html>";

    ui.credits->setHtml( html );
  }
}
