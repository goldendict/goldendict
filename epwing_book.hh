#ifndef __EPWING_BOOK_HH_INCLUDED__
#define __EPWING_BOOK_HH_INCLUDED__

#include <qglobal.h>

#if defined( Q_OS_WIN32 ) || defined( Q_OS_MAC )
#define _FILE_OFFSET_BITS 64
#endif

#include "dictionary.hh"
#include "ex.hh"
#include "mutex.hh"

#include <QString>
#include <QTextCodec>
#include <QMap>
#include <QVector>
#include <vector>
#include <string>

#ifdef _MSC_VER
#include <stub_msvc.h>
#endif

#include <eb/eb.h>

namespace Epwing {

DEF_EX_STR( exEbLibrary, "EB library report error", Dictionary::Ex )
DEF_EX_STR( exEpwing, "Epwing parsing error:", Dictionary::Ex )

void initialize();
void finalize();

namespace Book {

using std::vector;
using std::string;

enum {
  TextBufferSize = 4095,
  BinaryBufferSize = 50000,
  TextSizeLimit = 2800000
};

struct EpwingHeadword
{
  QString headword;
  quint32 page;
  quint32 offset;
};

class EpwingBook
{
  typedef QPair< int, int > EWPos;

  void setErrorString( QString const & func, EB_Error_Code code );

  EB_Book book;
  EB_Appendix appendix;
  EB_Hookset hookSet, refHookSet;
  EB_Subbook_Code subBookList[ EB_MAX_SUBBOOKS ];
  EB_Subbook_Code subAppendixList[ EB_MAX_SUBBOOKS ];
  EB_Position currentPosition, indexHeadwordsPosition;
  int subBookCount, subAppendixCount;
  int currentSubBook;
  QString error_string;
  QString mainCacheDir, rootDir;
  QString cacheImagesDir, cacheSoundsDir, cacheMoviesDir, cacheFontsDir;
  QString dictID;
  QTextCodec * codec_ISO, * codec_GB, * codec_Euc;
  QStack< unsigned int > decorationStack;
  int monoWidth, monoHeight;
  QStringList imageCacheList, soundsCacheList, moviesCacheList, fontsCacheList;
  QMap< QString, QString > baseFontsMap, customFontsMap;
  QVector< int > refPages, refOffsets;
  QMap< QString, EWPos > allHeadwordPositions;
  QVector< EWPos > LinksQueue;
  int refOpenCount, refCloseCount;
  static Mutex libMutex;

  QString createCacheDir( QString const & dir);

  // Close unslosed tags
  void finalizeText( QString & text );

  // Reset internal variables
  void prepareToRead();

  // Retrieve references from article
  void getReferencesFromText( int page, int offset );

  // Move to next article
  EB_Error_Code forwardText( EB_Position & startPos );

  // Retrieve article text from dictionary
  QString getText( int page, int offset, bool text_only );

  unsigned int normalizeDecorationCode( unsigned int code );

  QByteArray codeToUnicode( QString const & code );

public:

  enum DecorationCodes {
    UNKNOWN = 0,
    ITALIC = 1,
    BOLD = 3,
    EMPHASIS = 4,
    SUBSCRIPT = 5,
    SUPERSCRIPT = 6
  };

  EpwingBook();
  ~EpwingBook();

  Mutex & getLibMutex()
  { return libMutex; }

  QString const &errorString() const
  { return error_string; }

  QTextCodec * codecISO()
  { return codec_ISO; }

  QTextCodec * codecGB()
  { return codec_GB; }

  QTextCodec *codecEuc()
  { return codec_Euc; }

  int getSubBookCount()
  { return subBookCount; }

  void setDictID( const string & id )
  { dictID = QString::fromUtf8( id.c_str() ); }

  QString const & getImagesCacheDir()
  { return cacheImagesDir; }

  QString const & getSoundsCacheDir()
  { return cacheSoundsDir; }

  QString const & getMoviesCacheDir()
  { return cacheMoviesDir; }

  void clearBuffers()
  {
    allHeadwordPositions.clear();
    LinksQueue.clear();
  }


  // Make name for resource
  QString makeFName( QString const & ext, int page, int offset ) const;

  // Store all files in Epwing folder
  static void collectFilenames( QString const & directory,
                                vector< string > & files );

  // Initialize dictionary book
  int setBook( string const & directory );

  // Set subbook inside dictionary
  bool setSubBook( int book_nom );

  void setCacheDirectory( QString const & cacheDir );

  QString getCurrentSubBookDirectory();

  QString copyright();
  QString title();

  // Seek to first article
  void getFirstHeadword( EpwingHeadword & head );

  // Find next headword and article position
  bool getNextHeadword( EpwingHeadword & head );

  bool readHeadword( EB_Position const & pos,
                     QString & headword,
                     bool text_only );

  bool isHeadwordCorrect( QString const & headword );

  void fixHeadword( QString & headword );

  // Retrieve article from dictionary
  void getArticle( QString & headword, QString & articleText,
                   int page, int offset, bool text_only );

  const char * beginDecoration( unsigned int code );
  const char * endDecoration( unsigned int code );

  QByteArray handleColorImage( EB_Hook_Code code,
                               const unsigned int * argv );

  QByteArray handleMonoImage( EB_Hook_Code code,
                              const unsigned int * argv );

  QByteArray handleWave( EB_Hook_Code code,
                         const unsigned int * argv );

  QByteArray handleMpeg( EB_Hook_Code code,
                         const unsigned int * argv );

  QByteArray handleNarrowFont( const unsigned int * argv,
                               bool text_only );

  QByteArray handleWideFont( const unsigned int * argv,
                             bool text_only );

  QByteArray handleReference( EB_Hook_Code code,
                              const unsigned int * argv );

  bool getMatches( QString word, QVector< QString > & matches );

  bool getArticlePos( QString word, QVector< int > & pages, QVector< int > & offsets );
};

struct EContainer
{
  EpwingBook * book;
  bool textOnly;

  EContainer( EpwingBook * book_ ) :
    book( book_ ),
    textOnly( false )
  {}

  EContainer( EpwingBook * book_, bool text_only ) :
    book( book_ ),
    textOnly( text_only )
  {}
};


}

}


#endif // __EPWING_BOOK_HH_INCLUDED__
