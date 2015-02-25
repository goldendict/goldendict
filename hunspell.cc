/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "hunspell.hh"
#include "utf8.hh"
#include "htmlescape.hh"
#include "iconv.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include "language.hh"
#include "langcoder.hh"
#include <QRunnable>
#include <QThreadPool>
#include <QSemaphore>
#include <QRegExp>
#include <QDir>
#include <QCoreApplication>
#include <set>
#include <hunspell/hunspell.hxx>
#include "gddebug.hh"
#include "fsencoding.hh"
#include <QFileInfo>

namespace HunspellMorpho {

using namespace Dictionary;

using gd::wchar;

namespace {

class HunspellDictionary: public Dictionary::Class
{
  string name;
  Hunspell hunspell;

#ifdef Q_OS_WIN32
  static string Utf8ToLocal8Bit( string const & name )
  {
    return string( QString::fromUtf8( name.c_str() ).toLocal8Bit().data() );
  }
#endif

public:

  /// files[ 0 ] should be .aff file, files[ 1 ] should be .dic file.
  HunspellDictionary( string const & id, string const & name_,
                      vector< string > const & files ):
    Dictionary::Class( id, files ),
    name( name_ ),
#ifdef Q_OS_WIN32
    hunspell( Utf8ToLocal8Bit( files[ 0 ] ).c_str(), Utf8ToLocal8Bit( files[ 1 ] ).c_str() )
#else
    hunspell( files[ 0 ].c_str(), files[ 1 ].c_str() )
#endif
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

  virtual sptr< DataRequest > getArticle( wstring const &,
                                          vector< wstring > const & alts,
                                          wstring const & )
    throw( std::exception );

  virtual bool isLocalDictionary()
  { return true; }

  virtual vector< wstring > getAlternateWritings( const wstring & word ) throw();

protected:

  virtual void loadIcon() throw();

private:

  // We used to have a separate mutex for each Hunspell instance, assuming
  // that its code was reentrant (though probably not thread-safe). However,
  // crashes were discovered later when using several Hunspell dictionaries
  // simultaneously, and we've switched to have a single mutex for all hunspell
  // calls - evidently it's not really reentrant.
  static Mutex & getHunspellMutex()
  {
    static Mutex mutex;
    return mutex;
  }
//  Mutex hunspellMutex;
};

/// Encodes the given string to be passed to the hunspell object. May throw
/// Iconv::Ex
string encodeToHunspell( Hunspell &, wstring const & );

/// Decodes the given string returned by the hunspell object. May throw
/// Iconv::Ex
wstring decodeFromHunspell( Hunspell &, char const * );

/// Generates suggestions via hunspell
QVector< wstring > suggest( wstring & word, Mutex & hunspellMutex,
                            Hunspell & hunspell );

/// Generates suggestions for compound expression
void getSuggestionsForExpression( wstring const & expression,
                                  vector< wstring > & suggestions,
                                  Mutex & hunspellMutex,
                                  Hunspell & hunspell );

/// Returns true if the string contains whitespace, false otherwise
bool containsWhitespace( wstring const & str )
{
  wchar const * next = str.c_str();

  for( ; *next; ++next )
    if ( Folding::isWhitespace( *next ) )
      return true;

  return false;
}

void HunspellDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  QString fileName =
    QDir::fromNativeSeparators( FsEncoding::decode( getDictionaryFilenames()[ 0 ].c_str() ) );

  // Remove the extension
  fileName.chop( 3 );

  if( !loadIconFromFile( fileName ) )
  {
    // Load failed -- use default icons
    dictionaryNativeIcon = dictionaryIcon = QIcon(":/icons/icon32_hunspell.png");
  }

