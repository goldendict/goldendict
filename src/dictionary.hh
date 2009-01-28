/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __DICTIONARY_HH_INCLUDED__
#define __DICTIONARY_HH_INCLUDED__

#include <vector>
#include <string>
#include <map>
#include "sptr.hh"
#include "ex.hh"

/// Abstract dictionary-related stuff
namespace Dictionary {

using std::vector;
using std::string;
using std::wstring;
using std::map;

enum Property
{
  Author,
  Copyright,
  Description,
  Email
};

DEF_EX( Ex, "Dictionary error", std::exception )
DEF_EX( exNoSuchWord, "The given word does not exist", Ex )
DEF_EX( exNoSuchResource, "The given resource does not exist", Ex )

/// A dictionary. Can be used to query words.
class Class
{
  string id;
  vector< string > dictionaryFiles;

public:

  /// Creates a dictionary. The id should be made using
  /// Format::makeDictionaryId(), the dictionaryFiles is the file names the
  /// dictionary consists of.
  Class( string const & id, vector< string > const & dictionaryFiles );

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

  /// Returns the number of articles in the dictionary.
  virtual unsigned long getArticleCount() throw()=0;

  /// Returns the number of words in the dictionary. This can be equal to
  /// the number of articles, or can be larger if some synonyms are present.
  virtual unsigned long getWordCount() throw()=0;

  /// Looks up a given word in the dictionary, aiming for exact matches. The
  /// result is a list of such matches. If it is possible to also look up words
  /// that begin with the given substring without much expense, they should be
  /// put into the prefix results (if not, it should be left empty). Not more
  /// than maxPrefixResults prefix results should be stored. The whole
  /// operation is supposed to be fast and is executed in a GUI thread.
  virtual void findExact( wstring const &,
                          vector< wstring > & exactMatches,
                          vector< wstring > & prefixMatches,
                          unsigned long maxPrefixResults ) throw( std::exception )=0;

  /// Finds known headwords for the given word, that is, the words for which
  /// the given word is a synonym. If a dictionary can't perform this operation,
  /// it should leave the default implementation which always returns an empty
  /// vector.
  virtual vector< wstring > findHeadwordsForSynonym( wstring const & )
    throw( std::exception )
  { return vector< wstring >(); }

  /// Returns a definition for the given word. The definition should
  /// be an html fragment (without html/head/body tags) in an utf8 encoding.
  /// The 'alts' vector could contain a list of words the definitions of which
  /// should be included in the output as well, being treated as additional
  /// synonyms for the main word.
  virtual string getArticle( wstring const &, vector< wstring > const & alts )
    throw( exNoSuchWord, std::exception )=0;

  /// Loads contents of a resource named 'name' into the 'data' vector. This is
  /// usually a picture file referenced in the article or something like that.
  virtual void getResource( string const & name,
                            vector< char > & data ) throw( exNoSuchResource,
                                                           std::exception )
  { throw exNoSuchResource(); }

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

/// A dictionary format. This is a factory to create dictionaries' instances.
/// It is fed filenames to check if they are dictionaries, and it creates
/// instances when they are.
class Format
{
public:

  /// Should go through the given list of file names, trying each one as a
  /// possible dictionary of the supported format. Upon finding one, creates a
  /// corresponding dictionary instance. As a result, a list of dictionaries
  /// is created.
  /// indicesDir indicates a directory where index files can be created, should
  /// there be need for them. The index file name must be the same as the
  /// dictionary's id, made by makeDictionaryId() from the list of file names.
  /// Any exception thrown would terminate the program with an error.
  virtual vector< sptr< Class > > makeDictionaries( vector< string > const & fileNames,
                                                    string const & indicesDir,
                                                    Initializing & )
    throw( std::exception )=0;

  virtual ~Format()
  {}

public://protected:

  /// Generates an id based on the set of file names which the dictionary
  /// consists of. The resulting id is an alphanumeric hex value made by
  /// hashing the file names. This id should be used to identify dictionary
  /// and for the index file name, if one is needed.
  static string makeDictionaryId( vector< string > const & dictionaryFiles ) throw();
  /// Checks if it is needed to regenerate index file based on its timestamp
  /// and the timestamps of the dictionary files. If some files are newer than
  /// the index file, or the index file doesn't exist, returns true. If some
  /// dictionary files don't exist, returns true, too.
  static bool needToRebuildIndex( vector< string > const & dictionaryFiles,
                                  string const & indexFile ) throw();
};

}

#endif

