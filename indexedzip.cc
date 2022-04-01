/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "indexedzip.hh"
#include "zipfile.hh"
#include <zlib.h>
#include "gddebug.hh"
#include "utf8.hh"
#include "iconv.hh"
#include "wstring_qt.hh"
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
#include <QtCore5Compat/QTextCodec>
#else
#include <QTextCodec>
#endif
#include <QDebug>

using namespace BtreeIndexing;
using std::vector;

bool IndexedZip::openZipFile( QString const & name )
{
  zip.setFileName( name );

  zipIsOpen = zip.open( QFile::ReadOnly );

  return zipIsOpen;
}

bool IndexedZip::hasFile( gd::wstring const & name )
{
  if ( !zipIsOpen )
    return false;

  vector< WordArticleLink > links = findArticles( name );

  return !links.empty();
}

bool IndexedZip::loadFile( gd::wstring const & name, vector< char > & data )
{
  if ( !zipIsOpen )
    return false;

  vector< WordArticleLink > links = findArticles( name );

  if ( links.empty() )
    return false;

  return loadFile( links[ 0 ].articleOffset, data );
}

bool IndexedZip::loadFile( uint32_t offset, vector< char > & data )
{
  if ( !zipIsOpen )
    return false;

  // Now seek into the zip file and read its header

  if ( !zip.seek( offset ) )
    return false;

  ZipFile::LocalFileHeader header;

  if ( !ZipFile::readLocalHeader( zip, header ) )
  {
    GD_DPRINTF( "Failed to load header\n" );
    return false;
  }

  // Which algorithm was used?

  switch( header.compressionMethod )
  {
    case ZipFile::Uncompressed:
      GD_DPRINTF( "Uncompressed\n" );
      data.resize( header.uncompressedSize );
      return (size_t) zip.read( &data.front(), data.size() ) == data.size();

    case ZipFile::Deflated:
    {
      GD_DPRINTF( "Deflated\n" );

      // Now do the deflation

      QByteArray compressedData = zip.read( header.compressedSize );

      if ( compressedData.size() != (int)header.compressedSize )
        return false;

      data.resize( header.uncompressedSize );

      z_stream stream;

      memset( &stream, 0, sizeof( stream ) );

      stream.next_in = ( Bytef * ) compressedData.data();
      stream.avail_in = compressedData.size();
      stream.next_out = ( Bytef * ) &data.front();
      stream.avail_out = data.size();

      if ( inflateInit2( &stream, -MAX_WBITS ) != Z_OK )
      {
        data.clear();        
        return false;
      }

      if ( inflate( &stream, Z_FINISH ) != Z_STREAM_END )
      {
        GD_DPRINTF( "Not zstream end!" );

        data.clear();

        inflateEnd( &stream );

        return false;
      }

      inflateEnd( &stream );

      return true;
    }

    default:

      return false;
  }
}

bool IndexedZip::indexFile( BtreeIndexing::IndexedWords &zipFileNames, quint32 * filesCount )
{
    if ( !zipIsOpen )
        return false;
    if ( !ZipFile::positionAtCentralDir( zip ) )
        return false;

    // File seems to be a valid zip file


    QTextCodec * localeCodec = QTextCodec::codecForLocale();

    ZipFile::CentralDirEntry entry;

    bool alreadyCounted;
    if( filesCount )
      *filesCount = 0;

    while( ZipFile::readNextEntry( zip, entry ) )
    {
      if ( entry.compressionMethod == ZipFile::Unsupported )
      {
        qWarning( "Zip warning: compression method unsupported -- skipping file \"%s\"\n",
                  entry.fileName.data() );
        continue;
      }

      // Check if the file name has some non-ascii letters.

      unsigned char const * ptr = ( unsigned char const * )
                                    entry.fileName.constData();

      bool hasNonAscii = false;

      for( ; ; )
      {
        if ( *ptr & 0x80 )
        {
          hasNonAscii = true;
          break;
        }
        else
        if ( !*ptr++ )
          break;
      }

      alreadyCounted = false;

      if ( !hasNonAscii )
      {
        // Add entry as is

        zipFileNames.addSingleWord( Utf8::decode( entry.fileName.data() ),
                                    entry.localHeaderOffset );
        if( filesCount )
          *filesCount += 1;
      }
      else
      {
        // Try assuming different encodings. Those are UTF8, system locale and two
        // Russian ones (Windows and Windows OEM). Unfortunately, zip
        // files do not say which encoding they utilize.

        // Utf8
        try
        {
          wstring decoded = Utf8::decode( entry.fileName.constData() );

          zipFileNames.addSingleWord( decoded,
                                      entry.localHeaderOffset );
          if( filesCount != 0 && !alreadyCounted )
          {
            *filesCount += 1;
            alreadyCounted = true;
          }
        }
        catch( Utf8::exCantDecode & )
        {
          // Failed to decode
        }

        if( !entry.fileNameInUTF8 )
        {
          wstring nameInSystemLocale;

          // System locale
          if( localeCodec )
          {
            QString name = localeCodec->toUnicode( entry.fileName.constData(),
                                                   entry.fileName.size() );
            nameInSystemLocale = gd::toWString( name );
            if( !nameInSystemLocale.empty() )
            {
              zipFileNames.addSingleWord( nameInSystemLocale,
                                          entry.localHeaderOffset );

              if( filesCount != 0 && !alreadyCounted )
              {
                *filesCount += 1;
                alreadyCounted = true;
              }
            }
          }


          // CP866
          try
          {
            wstring decoded = Iconv::toWstring( "CP866", entry.fileName.constData(),
                                                entry.fileName.size() );

            if( nameInSystemLocale.compare( decoded ) != 0 )
            {
              zipFileNames.addSingleWord( decoded,
                                          entry.localHeaderOffset );

              if( filesCount != 0 && !alreadyCounted )
              {
                *filesCount += 1;
                alreadyCounted = true;
              }
            }
          }
          catch( Iconv::Ex & )
          {
              // Failed to decode
          }

          // CP1251
          try
          {
            wstring decoded = Iconv::toWstring( "CP1251", entry.fileName.constData(),
                                                entry.fileName.size() );

            if( nameInSystemLocale.compare( decoded ) != 0 )
            {
              zipFileNames.addSingleWord( decoded,
                                          entry.localHeaderOffset );

              if( filesCount != 0 && !alreadyCounted )
              {
                *filesCount += 1;
                alreadyCounted = true;
              }
            }
          }
          catch( Iconv::Ex & )
          {
            // Failed to decode
          }
        }
      }
    }
    return true;
}
