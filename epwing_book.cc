/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "epwing_book.hh"

#include <QDir>
#include <QTextStream>
#include <QTextDocumentFragment>
#include "gddebug.hh"
#include "fsencoding.hh"
#include "audiolink.hh"
#include "wstring.hh"
#include "wstring_qt.hh"
#include "folding.hh"
#include "epwing_charmap.hh"

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
#define _FILE_OFFSET_BITS 64
#endif

#include <eb/text.h>
#include <eb/appendix.h>
#include <eb/error.h>
#include <eb/binary.h>
#include <eb/font.h>

#define HitsBufferSize 512

namespace Epwing {

void initialize()
{
  eb_initialize_library();
}

void finalize()
{
  eb_finalize_library();
}

namespace Book {

#define HookFunc(name) \
    EB_Error_Code name(EB_Book *, EB_Appendix *, void*, EB_Hook_Code, int, \
                       const unsigned int*)

HookFunc( hook_newline );
HookFunc( hook_iso8859_1 );
HookFunc( hook_narrow_jisx0208 );
HookFunc( hook_wide_jisx0208 );
HookFunc( hook_gb2312 );
HookFunc( hook_superscript );
HookFunc( hook_subscript );
HookFunc( hook_decoration );
HookFunc( hook_color_image );
HookFunc( hook_gray_image );
HookFunc( hook_mono_image );
HookFunc( hook_wave );
HookFunc( hook_mpeg );
HookFunc( hook_narrow_font );
HookFunc( hook_wide_font );
HookFunc( hook_reference );

const EB_Hook hooks[] = {
  { EB_HOOK_NEWLINE, hook_newline },
  { EB_HOOK_ISO8859_1, hook_iso8859_1 },
  { EB_HOOK_NARROW_JISX0208, hook_narrow_jisx0208 },
  { EB_HOOK_WIDE_JISX0208, hook_wide_jisx0208 },
  { EB_HOOK_GB2312, hook_gb2312 },
  { EB_HOOK_BEGIN_SUBSCRIPT, hook_subscript },
  { EB_HOOK_END_SUBSCRIPT, hook_subscript },
  { EB_HOOK_BEGIN_SUPERSCRIPT, hook_superscript },
  { EB_HOOK_END_SUPERSCRIPT, hook_superscript },
  { EB_HOOK_BEGIN_DECORATION, hook_decoration },
  { EB_HOOK_END_DECORATION, hook_decoration },
  { EB_HOOK_BEGIN_COLOR_BMP, hook_color_image },
  { EB_HOOK_BEGIN_COLOR_JPEG, hook_color_image },
  { EB_HOOK_END_COLOR_GRAPHIC, hook_color_image },
  { EB_HOOK_BEGIN_IN_COLOR_BMP, hook_color_image },
  { EB_HOOK_BEGIN_IN_COLOR_JPEG, hook_color_image },
  { EB_HOOK_END_IN_COLOR_GRAPHIC, hook_color_image },
  { EB_HOOK_BEGIN_MONO_GRAPHIC, hook_mono_image },
  { EB_HOOK_END_MONO_GRAPHIC, hook_mono_image },
  { EB_HOOK_BEGIN_WAVE, hook_wave },
  { EB_HOOK_END_WAVE, hook_wave },
  { EB_HOOK_BEGIN_MPEG, hook_mpeg },
  { EB_HOOK_END_MPEG, hook_mpeg },
  { EB_HOOK_NARROW_FONT, hook_narrow_font },
  { EB_HOOK_WIDE_FONT, hook_wide_font },
  { EB_HOOK_BEGIN_REFERENCE, hook_reference },
  { EB_HOOK_END_REFERENCE, hook_reference },
  { EB_HOOK_NULL, NULL }
};

const EB_Hook refHooks[] = {
  { EB_HOOK_BEGIN_REFERENCE, hook_reference },
  { EB_HOOK_END_REFERENCE, hook_reference },
  { EB_HOOK_NULL, NULL }
};

#define EUC_TO_ASCII_TABLE_START	0xa0
#define EUC_TO_ASCII_TABLE_END		0xff

// Tables from EB library

static const unsigned char euc_a1_to_ascii_table[] = {
    0x00, 0x20, 0x00, 0x00, 0x2c, 0x2e, 0x00, 0x3a,     /* 0xa0 */
    0x3b, 0x3f, 0x21, 0x00, 0x00, 0x00, 0x60, 0x00,     /* 0xa8 */
    0x5e, 0x7e, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x2f,     /* 0xb8 */
    0x5c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x27,     /* 0xc0 */
    0x00, 0x22, 0x28, 0x29, 0x00, 0x00, 0x5b, 0x5d,     /* 0xc8 */
    0x7b, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd0 */
    0x00, 0x00, 0x00, 0x00, 0x2b, 0x2d, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x3d, 0x00, 0x3c, 0x3e, 0x00, 0x00, 0x00,     /* 0xe0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5c,     /* 0xe8 */
    0x24, 0x00, 0x00, 0x25, 0x23, 0x26, 0x2a, 0x40,     /* 0xf0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

static const unsigned char euc_a3_to_ascii_table[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xa8 */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,     /* 0xb0 */
    0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xb8 */
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,     /* 0xc0 */
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,     /* 0xc8 */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,     /* 0xd0 */
    0x58, 0x59, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xd8 */
    0x00, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,     /* 0xe0 */
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,     /* 0xe8 */
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,     /* 0xf0 */
    0x78, 0x79, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x00,     /* 0xf8 */
};

// Hook functions

EB_Error_Code hook_newline( EB_Book * book, EB_Appendix *, void * ptr,
                            EB_Hook_Code, int, const unsigned int * )
{
  EContainer * container = static_cast< EContainer * >( ptr );

  if( container->textOnly )
    eb_write_text_byte1( book,  '\n' );
  else
    eb_write_text_string( book, "<br>" );
  return EB_SUCCESS;
}

EB_Error_Code hook_iso8859_1( EB_Book * book, EB_Appendix *, void * container,
                              EB_Hook_Code, int, const unsigned int * argv )
{
  EpwingBook * ebook = static_cast< EpwingBook * >( container );
  if( ebook->codecISO() )
  {
    QByteArray b = ebook->codecISO()->toUnicode( (const char *)argv, 1 ).toUtf8();
    eb_write_text( book, b.data(), b.size() );
  }
  return EB_SUCCESS;
}

EB_Error_Code hook_narrow_jisx0208( EB_Book * book, EB_Appendix *,
                                    void * container, EB_Hook_Code, int,
                                    const unsigned int * argv )
{
  if( *argv == 0xA1E3 )
    eb_write_text_string( book, "&lt;" );
  else
  if( *argv == 0xA1E4 )
    eb_write_text_string( book, "&gt;" );
  else
  if( *argv == 0xA1F5 )
    eb_write_text_string( book, "&amp;" );
  else
  {
    unsigned char buf[ 2 ];
    buf[ 0 ] = argv[ 0 ] >> 8;
    buf[ 1 ] = argv[ 0 ] & 0xff;
    int out_code = 0;

    if( buf[ 0 ] == 0xA1 && buf[ 1 ] >= EUC_TO_ASCII_TABLE_START )
      out_code = euc_a1_to_ascii_table[ buf[ 1 ] - EUC_TO_ASCII_TABLE_START ];
    else
    if( buf[ 0 ] == 0xA3 && buf[ 1 ] >= EUC_TO_ASCII_TABLE_START )
      out_code = euc_a3_to_ascii_table[ buf[ 1 ] - EUC_TO_ASCII_TABLE_START ];

    if( out_code == 0 )
    {
      EContainer * cont = static_cast< EContainer * >( container );
      if( cont->book->codecEuc() )
      {
        QByteArray str = cont->book->codecEuc()->toUnicode( (const char *)buf, 2 ).toUtf8();
        eb_write_text( book, str.data(), str.size() );
      }
      else
        eb_write_text( book, (const char *)buf, 2 );
    }
    else
      eb_write_text_byte1( book, out_code );
  }

  return EB_SUCCESS;
}

EB_Error_Code hook_wide_jisx0208( EB_Book * book, EB_Appendix *, void * ptr,
                                  EB_Hook_Code, int, const unsigned int * argv )
{
  EpwingBook * ebook = static_cast< EContainer * >( ptr )->book;

  char buf[2];
  buf[1] = *argv & 0xFF;
  buf[0] = ( *argv & 0xFF00 ) >> 8;

  if( ebook->codecEuc() )
  {
    QByteArray b = ebook->codecEuc()->toUnicode( buf, 2 ).toUtf8();
    eb_write_text( book, b.data(), b.size() );
  }
  else
    eb_write_text_byte2( book, buf[ 0 ], buf[ 1 ] );

  return EB_SUCCESS;
}

EB_Error_Code hook_gb2312( EB_Book * book, EB_Appendix *, void * container,
                           EB_Hook_Code, int, const unsigned int * argv )
{
  EpwingBook * ebook = static_cast< EContainer * >( container )->book;

  char buf[2];
  buf[1] = *argv & 0xFF;
  buf[0] = ( *argv & 0xFF00 ) >> 8 ;

  if( ebook->codecGB() )
  {
    QByteArray b = ebook->codecGB()->toUnicode( buf, 2 ).toUtf8();
    eb_write_text( book, b.data(), b.size() );
  }
  else
    eb_write_text_byte2( book, buf[ 0 ], buf[ 1 ] );

  return EB_SUCCESS;
}

EB_Error_Code hook_subscript( EB_Book * book, EB_Appendix *, void * container,
                              EB_Hook_Code code, int, const unsigned int * )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  if( code == EB_HOOK_BEGIN_SUBSCRIPT )
    eb_write_text_string( book, cn->book->beginDecoration( EpwingBook::SUBSCRIPT ) );

