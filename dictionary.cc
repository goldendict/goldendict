/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <vector>
#include <algorithm>
#include <cstdio>
#include "dictionary.hh"

#include <QCryptographicHash>

// For needToRebuildIndex(), read below
#include <QFileInfo>
#include <QDateTime>

#include "config.hh"
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDateTime>
#include "fsencoding.hh"

namespace Dictionary {

bool Request::isFinished()
{
  return (int)isFinishedFlag;
}

void Request::update()
{
  if ( !isFinishedFlag )
    emit updated();
}

void Request::finish()
{
  if ( !isFinishedFlag )
  {
    isFinishedFlag.ref();

    emit finished();
  }
}

void Request::setErrorString( QString const & str )
{
  Mutex::Lock _( errorStringMutex );

  errorString = str;
}

QString Request::getErrorString()
{
  Mutex::Lock _( errorStringMutex );

  return errorString;
}


///////// WordSearchRequest
  
size_t WordSearchRequest::matchesCount()
{
  Mutex::Lock _( dataMutex );
  
  return matches.size();
}

WordMatch WordSearchRequest::operator [] ( size_t index ) throw( exIndexOutOfRange )
{
  Mutex::Lock _( dataMutex );
  
  if ( index >= matches.size() )
    throw exIndexOutOfRange();
  
  return matches[ index ];
}

vector< WordMatch > & WordSearchRequest::getAllMatches() throw( exRequestUnfinished )
{
  if ( !isFinished() )
    throw exRequestUnfinished();

  return matches;
}

////////////// DataRequest

long DataRequest::dataSize()
{
  Mutex::Lock _( dataMutex );
  
  return hasAnyData ? (long)data.size() : -1;
}

void DataRequest::getDataSlice( size_t offset, size_t size, void * buffer )
  throw( exSliceOutOfRange )
{
  Mutex::Lock _( dataMutex );

  if ( offset + size > data.size() || !hasAnyData )
    throw exSliceOutOfRange();

  memcpy( buffer, &data[ offset ], size );
}

vector< char > & DataRequest::getFullData() throw( exRequestUnfinished )
{
  if ( !isFinished() )
    throw exRequestUnfinished();

  return data;
}

Class::Class( string const & id_, vector< string > const & dictionaryFiles_ ):
  id( id_ ), dictionaryFiles( dictionaryFiles_ )
{
}

void Class::deferredInit()
{
}

sptr< WordSearchRequest > Class::stemmedMatch( wstring const & /*str*/,
                                               unsigned /*minLength*/,
                                               unsigned /*maxSuffixVariation*/,
                                               unsigned long /*maxResults*/ )
  throw( std::exception )
{
  return new WordSearchRequestInstant();
}

sptr< WordSearchRequest > Class::findHeadwordsForSynonym( wstring const & )
  throw( std::exception )
{
  return new WordSearchRequestInstant();
}

vector< wstring > Class::getAlternateWritings( wstring const & )
  throw()
{
  return vector< wstring >();
}

sptr< DataRequest > Class::getResource( string const & /*name*/ )
  throw( std::exception )
{
  return new DataRequestInstant( false );
}

QString const& Class::getDescription()
{
    return dictionaryDescription;
}

string makeDictionaryId( vector< string > const & dictionaryFiles ) throw()
{
  std::vector< string > sortedList;

  if ( Config::isPortableVersion() )
  {
    // For portable version, we use relative paths
    sortedList.reserve( dictionaryFiles.size() );

    QDir dictionariesDir( Config::getPortableVersionDictionaryDir() );

    for( unsigned x = 0; x < dictionaryFiles.size(); ++x )
    {
      string const & full( dictionaryFiles[ x ] );

      QFileInfo fileInfo( FsEncoding::decode( full.c_str() ) );

      if ( fileInfo.isAbsolute() )
        sortedList.push_back( FsEncoding::encode( dictionariesDir.relativeFilePath( fileInfo.filePath() ) ) );
      else
      {
        // Well, it's relative. We don't technically support those, but
        // what the heck
        sortedList.push_back( full );
      }
    }
  }
  else
    sortedList = dictionaryFiles;

  std::sort( sortedList.begin(), sortedList.end() );

  QCryptographicHash hash( QCryptographicHash::Md5 );

  for( std::vector< string >::const_iterator i = sortedList.begin();
       i != sortedList.end(); ++i )
    hash.addData( i->c_str(), i->size() + 1 );

  return hash.result().toHex().data();
}

// While this file is not supposed to have any Qt stuff since it's used by
// the dictionary backends, there's no platform-independent way to get hold
// of a timestamp of the file, so we use here Qt anyway. It is supposed to
// be fixed in the future when it's needed.
bool needToRebuildIndex( vector< string > const & dictionaryFiles,
                         string const & indexFile ) throw()
{
  unsigned long lastModified = 0;

  for( std::vector< string >::const_iterator i = dictionaryFiles.begin();
       i != dictionaryFiles.end(); ++i )
  {
    QFileInfo fileInfo( FsEncoding::decode( i->c_str() ) );

    if ( !fileInfo.exists() )
      return true;

    unsigned long ts = fileInfo.lastModified().toTime_t();

    if ( ts > lastModified )
      lastModified = ts;
  }

  QFileInfo fileInfo( FsEncoding::decode( indexFile.c_str() ) );

  if ( !fileInfo.exists() )
    return true;

  return fileInfo.lastModified().toTime_t() < lastModified;
}

QString generateRandomDictionaryId()
{
  return QString(
    QCryptographicHash::hash(
      QDateTime::currentDateTime().toString( "\"Random\"dd.MM.yyyy hh:mm:ss.zzz" ).toUtf8(),
      QCryptographicHash::Md5 ).toHex() );
}


}
