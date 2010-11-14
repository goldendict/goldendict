/* This file is (c) 2008-2011 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "about.hh"
#include <QtGui>

About::About( QWidget * parent ): QDialog( parent )
{
  ui.setupUi( this );

  QFile versionFile( ":/version.txt" );

  QString version;

  if ( !versionFile.open( QFile::ReadOnly ) )
    version = tr( "[Unknown]" );
  else
    version = QString::fromAscii( versionFile.readAll() ).trimmed();

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

      str = Qt::escape( str );

      int colon = str.indexOf( ":" );

      if ( colon != -1 )
      {
        str.replace( colon, 1, "</font><br>&nbsp;&nbsp;&nbsp;&nbsp;" );
        str.prepend( "<font color='blue'>" );
      }

      html += str;
      html += "<br>";
    }

    html += "</body></html>";

    ui.credits->setHtml( html );
  }
}