  if( code == EB_HOOK_END_SUBSCRIPT )
    eb_write_text_string( book, cn->book->endDecoration( EpwingBook::SUBSCRIPT ) );

  return EB_SUCCESS;
}

EB_Error_Code hook_superscript( EB_Book * book, EB_Appendix *, void * container,
                                EB_Hook_Code code, int, const unsigned int * )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  if( code == EB_HOOK_BEGIN_SUPERSCRIPT )
    eb_write_text_string( book, cn->book->beginDecoration( EpwingBook::SUPERSCRIPT ) );

  if( code == EB_HOOK_END_SUPERSCRIPT )
    eb_write_text_string( book, cn->book->endDecoration( EpwingBook::SUPERSCRIPT ) );

  return EB_SUCCESS;
}

EB_Error_Code hook_decoration( EB_Book * book, EB_Appendix *, void * container,
                               EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  if( code == EB_HOOK_BEGIN_DECORATION )
    eb_write_text_string( book, cn->book->beginDecoration( argv[ 1 ] ) );

  if( code == EB_HOOK_END_DECORATION )
    eb_write_text_string( book, cn->book->endDecoration( argv[ 1 ] ) );

  return EB_SUCCESS;
}

EB_Error_Code hook_color_image( EB_Book * book, EB_Appendix *, void * container,
                                EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  QByteArray str = cn->book->handleColorImage( code, argv );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_mono_image( EB_Book * book, EB_Appendix *, void * container,
                               EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  QByteArray str = cn->book->handleMonoImage( code, argv );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_wave( EB_Book * book, EB_Appendix *, void * container,
                         EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  QByteArray str = cn->book->handleWave( code, argv );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_mpeg( EB_Book * book, EB_Appendix *, void * container,
                         EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  QByteArray str = cn->book->handleMpeg( code, argv );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_narrow_font( EB_Book * book, EB_Appendix *, void * container,
                                EB_Hook_Code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  QByteArray str = cn->book->handleNarrowFont( argv, cn->textOnly );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_wide_font( EB_Book * book, EB_Appendix *, void * container,
                              EB_Hook_Code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  QByteArray str = cn->book->handleWideFont( argv, cn->textOnly );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

EB_Error_Code hook_reference( EB_Book * book, EB_Appendix *, void * container,
                              EB_Hook_Code code, int, const unsigned int * argv )
{
  EContainer * cn = static_cast< EContainer * >( container );

  if( cn->textOnly )
    return EB_SUCCESS;

  QByteArray str = cn->book->handleReference( code, argv );
  if( !str.isEmpty() )
    eb_write_text( book, str.data(), str.size() );

  return EB_SUCCESS;
}

// EpwingBook class

EpwingBook::EpwingBook() :
  currentSubBook( -1 )
{
  codec_ISO = QTextCodec::codecForName( "ISO8859-1" );
  codec_GB = QTextCodec::codecForName( "GB2312" );
  codec_Euc = QTextCodec::codecForName("EUC-JP");

  eb_initialize_book( &book );
  eb_initialize_appendix( &appendix );

  eb_initialize_hookset( &hookSet );
  eb_set_hooks( &hookSet, hooks );

  eb_initialize_hookset( &refHookSet );
  eb_set_hooks( &refHookSet, refHooks );
}

EpwingBook::~EpwingBook()
{
  eb_finalize_hookset( &hookSet );
  eb_finalize_appendix( &appendix );
  eb_finalize_book( &book );
}

void EpwingBook::setErrorString( QString const & func, EB_Error_Code code )
{
  error_string = QString( "EB \"%1\" function error: %2 (%3)" )
                 .arg( func )
                 .arg( QTextCodec::codecForLocale()->toUnicode( eb_error_string( code ) ) )
                 .arg( QTextCodec::codecForLocale()->toUnicode( eb_error_message( code ) ) );

  if( currentPosition.page != 0 )
    error_string += QString( " on page %1, offset %2" ).arg( QString::number( currentPosition.page ) )
                                                       .arg( QString::number( currentPosition.offset ) );
}

void EpwingBook::collectFilenames( QString const & directory, vector< string > & files )
{
  QDir dir( directory );
  QString catName;
  catName += QDir::separator();
  catName += "catalogs";

  QFileInfoList entries = dir.entryInfoList( QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot,
                                             QDir::Name | QDir::DirsLast );

  for( QFileInfoList::const_iterator i = entries.constBegin();
       i != entries.constEnd(); ++i )
  {
    QString fullName = QDir::toNativeSeparators( i->filePath() );

    if( i->isDir() )
      collectFilenames( fullName, files );
    else
      files.push_back( FsEncoding::encode( fullName ) );
  }
}

int EpwingBook::setBook( string const & directory )
{
  error_string.clear();

  if( directory.empty() )
    throw exEpwing( "No such directory" );

  currentPosition.page = 0;

  indexHeadwordsPosition.page = 0;
  indexHeadwordsPosition.offset = 0;

  currentSubBook = -1;

  EB_Error_Code ret = eb_bind( &book, directory.c_str() );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_bind", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
    return -1;
  }

  ret = eb_bind_appendix( &appendix, directory.c_str() );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_bind_appendix", ret );
  }

  ret = eb_subbook_list( &book, subBookList, &subBookCount );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_subbook_list", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  ret = eb_appendix_subbook_list( &appendix, subAppendixList, &subAppendixCount );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_appendix_subbook_list", ret );
  }

  if( !codec_Euc
      || ( book.character_code == EB_CHARCODE_ISO8859_1 && !codec_ISO )
      || ( book.character_code == EB_CHARCODE_JISX0208_GB2312 && !codec_GB ) )
    throw exEpwing( "No required codec to decode dictionary" );

  rootDir = FsEncoding::decode( directory.c_str() );

  return subBookCount;
}

bool EpwingBook::setSubBook( int book_nom )
{
  error_string.clear();

  customFontsMap.clear();

  currentPosition.page = 0;

  if( book_nom < 0 || book_nom >= subBookCount )
    throw exEpwing( "Invalid subbook number" );

  if( currentSubBook == book_nom )
    return true;

  EB_Error_Code ret = eb_set_subbook( &book, subBookList[ book_nom ] );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_set_subbook", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }
  currentSubBook = book_nom;

  if( book_nom < subAppendixCount )
  {
    ret = eb_set_appendix_subbook( &appendix, subAppendixList[ book_nom ] );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_set_appendix_subbook", ret );
    }
  }

  if( eb_have_font( &book, EB_FONT_16 ) )
  {
    ret = eb_set_font( &book, EB_FONT_16 );
    if( ret != EB_SUCCESS )
      setErrorString( "eb_set_font", ret );
  }

  // Load fonts list

  QString fileName = rootDir + QDir::separator()
                     + "afonts_" + QString::number( book_nom );

  QFile f( fileName );
  if( f.open( QFile::ReadOnly | QFile::Text ) )
  {
    QTextStream ts( &f );
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    ts.setCodec( "UTF-8" );
#else
    ts.setEncoding(QStringConverter::Utf8);
#endif

    QString line = ts.readLine();
    while( !line.isEmpty() )
    {
      QStringList list = line.remove( '\n' ).split( ' ', Qt::SkipEmptyParts );
      if( list.count() == 2 )
        customFontsMap[ list[ 0 ] ] = list[ 1 ];
      line = ts.readLine();
    }

    f.close();
  }

  return true;
}

void EpwingBook::setCacheDirectory( QString const & cacheDir )
{
  mainCacheDir = cacheDir;
  cacheImagesDir.clear();
  cacheSoundsDir.clear();
  cacheMoviesDir.clear();
  cacheFontsDir.clear();

  imageCacheList.clear();
  soundsCacheList.clear();
  moviesCacheList.clear();
  fontsCacheList.clear();
}

QString EpwingBook::createCacheDir( QString const & dirName )
{
  QDir dir;
  QFileInfo info( mainCacheDir );
  if( !info.exists() || !info.isDir() )
  {
    if( !dir.mkdir( mainCacheDir ) )
    {
      gdWarning( "Epwing: can't create cache directory \"%s\"", mainCacheDir.toUtf8().data() );
      return QString();
    }
  }

  QString cacheDir = mainCacheDir + QDir::separator() + dirName;
  info = QFileInfo( cacheDir );
  if( !info.exists() || !info.isDir() )
  {
    if( !dir.mkdir( cacheDir ) )
    {
      gdWarning( "Epwing: can't create cache directory \"%s\"", cacheDir.toUtf8().data() );
      return QString();
    }
  }
  return cacheDir;
}

QString EpwingBook::getCurrentSubBookDirectory()
{
  error_string.clear();

  if( currentSubBook < 0 || currentSubBook >= subBookCount )
  {
    setErrorString( "eb_subbook_directory2", EB_ERR_NO_SUCH_BOOK );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  char buffer[ EB_MAX_PATH_LENGTH + 1 ];

  EB_Error_Code ret = eb_subbook_directory2( &book, subBookList[ currentSubBook ], buffer );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_subbook_directory2", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  return QString::fromLocal8Bit( buffer );
}

QString EpwingBook::makeFName( QString const & ext, int page, int offset ) const
{
  QString name = QString::number( page )
                 + "x"
                 + QString::number( offset )
                 + "."
                 + ext;
  return name;
}

QString EpwingBook::title()
{
  char buf[ EB_MAX_TITLE_LENGTH + 1 ];
  error_string.clear();

  EB_Error_Code ret = eb_subbook_title( &book, buf );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_subbook_title", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  buf[ EB_MAX_TITLE_LENGTH ] = 0;
  if( codec_Euc )
    return codec_Euc->toUnicode( buf );

  return QString();
}

QString EpwingBook::copyright()
{
  error_string.clear();

  if( !eb_have_copyright( &book ) )
    return QString();

  EB_Position position;
  EB_Error_Code ret = eb_copyright( &book, &position );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_copyright", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  currentPosition = position;

  return getText( position.page, position.offset, true );
}

QString EpwingBook::getText( int page, int offset, bool text_only )
{
  error_string.clear();

  EB_Position pos;
  pos.page = page;
  pos.offset = offset;
  currentPosition = pos;

  EB_Error_Code ret = eb_seek_text(&book, &pos);
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_seek_text", ret );
    currentPosition.page = 0;
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  QByteArray buf;
  char buffer[ TextBufferSize + 1 ];
  ssize_t buffer_length;

  EContainer container( this, text_only );

  prepareToRead();

  for( ; ; )
  {
    ret = eb_read_text( &book, &appendix, &hookSet, &container,
                        TextBufferSize, buffer, &buffer_length );

    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_read_text", ret );
      break;
    }

    buf += QByteArray( buffer, buffer_length );

    if( eb_is_text_stopped( &book ) )
      break;

    if( buf.length() > TextSizeLimit )
    {
      error_string = "Data too large";
      currentPosition.page = 0;
      return QString();
    }
  }

  QString text = QString::fromUtf8( buf.data(), buf.size() ).trimmed();
  finalizeText( text );
  return text;
}

void EpwingBook::getReferencesFromText( int page, int offset )
{
  error_string.clear();

  EB_Position pos;
  pos.page = page;
  pos.offset = offset;

  currentPosition = pos;

  EB_Error_Code ret = eb_seek_text(&book, &pos);
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_seek_text", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  char buffer[ TextBufferSize + 1 ];
  ssize_t buffer_length;

  EContainer container( this, false );

  prepareToRead();

  for( ; ; )
  {
    ret = eb_read_text( &book, &appendix, &refHookSet, &container,
                        TextBufferSize, buffer, &buffer_length );

    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_read_text", ret );
      break;
    }

    if( eb_is_text_stopped( &book ) )
      break;
  }

  for( int x = 0; x < refPages.size(); x++ )
    LinksQueue.push_back( EWPos( refPages[ x ], refOffsets[ x ] ) );
}

