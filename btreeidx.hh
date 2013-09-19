/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __BTREEIDX_HH_INCLUDED__
#define __BTREEIDX_HH_INCLUDED__

#include "dictionary.hh"
#include "file.hh"

#include <string>
#include <vector>
#include <map>

#ifdef _MSC_VER
#include <stdint_msvc.h>
#else
#include <stdint.h>
#endif

/// A base for the dictionary which creates a btree index to look up
/// the words.
namespace BtreeIndexing {

using std::string;
using gd::wstring;
using std::vector;
using std::map;

enum
{
  /// This is to be bumped up each time the internal format changes.
  /// The value isn't used here by itself, it is supposed to be added
  /// to each dictionary's internal format version.
  FormatVersion = 4
};

// These exceptions which might be thrown during the index traversal

DEF_EX( exIndexWasNotOpened, "The index wasn't opened", Dictionary::Ex )
DEF_EX( exFailedToDecompressNode, "Failed to decompress a btree's node", Dictionary::Ex )
DEF_EX( exCorruptedChainData, "Corrupted chain data in the leaf of a btree encountered", Dictionary::Ex )

/// This structure describes a word linked to its translation. The
/// translation is represented as an abstract 32-bit offset.
struct WordArticleLink
{
  string word, prefix; // in utf8
  uint32_t articleOffset;

  WordArticleLink()
  {}

  WordArticleLink( string const & word_, uint32_t articleOffset_, string const & prefix_ = string() ):
    word( word_ ), prefix( prefix_ ), articleOffset( articleOffset_ )
  {}
};

/// Information needed to open the index
struct IndexInfo
{
  uint32_t btreeMaxElements, rootOffset;

  IndexInfo( uint32_t btreeMaxElements_, uint32_t rootOffset_ ):
    btreeMaxElements( btreeMaxElements_ ), rootOffset( rootOffset_ )
  {}
};

/// Base btree indexing class which allows using what buildIndex() function
/// created. It's quite low-lovel and is basically a set of 'bulding blocks'
/// functions.
class BtreeIndex
{
public:

  BtreeIndex();

  /// Opens the index. The file reference is saved to be used for
  /// subsequent lookups.
  /// The mutex is the one to be locked when working with the file.
  void openIndex( IndexInfo const &, File::Class &, Mutex & );

  /// Finds articles that match the given string. A case-insensitive search
  /// is performed.
  vector< WordArticleLink > findArticles( wstring const & );

protected:

  /// Finds the offset in the btree leaf for the given word, either matching
  /// by an exact match, or by finding the smallest entry that might match
  /// by prefix. It can return zero if there isn't even a possible prefx
  /// match. The input string must already be folded. The exactMatch is set
  /// to true when an exact match is located, and to false otherwise.
  /// The located leaf is loaded to 'leaf', and the pointer to the next
  /// leaf is saved to 'nextLeaf'.
  /// However, due to root node being permanently cached, the 'leaf' passed
  /// might not get used at all if the root node was the terminal one. In that
  /// case, the returned pointer wouldn't belong to 'leaf' at all. To that end,
  /// the leafEnd pointer always holds the pointer to the first byte outside
  /// the node data.
  char const * findChainOffsetExactOrPrefix( wstring const & target,
                                             bool & exactMatch,
                                             vector< char > & leaf,
                                             uint32_t & nextLeaf,
                                             char const * & leafEnd );

  /// Reads a node or leaf at the given offset. Just uncompresses its data
  /// to the given vector and does nothing more.
  void readNode( uint32_t offset, vector< char > & out );

  /// Reads the word-article links' chain at the given offset. The pointer
  /// is updated to point to the next chain, if there's any.
  vector< WordArticleLink > readChain( char const * & );

  /// Drops any alises which arose due to folding. Only case-folded aliases
  /// are left.
  void antialias( wstring const &, vector< WordArticleLink > & );

protected:

  Mutex * idxFileMutex;
  File::Class * idxFile;

private:

  uint32_t indexNodeSize;
  uint32_t rootOffset;
  bool rootNodeLoaded;
  vector< char > rootNode; // We load root note here and keep it at all times,
                           // since all searches always start with it.
};

class BtreeWordSearchRequest;

/// A base for the dictionary that utilizes a btree index build using
/// buildIndex() function declared below.
class BtreeDictionary: public Dictionary::Class, public BtreeIndex
{
public:

  BtreeDictionary( string const & id, vector< string > const & dictionaryFiles );

  /// Btree-indexed dictionaries are usually a good source for compound searches.
  virtual Dictionary::Features getFeatures() const throw()
  { return Dictionary::SuitableForCompoundSearching; }

  /// This function does the search using the btree index. Derivatives
  /// need not to implement this function.
  virtual sptr< Dictionary::WordSearchRequest > prefixMatch( wstring const &,
                                                             unsigned long )
    throw( std::exception );

  virtual sptr< Dictionary::WordSearchRequest > stemmedMatch( wstring const &,
                                                              unsigned minLength,
                                                              unsigned maxSuffixVariation,
                                                              unsigned long maxResults )
    throw( std::exception );

  virtual bool isLocalDictionary()
  { return true; }

protected:

  /// Called before each matching operation to ensure that any child init
  /// has completed. Mainly used for deferred init. The default implementation
  /// does nothing.
  /// The function returns an empty string if the initialization is or was
  /// successful, or a human-readable error string otherwise.
  virtual string const & ensureInitDone();

  friend class BtreeWordSearchRequest;
};

// Everything below is for building the index data.

/// This represents the index in its source form, as a map which binds folded
/// words to sequences of their unfolded source forms and the corresponding
/// article offsets. The words are utf8-encoded -- it doesn't break Unicode
/// sorting, but conserves space.
struct IndexedWords: public map< string, vector< WordArticleLink > >
{
  /// Instead of adding to the map directly, use this function. It does folding
  /// itself, and for phrases/sentences it adds additional entries beginning with
  /// each new word.
  void addWord( wstring const & word, uint32_t articleOffset, unsigned int maxHeadwordSize = 256U );

  /// Differs from addWord() in that it only adds a single entry. We use this
  /// for zip's file names.
  void addSingleWord( wstring const & word, uint32_t articleOffset );
};

/// Builds the index, as a compressed btree. Returns IndexInfo.
/// All the data is stored to the given file, beginning from its current
/// position.
IndexInfo buildIndex( IndexedWords const &, File::Class & file );

}

#endif

