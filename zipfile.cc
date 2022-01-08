/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "zipfile.hh"
#include <QtEndian>
#include <QByteArray>
#include <QFileInfo>

namespace ZipFile {

#pragma pack( push, 1 )

/// End-of-central-directory record, as is
struct EndOfCdirRecord
{
  quint32 signature;
  quint16 numDisk, numDiskCd, totalEntriesDisk, totalEntries;
  quint32 size, offset;
  quint16 commentLength;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct CentralFileHeaderRecord
{
  quint32 signature;
  quint16 verMadeBy, verNeeded, gpBits, compressionMethod, fileTime, fileDate;
  quint32 crc32, compressedSize, uncompressedSize;
  quint16 fileNameLength, extraFieldLength, fileCommentLength, diskNumberStart,
          intFileAttrs;
  quint32 externalFileAttrs, offsetOfLocalHeader;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

struct LocalFileHeaderRecord
{
  quint32 signature;
  quint16 verNeeded, gpBits, compressionMethod, fileTime, fileDate;
  quint32 crc32, compressedSize, uncompressedSize;
  quint16 fileNameLength, extraFieldLength;
}
#ifndef _MSC_VER
__attribute__((packed))
#endif
;

#pragma pack( pop )

static quint32 const endOfCdirRecordSignatureValue = qToLittleEndian( 0x06054b50 );
static quint32 const centralFileHeaderSignature = qToLittleEndian( 0x02014b50 );
static quint32 const localFileHeaderSignature = qToLittleEndian( 0x04034b50 );

static CompressionMethod getCompressionMethod( quint16 compressionMethod )
{
  switch( qFromLittleEndian( compressionMethod ) )
  {
  case 0:
    return Uncompressed;
  case 8:
    return Deflated;
  default:
    return Unsupported;
  }
}

bool positionAtCentralDir( SplitZipFile & zip )
{
  // Find the end-of-central-directory record

  int maxEofBufferSize = 65535 + sizeof( EndOfCdirRecord );

  if ( zip.size() > maxEofBufferSize )
    zip.seek( zip.size() - maxEofBufferSize );
  else
  if ( (size_t) zip.size() < sizeof( EndOfCdirRecord ) )
    return false;
  else
    zip.seek( 0 );

  QByteArray eocBuffer = zip.read( maxEofBufferSize );

  if ( eocBuffer.size() < (int)sizeof( EndOfCdirRecord ) )
    return false;

  int lastIndex = eocBuffer.size() - sizeof( EndOfCdirRecord );

  QByteArray endOfCdirRecordSignature( (char const *)&endOfCdirRecordSignatureValue,
                                       sizeof( endOfCdirRecordSignatureValue ) );

  EndOfCdirRecord endOfCdirRecord;

  quint32 cdir_offset;

  for( ; ; --lastIndex )
  {
    lastIndex = eocBuffer.lastIndexOf( endOfCdirRecordSignature, lastIndex );

    if ( lastIndex == -1 )
      return false;

    /// We need to copy it due to possible alignment issues on ARM etc
    memcpy( &endOfCdirRecord, eocBuffer.data() + lastIndex,
            sizeof( endOfCdirRecord ) );

    /// Sanitize the record by checking the offset

    cdir_offset = zip.calcAbsoluteOffset( qFromLittleEndian( endOfCdirRecord.offset ),
                                          qFromLittleEndian( endOfCdirRecord.numDiskCd ) );

    if ( !zip.seek( cdir_offset ) )
      continue;

    quint32 signature;

    if ( zip.read( (char *)&signature, sizeof( signature ) ) != sizeof( signature ) )
      continue;

    if ( signature == centralFileHeaderSignature )
      break;
  }

  // Found cdir -- position the file on the first header

  return zip.seek( cdir_offset );
}

bool readNextEntry( SplitZipFile & zip, CentralDirEntry & entry )
{
  CentralFileHeaderRecord record;

  if ( zip.read( (char *)&record, sizeof( record ) ) != sizeof( record ) )
    return false;

  if ( record.signature != centralFileHeaderSignature )
    return false;

  // Read file name

  int fileNameLength = qFromLittleEndian( record.fileNameLength );
  entry.fileName = zip.read( fileNameLength );

  if ( entry.fileName.size() != fileNameLength )
    return false;

  // Skip extra fields

  if ( !zip.seek( ( zip.pos() + qFromLittleEndian( record.extraFieldLength ) ) +
                  qFromLittleEndian( record.fileCommentLength ) ) )
    return false;

  entry.localHeaderOffset = zip.calcAbsoluteOffset( qFromLittleEndian( record.offsetOfLocalHeader ),
                                                    qFromLittleEndian( record.diskNumberStart ) );
  entry.compressedSize = qFromLittleEndian( record.compressedSize );
  entry.uncompressedSize = qFromLittleEndian( record.uncompressedSize );
  entry.compressionMethod = getCompressionMethod( record.compressionMethod );
  entry.fileNameInUTF8 = ( qFromLittleEndian( record.gpBits ) & 0x800 ) != 0;

  return true;
}

bool readLocalHeader( SplitZipFile & zip, LocalFileHeader & entry )
{
  LocalFileHeaderRecord record;

  if ( zip.read( (char *)&record, sizeof( record ) ) != sizeof( record ) )
    return false;

  if ( record.signature != localFileHeaderSignature )
    return false;

  // Read file name

  int fileNameLength = qFromLittleEndian( record.fileNameLength );
  entry.fileName = zip.read( fileNameLength );

  if ( entry.fileName.size() != fileNameLength )
    return false;

  // Skip extra field

  if ( !zip.seek( zip.pos() + qFromLittleEndian( record.extraFieldLength ) ) )
    return false;

  entry.compressedSize = qFromLittleEndian( record.compressedSize );
  entry.uncompressedSize = qFromLittleEndian( record.uncompressedSize );
  entry.compressionMethod = getCompressionMethod( record.compressionMethod );

  return true;
}

SplitZipFile::SplitZipFile( const QString & name )
{
  setFileName( name );
}

void SplitZipFile::setFileName( const QString & name )
{
  {
    QString lname = name.toLower();
    if( lname.endsWith( ".zips" ) )
    {
      appendFile( name );
      return;
    }

    if( !lname.endsWith( ".zip" ) )
      return;
  }

  if( QFileInfo( name ).isFile() )
  {
    for( int i = 1; i < 100; i++ )
    {
      QString name2 = name.left( name.size() - 2 ) + QString( "%1" ).arg( i, 2, 10, QChar( '0' ) );
      if( QFileInfo( name2 ).isFile() )
        appendFile( name2 );
      else
        break;
    }
    appendFile( name );
  }
  else
  {
    for( int i = 1; i < 1000; i++ )
    {
      QString name2 = name + QString( ".%1" ).arg( i, 3, 10, QChar( '0' ) );
      if( QFileInfo( name2 ).isFile() )
        appendFile( name2 );
      else
        break;
    }
  }
}

QDateTime SplitZipFile::lastModified() const
{
  unsigned long ts = 0;
  for( QVector< QFile * >::const_iterator i = files.begin(); i != files.end(); ++i )
  {
    unsigned long t = QFileInfo( (*i)->fileName() ).lastModified().toSecsSinceEpoch();
    if( t > ts )
      ts = t;
  }
  return QDateTime::fromSecsSinceEpoch( ts );
}

qint64 SplitZipFile::calcAbsoluteOffset( qint64 offset, quint16 partNo )
{
  if( partNo >= offsets.size() )
    return 0;

  return offsets.at( partNo ) + offset;
}

}
