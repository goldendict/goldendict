/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <vector>
#include <algorithm>
#include <cstdio>
#include "dictionary.hh"
#include "md5.h"

// For needToRebuildIndex(), read below
#include <QFileInfo>
#include <QDateTime>

namespace Dictionary {

Class::Class( string const & id_, vector< string > const & dictionaryFiles_ ):
  id( id_ ), dictionaryFiles( dictionaryFiles_ )
{
}

string Format::makeDictionaryId( vector< string > const & dictionaryFiles ) throw()
{
  std::vector< string > sortedList( dictionaryFiles );

  std::sort( sortedList.begin(), sortedList.end() );

  md5_state_t context;

  md5_init( &context );
  for( std::vector< string >::const_iterator i = sortedList.begin();
       i != sortedList.end(); ++i )
    md5_append( &context, (unsigned char const *)i->c_str(), i->size() + 1 );

  unsigned char digest[ 16 ];

  md5_finish( &context, digest );

  char result[ sizeof( digest ) * 2 + 1 ];

  for( unsigned x = 0; x < sizeof( digest ); ++x )
    sprintf( result + x * 2, "%02x", digest[ x ] );

  return result;
}

// While this file is not supposed to have any Qt stuff since it's used by
// the dictionary backends, there's no platform-independent way to get hold
// of a timestamp of the file, so we use here Qt anyway. It is supposed to
// be fixed in the future when it's needed.
bool Format::needToRebuildIndex( vector< string > const & dictionaryFiles,
                                 string const & indexFile ) throw()
{
  unsigned long lastModified = 0;

  for( std::vector< string >::const_iterator i = dictionaryFiles.begin();
       i != dictionaryFiles.end(); ++i )
  {
    QFileInfo fileInfo( QString::fromLocal8Bit( i->c_str() ) );

    if ( !fileInfo.exists() )
      return true;

    unsigned long ts = fileInfo.lastModified().toTime_t();

    if ( ts > lastModified )
      lastModified = ts;
  }

  QFileInfo fileInfo( QString::fromLocal8Bit( indexFile.c_str() ) );

  if ( !fileInfo.exists() )
    return true;

  return fileInfo.lastModified().toTime_t() < lastModified;
}

}
