/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __DICTIONARY_HH_INCLUDED__
#define __DICTIONARY_HH_INCLUDED__

#include <vector>
#include <string>
#include <map>
#include <QObject>
#include <QIcon>
#include "sptr.hh"
#include "ex.hh"
#include "mutex.hh"
#include "wstring.hh"
#include "langcoder.hh"

/// Abstract dictionary-related stuff
namespace Dictionary {

using std::vector;
using std::string;
using gd::wstring;
using std::map;

enum Property
{
  Author,
  Copyright,
  Description,
  Email
};

DEF_EX( Ex, "Dictionary error", std::exception )
DEF_EX( exIndexOutOfRange, "The supplied index is out of range", Ex )
DEF_EX( exSliceOutOfRange, "The requested data slice is out of range", Ex )
DEF_EX( exRequestUnfinished, "The request hasn't yet finished", Ex )

/// When you request a search to be performed in a dictionary, you get
/// this structure in return. It accumulates search results over time.
/// The finished() signal is emitted when the search has finished and there's
/// no more matches to be expected. Note that before connecting to it, check
/// the result of isFinished() -- if it's 'true', the search was instantaneous.
/// Destroy the object when you are not interested in results anymore.
///
/// Creating, destroying and calling member functions of the requests is done
/// in the GUI thread, however. Therefore, it is important to make sure those
/// operations are fast (this is most important for word searches, where
/// new requests are created and old ones deleted immediately upon a user
/// changing query).
class Request: public QObject
{
  Q_OBJECT

public:

  /// Returns whether the request has been processed in full and finished.
  /// This means that the data accumulated is final and won't change anymore.
  bool isFinished();

  /// Either returns an empty string in case there was no error processing
  /// the request, or otherwise a human-readable string describing the problem.
  /// Note that an empty result, such as a lack of word or of an article isn't
  /// an error -- but any kind of failure to connect to, or read the dictionary
  /// is.
  QString getErrorString();

  /// Cancels the ongoing request. This may make Request destruct faster some
  /// time in the future, Use this in preparation to destruct many Requests,
  /// so that they'd be cancelling in parallel. When the request was fully
  /// cancelled, it must emit the finished() signal, either as a result of an
  /// actual finish which has happened just before the cancellation, or solely as
  /// a result of a request being cancelled (in the latter case, the actual
  /// request result may be empty or incomplete). That is, finish() must be
  /// called by a derivative at least once if cancel() was called, either after
  /// or before it was called.
  virtual void cancel()=0;

  virtual ~Request()
  {}

signals:

  /// This signal is emitted when more data becomes available. Local
  /// dictionaries typically don't call this, since it is preferred that all
  /// data would be available from them at once, but network dictionaries
  /// might call that.
  void updated();

  /// This signal is emitted when the request has been processed in full and
  /// finished. That is, it's emitted when isFinished() turns true.
  void finished();

protected:

  /// Called by derivatives to signal update().
  void update();

  /// Called by derivatives to set isFinished() flag and signal finished().
  void finish();

  /// Sets the error string to be returned by getErrorString().
  void setErrorString( QString const & );

private:

  QAtomicInt isFinishedFlag;

  Mutex errorStringMutex;
  QString errorString;
};

/// This structure represents the word found. In addition to holding the
/// word itself, it also holds its weight. It is 0 by default. Negative
/// values should be used to store distance from Levenstein-like matching
/// algorithms. Positive values are used by morphology matches.
struct WordMatch
{
  wstring word;
  int weight;

  WordMatch(): weight( 0 ) {}
  WordMatch( wstring const & word_ ): word( word_ ), weight( 0 ){}
  WordMatch( wstring const & word_, int weight_ ): word( word_ ),
    weight( weight_ ) {}
};

/// This request type corresponds to all types of word searching operations.
class WordSearchRequest: public Request
{
  Q_OBJECT

public:

  WordSearchRequest(): uncertain( false )
  {}

  /// Returns the number of matches found. The value can grow over time
  /// unless isFinished() is true.
  size_t matchesCount();

  /// Returns the match with the given zero-based index, which should be less
  /// than matchesCount().
  WordMatch operator [] ( size_t index ) throw( exIndexOutOfRange );

  /// Returns all the matches found. Since no further locking can or would be
  /// done, this can only be called after the request has finished.
  vector< WordMatch > & getAllMatches() throw( exRequestUnfinished );

  /// Returns true if the match was uncertain -- that is, there may be more
  /// results in the dictionary itself, the dictionary index isn't good enough
  /// to tell that.
  bool isUncertain() const
  { return uncertain; }

protected:

  // Subclasses should be filling up the 'matches' array, locking the mutex when
  // whey work with it.
  Mutex dataMutex;

