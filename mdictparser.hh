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

using std::string;
using std::vector;
using std::pair;
using std::map;

class MdictParser
{
public:

  enum
  {
    kParserVersion = 0x0000009
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

  typedef vector< pair<qint64, qint64> > BlockInfoVector;
  typedef vector< pair<qint64, QString> > HeadWordIndex;
  typedef map<int, pair<QString, QString> > StyleSheets;

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

  bool open();
  void close();
  bool readNextHeadWordIndex( HeadWordIndex & headWordIndex );

  // helpers
  static QString toUtf16( const char * fromCode, const char * from, size_t fromSize );
  static inline QString toUtf16( QString const & fromCode, const char * from, size_t fromSize )
  {
    return toUtf16( fromCode.toLatin1().constData(), from, fromSize );
  }
  static bool parseCompressedBlock( size_t compressedBlockSize, const char * compressedBlockPtr,
                                    size_t decompressedBlockSize, QByteArray & decompressedBlock );

protected:
  MdictParser( char const * filename );

  ~MdictParser() {}

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

class MdxParser: public MdictParser
{
public:
  class ArticleHandler
  {
  public:
    virtual void handleAritcle( QString const & headWord, QString const & article ) = 0;
  };

  MdxParser( const char * filename ): MdictParser( filename ) {}
  ~MdxParser() {}

  bool readRecordBlock( HeadWordIndex & headWordIndex, ArticleHandler & articleHandler );
  static QString & substituteStylesheet( QString & article, StyleSheets const & styleSheets );
  static inline string substituteStylesheet( string const & article, StyleSheets const & styleSheets )
  {
    QString s = QString::fromUtf8( article.c_str() );
    substituteStylesheet( s, styleSheets );
    return string( s.toUtf8().constData() );
  }
};

class MddParser: public MdictParser
{
public:
  class ResourceHandler
  {
  public:
    virtual void handleResource( QString const & fileName, quint32 decompressedBlockSize,
                                 quint32 compressedBlockPos, quint32 compressedBlockSize,
                                 quint32 resourceOffset, quint32 resourceSize ) = 0;
  };

  MddParser( const char * filename ) : MdictParser( filename ) {}
  ~MddParser() {}

  bool readRecordBlock( HeadWordIndex & headWordIndex, ResourceHandler & resourceHandler );

private:

};

#endif // __MDICTPARSER_HH_INCLUDED__