EB_Error_Code EpwingBook::forwardText( EB_Position & startPos )
{
  EB_Error_Code ret = eb_seek_text( &book, &startPos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_seek_text", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  ret = eb_forward_text( &book, &appendix );
  while( ret == EB_ERR_END_OF_CONTENT )
  {
    ret = eb_tell_text( &book, &startPos );
    if( ret != EB_SUCCESS )
      break;

    if( startPos.page >= book.subbook_current->text.end_page )
      return EB_ERR_END_OF_CONTENT;

    startPos.offset += 2;
    currentPosition = startPos;

    ret = eb_seek_text( &book, &startPos );

    if( ret == EB_SUCCESS )
      ret = eb_forward_text( &book, &appendix );
  }
  return ret;
}

void EpwingBook::getFirstHeadword( EpwingHeadword & head )
{
  error_string.clear();

  EB_Position pos;

  EB_Error_Code ret = eb_text( &book, &pos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_text", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  ret = forwardText( pos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "forwardText", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  eb_backward_text( &book, &appendix );

  ret = eb_tell_text( &book, &pos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_tell_text", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  currentPosition = pos;
  indexHeadwordsPosition = pos;

  head.page = pos.page;
  head.offset = pos.offset;

  if( !readHeadword( pos, head.headword, true ) )
    throw exEbLibrary( error_string.toUtf8().data() );

  fixHeadword( head.headword );

  EWPos epos( pos.page, pos.offset );
  allHeadwordPositions[ head.headword ] << epos;
}

bool EpwingBook::getNextHeadword( EpwingHeadword & head )
{
  EB_Position pos;

  QRegularExpression badLinks( "#(v|n)\\d", QRegularExpression::UseUnicodePropertiesOption);

  // At first we check references queue
  while( !LinksQueue.isEmpty() )
  {
    EWPos epos = LinksQueue.last();
    LinksQueue.pop_back();

    pos.page = epos.first;
    pos.offset = epos.second;

    if( readHeadword( pos, head.headword, true ) )
    {
      if( head.headword.isEmpty()
          || head.headword.contains( badLinks ) )
        continue;

      fixHeadword( head.headword );

      head.page = pos.page;
      head.offset = pos.offset;

      if( allHeadwordPositions.contains( head.headword ) )
      {
        // existed position
        bool existed = false;
        foreach( EWPos epos, allHeadwordPositions[ head.headword ] )
        {
          if( pos.page == epos.first && abs( pos.offset - epos.second ) <= 4 )
          {
            existed = true;
            break;
          }
        }
        if( !existed )
        {
          allHeadwordPositions[ head.headword ] << EWPos( pos.page, pos.offset );
          return true;
        }
      }
      else
      {
        allHeadwordPositions[ head.headword ] << EWPos( pos.page, pos.offset );
        return true;
      }
    }
  }

  // No queued positions - forward to next article

  error_string.clear();

  pos = indexHeadwordsPosition;

  for( ; ; )
  {
    EB_Error_Code ret = forwardText( pos );
    indexHeadwordsPosition = pos;

    if( ret == EB_ERR_END_OF_CONTENT )
      return false;
    else
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_forward_text", ret );
      throw exEbLibrary( error_string.toUtf8().data() );
    }

    ret = eb_tell_text( &book, &pos );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_tell_text", ret );
      throw exEbLibrary( error_string.toUtf8().data() );
    }

    indexHeadwordsPosition = pos;

    try
    {
      getReferencesFromText( pos.page, pos.offset );
    }
    catch( std::exception & )
    {
    }

    head.page = pos.page;
    head.offset = pos.offset;

    if( !readHeadword( pos, head.headword, true ) )
      throw exEbLibrary( error_string.toUtf8().data() );

    if( head.headword.isEmpty() )
      continue;

    fixHeadword( head.headword );

    if( allHeadwordPositions.contains( head.headword ) )
    {
      // existed position
      bool existed = false;
      foreach( EWPos epos, allHeadwordPositions[ head.headword ] )
      {
        if( pos.page == epos.first && abs( pos.offset - epos.second ) <= 4 )
        {
          existed = true;
          break;
        }
      }
      if( !existed )
      {
        allHeadwordPositions[ head.headword ] << EWPos( pos.page, pos.offset );
        return true;
      }
    }
    else
    {
      allHeadwordPositions[ head.headword ] << EWPos( pos.page, pos.offset );
      return true;
    }
  }

  return true;
}

bool EpwingBook::readHeadword( EB_Position const& pos,
                               QString & headword,
                               bool text_only )
{
  EContainer container( this, text_only );
  ssize_t head_length;

  EB_Position newPos = pos;

  EB_Error_Code ret = eb_seek_text( &book, &newPos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_seek_text", ret );
    return false;
  }

  char buffer[ TextBufferSize + 1 ];

  ret = eb_read_heading( &book, &appendix, &hookSet, &container,
                         TextBufferSize, buffer, &head_length );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_read_heading", ret );
    return false;
  }

  headword = QString::fromUtf8( buffer, head_length );
  return true;
}