  vector< WordMatch > matches;
  bool uncertain;
};

/// This request type corresponds to any kinds of data responses where a
/// single large blob of binary data is returned. It currently used of article
/// bodies and resources.
class DataRequest: public Request
{
  Q_OBJECT

public:

  /// Returns the number of bytes read, with a -1 meaning that so far it's
  /// uncertain whether resource even exists or not, and any non-negative value
  /// meaning that that amount of bytes is not available.
  /// If -1 is still being returned after the request has finished, that means
  /// the resource wasn't found.
  long dataSize();

  /// Writes "size" bytes starting from "offset" of the data read to the given
  /// buffer. "size + offset" must be <= than dataSize().
  void getDataSlice( size_t offset, size_t size, void * buffer )
    throw( exSliceOutOfRange );

  /// Returns all the data read. Since no further locking can or would be
  /// done, this can only be called after the request has finished.
  vector< char > & getFullData() throw( exRequestUnfinished );

  DataRequest(): hasAnyData( false ) {}

protected:

  // Subclasses should be filling up the 'data' array, locking the mutex when
  // whey work with it.
  Mutex dataMutex;

  bool hasAnyData; // With this being false, dataSize() always returns -1
  vector< char > data;
};

/// A helper class for syncronous word search implementations.
class WordSearchRequestInstant: public WordSearchRequest
{
public:

  WordSearchRequestInstant()
  { finish(); }

  virtual void cancel()
  {}

  vector< WordMatch > & getMatches()
  { return matches; }

  void setUncertain( bool value )
  { uncertain = value; }
};

/// A helper class for syncronous data read implementations.
class DataRequestInstant: public DataRequest
{
public:

  DataRequestInstant( bool succeeded )
  { hasAnyData = succeeded; finish(); }

  DataRequestInstant( QString const & errorString )
  { setErrorString( errorString ); finish(); }

  virtual void cancel()
  {}

  vector< char > & getData()
  { return data; }
};

/// Dictionary features. Different dictionaries can possess different features,
/// which hint at some of their aspects.
enum Feature
{
  /// No features
  NoFeatures = 0,
  /// The dictionary is suitable to query when searching for compound expressions.
  SuitableForCompoundSearching = 1
};

Q_DECLARE_FLAGS( Features, Feature )
Q_DECLARE_OPERATORS_FOR_FLAGS( Features )

/// A dictionary. Can be used to query words.
class Class
{
  string id;
  vector< string > dictionaryFiles;

protected:
  QString dictionaryDescription;
  QIcon dictionaryIcon, dictionaryNativeIcon;
  bool dictionaryIconLoaded;

  // Load user icon if it exist
  // By default set icon to empty
  virtual void loadIcon() throw();

  // Load icon from filename directly if isFullName == true
  // else treat filename as name without extension
  bool loadIconFromFile( QString const & filename, bool isFullName = false );

  /// Make css content usable only for articles from this dictionary
  void isolateCSS( QString & css, QString const & wrapperSelector = QString() );

public:

  /// Creates a dictionary. The id should be made using
  /// Format::makeDictionaryId(), the dictionaryFiles is the file names the
  /// dictionary consists of.
  Class( string const & id, vector< string > const & dictionaryFiles );

  /// Called once after the dictionary is constructed. Usually called for each
  /// dictionaries once all dictionaries were made. The implementation should
  /// queue any initialization tasks the dictionary decided to postpone to
  /// threadpools, network requests etc, so the system could complete them
  /// in background.
  /// The default implementation does nothing.
  virtual void deferredInit();

  /// Returns the dictionary's id.
  string getId() throw()
  { return id; }

  /// Returns the list of file names the dictionary consists of.
  vector< string > const & getDictionaryFilenames() throw()
  { return dictionaryFiles; }

  /// Returns the dictionary's full name, utf8.
  virtual string getName() throw()=0;

  /// Returns all the available properties, like the author's name, copyright,
  /// description etc. All strings are in utf8.
  virtual map< Property, string > getProperties() throw()=0;

  /// Returns the features the dictionary possess. See the Feature enum for
  /// their list.
  virtual Features getFeatures() const throw()
  { return NoFeatures; }

  /// Returns the number of articles in the dictionary.
  virtual unsigned long getArticleCount() throw()=0;

  /// Returns the number of words in the dictionary. This can be equal to
  /// the number of articles, or can be larger if some synonyms are present.
  virtual unsigned long getWordCount() throw()=0;

  /// Returns the dictionary's icon.
  virtual QIcon const & getIcon() throw();

  /// Returns the dictionary's native icon. Dsl icons are usually rectangular,
  /// and are adapted by getIcon() to be square. This function allows getting
  /// the original icon with no geometry transformations applied.
  virtual QIcon const & getNativeIcon() throw();