  dictionaryIconLoaded = true;
}

vector< wstring > HunspellDictionary::getAlternateWritings( wstring const & word ) throw()
{
  vector< wstring > results;

  if( containsWhitespace( word ) )
  {
    getSuggestionsForExpression( word, results, getHunspellMutex(), hunspell );
  }

  return results;
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

        result += "<a href=\"bword:";
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
    gdWarning( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }
  catch( std::exception & e )
  {
    gdWarning( "Hunspell: error: %s\n", e.what() );
  }

  if ( suggestions )
  {
    Mutex::Lock _( hunspellMutex );

    hunspell.free_list( &suggestions, suggestionsCount );
  }

  finish();
}

sptr< DataRequest > HunspellDictionary::getArticle( wstring const & word,
                                                    vector< wstring > const &,
                                                    wstring const & )
  throw( std::exception )
{
  return new HunspellArticleRequest( word, getHunspellMutex(), hunspell );
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

  wstring trimmedWord = Folding::trimWhitespaceOrPunct( word );

  if ( trimmedWord.size() > 80 )
  {
    // We won't do anything for overly long sentences since that would probably
    // only waste time.
    finish();
    return;
  }

  if ( containsWhitespace( trimmedWord ) )
  {
    vector< wstring > results;

    getSuggestionsForExpression( trimmedWord, results, hunspellMutex, hunspell );

    Mutex::Lock _( dataMutex );
    for( unsigned i = 0; i < results.size(); i++ )
      matches.push_back( results[ i ] );

  }
  else
  {
    QVector< wstring > suggestions = suggest( trimmedWord, hunspellMutex, hunspell );

    if ( !suggestions.empty() )
    {
      Mutex::Lock _( dataMutex );

      for( int x = 0; x < suggestions.size(); ++x )
        matches.push_back( suggestions[ x ] );
    }
  }

  finish();
}

QVector< wstring > suggest( wstring & word, Mutex & hunspellMutex, Hunspell & hunspell )
{
  QVector< wstring > result;

  // We'd need to free this if it gets allocated and an exception shows up
  char ** suggestions = 0;
  int suggestionsCount = 0;

  try
  {
    Mutex::Lock _( hunspellMutex );

    string encodedWord = encodeToHunspell( hunspell, word );

    suggestionsCount = hunspell.analyze( &suggestions, encodedWord.c_str() );

    if ( suggestionsCount )
    {
      // There were some suggestions made for us. Make an appropriate output.

      wstring lowercasedWord = Folding::applySimpleCaseOnly( word );

      static QRegExp cutStem( "^\\s*st:(((\\s+(?!\\w{2}:))|\\S+)+)" );

      for( int x = 0; x < suggestionsCount; ++x )
      {
        QString suggestion = gd::toQString( decodeFromHunspell( hunspell, suggestions[ x ] ) );

        GD_DPRINTF( ">>>Sugg: %s\n", suggestion.toLocal8Bit().data() );

        if ( cutStem.indexIn( suggestion ) != -1 )
        {
          wstring alt = gd::toWString( cutStem.cap( 1 ) );

          if ( Folding::applySimpleCaseOnly( alt ) != lowercasedWord ) // No point in providing same word
          {
#ifdef QT_DEBUG
            qDebug() << ">>>>>Alt:" << gd::toQString( alt );
#endif
            result.append( alt );
          }
        }
      }
    }
  }
  catch( Iconv::Ex & e )
  {
    gdWarning( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }

  if ( suggestions )
  {
    Mutex::Lock _( hunspellMutex );

    hunspell.free_list( &suggestions, suggestionsCount );
  }

  return result;
}


sptr< WordSearchRequest > HunspellDictionary::findHeadwordsForSynonym( wstring const & word )
  throw( std::exception )
{
  return new HunspellHeadwordsRequest( word, getHunspellMutex(), hunspell );
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
    gdWarning( "Hunspell: charset convertion error, no processing's done: %s\n", e.what() );
  }

  finish();
}

sptr< WordSearchRequest > HunspellDictionary::prefixMatch( wstring const & word,
                                                           unsigned long /*maxResults*/ )
  throw( std::exception )
{
  return new HunspellPrefixMatchRequest( word, getHunspellMutex(), hunspell );
}

void getSuggestionsForExpression( wstring const & expression,
                                  vector<wstring> & suggestions,
                                  Mutex & hunspellMutex,
                                  Hunspell & hunspell )
{
  // Analyze each word separately and use the first two suggestions, if any.
  // This is useful for compound expressions where some words is
  // in different form, e.g. "dozing off" -> "doze off".

  wstring trimmedWord = Folding::trimWhitespaceOrPunct( expression );
  wstring word, punct;
  QVector< wstring > words;

  suggestions.clear();

  // Parse string to separate words

  for( wchar const * c = trimmedWord.c_str(); ; ++c )
  {
    if ( !*c || Folding::isPunct( *c ) || Folding::isWhitespace( * c ) )
    {
      if ( word.size() )
      {
        words.push_back( word );
        word.clear();
      }
      if ( *c )
        punct.push_back( *c );
    }
    else
    {
      if( punct.size() )
      {
        words.push_back( punct );
        punct.clear();
      }
      if( *c )
        word.push_back( *c );
    }
    if( !*c )
      break;
  }

  if( words.size() > 21 )
  {
    // Too many words - no suggestions
    return;
  }

  // Combine result strings from suggestions

  QVector< wstring > results;

  for( int i = 0; i < words.size(); i++ )
  {
    word = words.at( i );
    if( Folding::isPunct( word[ 0 ] ) || Folding::isWhitespace( word[ 0 ] ) )
    {
      for( int j = 0; j < results.size(); j++ )
        results[ j ].append( word );
    }
    else
    {
      QVector< wstring > sugg = suggest( word, hunspellMutex, hunspell );
      int suggNum = sugg.size() + 1;
      if( suggNum > 3 )
        suggNum = 3;
      int resNum = results.size();
      wstring resultStr;

      if( resNum == 0 )
      {
        for( int k = 0; k < suggNum; k++ )
          results.push_back( k == 0 ? word : sugg.at( k - 1 ) );
      }
      else
      {
        for( int j = 0; j < resNum; j++ )
        {
          resultStr = results.at( j );
          for( int k = 0; k < suggNum; k++ )
          {
            if( k == 0)
              results[ j ].append( word );
            else
              results.push_back( resultStr + sugg.at( k - 1 ) );
          }
        }
      }
    }
  }

  for( int i = 0; i < results.size(); i++ )
    if( results.at( i ) != trimmedWord )
      suggestions.push_back( results.at( i ) );
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


  for( int x = 0; x < cfg.enabledDictionaries.size(); ++x )
  {
    for( unsigned d = dataFiles.size(); d--; )
    {
      if ( dataFiles[ d ].dictId == cfg.enabledDictionaries[ x ] )
      {
        // Found it

        vector< string > dictFiles;

        dictFiles.push_back(
          FsEncoding::encode( QDir::toNativeSeparators( dataFiles[ d ].affFileName ) ) );
        dictFiles.push_back(
          FsEncoding::encode( QDir::toNativeSeparators( dataFiles[ d ].dicFileName ) ) );

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

    QString localizedName;

    if ( dictBaseId.size() == 2 )
      localizedName = Language::localizedNameForId( LangCoder::code2toInt( dictBaseId.toLatin1().data() ) );

    QString dictName = dictId;

    if ( localizedName.size() )
    {
      dictName = localizedName;

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
