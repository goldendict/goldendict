/* This file is (c) 2013 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QString>
#include "gddebug.hh"
#include <QDebug>
#if(QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif

QFile * logFilePtr;
static QTextCodec * utf8Codec;

void gdWarning(const char *msg, ...)
{
va_list ap;
va_start(ap, msg);
QTextCodec *localeCodec = 0;

  if( logFilePtr && logFilePtr->isOpen() )
  {
    if( utf8Codec == 0 )
      utf8Codec = QTextCodec::codecForName( "UTF8" );

    localeCodec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale( utf8Codec );
  }

  qWarning() << QString().vasprintf( msg, ap );

  if( logFilePtr && logFilePtr->isOpen() )
  {
    QTextCodec::setCodecForLocale( localeCodec );
  }

  va_end(ap);
}

void gdDebug(const char *msg, ...)
{
va_list ap;
va_start(ap, msg);
// QTextCodec *localeCodec = 0;

  // if( logFilePtr && logFilePtr->isOpen() )
  // {
  //   if( utf8Codec == 0 )
  //     utf8Codec = QTextCodec::codecForName( "UTF8" );

  //   localeCodec = QTextCodec::codecForLocale();
  //   QTextCodec::setCodecForLocale( utf8Codec );
  // }

  qDebug()<< QString().vasprintf( msg, ap );

  // if( logFilePtr && logFilePtr->isOpen() )
  // {
  //   QTextCodec::setCodecForLocale( localeCodec );
  // }

  va_end(ap);
}
