// https://bitbucket.org/xwang/mdict-analysis
// Octopus MDict Dictionary File (.mdx) and Resource File (.mdd) Analyser
//
// Copyright (C) 2012, 2013 Xiaoqiang Wang <xiaoqiangwang AT gmail DOT com>
// Copyright (C) 2013 Timon Wong <timon86.wang AT gmail DOT com>
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

#ifndef __MDICTPARSER_HH_INCLUDED__
#define __MDICTPARSER_HH_INCLUDED__

#include <string>
#include <vector>
#include <map>
#include <utility>

#include <QPointer>
#include <QFile>

namespace Mdict
{

using std::string;
using std::vector;
using std::pair;
using std::map;

// A helper class to handle memory map for QFile
class ScopedMemMap
{
  QFile & file;
  uchar * address;

public:
  ScopedMemMap( QFile & file, qint64 offset, qint64 size ) :
    file( file ),
    address( file.map( offset, size ) )
  {
  }

  ~ScopedMemMap()
  {
    if ( address )
      file.unmap( address );
  }

  inline uchar * startAddress()
  {
    return address;
  }
};

class MdictParser
{
public:

  enum
  {
    kParserVersion = 0x000000d
  };

  struct RecordIndex
  {
    qint64 startPos;
    qint64 endPos;
    qint64 shadowStartPos;
    qint64 shadowEndPos;
    qint64 compressedSize;
    qint64 decompressedSize;

    inline bool operator==( qint64 rhs ) const
    {
      return ( shadowStartPos <= rhs ) && ( rhs < shadowEndPos );
    }

    inline bool operator<( qint64 rhs ) const
    {
      return shadowEndPos <= rhs;
    }

    inline bool operator>( qint64 rhs ) const
    {
      return shadowStartPos > rhs;
    }

    static size_t bsearch( vector<RecordIndex> const & offsets, qint64 val );
  };

  struct RecordInfo
  {
    qint64 compressedBlockPos;
    qint64 recordOffset;

    qint64 decompressedBlockSize;
    qint64 compressedBlockSize;
    qint64 recordSize;
  };

  class RecordHandler
  {
  public:
    virtual void handleRecord( QString const & name, RecordInfo const & recordInfo ) = 0;
  };

  typedef vector< pair<qint64, qint64> > BlockInfoVector;
  typedef vector< pair<qint64, QString> > HeadWordIndex;
  typedef map<qint32, pair<QString, QString> > StyleSheets;

  inline QString const & title() const
  {
    return title_;
  }

  inline QString const & description() const
  {
    return description_;
  }

  inline StyleSheets const & styleSheets() const
  {
    return styleSheets_;
  }

  inline quint32 wordCount() const
  {
    return wordCount_;
  }

  inline QString const & encoding() const
  {
    return encoding_;
  }

  inline QString const & filename() const
  {
    return filename_;
  }

  inline bool isRightToLeft() const
  {
    return rtl_;
  }

  MdictParser();
  ~MdictParser() {}

  bool open( const char * filename );
  bool readNextHeadWordIndex( HeadWordIndex & headWordIndex );
  bool readRecordBlock( HeadWordIndex & headWordIndex, RecordHandler & recordHandler );

  // helpers
  static QString toUtf16( const char * fromCode, const char * from, size_t fromSize );
  static inline QString toUtf16( QString const & fromCode, const char * from, size_t fromSize )
  {
    return toUtf16( fromCode.toLatin1().constData(), from, fromSize );
  }
  static bool parseCompressedBlock( qint64 compressedBlockSize, const char * compressedBlockPtr,
                                    qint64 decompressedBlockSize, QByteArray & decompressedBlock );

  static QString & substituteStylesheet( QString & article, StyleSheets const & styleSheets );
  static inline string substituteStylesheet( string const & article, StyleSheets const & styleSheets )
  {
    QString s = QString::fromUtf8( article.c_str() );
    substituteStylesheet( s, styleSheets );
    return string( s.toUtf8().constData() );
  }

protected:
  qint64 readNumber( QDataStream & in );
  static quint32 readU8OrU16( QDataStream & in, bool isU16 );
  bool readHeader( QDataStream & in );
  bool readHeadWordBlockInfos( QDataStream & in );
  bool readRecordBlockInfos();
  BlockInfoVector decodeHeadWordBlockInfo( QByteArray const & headWordBlockInfo );
  HeadWordIndex splitHeadWordBlock( QByteArray const & block );

protected:
  QString filename_;
  QPointer<QFile> file_;
  StyleSheets styleSheets_;
  BlockInfoVector headWordBlockInfos_;
  BlockInfoVector::iterator headWordBlockInfosIter_;
  vector<RecordIndex> recordBlockInfos_;

  QString encoding_;
  QString title_;
  QString description_;

  double version_;
  qint64 numHeadWordBlocks_;
  qint64 headWordBlockInfoSize_;
  qint64 headWordBlockSize_;
  qint64 headWordBlockInfoPos_;
  qint64 headWordPos_;
  qint64 totalRecordsSize_;
  qint64 recordPos_;

  quint32 wordCount_;
  int numberTypeSize_;
  bool rtl_;
  bool bruteForce_;
  bool bruteForceEnd_;
};

}

#endif // __MDICTPARSER_HH_INCLUDED__
