/* This file is (c) 2013 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QMutex>
#include <QMutexLocker>
#include <QTextCodec>
#include <QString>
#include "gddebug.hh"

#define TO_LOG_MESSAGE( msg, ap ) QString().vsprintf( msg, ap ).toUtf8().constData()

QFile * logFilePtr;

namespace {

class Utf8CodecForLocaleReplacer
{
public:
  static QTextCodec * originalCodecForLocale()
  {
    if( !shouldReplaceCodecForLocale() )
      return QTextCodec::codecForLocale();
    // codecForLocaleMutex is locked while the UTF8 codec replaces the original codec.
    // Thus calling QTextCodec::codecForLocale() while holding a lock is guaranteed to
    // return the original codec, not its temporary UTF8 replacement.
    QMutexLocker _( &codecForLocaleMutex );
    return QTextCodec::codecForLocale();
  }

  Utf8CodecForLocaleReplacer():
    replaceCodecForLocale( shouldReplaceCodecForLocale() ), localeCodec( 0 )
  {
    if( !replaceCodecForLocale )
      return;
    codecForLocaleMutex.lock();
    localeCodec = QTextCodec::codecForLocale();
    // This static local variable caches the result of a possibly slow call to codecForName().
    static QTextCodec * const utf8Codec = QTextCodec::codecForName( "UTF8" );
    QTextCodec::setCodecForLocale( utf8Codec );
  }

  ~Utf8CodecForLocaleReplacer()
  {
    if( !replaceCodecForLocale )
      return;
    QTextCodec::setCodecForLocale( localeCodec );
    codecForLocaleMutex.unlock();
  }

private:
  static bool shouldReplaceCodecForLocale()
  { return logFilePtr && logFilePtr->isOpen(); }

  static QMutex codecForLocaleMutex;

  bool const replaceCodecForLocale;
  QTextCodec * localeCodec;
};

QMutex Utf8CodecForLocaleReplacer::codecForLocaleMutex;

} // unnamed namespace

void gdWarning(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  {
    Utf8CodecForLocaleReplacer codecReplacer;
    qWarning( "%s", TO_LOG_MESSAGE( msg, ap ) );
  }
  va_end(ap);
}

void gdDebug(const char *msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  {
    Utf8CodecForLocaleReplacer codecReplacer;
    qDebug( "%s", TO_LOG_MESSAGE( msg, ap ) );
  }
  va_end(ap);
}

QTextCodec * gdCodecForLocale()
{
  return Utf8CodecForLocaleReplacer::originalCodecForLocale();
}
