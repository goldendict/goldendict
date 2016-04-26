// https://bitbucket.org/xwang/mdict-analysis
// https://github.com/zhansliu/writemdict/blob/master/fileformat.md
// Octopus MDict Dictionary File (.mdx) and Resource File (.mdd) Analyser
//
// Copyright (C) 2012, 2013 Xiaoqiang Wang <xiaoqiangwang AT gmail DOT com>
// Copyright (C) 2013 Timon Wong <timon86.wang AT gmail DOT com>
// Copyright (C) 2015 Zhe Wang <0x1998 AT gmail DOT com>
//
// This program is a free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// You can get a copy of GNU General Public License along this program
// But you can always get it from http://www.gnu.org/licenses/gpl.txt
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

#include "mdictparser.hh"

#include <errno.h>
#include <zlib.h>
#include <iconv.h>
#include <lzo/lzo1x.h>

#include <QtEndian>
#include <QStringList>
#include <QByteArray>
#include <QFileInfo>
#include <QRegExp>
#include <QDomDocument>
#include <QTextDocumentFragment>
#include <QDataStream>

#include "decompress.hh"
#include "gddebug.hh"
#include "ripemd.hh"

namespace Mdict
{

enum EncryptedSection
{
  EcryptedHeadWordHeader = 1,
  EcryptedHeadWordIndex = 2
};

static inline int u16StrSize( const ushort * unicode )
{
  int size = 0;
  if ( unicode )
  {
    while ( unicode[size] != 0 )
      size++;
  }
  return size;
}

static QDomNamedNodeMap parseHeaderAttributes( const QString & headerText )
{
  QDomNamedNodeMap attributes;
  QDomDocument doc;
  doc.setContent( headerText );

  QDomElement docElem = doc.documentElement();
  attributes = docElem.attributes();

  for ( int i = 0; i < attributes.count(); i++ )
  {
    QDomAttr attr = attributes.item( i ).toAttr();
  }

  return attributes;
}

size_t MdictParser::RecordIndex::bsearch( const vector<MdictParser::RecordIndex> & offsets, qint64 val )
{
  if ( offsets.size() == 0 )
    return ( size_t ) ( -1 );

  size_t lo = 0;
  size_t hi = offsets.size() - 1;

  while ( lo <= hi )
  {
    size_t mid = ( lo + hi ) >> 1;
    RecordIndex const & p = offsets[mid];
    if ( p == val )
      return mid;
    else if ( p < val )
      lo = mid + 1;
    else
      hi = mid - 1;
  }

  return ( size_t ) ( -1 );
}

MdictParser::MdictParser() :
  version_( 0 ),
  numHeadWordBlocks_( 0 ),
  headWordBlockInfoSize_( 0 ),
  headWordBlockSize_( 0 ),
  headWordBlockInfoPos_( 0 ),
  headWordPos_( 0 ),
  totalRecordsSize_( 0 ),
  recordPos_( 0 ),
  wordCount_( 0 ),
  numberTypeSize_( 0 ),
  encrypted_( 0 ),
  rtl_( false )
{
}

bool MdictParser::open( const char * filename )
{
  filename_ = QString::fromUtf8( filename );
  file_ = new QFile( filename_ );

  GD_DPRINTF( "MdictParser: open %s\n", filename );

  if ( file_.isNull() || !file_->exists() )
    return false;

  if ( !file_->open( QIODevice::ReadOnly ) )
    return false;

  QDataStream in( file_ );
  in.setByteOrder( QDataStream::BigEndian );

  if ( !readHeader( in ) )
    return false;

  if ( !readHeadWordBlockInfos( in ) )
    return false;

  if ( !readRecordBlockInfos() )
    return false;

  return true;
}

bool MdictParser::readNextHeadWordIndex( MdictParser::HeadWordIndex & headWordIndex )
{
  if ( headWordBlockInfosIter_ == headWordBlockInfos_.end() )
    return false;

  qint64 compressedSize = headWordBlockInfosIter_->first;
  qint64 decompressedSize = headWordBlockInfosIter_->second;

  if ( compressedSize < 8 )
    return false;

  ScopedMemMap compressed( *file_, headWordPos_, compressedSize );
  if ( !compressed.startAddress() )
    return false;

  headWordPos_ += compressedSize;
  QByteArray decompressed;
  if ( !parseCompressedBlock( compressedSize, ( char * )compressed.startAddress(),
                              decompressedSize, decompressed ) )
    return false;

  headWordIndex = splitHeadWordBlock( decompressed );
  headWordBlockInfosIter_++;
  return true;
}

bool MdictParser::checkAdler32(const char * buffer, unsigned int len, quint32 checksum)
{
  uLong adler = adler32( 0L, Z_NULL, 0 );
  adler = adler32( adler, ( const Bytef * ) buffer, len );
  return (adler & 0xFFFFFFFF) == checksum;
}

QString MdictParser::toUtf16( const char * fromCode, const char * from, size_t fromSize )
{
  if ( !fromCode || !from )
    return QString();

  iconv_t conv = iconv_open( "UTF-16//IGNORE", fromCode );
  if ( conv == ( iconv_t ) - 1 )
    return QString();

  vector<char> result;
  const static int CHUNK_SIZE = 512;
  char buf[CHUNK_SIZE];
  char ** inBuf = ( char ** )&from;

  while ( fromSize )
  {
    char * outBuf = buf;
    size_t outBytesLeft = CHUNK_SIZE;
    size_t ret = iconv( conv, inBuf, &fromSize, &outBuf, &outBytesLeft );

    if ( ret == ( size_t ) - 1 )
    {
      if ( errno != E2BIG )
      {
        // Real problem
        result.clear();
        break;
      }
    }

    result.insert( result.end(), buf, buf + CHUNK_SIZE - outBytesLeft );
  }

  iconv_close( conv );
  if ( result.size() <= 2 )
    return QString();
  return QString::fromUtf16( ( const ushort * )&result.front() );
}

bool MdictParser::decryptHeadWordIndex(char * buffer, qint64 len)
{
  RIPEMD128 ripemd;
  ripemd.update( ( const uchar * ) buffer + 4, 4 );
  ripemd.update( ( const uchar * ) "\x95\x36\x00\x00", 4 );

  uint8_t key[16];
  ripemd.digest( key );

  buffer += 8;
  len -= 8;
  uint8_t prev = 0x36;
  for (qint64 i = 0; i < len; ++i)
  {
    uint8_t byte = buffer[i];
    byte = (byte >> 4) | (byte << 4);
    byte = byte ^ prev ^ (i & 0xFF) ^ key[i % 16];
    prev = buffer[i];
    buffer[i] = byte;
  }
  return true;
}

bool MdictParser::parseCompressedBlock( qint64 compressedBlockSize,
                                        const char * compressedBlockPtr,
                                        qint64 decompressedBlockSize,
                                        QByteArray & decompressedBlock )
{
  if ( compressedBlockSize <= 8 )
    return false;

  // compression type
  quint32 type = qFromBigEndian<quint32>( ( const uchar * ) compressedBlockPtr );
  quint32 checksum = qFromBigEndian<quint32>( ( const uchar * )compressedBlockPtr + 4 );
  const char * buf = compressedBlockPtr + 8;
  qint64 size = compressedBlockSize - 8;

  switch ( type )
  {
    case 0x00000000:
      // No compression
      if ( !checkAdler32( buf, size, checksum ) )
      {
        gdWarning( "MDict: parseCompressedBlock: plain: checksum not match" );
        return false;
      }

      decompressedBlock = QByteArray( buf, size );
      return true;

    case 0x01000000:
      {
        // LZO compression
        int result;
        lzo_uint blockSize = ( lzo_uint )decompressedBlockSize;
        decompressedBlock.resize( blockSize );
        result = lzo1x_decompress_safe( ( const uchar * ) buf, size,
                                        ( uchar * )decompressedBlock.data(),
                                        &blockSize, NULL );

        if ( result != LZO_E_OK || blockSize != ( lzo_uint )decompressedBlockSize )
        {
          gdWarning( "MDict: parseCompressedBlock: decompression failed" );
          return false;
        }

        if ( checksum != lzo_adler32( lzo_adler32( 0, NULL, 0 ),
                                      ( const uchar * )decompressedBlock.constData(),
                                      blockSize ) )
        {
          gdWarning( "MDict: parseCompressedBlock: lzo: checksum does not match" );
          return false;
        }
      }
      break;

    case 0x02000000:
      // zlib compression
      decompressedBlock = zlibDecompress( buf, size );

      if ( !checkAdler32( decompressedBlock.constData(), decompressedBlock.size(),
                          checksum ) )
      {
        gdWarning( "MDict: parseCompressedBlock: zlib: checksum does not match" );
        return false;
      }
      break;

    default:
      gdWarning( "MDict: parseCompressedBlock: unknown type" );
      return false;
  }

  return true;
}

qint64 MdictParser::readNumber( QDataStream & in )
{
  if ( numberTypeSize_ == 8 )
  {
    qint64 val;
    in >> val;
    return val;
  }
  else
  {
    quint32 val;
    in >> val;
    return val;
  }
}

quint32 MdictParser::readU8OrU16( QDataStream & in, bool isU16 )
{
  if ( isU16 )
  {
    quint16 val;
    in >> val;
    return val;
  }
  else
  {
    quint8 val;
    in >> val;
    return val;
  }
}

bool MdictParser::readHeader( QDataStream & in )
{
  qint32 headerTextSize;
  in >> headerTextSize;

  QByteArray headerTextUtf16 = file_->read( headerTextSize );
  if ( headerTextUtf16.size() != headerTextSize )
    return false;

  QString headerText = toUtf16( "UTF-16LE", headerTextUtf16.constData(), headerTextUtf16.size() );

  // Adler-32 checksum of the header text (little-endian)
  quint32 checksum;
  in.setByteOrder( QDataStream::LittleEndian );
  in >> checksum;
  if ( !checkAdler32( headerTextUtf16.constData(), headerTextUtf16.size(), checksum ) )
  {
    gdWarning( "MDict: readHeader: checksum does not match" );
    return false;
  }
  headerTextUtf16.clear();
  in.setByteOrder( QDataStream::BigEndian );

  QDomNamedNodeMap headerAttributes = parseHeaderAttributes( headerText );

  encoding_ = headerAttributes.namedItem( "Encoding" ).toAttr().value();
  if ( encoding_ == "GBK" || encoding_ == "GB2312" )
  {
    encoding_ = "GB18030";
  }
  else if ( encoding_.isEmpty() || encoding_ == "UTF-16" )
  {
    encoding_ = "UTF-16LE";
  }

  // stylesheet attribute if present takes form of:
  //   styleId # 1-255
  //   style.prefix
  //   style.suffix
  if ( headerAttributes.contains( "StyleSheet" ) )
  {
    QString styleSheets = headerAttributes.namedItem( "StyleSheet" ).toAttr().value();
    QStringList lines = styleSheets.split( QRegExp( "[\r\n]" ), QString::KeepEmptyParts );
    for ( int i = 0; i < lines.size() - 3; i += 3 )
    {
      styleSheets_[lines[i].toInt()] = pair<QString, QString>( lines[i + 1], lines[i + 2] );
    }
  }

  // before version 2.0, number is 4 bytes integer
  // version 2.0 and above uses 8 bytes
  version_ = headerAttributes.namedItem( "GeneratedByEngineVersion" ).toAttr().value().toDouble();
  if ( version_ < 2.0 )
    numberTypeSize_ = 4;
  else
    numberTypeSize_ = 8;

  // Encrypted ?
  encrypted_ = headerAttributes.namedItem("Encrypted").toAttr().value().toInt();

  // Read metadata
  rtl_ = headerAttributes.namedItem( "Left2Right" ).toAttr().value() != "Yes";
  QString title = headerAttributes.namedItem( "Title" ).toAttr().value();
  if ( title.isEmpty() || title.length() < 5 || title == "Title (No HTML code allowed)" )
  {
    // Use filename instead
    QFileInfo fi( filename_ );
    title_ = fi.baseName();
  }
  else
  {
    if ( title.contains( '<' ) || title.contains( '>' ) )
      title_ = QTextDocumentFragment::fromHtml( title ).toPlainText();
    else
      title_ = title;
  }
  QString description = headerAttributes.namedItem( "Description" ).toAttr().value();
  description_ = QTextDocumentFragment::fromHtml( description ).toPlainText();
  return true;
}

bool MdictParser::readHeadWordBlockInfos( QDataStream & in )
{
  QByteArray header = file_->read( version_ >= 2.0 ? ( numberTypeSize_ * 5 )
                                                   : ( numberTypeSize_ * 4 ) );
  QDataStream stream( header );

  // number of headword blocks
  numHeadWordBlocks_ = readNumber( stream );
  // number of entries
  wordCount_ = readNumber( stream );

  // number of bytes of a headword block info after decompression
  qint64 decompressedSize;
  if ( version_ >= 2.0 )
    stream >> decompressedSize;

  // number of bytes of a headword block info before decompression
  headWordBlockInfoSize_ = readNumber( stream );
  // number of bytes of a headword block
  headWordBlockSize_ = readNumber( stream );

  // Adler-32 checksum of the header. If those are encrypted, it is
  // the checksum of the decrypted version
  if ( version_ >= 2.0 )
  {
    quint32 checksum;
    in >> checksum;
    if ( !checkAdler32( header.constData(), numberTypeSize_ * 5, checksum ) )
      return false;
  }

  headWordBlockInfoPos_ = file_->pos();

  // read headword block info
  QByteArray headWordBlockInfo = file_->read( headWordBlockInfoSize_ );
  if ( headWordBlockInfo.size() != headWordBlockInfoSize_ )
    return false;

  if ( version_ >= 2.0 )
  {
    // decrypt
    if ( encrypted_ & EcryptedHeadWordIndex )
    {
      if ( !decryptHeadWordIndex( headWordBlockInfo.data(),
                                  headWordBlockInfo.size() ) )
        return false;
    }

    QByteArray decompressed;
    if ( !parseCompressedBlock( headWordBlockInfo.size(), headWordBlockInfo.data(),
                                decompressedSize, decompressed) )
      return false;

    headWordBlockInfos_ = decodeHeadWordBlockInfo( decompressed );
  }
  else
  {
    headWordBlockInfos_ = decodeHeadWordBlockInfo( headWordBlockInfo );
  }

  headWordPos_ = file_->pos();
  headWordBlockInfosIter_ = headWordBlockInfos_.begin();
  return true;
}

bool MdictParser::readRecordBlockInfos()
{
  file_->seek( headWordBlockInfoPos_ + headWordBlockInfoSize_ +
               headWordBlockSize_ );

  QDataStream in( file_ );
  in.setByteOrder( QDataStream::BigEndian );
  qint64 numRecordBlocks = readNumber( in );
  readNumber( in ); // total number of records, skip
  qint64 recordInfoSize = readNumber( in );
  totalRecordsSize_ = readNumber( in );
  recordPos_ = file_->pos() + recordInfoSize;

  // Build record block index
  recordBlockInfos_.reserve( numRecordBlocks );

  qint64 acc1 = 0;
  qint64 acc2 = 0;
  for ( qint64 i = 0; i < numRecordBlocks; i++ )
  {
    RecordIndex r;
    r.compressedSize = readNumber( in );
    r.decompressedSize = readNumber( in );
    r.startPos = acc1;
    r.endPos = acc1 + r.compressedSize;
    r.shadowStartPos = acc2;
    r.shadowEndPos = acc2 + r.decompressedSize;
    recordBlockInfos_.push_back( r );

    acc1 = r.endPos;
    acc2 = r.shadowEndPos;
  }

  return true;
}

MdictParser::BlockInfoVector MdictParser::decodeHeadWordBlockInfo( QByteArray const & headWordBlockInfo )
{
  BlockInfoVector headWordBlockInfos;

  QDataStream s( headWordBlockInfo );
  s.setByteOrder( QDataStream::BigEndian );

  bool isU16 = false;
  int textTermSize = 0;

  if ( version_ >= 2.0 )
  {
    isU16 = true;
    textTermSize = 1;
  }

  while ( !s.atEnd() )
  {
    // Number of keywords in the block
    s.skipRawData( numberTypeSize_ );
    // Size of the first headword in the block
    quint32 textHeadSize = readU8OrU16( s, isU16 );
    // The first headword
    if ( encoding_ != "UTF-16LE" )
      s.skipRawData( textHeadSize + textTermSize );
    else
      s.skipRawData( ( textHeadSize + textTermSize ) * 2 );
    // Size of the last headword in the block
    quint32 textTailSize = readU8OrU16( s, isU16 );
    // The last headword
    if ( encoding_ != "UTF-16LE" )
      s.skipRawData( textTailSize + textTermSize );
    else
      s.skipRawData( ( textTailSize + textTermSize ) * 2 );

    // headword block compressed size
    qint64 compressedSize = readNumber( s );
    // headword block decompressed size
    qint64 decompressedSize = readNumber( s );
    headWordBlockInfos.push_back( BlockInfoVector::value_type( compressedSize, decompressedSize ) );
  }

  return headWordBlockInfos;
}

MdictParser::HeadWordIndex MdictParser::splitHeadWordBlock( QByteArray const & block )
{
  HeadWordIndex index;

  const char * p = block.constData();
  const char * end = p + block.size();

  while ( p < end )
  {
    qint64 headWordId = ( numberTypeSize_ == 8 ) ?
                        qFromBigEndian<qint64>( ( const uchar * )p ) :
                        qFromBigEndian<quint32>( ( const uchar * )p );
    p += numberTypeSize_;
    QByteArray headWordBuf;

    if ( encoding_ == "UTF-16LE" )
    {
      int headWordLength = u16StrSize( ( const ushort * )p );
      headWordBuf = QByteArray( p, ( headWordLength + 1 ) * 2 );
    }
    else
    {
      int headWordLength = strlen( p );
      headWordBuf = QByteArray( p, headWordLength + 1 );
    }
    p += headWordBuf.size();
    QString headWord = toUtf16( encoding_, headWordBuf.constBegin(), headWordBuf.size() );
    index.push_back( HeadWordIndex::value_type( headWordId, headWord ) );
  }

  return index;
}

bool MdictParser::readRecordBlock( MdictParser::HeadWordIndex & headWordIndex,
                                   MdictParser::RecordHandler & recordHandler )
{
  // cache the index, the headWordIndex is already sorted
  size_t idx = 0;

  for ( HeadWordIndex::const_iterator i = headWordIndex.begin(); i != headWordIndex.end(); i++ )
  {
    if ( recordBlockInfos_[idx].endPos <= i->first )
      idx = RecordIndex::bsearch( recordBlockInfos_, i->first );

    if ( idx == ( size_t )( -1 ) )
      return false;

    RecordIndex const & recordIndex = recordBlockInfos_[idx];
    HeadWordIndex::const_iterator iNext = i + 1;
    qint64 recordSize;
    if ( iNext == headWordIndex.end() )
      recordSize = recordIndex.shadowEndPos - i->first;
    else
      recordSize = iNext->first - i->first;

    RecordInfo recordInfo;
    recordInfo.compressedBlockPos = recordPos_ + recordIndex.startPos;
    recordInfo.recordOffset = i->first - recordIndex.shadowStartPos;
    recordInfo.decompressedBlockSize = recordIndex.decompressedSize;
    recordInfo.compressedBlockSize = recordIndex.compressedSize;
    recordInfo.recordSize = recordSize;

    recordHandler.handleRecord( i->second, recordInfo );
  }

  return true;
}

QString & MdictParser::substituteStylesheet( QString & article, MdictParser::StyleSheets const & styleSheets )
{
  QRegExp rx( "`(\\d+)`" );
  QString endStyle;
  int pos = 0;

  while ( ( pos = rx.indexIn( article, pos ) ) != -1 )
  {
    int styleId = rx.cap( 1 ).toInt();
    StyleSheets::const_iterator iter = styleSheets.find( styleId );

    if ( iter != styleSheets.end() )
    {
      QString rep = endStyle + iter->second.first;
      article.replace( pos, rx.cap( 0 ).length(), rep );
      pos += rep.length();
      endStyle = iter->second.second;
    }
    else
    {
      article.replace( pos, rx.cap( 0 ).length(), endStyle );
      pos += endStyle.length();
      endStyle = "";
    }
  }
  article += endStyle;
  return article;
}

}