bool EpwingBook::isHeadwordCorrect( QString const & headword )
{
  QByteArray buf, buf2;
  EB_Hit hits[ 2 ];
  int hit_count;
  EB_Error_Code ret;

  if( headword.isEmpty() )
    return false;

  if( book.character_code == EB_CHARCODE_ISO8859_1 && codec_ISO )
    buf = codec_ISO->fromUnicode( headword );
  else
  if( ( book.character_code == EB_CHARCODE_JISX0208 || book.character_code == EB_CHARCODE_JISX0208_GB2312 )
      && codec_Euc )
    buf = codec_Euc->fromUnicode( headword );

  if( book.character_code == EB_CHARCODE_JISX0208_GB2312 && codec_GB )
    buf2 = codec_GB->fromUnicode( headword );

  if( !buf.isEmpty() && eb_search_exactword( &book, buf.data() ) == EB_SUCCESS )
  {
    ret = eb_hit_list( &book, 2, hits, &hit_count );
    if( ret == EB_SUCCESS && hit_count > 0 )
      return true;
  }

  if( !buf2.isEmpty() && eb_search_exactword( &book, buf2.data() ) == EB_SUCCESS )
  {
    ret = eb_hit_list( &book, 2, hits, &hit_count );
    if( ret == EB_SUCCESS && hit_count > 0 )
      return true;
  }

  return false;
}

