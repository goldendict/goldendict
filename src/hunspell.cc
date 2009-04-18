/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "hunspell.hh"
#include "utf8.hh"
#include "htmlescape.hh"
#include "iconv.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include <QRunnable>
#include <QThreadPool>
#include <QSemaphore>
#include <QRegExp>
#include <QDir>
#include <QCoreApplication>
#include <set>
#include <hunspell/hunspell.hxx>

namespace HunspellMorpho {

using namespace Dictionary;

using gd::wchar;

namespace {

class HunspellDictionary: public Dictionary::Class
{
  string name;
  Mutex hunspellMutex;
  Hunspell hunspell;
    
public:

  /// files[ 0 ] should be .aff file, files[ 1 ] should be .dic file.
  HunspellDictionary( string const & id, string const & name_,
                      vector< string > const & files ):
    Dictionary::Class( id, files ),
    name( name_ ),
    hunspell( files[ 0 ].c_str(), files[ 1 ].c_str() )
  {
  }
  
  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const &,
                                                 unsigned long maxResults )
    throw( std::exception );

  virtual sptr< WordSearchRequest > findHeadwordsForSynonym( wstring const & )
    throw( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &, vector< wstring > const & alts )
    throw( std::exception );
};

/// Encodes the given string to be passed to the hunspell object. May throw
/// Iconv::Ex
string encodeToHunspell( Hunspell &, wstring const & );

/// Decodes the given string returned by the hunspell object. May throw
/// Iconv::Ex
wstring decodeFromHunspell( Hunspell &, char const * );

/// Returns true if the string contains whitespace, false otherwise
bool containsWhitespace( wstring const & str )
{
  wchar const * next = str.c_str();

  for( ; *next; ++next )
    if ( Folding::isWhitespace( *next ) )
      return true;

  return false;
}

/// HunspellDictionary::getArticle()

class HunspellArticleRequest;

class HunspellArticleRequestRunnable: public QRunnable
{
  HunspellArticleRequest & r;
  QSemaphore & hasExited;
  
public:

  HunspellArticleRequestRunnable( HunspellArticleRequest & r_,
                                  QSemaphore & hasExited_ ): r( r_ ),
                                                             hasExited( hasExited_ )
  {}

  ~HunspellArticleRequestRunnable()
  {
    hasExited.release();
  }
  
  virtual void run();
};

class HunspellArticleRequest: public Dictionary::DataRequest
{
  friend class HunspellArticleRequestRunnable;

