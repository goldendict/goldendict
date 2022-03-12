/* This file is (c) 2017 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "splitfile.hh"
#include "fsencoding.hh"

namespace SplitFile
{

SplitFile::SplitFile() :
  currentFile( 0 )
{
}

SplitFile::~SplitFile()
{
  close();
}

void SplitFile::appendFile( const QString & name )
{
  if( offsets.isEmpty() )
    offsets.append( 0 );
  else
    offsets.append( offsets.last() + files.last()->size() );
  files.append( new QFile( name ) );
}

void SplitFile::close()
{
  for( QVector< QFile * >::const_iterator i = files.begin(); i != files.end(); ++i )
  {
    (*i)->close();
    delete (*i);
  }

  files.clear();
  offsets.clear();

  currentFile = 0;
}

void SplitFile::getFilenames( vector< string > &names ) const
{
  for( QVector< QFile * >::const_iterator i = files.begin(); i != files.end(); ++i )
    names.push_back( FsEncoding::encode( (*i)->fileName() ) );
}

bool SplitFile::open( QFile::OpenMode mode )
{
  for( QVector< QFile * >::iterator i = files.begin(); i != files.end(); ++i )
    if( !(*i)->open( mode ) )
    {
      close();
      return false;
    }

  return true;
}

bool SplitFile::seek( quint64 pos )
{
  if( offsets.isEmpty() )
    return false;

  int fileNom;

  for( fileNom = 0; fileNom < offsets.size() - 1; fileNom++ )
    if( pos < offsets.at( fileNom + 1 ) )
      break;

  pos -= offsets.at( fileNom );

  currentFile = fileNom;
  return files.at( fileNom )->seek( pos );
}

qint64 SplitFile::read( char *data, qint64 maxSize )
{
  if( offsets.isEmpty() )
    return 0;

  quint64 bytesReaded = 0;
  for( int i = currentFile; i < files.size(); i++ )
  {
    if( i != currentFile )
    {
      files.at( i )->seek( 0 );
      currentFile = i;
    }

    qint64 ret = files.at( i )->read( data + bytesReaded, maxSize );
    if( ret < 0 )
      break;

    bytesReaded += ret;
    maxSize -= ret;

    if( maxSize <= 0 )
      break;
  }
  return bytesReaded;
}

QByteArray SplitFile::read( qint64 maxSize )
{
  QByteArray data;
  data.resize( maxSize );

  qint64 ret = read( data.data(), maxSize );

  if( ret != maxSize )
    data.resize( ret );

  return data;
}

bool SplitFile::getChar( char *c )
{
  char ch;
  return read( c ? c : &ch, 1 ) == 1;
}

qint64 SplitFile::pos() const
{
  if( offsets.isEmpty() )
    return 0;

  return offsets.at( currentFile ) + files.at( currentFile )->pos();
}

} // namespace SplitFile