void EpwingBook::fixHeadword( QString & headword )
{
  if(headword.isEmpty() )
    return;

  headword.remove( QChar( 0x30FB ) ); // Used in Japan transcription

  if( isHeadwordCorrect( headword) )
    return;

  QString fixed = headword;
  QRegularExpression leadingSlashRx( "/[^/]+/" );
  fixed.remove(leadingSlashRx );

  if( isHeadwordCorrect( fixed ) )
  {
    headword = fixed;
    return;
  }

  gd::wstring folded = Folding::applyPunctOnly( gd::toWString( fixed ) );
  fixed = gd::toQString( folded );

  if( isHeadwordCorrect( fixed ) )
  {
    headword = fixed;
    return;
  }

  folded = Folding::applyDiacriticsOnly( folded );
  fixed = gd::toQString( folded );

  if( isHeadwordCorrect( fixed ) )
  {
    headword = fixed;
    return;
  }

  folded = Folding::applyWhitespaceOnly( folded );
  fixed = gd::toQString( folded );

  if( isHeadwordCorrect( fixed ) )
    headword = fixed;
}

void EpwingBook::getArticle( QString & headword, QString & articleText,
                             int page, int offset, bool text_only)
{
  error_string.clear();
  char buffer[ TextBufferSize + 1 ];

  EB_Position pos;
  pos.page = page;
  pos.offset = offset;

  currentPosition = pos;

  EB_Error_Code ret = eb_seek_text( &book, &pos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_seek_text", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  EContainer container( this, text_only );
  ssize_t length;

  prepareToRead();

  ret = eb_read_heading( &book, &appendix, &hookSet, &container,
                         TextBufferSize, buffer, &length );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_read_heading", ret );
    throw exEbLibrary( error_string.toUtf8().data() );
  }

  headword = QString::fromUtf8( buffer, length );
  finalizeText( headword );

  if( text_only )
    fixHeadword( headword );

  articleText = getText( pos.page, pos.offset, text_only );
}

const char * EpwingBook::beginDecoration( unsigned int code )
{
  const char * str = "";

  code = normalizeDecorationCode( code );

  switch( code )
  {
    case ITALIC:      str = "<i>";
                      break;
    case BOLD:        str = "<b>";
                      break;
    case EMPHASIS:    str = "<em>";
                      break;
    case SUBSCRIPT:   str = "<sub>";
                      break;
    case SUPERSCRIPT: str = "<sup>";
                      break;
    default:          gdWarning( "Epwing: Unknown decoration code %i", code );
                      code = UNKNOWN;
                      break;
  }
  decorationStack.push( code );
  return str;
}

const char * EpwingBook::endDecoration( unsigned int code )
{
  const char * str = "";

  code = normalizeDecorationCode( code );

  if( code != ITALIC && code != BOLD && code != EMPHASIS
      && code != SUBSCRIPT && code != SUPERSCRIPT )
    code = UNKNOWN;

  unsigned int storedCode = UNKNOWN;
  if( !decorationStack.isEmpty() )
    storedCode = decorationStack.pop();

  if( storedCode != code )
  {
    gdWarning( "Epwing: tags mismatch detected" );
    if( storedCode == UNKNOWN )
      storedCode = code;
  }

  switch( storedCode )
  {
    case ITALIC:      str = "</i>";
                      break;
    case BOLD:        str = "</b>";
                      break;
    case EMPHASIS:    str = "</em>";
                      break;
    case SUBSCRIPT:   str = "</sub>";
                      break;
    case SUPERSCRIPT: str = "</sup>";
                      break;
    case UNKNOWN:     break;
  }

  return str;
}

unsigned int EpwingBook::normalizeDecorationCode( unsigned int code )
{
  // Some non-standard codes
  switch( code )
  {
    case 0x1101: return BOLD;
    case 0x1103: return ITALIC;
  }
  return code;
}

