/* This file is (c) 2013 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QString>
#include "gddebug.hh"
#include <QDebug>
#if( QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 ) )
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif

QFile * logFilePtr;

void gdWarning( const char * msg, ... )
{
  va_list ap;
  va_start( ap, msg );

  qWarning() << QString().vasprintf( msg, ap );

  va_end( ap );
}

void gdDebug( const char * msg, ... )
{
  va_list ap;
  va_start( ap, msg );

  qDebug().noquote() << QString().vasprintf( msg, ap );

  va_end( ap );
}
