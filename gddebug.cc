/* This file is (c) 2013 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QMutex>
#include <QMutexLocker>
#include <QTextCodec>
#include <QString>
#include "gddebug.hh"

QFile * logFilePtr;
static QTextCodec * utf8Codec;
static QMutex loggingMutex;

void gdWarning(const char *msg, ...)
{
va_list ap;
va_start(ap, msg);
QTextCodec *localeCodec = 0;

  if( logFilePtr && logFilePtr->isOpen() )
  {
    QMutexLocker _( &loggingMutex );

    if( utf8Codec == 0 )
      utf8Codec = QTextCodec::codecForName( "UTF8" );

    localeCodec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale( utf8Codec );

    qWarning( "%s", QString().vsprintf( msg, ap ).toUtf8().constData() );

    QTextCodec::setCodecForLocale( localeCodec );
  }
  else
    qWarning( "%s", QString().vsprintf( msg, ap ).toUtf8().constData() );

  va_end(ap);
}

void gdDebug(const char *msg, ...)
{
va_list ap;
va_start(ap, msg);
QTextCodec *localeCodec = 0;

  if( logFilePtr && logFilePtr->isOpen() )
  {
    QMutexLocker _( &loggingMutex );

    if( utf8Codec == 0 )
      utf8Codec = QTextCodec::codecForName( "UTF8" );

    localeCodec = QTextCodec::codecForLocale();
    QTextCodec::setCodecForLocale( utf8Codec );

    qDebug( "%s", QString().vsprintf( msg, ap ).toUtf8().constData() );

    QTextCodec::setCodecForLocale( localeCodec );
  }
  else
    qDebug( "%s", QString().vsprintf( msg, ap ).toUtf8().constData() );

  va_end(ap);
}