void EpwingBook::finalizeText( QString & text )
{
  // Close unclosed tags
  while( !decorationStack.isEmpty() )
  {
    unsigned int code = decorationStack.pop();
    switch( code )
    {
      case ITALIC:      text += "</i>";
                        break;
      case BOLD:        text += "</b>";
                        break;
      case EMPHASIS:    text += "</em>";
                        break;
      case SUBSCRIPT:   text += "</sub>";
                        break;
      case SUPERSCRIPT: text += "</sup>";
                        break;
    }
  }

  // Replace references

  int pos = 0;
  QString reg1( "<R%1>");
  QString reg2( "</R%1>");

  EContainer cont( this, true );

  char buf[ TextBufferSize + 1 ];

  for( int x = 0; x < refCloseCount; x++ )
  {
    auto tag1=reg1.arg(x);
    auto tag2=reg2.arg(x);
    pos = text.indexOf( tag1, pos );
    if( pos < 0 )
      continue;

    EB_Position ebpos;
    ebpos.page = refPages[ x ];
    ebpos.offset = refOffsets[ x ];

    QUrl url;
    url.setScheme( "gdlookup" );
    url.setHost( "localhost" );

    // Read headword

    eb_seek_text( &book, &ebpos );

    ssize_t length;
    EB_Error_Code ret = eb_read_heading( &book, &appendix, &hookSet, &cont,
                                         TextBufferSize, buf, &length );
    if( ret == EB_SUCCESS )
    {
      QString headword = QString::fromUtf8( buf, length );
      fixHeadword( headword );
      url.setPath( Utils::Url::ensureLeadingSlash( headword ) );
    }

    QString link = "<a href=\"" + url.toEncoded() + "\">";

    text.replace( tag1, link );

    pos = text.indexOf( tag2, pos );
    if( pos < 0 )
      continue;

    text.replace( tag2, "</a>" );
  }
}

void EpwingBook::prepareToRead()
{
  refPages.clear();
  refOffsets.clear();
  refOpenCount = refCloseCount = 0;
}

QByteArray EpwingBook::handleColorImage( EB_Hook_Code code,
                                         const unsigned int * argv )
{
  QString name, fullName;
  EB_Position pos;

  if( code == EB_HOOK_END_COLOR_GRAPHIC
      || code == EB_HOOK_END_IN_COLOR_GRAPHIC )
    return QByteArray();

  pos.page = argv[ 2 ];
  pos.offset = argv[ 3 ];

  EB_Error_Code ret = eb_set_binary_color_graphic( &book, & pos );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_set_binary_color_graphic", ret );
    gdWarning( "Epwing image retrieve error: %s",
               error_string.toUtf8().data() );
    return QByteArray();
  }

  if( cacheImagesDir.isEmpty() )
    cacheImagesDir = createCacheDir( "images" );

  if( code == EB_HOOK_BEGIN_COLOR_BMP
      || code == EB_HOOK_BEGIN_IN_COLOR_BMP )
    name = makeFName( "bmp", pos.page, pos.offset );
  else
  if( code == EB_HOOK_BEGIN_COLOR_JPEG
      || code == EB_HOOK_BEGIN_IN_COLOR_JPEG )
    name = makeFName( "jpg", pos.page, pos.offset );

  if( !cacheImagesDir.isEmpty() )
    fullName = cacheImagesDir + QDir::separator() + name;

  QUrl url;
  url.setScheme( "bres" );
  url.setHost( dictID );
  url.setPath( Utils::Url::ensureLeadingSlash( name ) );
  QByteArray urlStr = "<p class=\"epwing_image\"><img src=\"" + url.toEncoded()
                      + "\" alt=\"" + name.toUtf8() + "\"></p>";

  if( imageCacheList.contains( name, Qt::CaseSensitive ) )
  {
    // We already have this image in cache
    return urlStr;
  }

  if( !fullName.isEmpty() )
  {
    QFile f( fullName );
    if( f.open( QFile::WriteOnly ) )
    {
      QByteArray buffer;
      buffer.resize( BinaryBufferSize );
      ssize_t length;

      for( ; ; )
      {
        ret = eb_read_binary( &book, BinaryBufferSize,
                              buffer.data(), &length );
        if( ret != EB_SUCCESS )
        {
          setErrorString( "eb_read_binary", ret );
          gdWarning( "Epwing image retrieve error: %s",
                     error_string.toUtf8().data() );
          break;
        }

        f.write( buffer.data(), length );

        if( length < BinaryBufferSize )
          break;
      }
      f.close();

      imageCacheList.append( name );
    }
  }

  return urlStr;
}

QByteArray EpwingBook::handleMonoImage( EB_Hook_Code code,
                                        const unsigned int * argv )
{
  QString name, fullName;
  EB_Position pos;

  if( code == EB_HOOK_BEGIN_MONO_GRAPHIC )
  {
    monoHeight = argv[ 2 ];
    monoWidth = argv[ 3 ];
    return QByteArray();
  }

  // Handle EB_HOOK_END_MONO_GRAPHIC hook

  pos.page = argv[ 1 ];
  pos.offset = argv[ 2 ];

  EB_Error_Code ret = eb_set_binary_mono_graphic( &book, &pos, monoWidth, monoHeight );
  if( ret != EB_SUCCESS )
  {
    setErrorString( "eb_set_binary_mono_graphic", ret );
    gdWarning( "Epwing image retrieve error: %s",
               error_string.toUtf8().data() );
    return QByteArray();
  }

  if( cacheImagesDir.isEmpty() )
    cacheImagesDir = createCacheDir( "images" );

  name = makeFName( "bmp", pos.page, pos.offset );

  if( !cacheImagesDir.isEmpty() )
    fullName = cacheImagesDir + QDir::separator() + name;

  QUrl url;
  url.setScheme( "bres" );
  url.setHost( dictID );
  url.setPath( Utils::Url::ensureLeadingSlash( name ) );
  QByteArray urlStr = "<span class=\"epwing_image\"><img src=\"" + url.toEncoded()
                      + "\" alt=\"" + name.toUtf8() + "\"/></span>";

  if( imageCacheList.contains( name, Qt::CaseSensitive ) )
  {
    // We already have this image in cache
    return urlStr;
  }

  if( !fullName.isEmpty() )
  {
    QFile f( fullName );
    if( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      QByteArray buffer;
      buffer.resize( BinaryBufferSize );
      ssize_t length;

      for( ; ; )
      {
        ret = eb_read_binary( &book, BinaryBufferSize,
                              buffer.data(), &length );
        if( ret != EB_SUCCESS )
        {
          setErrorString( "eb_read_binary", ret );
          gdWarning( "Epwing image retrieve error: %s",
                     error_string.toUtf8().data() );
          break;
        }

        f.write( buffer.data(), length );

        if( length < BinaryBufferSize )
          break;
      }
      f.close();

      imageCacheList.append( name );
    }
  }

  return urlStr;
}

