/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifdef MAKE_EXTRA_TIFF_HANDLER

#include "tiff.hh"

#if defined (Q_OS_WIN)
#include "tiff/tiff.h"
#include "tiff/tiffio.h"
#else
#include "tiff.h"
#include "tiffio.h"
#endif

#include <QBuffer>
#include <QApplication>
#include <QScreen>

namespace GdTiff
{

tsize_t tiffReadProc( thandle_t fd, tdata_t buf, tsize_t size )
{
  return static_cast< QIODevice * >( fd )->read( static_cast< char * >( buf ), size );
}

tsize_t tiffWriteProc( thandle_t fd, tdata_t buf, tsize_t size )
{
Q_UNUSED( fd )
Q_UNUSED( buf )
Q_UNUSED( size )
  return 0;
}

toff_t tiffSeekProc( thandle_t fd, toff_t off, int whence )
{
  QIODevice * device = static_cast< QIODevice * >( fd );
  switch (whence) {
    case SEEK_SET:
        device->seek( off );
        break;
    case SEEK_CUR:
        device->seek( device->pos() + off );
        break;
    case SEEK_END:
        device->seek( device->size() + off );
        break;
  }

  return device->pos();
}

int tiffCloseProc( thandle_t fd )
{
Q_UNUSED( fd )
  return 0;
}

toff_t tiffSizeProc( thandle_t fd )
{
  return static_cast< QIODevice * >( fd )->size();
}

int tiffMapProc( thandle_t fd, tdata_t * pbase, toff_t * psize )
{
Q_UNUSED( fd )
Q_UNUSED( pbase )
Q_UNUSED( psize )
  return 0;
}

void tiffUnmapProc( thandle_t fd, tdata_t base, toff_t size )
{
Q_UNUSED( fd )
Q_UNUSED( base )
Q_UNUSED( size )
}


bool tiffToQImage( const char * data, int size, QImage & image )
{
  QByteArray arr = QByteArray::fromRawData( data, size );
  QBuffer buf;
  buf.setData( arr );
  buf.open( QIODevice::ReadOnly );

  TIFF *const tiff = TIFFClientOpen( "foo",
                                     "r",
                                     &buf,
                                     tiffReadProc,
                                     tiffWriteProc,
                                     tiffSeekProc,
                                     tiffCloseProc,
                                     tiffSizeProc,
                                     tiffMapProc,
                                     tiffUnmapProc );
  if( !tiff )
    return false;

  uint32 width, height;
  if( !TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width )
      || !TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height ) )
  {
    TIFFClose( tiff );
    return false;
  }

  uint16 bitPerSample;
  if( !TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitPerSample ) )
    bitPerSample = 1;

  uint16 samplesPerPixel; // they may be e.g. grayscale with 2 samples per pixel
  if( !TIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel ) )
    samplesPerPixel = 1;

  if( bitPerSample == 1 && samplesPerPixel == 1 )
  {
    QImage tiffImage( width, height, QImage::Format_Mono );

    QVector<QRgb> colortable( 2 );
    colortable[0] = 0xffffffff;
    colortable[1] = 0xff000000;
    tiffImage.setColorTable( colortable );

    for ( uint32 y = 0; y < height; ++y )
    {
      if( TIFFReadScanline( tiff, tiffImage.scanLine( y ), y, 0 ) < 0 )
      {
        TIFFClose( tiff );
        return false;
      }
    }

    image = tiffImage;
    TIFFClose( tiff );
    return true;
  }

  TIFFClose( tiff );
  return false;
}

void tiff2img( vector< char > & data, const char * format )
{
  QImage img = QImage::fromData( (unsigned char *)&data.front(), data.size() );

#ifdef MAKE_EXTRA_TIFF_HANDLER
  if( img.isNull() )
    tiffToQImage( &data.front(), data.size(), img );
#endif

  if( !img.isNull() )
  {
    QByteArray ba;
    QBuffer buffer( &ba );
    buffer.open( QIODevice::WriteOnly );
    QSize screenSize = QApplication::primaryScreen()->availableSize();
    QSize imgSize    = img.size();
    int scaleSize    = qMin( imgSize.width(), screenSize.width() );

    img.scaledToWidth( scaleSize ).save( &buffer, format );

    data.resize( buffer.size() );
    memcpy( &data.front(), buffer.data(), data.size() );
    buffer.close();
  }
}

} // namespace

#endif