  /// Returns the dictionary's source language.
  virtual quint32 getLangFrom() const
  { return 0; }

  /// Returns the dictionary's target language.
  virtual quint32 getLangTo() const
  { return 0; }

  /// Looks up a given word in the dictionary, aiming for exact matches and
  /// prefix matches. If it's not possible to locate any prefix matches, no
  /// prefix results should be added. Not more than maxResults results should
  /// be stored. The whole operation is supposed to be fast, though some
  /// dictionaries, the network ones particularly, may of course be slow.
  virtual sptr< WordSearchRequest > prefixMatch( wstring const &,
                                                 unsigned long maxResults ) throw( std::exception )=0;

  /// Looks up a given word in the dictionary, aiming to find different forms
  /// of the given word by allowing suffix variations. This means allowing words
  /// which can be as short as the input word size minus maxSuffixVariation, or as
  /// long as the input word size plus maxSuffixVariation, which share at least
  /// the input word size minus maxSuffixVariation initial symbols.
  /// Since the goal is to find forms of the words, no matches where a word
  /// in the middle of a phrase got matched should be returned.
  /// The default implementation does nothing, returning an empty result.
  virtual sptr< WordSearchRequest > stemmedMatch( wstring const &,
                                                  unsigned minLength,
                                                  unsigned maxSuffixVariation,
                                                  unsigned long maxResults ) throw( std::exception );

  /// Finds known headwords for the given word, that is, the words for which
  /// the given word is a synonym. If a dictionary can't perform this operation,
  /// it should leave the default implementation which always returns an empty
  /// result.
  virtual sptr< WordSearchRequest > findHeadwordsForSynonym( wstring const & )
    throw( std::exception );

  /// For a given word, provides alternate writings of it which are to be looked
  /// up alongside with it. Transliteration dictionaries implement this. The
  /// default implementation returns an empty list. Note that this function is
  /// supposed to be very fast and simple, and the results are thus returned
  /// syncronously.
  virtual vector< wstring > getAlternateWritings( wstring const & )
    throw();
  
  /// Returns a definition for the given word. The definition should
  /// be an html fragment (without html/head/body tags) in an utf8 encoding.
  /// The 'alts' vector could contain a list of words the definitions of which
  /// should be included in the output as well, being treated as additional
  /// synonyms for the main word.
  /// context is a dictionary-specific data, currently only used for the
  /// 'Websites' feature.
  virtual sptr< DataRequest > getArticle( wstring const &,
                                          vector< wstring > const & alts,
                                          wstring const & context = wstring() )
    throw( std::exception )=0;

  /// Loads contents of a resource named 'name' into the 'data' vector. This is
  /// usually a picture file referenced in the article or something like that.
  /// The default implementation always returns the non-existing resource
  /// response.
  virtual sptr< DataRequest > getResource( string const & /*name*/ )
    throw( std::exception );

  // Return dictionary description if presented
  virtual QString const& getDescription();

  // Return dictionary main file name
  virtual QString getMainFilename();

  /// Check text direction
  bool isFromLanguageRTL()
  { return LangCoder::isLanguageRTL( getLangFrom() ); }
  bool isToLanguageRTL()
  { return LangCoder::isLanguageRTL( getLangTo() ); }

  /// Return true if dictionary is local dictionary
  virtual bool isLocalDictionary()
  { return false; }

  virtual ~Class()
  {}
};

/// Callbacks to be used when the dictionaries are being initialized.
class Initializing
{
public:

  /// Called by the Format instance to notify the caller that the given
  /// dictionary is being indexed. Since indexing can take some time, this
  /// is useful to show in some kind of a splash screen.
  /// The dictionaryName is in utf8.
  virtual void indexingDictionary( string const & dictionaryName ) throw()=0;

  virtual ~Initializing()
  {}
};

/// Generates an id based on the set of file names which the dictionary
/// consists of. The resulting id is an alphanumeric hex value made by
/// hashing the file names. This id should be used to identify dictionary
/// and for the index file name, if one is needed.
/// This function is supposed to be used by dictionary implementations.
string makeDictionaryId( vector< string > const & dictionaryFiles ) throw();

/// Checks if it is needed to regenerate index file based on its timestamp
/// and the timestamps of the dictionary files. If some files are newer than
/// the index file, or the index file doesn't exist, returns true. If some
/// dictionary files don't exist, returns true, too.
/// This function is supposed to be used by dictionary implementations.
bool needToRebuildIndex( vector< string > const & dictionaryFiles,
                         string const & indexFile ) throw();

/// Returns a random dictionary id useful for interactively created
/// dictionaries.
QString generateRandomDictionaryId();

}

#endif