QByteArray EpwingBook::handleWave( EB_Hook_Code code, const unsigned int * argv )
{

  if( code == EB_HOOK_END_WAVE )
    return QByteArray( "<img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" align=\"absmiddle\" alt=\"Play\"/></a></span>" );

  // Handle EB_HOOK_BEGIN_WAVE

  EB_Position spos, epos;
  spos.page = argv[ 2 ];
  spos.offset = argv[ 3 ];
  epos.page = argv[ 4 ];
  epos.offset = argv[ 5 ];

  eb_set_binary_wave( &book, &spos, &epos );

  if( cacheSoundsDir.isEmpty() )
    cacheSoundsDir = createCacheDir( "sounds" );

  QString name = makeFName( "wav", spos.page, spos.offset );
  QString fullName;

  if( !cacheSoundsDir.isEmpty() )
    fullName = cacheSoundsDir + QDir::separator() + name;

  QUrl url;
  url.setScheme( "gdau" );
  url.setHost( dictID );
  url.setPath( Utils::Url::ensureLeadingSlash( name ) );

  string ref = string( "\"" )+ url.toEncoded().data() + "\"";
  QByteArray result = addAudioLink( ref , dictID.toUtf8().data() ).c_str();

  result += QByteArray( "<span class=\"epwing_wav\"><a href=" ) + ref.c_str() + ">";

  if( soundsCacheList.contains( name, Qt::CaseSensitive ) )
  {
    // We already have this sound in cache
    return result;
  }

  if( !fullName.isEmpty() )
  {
    QFile f( fullName );
    if( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      QByteArray buffer;
      buffer.resize( BinaryBufferSize );
      ssize_t length;

      for( ; ; )
      {
        EB_Error_Code ret = eb_read_binary( &book, BinaryBufferSize,
                                            buffer.data(), &length );
        if( ret != EB_SUCCESS )
        {
          setErrorString( "eb_read_binary", ret );
          gdWarning( "Epwing sound retrieve error: %s",
                     error_string.toUtf8().data() );
          break;
        }

        f.write( buffer.data(), length );

        if( length < BinaryBufferSize )
          break;
      }
      f.close();

      soundsCacheList.append( name );
    }
  }
  return result;
}

QByteArray EpwingBook::handleMpeg( EB_Hook_Code code, const unsigned int * argv )
{

  if( code == EB_HOOK_END_MPEG )
    return QByteArray( "</a></span>" );

  // Handle EB_HOOK_BEGIN_MPEG

  char file[ EB_MAX_PATH_LENGTH + 1 ];

  unsigned int *p = (unsigned int *)( argv + 2 );
  eb_compose_movie_path_name( &book, p, file );

  QString name = QString::fromLocal8Bit( file );
  name = QFileInfo( name ).fileName();
  name += ".mpg";

  eb_set_binary_mpeg( &book, argv + 2 );

  if( cacheMoviesDir.isEmpty() )
    cacheMoviesDir = createCacheDir( "movies" );

  QString fullName;

  if( !cacheMoviesDir.isEmpty() )
    fullName = cacheMoviesDir + QDir::separator() + name;

  QUrl url;
  url.setScheme( "gdvideo" );
  url.setHost( dictID );
  url.setPath( Utils::Url::ensureLeadingSlash( name ) );

  QByteArray result = QByteArray( "<span class=\"epwing_mpeg\"><a href=" ) + url.toEncoded() + ">";

  if( moviesCacheList.contains( name, Qt::CaseSensitive ) )
  {
    // We already have this movie in cache
    return result;
  }

  if( !fullName.isEmpty() )
  {
    QFile f( fullName );
    if( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      QByteArray buffer;
      buffer.resize( BinaryBufferSize );
      ssize_t length;

      for( ; ; )
      {
        EB_Error_Code ret = eb_read_binary( &book, BinaryBufferSize,
                                            buffer.data(), &length );
        if( ret != EB_SUCCESS )
        {
          setErrorString( "eb_read_binary", ret );
          gdWarning( "Epwing movie retrieve error: %s",
                     error_string.toUtf8().data() );
          break;
        }

        f.write( buffer.data(), length );

        if( length < BinaryBufferSize )
          break;
      }
      f.close();

      moviesCacheList.append( name );
    }
  }
  return result;
}

QByteArray EpwingBook::codeToUnicode( QString const & code )
{
  QString subst;

  if( !customFontsMap.isEmpty() && customFontsMap.contains( code ) )
  {
    subst = QTextDocumentFragment::fromHtml( customFontsMap[ code ] )
                                   .toPlainText();
    return subst.toUtf8();
  }

  return EpwingCharmap::instance().mapToUtf8( code );
}

QByteArray EpwingBook::handleNarrowFont( const unsigned int * argv,
                                         bool text_only )
{
  QString fcode = "n" + QString::number( *argv, 16 );

  // Check substitution list

  QByteArray b = codeToUnicode( fcode );
  if( !b.isEmpty() || text_only )
    return b;

  // Find font image in book

  if( !eb_have_narrow_font( &book ) )
    return QByteArray( "?" );

  QString fname = fcode + ".png";

  if( cacheFontsDir.isEmpty() )
    cacheFontsDir = createCacheDir( "fonts" );

  QString fullName = cacheFontsDir + QDir::separator() + fname;

  QUrl url;
  url.setScheme( "file" );
  url.setHost( "/");
  url.setPath( Utils::Url::ensureLeadingSlash( QDir::fromNativeSeparators( fullName ) ) );

  QByteArray link = "<img class=\"epwing_narrow_font\" src=\"" + url.toEncoded() + "\"/>";

  if( fontsCacheList.contains( fname, Qt::CaseSensitive ) )
  {
    // We already have this image in cache
    return link;
  }

  if( !cacheFontsDir.isEmpty() )
  {
    char bitmap[EB_SIZE_NARROW_FONT_16];
    EB_Error_Code ret = eb_narrow_font_character_bitmap( &book, *argv, bitmap );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_narrow_font_character_bitmap", ret );
      gdWarning( "Epwing: Font retrieve error: %s", error_string.toUtf8().data() );
      return QByteArray( "?" );
    }

      size_t nlen;
      char buff[EB_SIZE_NARROW_FONT_16_PNG];
      ret = eb_bitmap_to_png( bitmap, 8, 16, buff, &nlen );
      if( ret != EB_SUCCESS )
      {
        setErrorString( "eb_bitmap_to_png", ret );
        gdWarning( "Epwing: Font retrieve error: %s", error_string.toUtf8().data() );
        return QByteArray( "?" );
      }


    QFile f( fullName );
    if( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      f.write( buff, nlen );
      f.close();
      fontsCacheList.append( fname );
    }
  }

  return link;
}