  Mutex & hunspellMutex;
  Hunspell & hunspell;
  wstring word;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  HunspellArticleRequest( wstring const & word_,
                          Mutex & hunspellMutex_,
                          Hunspell & hunspell_ ):
    hunspellMutex( hunspellMutex_ ),
    hunspell( hunspell_ ),
    word( word_ )
  {
    QThreadPool::globalInstance()->start(
      new HunspellArticleRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by HunspellArticleRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }
  
  ~HunspellArticleRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void HunspellArticleRequestRunnable::run()
{
  r.run();
}

void HunspellArticleRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  // We'd need to free this if it gets allocated and an exception shows up
  char ** suggestions = 0;
  int suggestionsCount = 0;

  try
  {
    wstring trimmedWord = Folding::trimWhitespaceOrPunct( word );

    if ( containsWhitespace( trimmedWord ) )
    {
      // For now we don't analyze whitespace-containing phrases
      finish();
      return;
    }

    Mutex::Lock _( hunspellMutex );

    string encodedWord = encodeToHunspell( hunspell, trimmedWord );

    if ( hunspell.spell( encodedWord.c_str() ) )
    {
      // Good word -- no spelling suggestions then.
      finish();
      return;
    }

    suggestionsCount = hunspell.suggest( &suggestions, encodedWord.c_str() );

    if ( suggestionsCount )
    {
      // There were some suggestions made for us. Make an appropriate output.

      string result = "<div class=\"gdspellsuggestion\">" + 
        Html::escape( QCoreApplication::translate( "Hunspell", "Spelling suggestions: " ).toUtf8().data() );

      wstring lowercasedWord = Folding::applySimpleCaseOnly( word );

      for( int x = 0; x < suggestionsCount; ++x )
      {
        wstring suggestion = decodeFromHunspell( hunspell, suggestions[ x ] );

        if ( Folding::applySimpleCaseOnly( suggestion ) == lowercasedWord )
        {
          // If among suggestions we see the same word just with the different
          // case, we botch the search -- our searches are case-insensitive, and
          // there's no need for suggestions on a good word.
          
          finish();

          hunspell.free_list( &suggestions, suggestionsCount );
          return;
        }
        string suggestionUtf8 = Utf8::encode( suggestion );

        result += "<a href=\"bword://";
        result += Html::escape( suggestionUtf8 ) + "\">";
        result += Html::escape( suggestionUtf8 ) + "</a>";

        if ( x != suggestionsCount - 1 )
          result += ", ";
      }

      result += "</div>";

      Mutex::Lock _( dataMutex );

      data.resize( result.size() );

      memcpy( &data.front(), result.data(), result.size() );

      hasAnyData = true;
    }
  }
  catch( Iconv::Ex & e )
  {
    printf( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }

  if ( suggestions )
  {
    Mutex::Lock _( hunspellMutex );

    hunspell.free_list( &suggestions, suggestionsCount );
  }

  finish();
}

sptr< DataRequest > HunspellDictionary::getArticle( wstring const & word, vector< wstring > const & )
  throw( std::exception )
{
  return new HunspellArticleRequest( word, hunspellMutex, hunspell );
}

/// HunspellDictionary::findHeadwordsForSynonym()

class HunspellHeadwordsRequest;

class HunspellHeadwordsRequestRunnable: public QRunnable
{
  HunspellHeadwordsRequest & r;
  QSemaphore & hasExited;
  
public:

  HunspellHeadwordsRequestRunnable( HunspellHeadwordsRequest & r_,
                                  QSemaphore & hasExited_ ): r( r_ ),
                                                             hasExited( hasExited_ )
  {}

  ~HunspellHeadwordsRequestRunnable()
  {
    hasExited.release();
  }
  
  virtual void run();
};

class HunspellHeadwordsRequest: public Dictionary::WordSearchRequest
{
  friend class HunspellHeadwordsRequestRunnable;

  Mutex & hunspellMutex;
  Hunspell & hunspell;
  wstring word;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  HunspellHeadwordsRequest( wstring const & word_,
                            Mutex & hunspellMutex_,
                            Hunspell & hunspell_ ):
    hunspellMutex( hunspellMutex_ ),
    hunspell( hunspell_ ),
    word( word_ )
  {
    QThreadPool::globalInstance()->start(
      new HunspellHeadwordsRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by HunspellHeadwordsRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }
  
  ~HunspellHeadwordsRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void HunspellHeadwordsRequestRunnable::run()
{
  r.run();
}

void HunspellHeadwordsRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  // We'd need to free this if it gets allocated and an exception shows up
  char ** suggestions = 0;
  int suggestionsCount = 0;

  try
  {
    wstring trimmedWord = Folding::trimWhitespaceOrPunct( word );

    if ( containsWhitespace( trimmedWord ) )
    {
      // For now we don't analyze whitespace-containing phrases
      finish();
      return;
    }

    Mutex::Lock _( hunspellMutex );

    string encodedWord = encodeToHunspell( hunspell, trimmedWord );

    suggestionsCount = hunspell.analyze( &suggestions, encodedWord.c_str() );

    if ( suggestionsCount )
    {
      // There were some suggestions made for us. Make an appropriate output.

      wstring lowercasedWord = Folding::applySimpleCaseOnly( word );

      QRegExp cutStem( "^\\s*st:(((\\s+(?!\\w{2}:))|\\S+)+)" );

      for( int x = 0; x < suggestionsCount; ++x )
      {
        QString suggestion = gd::toQString( decodeFromHunspell( hunspell, suggestions[ x ] ) );

        printf( ">>>Sugg: %s\n", suggestion.toLocal8Bit().data() );

        if ( cutStem.indexIn( suggestion ) != -1 )
        {
          wstring alt = gd::toWString( cutStem.cap( 1 ) );

          if ( Folding::applySimpleCaseOnly( alt ) != lowercasedWord ) // No point in providing same word
          {
            printf( ">>>>>Alt: %ls\n", alt.c_str() );

            Mutex::Lock _( dataMutex );

            matches.push_back( alt );
          }
        }
      }
    }
  }
  catch( Iconv::Ex & e )
  {
    printf( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }

  if ( suggestions )
  {
    Mutex::Lock _( hunspellMutex );

    hunspell.free_list( &suggestions, suggestionsCount );
  }

  finish();
}

sptr< WordSearchRequest > HunspellDictionary::findHeadwordsForSynonym( wstring const & word )
  throw( std::exception )
{
  return new HunspellHeadwordsRequest( word, hunspellMutex, hunspell );
}


/// HunspellDictionary::prefixMatch()

class HunspellPrefixMatchRequest;

class HunspellPrefixMatchRequestRunnable: public QRunnable
{
  HunspellPrefixMatchRequest & r;
  QSemaphore & hasExited;
  
public:

  HunspellPrefixMatchRequestRunnable( HunspellPrefixMatchRequest & r_,
                                      QSemaphore & hasExited_ ): r( r_ ),
                                                                 hasExited( hasExited_ )
  {}

  ~HunspellPrefixMatchRequestRunnable()
  {
    hasExited.release();
  }
  
  virtual void run();
};

class HunspellPrefixMatchRequest: public Dictionary::WordSearchRequest
{
  friend class HunspellPrefixMatchRequestRunnable;

  Mutex & hunspellMutex;
  Hunspell & hunspell;
  wstring word;

  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  HunspellPrefixMatchRequest( wstring const & word_,
                              Mutex & hunspellMutex_,
                              Hunspell & hunspell_ ):
    hunspellMutex( hunspellMutex_ ),
    hunspell( hunspell_ ),
    word( word_ )
  {
    QThreadPool::globalInstance()->start(
      new HunspellPrefixMatchRequestRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by HunspellPrefixMatchRequestRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }
  
  ~HunspellPrefixMatchRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void HunspellPrefixMatchRequestRunnable::run()
{
  r.run();
}

void HunspellPrefixMatchRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  try
  {
    wstring trimmedWord = Folding::trimWhitespaceOrPunct( word );

    if ( trimmedWord.empty() || containsWhitespace( trimmedWord ) )
    {
      // For now we don't analyze whitespace-containing phrases
      finish();
      return;
    }

    Mutex::Lock _( hunspellMutex );

    string encodedWord = encodeToHunspell( hunspell, trimmedWord );

    if ( hunspell.spell( encodedWord.c_str() ) )
    {
      // Known word -- add it to the result
      
      Mutex::Lock _( dataMutex );

      matches.push_back( WordMatch( trimmedWord, 1 ) );
    }
  }
  catch( Iconv::Ex & e )
  {
    printf( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }

  finish();
}

sptr< WordSearchRequest > HunspellDictionary::prefixMatch( wstring const & word,
                                                           unsigned long /*maxResults*/ )
  throw( std::exception )
{
  return new HunspellPrefixMatchRequest( word, hunspellMutex, hunspell );
}


string encodeToHunspell( Hunspell & hunspell, wstring const & str )
{
  Iconv conv( hunspell.get_dic_encoding(), Iconv::GdWchar );

  void const * in = str.data();
  size_t inLeft = str.size() * sizeof( wchar );

  vector< char > result( str.size() * 4 + 1 ); // +1 isn't actually needed,
                                               // but then iconv complains on empty
                                               // words

  void * out = &result.front();
  size_t outLeft = result.size();

  if ( conv.convert( in, inLeft, out, outLeft ) != Iconv::Success )
    throw Iconv::Ex();

  return string( &result.front(), result.size() - outLeft );
}

wstring decodeFromHunspell( Hunspell & hunspell, char const * str )
{
  Iconv conv( Iconv::GdWchar, hunspell.get_dic_encoding() );

  void const * in = str;
  size_t inLeft = strlen( str );

  vector< wchar > result( inLeft + 1 ); // +1 isn't needed, but see above

  void * out = &result.front();
  size_t outLeft = result.size() * sizeof( wchar );

  if ( conv.convert( in, inLeft, out, outLeft ) != Iconv::Success )
    throw Iconv::Ex();

  return wstring( &result.front(), result.size() - outLeft/sizeof( wchar ) );
}

}

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Hunspell const & cfg )
    throw( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  vector< DataFiles > dataFiles = findDataFiles( cfg.dictionariesPath );


  for( unsigned x = 0; x < cfg.enabledDictionaries.size(); ++x )
  {
    for( unsigned d = dataFiles.size(); d--; )
    {
      if ( dataFiles[ d ].dictId == cfg.enabledDictionaries[ x ] )
      {
        // Found it

        vector< string > dictFiles;

        dictFiles.push_back(
          QDir::toNativeSeparators( dataFiles[ d ].affFileName ).toLocal8Bit().data() );
        dictFiles.push_back(
          QDir::toNativeSeparators( dataFiles[ d ].dicFileName ).toLocal8Bit().data() );

        result.push_back(
          new HunspellDictionary( Dictionary::makeDictionaryId( dictFiles ),
                                  dataFiles[ d ].dictName.toUtf8().data(),
                                  dictFiles ) );
        break;
      }
    }
  }

  return result;
}

vector< DataFiles > findDataFiles( QString const & path )
{
  // Empty path means unconfigured directory
  if ( path.isEmpty() )
    return vector< DataFiles >();

  QDir dir( path );

  // Find all affix files

  QFileInfoList affixFiles = dir.entryInfoList( ( QStringList() << "*.aff" << "*.AFF" ), QDir::Files );

  vector< DataFiles > result;
  std::set< QString > presentNames;

  for( QFileInfoList::const_iterator i = affixFiles.constBegin();
       i != affixFiles.constEnd(); ++i )
  {
    QString affFileName = i->absoluteFilePath();

    // See if there's a corresponding .dic file
    QString dicFileNameBase = affFileName.mid( 0, affFileName.size() - 3 );

    QString dicFileName = dicFileNameBase + "dic";

    if ( !QFile( dicFileName ).exists() )
    {
      dicFileName = dicFileNameBase + "DIC";
      if ( !QFile( dicFileName ).exists() )
        continue; // No dic file
    }

    QString dictId = i->fileName();
    dictId.chop( 4 );

    QString dictBaseId = dictId.size() < 3 ? dictId :
      ( ( dictId[ 2 ] == '-' || dictId[ 2 ] == '_' ) ? dictId.mid( 0, 2 ) : QString() );

    dictBaseId = dictBaseId.toLower();

    // Try making up good readable name from dictBaseId

    QString dictName = dictId;

    bool readableNameSucceeded = true;

    #define TRY_LANG( code, translation ) if ( dictBaseId == ( code ) ) dictName = ( translation ); else

    // The following list was taken from the Wikipedia article
    // "List of ISO 639-1 codes"

    TRY_LANG( "aa", QCoreApplication::translate( "Hunspell", "Afar" ) )
    TRY_LANG( "ab", QCoreApplication::translate( "Hunspell", "Abkhazian" ) )
    TRY_LANG( "ae", QCoreApplication::translate( "Hunspell", "Avestan" ) )
    TRY_LANG( "af", QCoreApplication::translate( "Hunspell", "Afrikaans" ) )
    TRY_LANG( "ak", QCoreApplication::translate( "Hunspell", "Akan" ) )
    TRY_LANG( "am", QCoreApplication::translate( "Hunspell", "Amharic" ) )
    TRY_LANG( "an", QCoreApplication::translate( "Hunspell", "Aragonese" ) )
    TRY_LANG( "ar", QCoreApplication::translate( "Hunspell", "Arabic" ) )
    TRY_LANG( "as", QCoreApplication::translate( "Hunspell", "Assamese" ) )
    TRY_LANG( "av", QCoreApplication::translate( "Hunspell", "Avaric" ) )
    TRY_LANG( "ay", QCoreApplication::translate( "Hunspell", "Aymara" ) )
    TRY_LANG( "az", QCoreApplication::translate( "Hunspell", "Azerbaijani" ) )
    TRY_LANG( "ba", QCoreApplication::translate( "Hunspell", "Bashkir" ) )
    TRY_LANG( "be", QCoreApplication::translate( "Hunspell", "Belarusian" ) )
    TRY_LANG( "bg", QCoreApplication::translate( "Hunspell", "Bulgarian" ) )
    TRY_LANG( "bh", QCoreApplication::translate( "Hunspell", "Bihari" ) )
    TRY_LANG( "bi", QCoreApplication::translate( "Hunspell", "Bislama" ) )
    TRY_LANG( "bm", QCoreApplication::translate( "Hunspell", "Bambara" ) )
    TRY_LANG( "bn", QCoreApplication::translate( "Hunspell", "Bengali" ) )
    TRY_LANG( "bo", QCoreApplication::translate( "Hunspell", "Tibetan" ) )
    TRY_LANG( "br", QCoreApplication::translate( "Hunspell", "Breton" ) )
    TRY_LANG( "bs", QCoreApplication::translate( "Hunspell", "Bosnian" ) )
    TRY_LANG( "ca", QCoreApplication::translate( "Hunspell", "Catalan" ) )
    TRY_LANG( "ce", QCoreApplication::translate( "Hunspell", "Chechen" ) )
    TRY_LANG( "ch", QCoreApplication::translate( "Hunspell", "Chamorro" ) )
    TRY_LANG( "co", QCoreApplication::translate( "Hunspell", "Corsican" ) )
    TRY_LANG( "cr", QCoreApplication::translate( "Hunspell", "Cree" ) )
    TRY_LANG( "cs", QCoreApplication::translate( "Hunspell", "Czech" ) )
    TRY_LANG( "cu", QCoreApplication::translate( "Hunspell", "Church Slavic" ) )
    TRY_LANG( "cv", QCoreApplication::translate( "Hunspell", "Chuvash" ) )
    TRY_LANG( "cy", QCoreApplication::translate( "Hunspell", "Welsh" ) )
    TRY_LANG( "da", QCoreApplication::translate( "Hunspell", "Danish" ) )
    TRY_LANG( "de", QCoreApplication::translate( "Hunspell", "German" ) )
    TRY_LANG( "dv", QCoreApplication::translate( "Hunspell", "Divehi" ) )
    TRY_LANG( "dz", QCoreApplication::translate( "Hunspell", "Dzongkha" ) )
    TRY_LANG( "ee", QCoreApplication::translate( "Hunspell", "Ewe" ) )
    TRY_LANG( "el", QCoreApplication::translate( "Hunspell", "Greek" ) )
    TRY_LANG( "en", QCoreApplication::translate( "Hunspell", "English" ) )
    TRY_LANG( "eo", QCoreApplication::translate( "Hunspell", "Esperanto" ) )
    TRY_LANG( "es", QCoreApplication::translate( "Hunspell", "Spanish" ) )
    TRY_LANG( "et", QCoreApplication::translate( "Hunspell", "Estonian" ) )
    TRY_LANG( "eu", QCoreApplication::translate( "Hunspell", "Basque" ) )
    TRY_LANG( "fa", QCoreApplication::translate( "Hunspell", "Persian" ) )
    TRY_LANG( "ff", QCoreApplication::translate( "Hunspell", "Fulah" ) )
    TRY_LANG( "fi", QCoreApplication::translate( "Hunspell", "Finnish" ) )
    TRY_LANG( "fj", QCoreApplication::translate( "Hunspell", "Fijian" ) )
    TRY_LANG( "fo", QCoreApplication::translate( "Hunspell", "Faroese" ) )
    TRY_LANG( "fr", QCoreApplication::translate( "Hunspell", "French" ) )
    TRY_LANG( "fy", QCoreApplication::translate( "Hunspell", "Western Frisian" ) )
    TRY_LANG( "ga", QCoreApplication::translate( "Hunspell", "Irish" ) )
    TRY_LANG( "gd", QCoreApplication::translate( "Hunspell", "Scottish Gaelic" ) )
    TRY_LANG( "gl", QCoreApplication::translate( "Hunspell", "Galician" ) )
    TRY_LANG( "gn", QCoreApplication::translate( "Hunspell", "Guarani" ) )
    TRY_LANG( "gu", QCoreApplication::translate( "Hunspell", "Gujarati" ) )
    TRY_LANG( "gv", QCoreApplication::translate( "Hunspell", "Manx" ) )
    TRY_LANG( "ha", QCoreApplication::translate( "Hunspell", "Hausa" ) )
    TRY_LANG( "he", QCoreApplication::translate( "Hunspell", "Hebrew" ) )
    TRY_LANG( "hi", QCoreApplication::translate( "Hunspell", "Hindi" ) )
    TRY_LANG( "ho", QCoreApplication::translate( "Hunspell", "Hiri Motu" ) )
    TRY_LANG( "hr", QCoreApplication::translate( "Hunspell", "Croatian" ) )
    TRY_LANG( "ht", QCoreApplication::translate( "Hunspell", "Haitian" ) )
    TRY_LANG( "hu", QCoreApplication::translate( "Hunspell", "Hungarian" ) )
    TRY_LANG( "hy", QCoreApplication::translate( "Hunspell", "Armenian" ) )
    TRY_LANG( "hz", QCoreApplication::translate( "Hunspell", "Herero" ) )
    TRY_LANG( "ia", QCoreApplication::translate( "Hunspell", "Interlingua" ) )
    TRY_LANG( "id", QCoreApplication::translate( "Hunspell", "Indonesian" ) )
    TRY_LANG( "ie", QCoreApplication::translate( "Hunspell", "Interlingue" ) )
    TRY_LANG( "ig", QCoreApplication::translate( "Hunspell", "Igbo" ) )
    TRY_LANG( "ii", QCoreApplication::translate( "Hunspell", "Sichuan Yi" ) )
    TRY_LANG( "ik", QCoreApplication::translate( "Hunspell", "Inupiaq" ) )
    TRY_LANG( "io", QCoreApplication::translate( "Hunspell", "Ido" ) )
    TRY_LANG( "is", QCoreApplication::translate( "Hunspell", "Icelandic" ) )
    TRY_LANG( "it", QCoreApplication::translate( "Hunspell", "Italian" ) )
    TRY_LANG( "iu", QCoreApplication::translate( "Hunspell", "Inuktitut" ) )
    TRY_LANG( "ja", QCoreApplication::translate( "Hunspell", "Japanese" ) )
    TRY_LANG( "jv", QCoreApplication::translate( "Hunspell", "Javanese" ) )
    TRY_LANG( "ka", QCoreApplication::translate( "Hunspell", "Georgian" ) )
    TRY_LANG( "kg", QCoreApplication::translate( "Hunspell", "Kongo" ) )
    TRY_LANG( "ki", QCoreApplication::translate( "Hunspell", "Kikuyu" ) )
    TRY_LANG( "kj", QCoreApplication::translate( "Hunspell", "Kwanyama" ) )
    TRY_LANG( "kk", QCoreApplication::translate( "Hunspell", "Kazakh" ) )
    TRY_LANG( "kl", QCoreApplication::translate( "Hunspell", "Kalaallisut" ) )
    TRY_LANG( "km", QCoreApplication::translate( "Hunspell", "Khmer" ) )
    TRY_LANG( "kn", QCoreApplication::translate( "Hunspell", "Kannada" ) )
    TRY_LANG( "ko", QCoreApplication::translate( "Hunspell", "Korean" ) )
    TRY_LANG( "kr", QCoreApplication::translate( "Hunspell", "Kanuri" ) )
    TRY_LANG( "ks", QCoreApplication::translate( "Hunspell", "Kashmiri" ) )
    TRY_LANG( "ku", QCoreApplication::translate( "Hunspell", "Kurdish" ) )
    TRY_LANG( "kv", QCoreApplication::translate( "Hunspell", "Komi" ) )
    TRY_LANG( "kw", QCoreApplication::translate( "Hunspell", "Cornish" ) )
    TRY_LANG( "ky", QCoreApplication::translate( "Hunspell", "Kirghiz" ) )
    TRY_LANG( "la", QCoreApplication::translate( "Hunspell", "Latin" ) )
    TRY_LANG( "lb", QCoreApplication::translate( "Hunspell", "Luxembourgish" ) )
    TRY_LANG( "lg", QCoreApplication::translate( "Hunspell", "Ganda" ) )
    TRY_LANG( "li", QCoreApplication::translate( "Hunspell", "Limburgish" ) )
    TRY_LANG( "ln", QCoreApplication::translate( "Hunspell", "Lingala" ) )
    TRY_LANG( "lo", QCoreApplication::translate( "Hunspell", "Lao" ) )
    TRY_LANG( "lt", QCoreApplication::translate( "Hunspell", "Lithuanian" ) )
    TRY_LANG( "lu", QCoreApplication::translate( "Hunspell", "Luba-Katanga" ) )
    TRY_LANG( "lv", QCoreApplication::translate( "Hunspell", "Latvian" ) )
    TRY_LANG( "mg", QCoreApplication::translate( "Hunspell", "Malagasy" ) )
    TRY_LANG( "mh", QCoreApplication::translate( "Hunspell", "Marshallese" ) )
    TRY_LANG( "mi", QCoreApplication::translate( "Hunspell", "Maori" ) )
    TRY_LANG( "mk", QCoreApplication::translate( "Hunspell", "Macedonian" ) )
    TRY_LANG( "ml", QCoreApplication::translate( "Hunspell", "Malayalam" ) )
    TRY_LANG( "mn", QCoreApplication::translate( "Hunspell", "Mongolian" ) )
    TRY_LANG( "mr", QCoreApplication::translate( "Hunspell", "Marathi" ) )
    TRY_LANG( "ms", QCoreApplication::translate( "Hunspell", "Malay" ) )
    TRY_LANG( "mt", QCoreApplication::translate( "Hunspell", "Maltese" ) )
    TRY_LANG( "my", QCoreApplication::translate( "Hunspell", "Burmese" ) )
    TRY_LANG( "na", QCoreApplication::translate( "Hunspell", "Nauru" ) )
    TRY_LANG( "nb", QCoreApplication::translate( "Hunspell", "Norwegian Bokmal" ) )
    TRY_LANG( "nd", QCoreApplication::translate( "Hunspell", "North Ndebele" ) )
    TRY_LANG( "ne", QCoreApplication::translate( "Hunspell", "Nepali" ) )
    TRY_LANG( "ng", QCoreApplication::translate( "Hunspell", "Ndonga" ) )
    TRY_LANG( "nl", QCoreApplication::translate( "Hunspell", "Dutch" ) )
    TRY_LANG( "nn", QCoreApplication::translate( "Hunspell", "Norwegian Nynorsk" ) )
    TRY_LANG( "no", QCoreApplication::translate( "Hunspell", "Norwegian" ) )
    TRY_LANG( "nr", QCoreApplication::translate( "Hunspell", "South Ndebele" ) )
    TRY_LANG( "nv", QCoreApplication::translate( "Hunspell", "Navajo" ) )
    TRY_LANG( "ny", QCoreApplication::translate( "Hunspell", "Chichewa" ) )
    TRY_LANG( "oc", QCoreApplication::translate( "Hunspell", "Occitan" ) )
    TRY_LANG( "oj", QCoreApplication::translate( "Hunspell", "Ojibwa" ) )
    TRY_LANG( "om", QCoreApplication::translate( "Hunspell", "Oromo" ) )
    TRY_LANG( "or", QCoreApplication::translate( "Hunspell", "Oriya" ) )
    TRY_LANG( "os", QCoreApplication::translate( "Hunspell", "Ossetian" ) )
    TRY_LANG( "pa", QCoreApplication::translate( "Hunspell", "Panjabi" ) )
    TRY_LANG( "pi", QCoreApplication::translate( "Hunspell", "Pali" ) )
    TRY_LANG( "pl", QCoreApplication::translate( "Hunspell", "Polish" ) )
    TRY_LANG( "ps", QCoreApplication::translate( "Hunspell", "Pashto" ) )
    TRY_LANG( "pt", QCoreApplication::translate( "Hunspell", "Portuguese" ) )
    TRY_LANG( "qu", QCoreApplication::translate( "Hunspell", "Quechua" ) )
    TRY_LANG( "rm", QCoreApplication::translate( "Hunspell", "Raeto-Romance" ) )
    TRY_LANG( "rn", QCoreApplication::translate( "Hunspell", "Kirundi" ) )
    TRY_LANG( "ro", QCoreApplication::translate( "Hunspell", "Romanian" ) )
    TRY_LANG( "ru", QCoreApplication::translate( "Hunspell", "Russian" ) )
    TRY_LANG( "rw", QCoreApplication::translate( "Hunspell", "Kinyarwanda" ) )
    TRY_LANG( "sa", QCoreApplication::translate( "Hunspell", "Sanskrit" ) )
    TRY_LANG( "sc", QCoreApplication::translate( "Hunspell", "Sardinian" ) )
    TRY_LANG( "sd", QCoreApplication::translate( "Hunspell", "Sindhi" ) )
    TRY_LANG( "se", QCoreApplication::translate( "Hunspell", "Northern Sami" ) )
    TRY_LANG( "sg", QCoreApplication::translate( "Hunspell", "Sango" ) )
    TRY_LANG( "sh", QCoreApplication::translate( "Hunspell", "Serbo-Croatian" ) )
    TRY_LANG( "si", QCoreApplication::translate( "Hunspell", "Sinhala" ) )
    TRY_LANG( "sk", QCoreApplication::translate( "Hunspell", "Slovak" ) )
    TRY_LANG( "sl", QCoreApplication::translate( "Hunspell", "Slovenian" ) )
    TRY_LANG( "sm", QCoreApplication::translate( "Hunspell", "Samoan" ) )
    TRY_LANG( "sn", QCoreApplication::translate( "Hunspell", "Shona" ) )
    TRY_LANG( "so", QCoreApplication::translate( "Hunspell", "Somali" ) )
    TRY_LANG( "sq", QCoreApplication::translate( "Hunspell", "Albanian" ) )
    TRY_LANG( "sr", QCoreApplication::translate( "Hunspell", "Serbian" ) )
    TRY_LANG( "ss", QCoreApplication::translate( "Hunspell", "Swati" ) )
    TRY_LANG( "st", QCoreApplication::translate( "Hunspell", "Southern Sotho" ) )
    TRY_LANG( "su", QCoreApplication::translate( "Hunspell", "Sundanese" ) )
    TRY_LANG( "sv", QCoreApplication::translate( "Hunspell", "Swedish" ) )
    TRY_LANG( "sw", QCoreApplication::translate( "Hunspell", "Swahili" ) )
    TRY_LANG( "ta", QCoreApplication::translate( "Hunspell", "Tamil" ) )
    TRY_LANG( "te", QCoreApplication::translate( "Hunspell", "Telugu" ) )
    TRY_LANG( "tg", QCoreApplication::translate( "Hunspell", "Tajik" ) )
    TRY_LANG( "th", QCoreApplication::translate( "Hunspell", "Thai" ) )
    TRY_LANG( "ti", QCoreApplication::translate( "Hunspell", "Tigrinya" ) )
    TRY_LANG( "tk", QCoreApplication::translate( "Hunspell", "Turkmen" ) )
    TRY_LANG( "tl", QCoreApplication::translate( "Hunspell", "Tagalog" ) )
    TRY_LANG( "tn", QCoreApplication::translate( "Hunspell", "Tswana" ) )
    TRY_LANG( "to", QCoreApplication::translate( "Hunspell", "Tonga" ) )
    TRY_LANG( "tr", QCoreApplication::translate( "Hunspell", "Turkish" ) )
    TRY_LANG( "ts", QCoreApplication::translate( "Hunspell", "Tsonga" ) )
    TRY_LANG( "tt", QCoreApplication::translate( "Hunspell", "Tatar" ) )
    TRY_LANG( "tw", QCoreApplication::translate( "Hunspell", "Twi" ) )
    TRY_LANG( "ty", QCoreApplication::translate( "Hunspell", "Tahitian" ) )
    TRY_LANG( "ug", QCoreApplication::translate( "Hunspell", "Uighur" ) )
    TRY_LANG( "uk", QCoreApplication::translate( "Hunspell", "Ukrainian" ) )
    TRY_LANG( "ur", QCoreApplication::translate( "Hunspell", "Urdu" ) )
    TRY_LANG( "uz", QCoreApplication::translate( "Hunspell", "Uzbek" ) )
    TRY_LANG( "ve", QCoreApplication::translate( "Hunspell", "Venda" ) )
    TRY_LANG( "vi", QCoreApplication::translate( "Hunspell", "Vietnamese" ) )
    TRY_LANG( "vo", QCoreApplication::translate( "Hunspell", "Volapuk" ) )
    TRY_LANG( "wa", QCoreApplication::translate( "Hunspell", "Walloon" ) )
    TRY_LANG( "wo", QCoreApplication::translate( "Hunspell", "Wolof" ) )
    TRY_LANG( "xh", QCoreApplication::translate( "Hunspell", "Xhosa" ) )
    TRY_LANG( "yi", QCoreApplication::translate( "Hunspell", "Yiddish" ) )
    TRY_LANG( "yo", QCoreApplication::translate( "Hunspell", "Yoruba" ) )
    TRY_LANG( "za", QCoreApplication::translate( "Hunspell", "Zhuang" ) )
    TRY_LANG( "zh", QCoreApplication::translate( "Hunspell", "Chinese" ) )
    TRY_LANG( "zu", QCoreApplication::translate( "Hunspell", "Zulu" ) )
      readableNameSucceeded = false; // This ends the last else, so it's conditional

    if ( readableNameSucceeded )
    {
      if ( dictId.size() > 2 && ( dictId[ 2 ] == '-' || dictId[ 2 ] == '_' ) &&
           dictId.mid( 3 ).toLower() != dictBaseId )
        dictName += " (" + dictId.mid( 3 ) + ")";
    }

    dictName = QCoreApplication::translate( "Hunspell", "%1 Morphology" ).arg( dictName );

    if ( presentNames.insert( dictName ).second )
    {
      // Only include dictionaries with unique names. This combats stuff
      // like symlinks en-US->en_US and such
      
      result.push_back( DataFiles( affFileName, dicFileName, dictId, dictName ) );
    }
  }

  return result;
}

}
