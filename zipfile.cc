/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "zipfile.hh"
#include <QtEndian>
#include <QByteArray>

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

bool positionAtCentralDir( QFile & zip )
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

  for( ; ; --lastIndex )
  {
    lastIndex = eocBuffer.lastIndexOf( endOfCdirRecordSignature, lastIndex );

    if ( lastIndex == -1 )
      return false;

    /// We need to copy it due to possible alignment issues on ARM etc
    memcpy( &endOfCdirRecord, eocBuffer.data() + lastIndex,
            sizeof( endOfCdirRecord ) );

    /// Sanitize the record by checking the offset

    if ( !zip.seek( qFromLittleEndian( endOfCdirRecord.offset ) ) )
      continue;

    quint32 signature;

    if ( zip.read( (char *)&signature, sizeof( signature ) ) != sizeof( signature ) )
      continue;

    if ( signature == centralFileHeaderSignature )
      break;
  }

  // Found cdir -- position the file on the first header

  return zip.seek( qFromLittleEndian( endOfCdirRecord.offset ) );
}

bool readNextEntry( QFile & zip, CentralDirEntry & entry )
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

  entry.localHeaderOffset = qFromLittleEndian( record.offsetOfLocalHeader );
  entry.compressedSize = qFromLittleEndian( record.compressedSize );
  entry.uncompressedSize = qFromLittleEndian( record.uncompressedSize );
  entry.compressionMethod = getCompressionMethod( record.compressionMethod );

  return true;
}

bool readLocalHeader( QFile & zip, LocalFileHeader & entry )
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

}
