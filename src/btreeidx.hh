/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __BTREEIDX_HH_INCLUDED__
#define __BTREEIDX_HH_INCLUDED__

#include "dictionary.hh"
#include "file.hh"
#include <string>
#include <vector>
#include <map>

/// A base for the dictionary which creates a btree index to look up
/// the words.
namespace BtreeIndexing {

using std::string;
using std::wstring;
using std::vector;
using std::map;

enum
{
  /// This is to be bumped up each time the internal format changes.
  /// The value isn't used here by itself, it is supposed to be added
  /// to each dictionary's internal format version.
  FormatVersion = 1
};

// These exceptions which might be thrown during the index traversal

DEF_EX( exIndexWasNotOpened, "The index wasn't opened", Dictionary::Ex )
DEF_EX( exFailedToDecompressNode, "Failed to decompress a btree's node", Dictionary::Ex )
DEF_EX( exCorruptedChainData, "Corrupted chain data in the leaf of a btree encountered", Dictionary::Ex )

/// This structure describes a word linked to its translation. The
/// translation is represented as an abstract 32-bit offset.
struct WordArticleLink
{
  string word; // in utf8
  uint32_t articleOffset;

  WordArticleLink()
  {}

  WordArticleLink( string const & word_, uint32_t articleOffset_ ):
    word( word_ ), articleOffset( articleOffset_ )
  {}
};

/// A base for the dictionary that utilizes a btree index build using
/// buildIndex() function declared below.
class BtreeDictionary: public Dictionary::Class
{
public:

  BtreeDictionary( string const & id, vector< string > const & dictionaryFiles );

  /// This function does the search using the btree index. Derivatives
  /// need not to implement this function.
  virtual void findExact( wstring const &,
                          vector< wstring > &,
                          vector< wstring > &,
                          unsigned long ) throw( std::exception );

protected:

  /// Opens the index. The file must be positioned at the offset previously
  /// returned by buildIndex(). The file reference is saved to be used for
  /// subsequent lookups.
  void openIndex( File::Class & );

  /// Finds articles that match the given string. A case-insensitive search
  /// is performed.
  vector< WordArticleLink > findArticles( wstring const & );

private:

  File::Class * idxFile;
  uint32_t indexNodeSize;
  uint32_t rootOffset;

  /// Finds the offset in the btree leaf for the given word, either matching
  /// by an exact match, or by finding the smallest entry that might match
  /// by prefix. It can return zero if there isn't even a possible prefx
  /// match. The input string must already be folded. The exactMatch is set
  /// to true when an exact match is located, and to false otherwise.
  /// The located leaf is loaded to 'leaf', and the pointer to the next
  /// leaf is saved to 'nextLeaf'.
  char const * findChainOffsetExactOrPrefix( wstring const & target,
                                             bool & exactMatch,
                                             vector< char > & leaf,
                                             uint32_t & nextLeaf );

  /// Reads a node or leaf at the given offset. Just uncompresses its data
  /// to the given vector and does nothing more.
  void readNode( uint32_t offset, vector< char > & out );

  /// Reads the word-article links' chain at the given offset. The pointer
  /// is updated to point to the next chain, if there's any.
  vector< WordArticleLink > readChain( char const * & );

  /// Converts words in a chain to a vector of wide strings. The article
  /// offsets don't get used.
  vector< wstring > convertChainToWstrings( vector< WordArticleLink > const & );

  /// Drops any alises which arose due to folding. Only case-folded aliases
  /// are left.
  void antialias( wstring const &, vector< WordArticleLink > & );
};

// Everything below is for building the index data.

/// This represents the index in its source form, as a map which binds folded
/// words to sequences of their unfolded source forms and the corresponding
/// article offsets.
typedef map< wstring, vector< WordArticleLink > > IndexedWords;


/// Builds the index, as a compressed btree. Returns offset to its root.
/// All the data is stored to the given file, beginning from its current
/// position.
uint32_t buildIndex( IndexedWords const &, File::Class & file );

}

#endif