QByteArray EpwingBook::handleWideFont( const unsigned int * argv,
                                       bool text_only )
{
  QString fcode = "w" + QString::number( *argv, 16 );

  // Check substitution list

  QByteArray b = codeToUnicode( fcode );
  if( !b.isEmpty() || text_only )
    return b;

  // Find font image in book

  if( !eb_have_wide_font( &book ) )
    return QByteArray( "?" );

  QString fname = fcode + ".png";

  if( cacheFontsDir.isEmpty() )
    cacheFontsDir = createCacheDir( "fonts" );

  QString fullName = cacheFontsDir + QDir::separator() + fname;

  QUrl url;
  url.setScheme( "file" );
  url.setHost( "/");
  url.setPath( Utils::Url::ensureLeadingSlash( QDir::fromNativeSeparators( fullName ) ) );

  QByteArray link = "<img class=\"epwing_wide_font\" src=\"" + url.toEncoded() + "\"/>";

  if( fontsCacheList.contains( fname, Qt::CaseSensitive ) )
  {
    // We already have this image in cache
    return link;
  }

  if( !cacheFontsDir.isEmpty() )
  {
    char bitmap[EB_SIZE_WIDE_FONT_16];
    EB_Error_Code ret = eb_wide_font_character_bitmap( &book, *argv, bitmap );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_wide_font_character_bitmap", ret );
      gdWarning( "Epwing: Font retrieve error: %s", error_string.toUtf8().data() );
      return QByteArray( "?" );
    }

    size_t wlen;
    char buff[EB_SIZE_WIDE_FONT_16_PNG];
    ret = eb_bitmap_to_png( bitmap, 16, 16, buff, &wlen );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_bitmap_to_png", ret );
      gdWarning( "Epwing: Font retrieve error: %s", error_string.toUtf8().data() );
      return QByteArray( "?" );
    }

    QFile f( fullName );
    if( f.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      f.write( buff, wlen );
      f.close();
      fontsCacheList.append( fname );
    }
  }

  return link;
}

QByteArray EpwingBook::handleReference( EB_Hook_Code code, const unsigned int * argv )
{
  if( code == EB_HOOK_BEGIN_REFERENCE )
  {
    if( refOpenCount > refCloseCount )
      return QByteArray();

    QString str=QString( "<R%1>").arg( refOpenCount );
    refOpenCount += 1;
    return str.toUtf8();
  }

  // EB_HOOK_END_REFERENCE

  if( refCloseCount >= refOpenCount )
    return QByteArray();

  refPages.append( argv[ 1 ] );
  refOffsets.append( argv[ 2 ] );

  QString str = QString( "</R%1>" ).arg( refCloseCount );
  refCloseCount += 1;

  return str.toUtf8();
}

bool EpwingBook::getMatches( QString word, QVector< QString > & matches )
{
  QByteArray bword, bword2;
  EB_Hit hits[ HitsBufferSize ];
  int hitCount = 0;

  if( book.character_code == EB_CHARCODE_ISO8859_1 && codec_ISO )
    bword = codec_ISO->fromUnicode( word );
  else
  if( ( book.character_code == EB_CHARCODE_JISX0208 || book.character_code == EB_CHARCODE_JISX0208_GB2312 )
      && codec_Euc )
    bword = codec_Euc->fromUnicode( word );

  if( book.character_code == EB_CHARCODE_JISX0208_GB2312 && codec_GB )
    bword2 = codec_GB->fromUnicode( word );

  if( !bword.isEmpty() )
  {
    EB_Error_Code ret = eb_search_word( &book, bword.data() );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_search_word", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }

    ret = eb_hit_list( &book, 10, hits, &hitCount );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_hit_list", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }
  }

  if( hitCount == 0 && !bword2.isEmpty() )
  {
    EB_Error_Code ret = eb_search_word( &book, bword2.data() );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_search_word", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }

    ret = eb_hit_list( &book, 10, hits, &hitCount );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_hit_list", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }
  }

  QVector< int > pages, offsets;

  for( int i = 0; i < hitCount; i++ )
  {
    bool same_article = false;
    for( int n = 0; n < pages.size(); n++ )
    {
      if( pages.at( n ) == hits[ i ].text.page
          && offsets.at( n ) == hits[ i ].text.offset )
      {
        same_article = true;
        continue;
      }
    }
    if( !same_article )
    {
      pages.push_back( hits[ i ].text.page );
      offsets.push_back( hits[ i ].text.offset );

      QString headword;
      if( readHeadword( hits[ i ].heading, headword, true ) )
      {
        if( isHeadwordCorrect( headword ) )
          matches.push_back( headword );
      }
    }
  }
  return true;
}

bool EpwingBook::getArticlePos( QString word, QVector< int > & pages, QVector< int > & offsets )
{
  QByteArray bword, bword2;
  EB_Hit hits[ HitsBufferSize ];
  int hitCount = 0;

  if( book.character_code == EB_CHARCODE_ISO8859_1 && codec_ISO )
    bword = codec_ISO->fromUnicode( word );
  else
  if( ( book.character_code == EB_CHARCODE_JISX0208 || book.character_code == EB_CHARCODE_JISX0208_GB2312 )
      && codec_Euc )
    bword = codec_Euc->fromUnicode( word );

  if( book.character_code == EB_CHARCODE_JISX0208_GB2312 && codec_GB )
    bword2 = codec_GB->fromUnicode( word );

  if( !bword.isEmpty() )
  {
    EB_Error_Code ret = eb_search_exactword( &book, bword.data() );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_search_word", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }

    ret = eb_hit_list( &book, HitsBufferSize, hits, &hitCount );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_hit_list", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }
  }

  if( hitCount == 0 && !bword2.isEmpty() )
  {
    EB_Error_Code ret = eb_search_exactword( &book, bword2.data() );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_search_word", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }

    ret = eb_hit_list( &book, HitsBufferSize, hits, &hitCount );
    if( ret != EB_SUCCESS )
    {
      setErrorString( "eb_hit_list", ret );
      gdWarning( "Epwing word search error: %s",
                   error_string.toUtf8().data() );
      return false;
    }
  }

  for( int i = 0; i < hitCount; i++ )
  {
    bool same_article = false;
    for( int n = 0; n < pages.size(); n++ )
    {
      if( pages.at( n ) == hits[ i ].text.page
          && offsets.at( n ) == hits[ i ].text.offset )
      {
        same_article = true;
        continue;
      }
    }
    if( !same_article )
    {
      pages.push_back( hits[ i ].text.page );
      offsets.push_back( hits[ i ].text.offset );
    }
  }

  return !pages.empty();
}

Mutex EpwingBook::libMutex;

} // namespace Book

} // namespace Epwing
