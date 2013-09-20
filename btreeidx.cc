/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "btreeidx.hh"
#include "folding.hh"
#include "utf8.hh"
#include <QRunnable>
#include <QThreadPool>
#include <QSemaphore>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "dprintf.hh"
#include "wstring_qt.hh"

//#define __BTREE_USE_LZO
// LZO mode is experimental and unsupported. Tests didn't show any substantial
// speed improvements.

#ifdef __BTREE_USE_LZO
#include <lzo/lzo1x.h>

namespace {
struct __LzoInit
{
  __LzoInit()
  {
    lzo_init();
  }
} __lzoInit;
}

#else
#include <zlib.h>
#endif

namespace BtreeIndexing {

using gd::wstring;
using gd::wchar;

enum
{
  BtreeMinElements = 64,
  BtreeMaxElements = 2048
};

BtreeIndex::BtreeIndex():
  idxFile( 0 ), rootNodeLoaded( false )
{
}

BtreeDictionary::BtreeDictionary( string const & id,
                                  vector< string > const & dictionaryFiles ):
  Dictionary::Class( id, dictionaryFiles )
{
}

string const & BtreeDictionary::ensureInitDone()
{
  static string empty;

  return empty;
}

void BtreeIndex::openIndex( IndexInfo const & indexInfo,
                            File::Class & file, Mutex & mutex )
{
  indexNodeSize = indexInfo.btreeMaxElements;
  rootOffset = indexInfo.rootOffset;

  idxFile = &file;
  idxFileMutex = &mutex;

  rootNodeLoaded = false;
  rootNode.clear();
}

vector< WordArticleLink > BtreeIndex::findArticles( wstring const & str )
{
  vector< WordArticleLink > result;

  try
  {
    wstring folded = Folding::apply( str );
    if( folded.empty() )
      folded = Folding::applyWhitespaceOnly( str );

    bool exactMatch;

    vector< char > leaf;
    uint32_t nextLeaf;

    char const * leafEnd;

    char const * chainOffset = findChainOffsetExactOrPrefix( folded, exactMatch,
                                                             leaf, nextLeaf,
                                                             leafEnd );

    if ( chainOffset && exactMatch )
    {
      result = readChain( chainOffset );

      antialias( str, result );
    }
  }
  catch( std::exception & e )
  {
    qWarning( "Articles searching failed, error: %s\n", e.what() );
    result.clear();
  }
  catch(...)
  {
    qWarning( "Articles searching failed\n" );
    result.clear();
  }

  return result;
}

class BtreeWordSearchRequest;
  
class BtreeWordSearchRunnable: public QRunnable
{
  BtreeWordSearchRequest & r;
  QSemaphore & hasExited;
  
public:

  BtreeWordSearchRunnable( BtreeWordSearchRequest & r_,
                           QSemaphore & hasExited_ ): r( r_ ),
                                                      hasExited( hasExited_ )
  {}

  ~BtreeWordSearchRunnable()
  {
    hasExited.release();
  }
  
  virtual void run();
};

class BtreeWordSearchRequest: public Dictionary::WordSearchRequest
{
  friend class BtreeWordSearchRunnable;

  BtreeDictionary & dict;
  wstring str;
  unsigned long maxResults;
  unsigned minLength;
  int maxSuffixVariation;
  bool allowMiddleMatches;
  QAtomicInt isCancelled;
  QSemaphore hasExited;

public:

  BtreeWordSearchRequest( BtreeDictionary & dict_,
                          wstring const & str_,
                          unsigned minLength_,
                          int maxSuffixVariation_,
                          bool allowMiddleMatches_,
                          unsigned long maxResults_ ):
    dict( dict_ ), str( str_ ),
    maxResults( maxResults_ ),
    minLength( minLength_ ),
    maxSuffixVariation( maxSuffixVariation_ ),
    allowMiddleMatches( allowMiddleMatches_ )
  {
    QThreadPool::globalInstance()->start(
      new BtreeWordSearchRunnable( *this, hasExited ) );
  }

  void run(); // Run from another thread by BtreeWordSearchRunnable

  virtual void cancel()
  {
    isCancelled.ref();
  }
  
  ~BtreeWordSearchRequest()
  {
    isCancelled.ref();
    hasExited.acquire();
  }
};

void BtreeWordSearchRunnable::run()
{
  r.run();
}

void BtreeWordSearchRequest::run()
{
  if ( isCancelled )
  {
    finish();
    return;
  }

  if ( dict.ensureInitDone().size() )
  {
    setErrorString( QString::fromUtf8( dict.ensureInitDone().c_str() ) );
    finish();
    return;
  }
  
  wstring folded = Folding::apply( str );
  if( folded.empty() )
    folded = Folding::applyWhitespaceOnly( str );

  int initialFoldedSize = folded.size();

  int charsLeftToChop = 0;

  if ( maxSuffixVariation >= 0 )
  {
    charsLeftToChop = initialFoldedSize - (int)minLength;

    if ( charsLeftToChop < 0 )
      charsLeftToChop = 0;
    else
    if ( charsLeftToChop > maxSuffixVariation )
      charsLeftToChop = maxSuffixVariation;
  }

  try
  {
    for( ; ; )
    {
      bool exactMatch;

      vector< char > leaf;
      uint32_t nextLeaf;
      char const * leafEnd;

      char const * chainOffset = dict.findChainOffsetExactOrPrefix( folded, exactMatch,
                                                                    leaf, nextLeaf,
                                                                    leafEnd );

      if ( chainOffset )
      for( ; ; )
      {
        if ( isCancelled )
          break;

        //DPRINTF( "offset = %u, size = %u\n", chainOffset - &leaf.front(), leaf.size() );

        vector< WordArticleLink > chain = dict.readChain( chainOffset );

        wstring chainHead = Utf8::decode( chain[ 0 ].word );

        wstring resultFolded = Folding::apply( chainHead );
        if( resultFolded.empty() )
          resultFolded = Folding::applyWhitespaceOnly( chainHead );

        if ( resultFolded.size() >= folded.size() && !resultFolded.compare( 0, folded.size(), folded ) )
        {
          // Exact or prefix match

          Mutex::Lock _( dataMutex );

          for( unsigned x = 0; x < chain.size(); ++x )
          {
            // Skip middle matches, if requested. If suffix variation is specified,
            // make sure the string isn't larger than requested.
            if ( ( allowMiddleMatches || Folding::apply( Utf8::decode( chain[ x ].prefix ) ).empty() ) &&
                 ( maxSuffixVariation < 0 || (int)resultFolded.size() - initialFoldedSize <= maxSuffixVariation ) )
                matches.push_back( Utf8::decode( chain[ x ].prefix + chain[ x ].word ) );
          }

          if ( matches.size() >= maxResults )
          {
            // For now we actually allow more than maxResults if the last
            // chain yield more than one result. That's ok and maybe even more
            // desirable.
            break;
          }
        }
        else
          // Neither exact nor a prefix match, end this
          break;

        // Fetch new leaf if we're out of chains here

        if ( chainOffset >= leafEnd )
        {
          // We're past the current leaf, fetch the next one

          //DPRINTF( "advancing\n" );

          if ( nextLeaf )
          {
            Mutex::Lock _( *dict.idxFileMutex );

            dict.readNode( nextLeaf, leaf );
            leafEnd = &leaf.front() + leaf.size();

            nextLeaf = dict.idxFile->read< uint32_t >();
            chainOffset = &leaf.front() + sizeof( uint32_t );

            uint32_t leafEntries = *(uint32_t *)&leaf.front();

            if ( leafEntries == 0xffffFFFF )
            {
              //DPRINTF( "bah!\n" );
              exit( 1 );
            }
          }
          else
            break; // That was the last leaf
        }
      }

      if ( isCancelled )
        break;

      if ( charsLeftToChop && !isCancelled )
      {
        --charsLeftToChop;
        folded.resize( folded.size() - 1 );
      }
      else
        break;
    }
  }
  catch( std::exception & e )
  {
    qWarning( "Index searching failed: \"%s\", error: %s\n", e.what(),
              dict.getName().c_str() );
  }
  catch(...)
  {
    qWarning( "Index searching failed: \"%s\"\n", dict.getName().c_str() );
  }

  finish();
}

sptr< Dictionary::WordSearchRequest > BtreeDictionary::prefixMatch(
  wstring const & str, unsigned long maxResults )
  throw( std::exception )
{
  return new BtreeWordSearchRequest( *this, str, 0, -1, true, maxResults );
}

sptr< Dictionary::WordSearchRequest > BtreeDictionary::stemmedMatch(
  wstring const & str, unsigned minLength, unsigned maxSuffixVariation,
  unsigned long maxResults )
  throw( std::exception )
{
  return new BtreeWordSearchRequest( *this, str, minLength, (int)maxSuffixVariation,
                                     false, maxResults );
}

void BtreeIndex::readNode( uint32_t offset, vector< char > & out )
{
  idxFile->seek( offset );

  uint32_t uncompressedSize = idxFile->read< uint32_t >();
  uint32_t compressedSize = idxFile->read< uint32_t >();

  //DPRINTF( "%x,%x\n", uncompressedSize, compressedSize );

  out.resize( uncompressedSize );

  vector< unsigned char > compressedData( compressedSize );

  idxFile->read( &compressedData.front(), compressedData.size() );

  #ifdef __BTREE_USE_LZO

  lzo_uint decompressedLength = out.size();

  if ( lzo1x_decompress( &compressedData.front(), compressedData.size(),
                         (unsigned char *)&out.front(), &decompressedLength, 0 )
       != LZO_E_OK || decompressedLength != out.size() )
    throw exFailedToDecompressNode();

  #else

  unsigned long decompressedLength = out.size();

  if ( uncompress( (unsigned char *)&out.front(),
                   &decompressedLength,
                   &compressedData.front(),
                   compressedData.size() ) != Z_OK ||
       decompressedLength != out.size() )
    throw exFailedToDecompressNode();
  #endif
}

char const * BtreeIndex::findChainOffsetExactOrPrefix( wstring const & target,
                                                       bool & exactMatch,
                                                       vector< char > & extLeaf,
                                                       uint32_t & nextLeaf,
                                                       char const * & leafEnd )
{
  if ( !idxFile )
    throw exIndexWasNotOpened();
  
  Mutex::Lock _( *idxFileMutex );
  
  // Lookup the index by traversing the index btree

  vector< wchar > wcharBuffer;

  exactMatch = false;

  // Read a node

  uint32_t currentNodeOffset = rootOffset;

  if ( !rootNodeLoaded )
  {
    // Time to load our root node. We do it only once, at the first request.
    readNode( rootOffset, rootNode );
    rootNodeLoaded = true;
  }

  char const * leaf = &rootNode.front();
  leafEnd = leaf + rootNode.size();

  for( ; ; )
  {
    // Is it a leaf or a node?

    uint32_t leafEntries = *(uint32_t *)leaf;

    if ( leafEntries == 0xffffFFFF )
    {
      // A node

      //DPRINTF( "=>a node\n" );

      uint32_t const * offsets = (uint32_t *)leaf + 1;

      char const * ptr = leaf + sizeof( uint32_t ) +
                         ( indexNodeSize + 1 ) * sizeof( uint32_t );

      // ptr now points to a span of zero-separated strings, up to leafEnd.
      // We find our match using a binary search.

      char const * closestString;

      int compareResult;

      char const * window = ptr;
      unsigned windowSize = leafEnd - ptr;

      for( ; ; )
      {  
        // We boldly shoot in the middle of the whole mess, and then adjust
        // to the beginning of the string that we've hit.
        char const * testPoint = window + windowSize/2;
  
        closestString = testPoint;
  
        while( closestString > ptr && closestString[ -1 ] )
          --closestString;
  
        size_t wordSize = strlen( closestString );
  
        if ( wcharBuffer.size() <= wordSize )
          wcharBuffer.resize( wordSize + 1 );
  
        long result = Utf8::decode( closestString, wordSize, &wcharBuffer.front() );

        if ( result < 0 )
          throw Utf8::exCantDecode( closestString );
  
        wcharBuffer[ result ] = 0;

        //DPRINTF( "Checking against %s\n", closestString );

        compareResult = target.compare( &wcharBuffer.front() );
  
        if ( !compareResult )
        {
          // The target string matches the current one. Finish the search.
          break;
        }
        if ( compareResult < 0 )
        {
          // The target string is smaller than the current one.
          // Go to the left.
          windowSize = closestString - window;

          if ( !windowSize )
            break;
        }
        else
        {
          // The target string is larger than the current one.
          // Go to the right.
          windowSize -= ( closestString - window )  + wordSize + 1;
          window = closestString + wordSize + 1;

          if ( !windowSize )
            break;
        }
      }

      #if 0
      DPRINTF( "The winner is %s, compareResult = %d\n", closestString, compareResult );

      if ( closestString != ptr )
      {
        char const * left = closestString -1;

        while( left != ptr && left[ -1 ] )
          --left;

        DPRINTF( "To the left: %s\n", left );
      }
      else
        DPRINTF( "To the lest -- nothing\n" );

      char const * right = closestString + strlen( closestString ) + 1;

      if ( right != leafEnd )
      {
        DPRINTF( "To the right: %s\n", right );
      }
      else
        DPRINTF( "To the right -- nothing\n" );
      #endif

      // Now, whatever the outcome (compareResult) is, we need to find
      // entry number for the closestMatch string.
       
      unsigned entry = 0;

      for( char const * next = ptr; next != closestString;
           next += strlen( next ) + 1, ++entry ) ;

      // Ok, now check the outcome

      if ( !compareResult )
      {
        // The target string matches the one found.
        // Go to the right, since it's there where we store such results.
        currentNodeOffset = offsets[ entry + 1 ];
      }
      if ( compareResult < 0 )
      {
        // The target string is smaller than the one found.
        // Go to the left.
        currentNodeOffset = offsets[ entry ];
      }
      else
      {
        // The target string is larger than the one found.
        // Go to the right.
        currentNodeOffset = offsets[ entry + 1 ];
      }

      //DPRINTF( "reading node at %x\n", currentNodeOffset );
      readNode( currentNodeOffset, extLeaf );
      leaf = &extLeaf.front();
      leafEnd = leaf + extLeaf.size();
    }
    else
    {
      //DPRINTF( "=>a leaf\n" );
      // A leaf

      // If this leaf is the root, there's no next leaf, it just can't be.
      // We do this check because the file's position indicator just won't
      // be in the right place for root node anyway, since we precache it.
      nextLeaf = ( currentNodeOffset != rootOffset ? idxFile->read< uint32_t >() : 0 );

      if ( !leafEntries )
      {
        // Empty leaf? This may only be possible for entirely empty trees only.
        if ( currentNodeOffset != rootOffset )
          throw exCorruptedChainData();
        else
          return 0; // No match
      }

      // Build an array containing all chain pointers
      char const * ptr = leaf + sizeof( uint32_t );

      uint32_t chainSize;

      vector< char const * > chainOffsets( leafEntries );

      {
        char const ** nextOffset = &chainOffsets.front();

        while( leafEntries-- )
        {
          *nextOffset++ = ptr;

          memcpy( &chainSize, ptr, sizeof( uint32_t ) );

          //DPRINTF( "%s + %s\n", ptr + sizeof( uint32_t ), ptr + sizeof( uint32_t ) + strlen( ptr + sizeof( uint32_t ) ) + 1 );

          ptr += sizeof( uint32_t ) + chainSize;
        }
      }

      // Now do a binary search in it, aiming to find where our target
      // string lands.

      char const ** window = &chainOffsets.front();
      unsigned windowSize = chainOffsets.size();

      for( ; ; )
      {
        //DPRINTF( "window = %u, ws = %u\n", window - &chainOffsets.front(), windowSize );

        char const ** chainToCheck = window + windowSize/2;
        ptr = *chainToCheck;
  
        memcpy( &chainSize, ptr, sizeof( uint32_t ) );
        ptr += sizeof( uint32_t );
  
        size_t wordSize = strlen( ptr );
  
        if ( wcharBuffer.size() <= wordSize )
          wcharBuffer.resize( wordSize + 1 );
  
        //DPRINTF( "checking agaist word %s, left = %u\n", ptr, leafEntries );
  
        long result = Utf8::decode( ptr, wordSize, &wcharBuffer.front() );
  
        if ( result < 0 )
          throw Utf8::exCantDecode( ptr );
  
        wcharBuffer[ result ] = 0;
  
        wstring foldedWord = Folding::apply( &wcharBuffer.front() );
        if( foldedWord.empty() )
          foldedWord = Folding::applyWhitespaceOnly( &wcharBuffer.front() );
  
        int compareResult = target.compare( foldedWord );
  
        if ( !compareResult )
        {
          // Exact match -- return and be done
          exactMatch = true;
  
          return ptr - sizeof( uint32_t );
        }
        else
        if ( compareResult < 0 )
        {
          // The target string is smaller than the current one.
          // Go to the first half
           
          windowSize /= 2;

          if ( !windowSize )
          {
            // That finishes our search. Since our target string
            // landed before the last tested chain, we return a possible
            // prefix match against that chain.
            return ptr - sizeof( uint32_t );
          }
        }
        else
        {
          // The target string is larger than the current one.
          // Go to the second half

          windowSize -= windowSize/2 + 1;

          if ( !windowSize )
          {
            // That finishes our search. Since our target string
            // landed after the last tested chain, we return the next
            // chain. If there's no next chain in this leaf, this
            // would mean the first element in the next leaf.
            if ( chainToCheck == &chainOffsets.back() )
            {
              if ( nextLeaf )
              {
                readNode( nextLeaf, extLeaf );
  
                leafEnd = &extLeaf.front() + extLeaf.size();
  
                nextLeaf = idxFile->read< uint32_t >();
  
                return &extLeaf.front() + sizeof( uint32_t );
              }
              else
                return 0; // This was the last leaf
            }
            else
              return chainToCheck[ 1 ];
          }

          window = chainToCheck + 1;
        }
      }
    }
  }
}

vector< WordArticleLink > BtreeIndex::readChain( char const * & ptr )
{
  uint32_t chainSize;

  memcpy( &chainSize, ptr, sizeof( uint32_t ) );

  ptr += sizeof( uint32_t );

  vector< WordArticleLink > result;

  vector< char > charBuffer;

  while( chainSize )
  {
    string str = ptr;
    ptr += str.size() + 1;

    string prefix = ptr;
    ptr += prefix.size() + 1;

    uint32_t articleOffset;

    memcpy( &articleOffset, ptr, sizeof( uint32_t ) );

    ptr += sizeof( uint32_t );

    result.push_back( WordArticleLink( str, articleOffset, prefix ) );

    if ( chainSize < str.size() + 1 + prefix.size() + 1 + sizeof( uint32_t ) )
      throw exCorruptedChainData();
    else
      chainSize -= str.size() + 1 + prefix.size() + 1 + sizeof( uint32_t );
  }

  return result;
}

void BtreeIndex::antialias( wstring const & str,
                            vector< WordArticleLink > & chain )
{
  wstring caseFolded = Folding::applySimpleCaseOnly( gd::normalize( str ) );

  for( unsigned x = chain.size(); x--; )
  {
    // If after applying case folding to each word they wouldn't match, we
    // drop the entry.
    if ( Folding::applySimpleCaseOnly( gd::normalize( Utf8::decode( chain[ x ].prefix + chain[ x ].word ) ) ) !=
         caseFolded )
      chain.erase( chain.begin() + x );
    else
    if ( chain[ x ].prefix.size() ) // If there's a prefix, merge it with the word,
                                    // since it's what dictionaries expect
    {
      chain[ x ].word.insert( 0, chain[ x ].prefix );
      chain[ x ].prefix.clear();
    }
  }
}


/// A function which recursively creates btree node.
/// The nextIndex iterator is being iterated over and increased when building
/// leaf nodes.
static uint32_t buildBtreeNode( IndexedWords::const_iterator & nextIndex,
                                size_t indexSize,
                                File::Class & file, size_t maxElements,
                                uint32_t & lastLeafLinkOffset )
{
  // We compress all the node data. This buffer would hold it.
  vector< unsigned char > uncompressedData;

  bool isLeaf = indexSize <= maxElements;

  if ( isLeaf )
  {
    // A leaf.

    uint32_t totalChainsLength = 0;

    IndexedWords::const_iterator nextWord = nextIndex;

    for( unsigned x = indexSize; x--; ++nextWord )
    {
      totalChainsLength += sizeof( uint32_t );

      vector< WordArticleLink > const & chain = nextWord->second;

      for( unsigned y = 0; y < chain.size(); ++y )
        totalChainsLength += chain[ y ].word.size() + 1 + chain[ y ].prefix.size() + 1 + sizeof( uint32_t );
    }

    uncompressedData.resize( sizeof( uint32_t ) + totalChainsLength );

    // First uint32_t indicates that this is a leaf.
    *(uint32_t *)&uncompressedData.front() = indexSize;

    unsigned char * ptr = &uncompressedData.front() + sizeof( uint32_t );

    for( unsigned x = indexSize; x--; ++nextIndex )
    {
      vector< WordArticleLink > const & chain = nextIndex->second;

      unsigned char * saveSizeHere = ptr;

      ptr += sizeof( uint32_t );

      uint32_t size = 0;

      for( unsigned y = 0; y < chain.size(); ++y )
      {
        memcpy( ptr, chain[ y ].word.c_str(), chain[ y ].word.size() + 1 );
        ptr += chain[ y ].word.size() + 1;

        memcpy( ptr, chain[ y ].prefix.c_str(), chain[ y ].prefix.size() + 1 );
        ptr += chain[ y ].prefix.size() + 1;

        memcpy( ptr, &(chain[ y ].articleOffset), sizeof( uint32_t ) );
        ptr += sizeof( uint32_t );

        size += chain[ y ].word.size() + 1 + chain[ y ].prefix.size() + 1 + sizeof( uint32_t );
      }

      memcpy( saveSizeHere, &size, sizeof( uint32_t ) );
    }
  }
  else
  {
    // A node which will have children.

    uncompressedData.resize( sizeof( uint32_t ) + ( maxElements + 1 ) * sizeof( uint32_t ) );

    // First uint32_t indicates that this is a node.
    *(uint32_t *)&uncompressedData.front() = 0xffffFFFF;

    unsigned prevEntry = 0;

    for( unsigned x = 0; x < maxElements; ++x )
    {
      unsigned curEntry = (uint64_t) indexSize * ( x + 1 ) / ( maxElements + 1 );

      uint32_t offset = buildBtreeNode( nextIndex,
                                        curEntry - prevEntry,
                                        file, maxElements,
                                        lastLeafLinkOffset );

      memcpy( &uncompressedData.front() + sizeof( uint32_t ) + x * sizeof( uint32_t ), &offset, sizeof( uint32_t ) );

      size_t sz = nextIndex->first.size() + 1;

      size_t prevSize = uncompressedData.size();
      uncompressedData.resize( prevSize + sz );

      memcpy( &uncompressedData.front() + prevSize, nextIndex->first.c_str(),
              sz );

      prevEntry = curEntry;
    }

    // Rightmost child
    uint32_t offset = buildBtreeNode( nextIndex,
                                      indexSize - prevEntry,
                                      file, maxElements,
                                      lastLeafLinkOffset );
    memcpy( &uncompressedData.front() + sizeof( uint32_t ) +
            maxElements * sizeof( uint32_t ), &offset, sizeof( offset ) );
  }

  // Save the result.

  #ifdef __BTREE_USE_LZO

  vector< unsigned char > compressedData( uncompressedData.size() + uncompressedData.size() / 16 + 64 + 3 );

  char workMem[ LZO1X_1_MEM_COMPRESS ];

  lzo_uint compressedSize;

  if ( lzo1x_1_compress( &uncompressedData.front(), uncompressedData.size(),
                         &compressedData.front(), &compressedSize, workMem )
       != LZO_E_OK )
  {
    FDPRINTF( stderr, "Failed to compress btree node.\n" );
    abort();
  }

  #else

  vector< unsigned char > compressedData( compressBound( uncompressedData.size() ) );

  unsigned long compressedSize = compressedData.size();

  if ( compress( &compressedData.front(), &compressedSize,
                 &uncompressedData.front(), uncompressedData.size() ) != Z_OK )
  {
    qFatal( "Failed to compress btree node." );
    abort();
  }

  #endif

  uint32_t offset = file.tell();

  file.write< uint32_t >( uncompressedData.size() );
  file.write< uint32_t >( compressedSize );
  file.write( &compressedData.front(), compressedSize );

  if ( isLeaf )
  {
    // A link to the next leef, which is zero and which will be updated
    // should we happen to have another leaf.
    
    file.write( ( uint32_t ) 0 );

    uint32_t here = file.tell();

    if ( lastLeafLinkOffset )
    {
      // Update the previous leaf to have the offset of this one.
      file.seek( lastLeafLinkOffset );
      file.write( offset );
      file.seek( here );
    }

    // Make sure next leaf knows where to write its offset for us.
    lastLeafLinkOffset = here - sizeof( uint32_t );
  }

  return offset;
}

void IndexedWords::addWord( wstring const & word, uint32_t articleOffset, unsigned int maxHeadwordSize )
{
  wchar const * wordBegin = word.c_str();
  string::size_type wordSize = word.size();

  // Safeguard us against various bugs here. Don't attempt adding words
  // which are freakishly huge.
  if ( wordSize > maxHeadwordSize )
    return;

  // Skip any leading whitespace
  while( *wordBegin && Folding::isWhitespace( *wordBegin ) )
  {
    ++wordBegin;
    --wordSize;
  }

  // Skip any trailing whitespace
  while( wordSize && Folding::isWhitespace( wordBegin[ wordSize - 1 ] ) )
    --wordSize;

  wchar const * nextChar = wordBegin;

  vector< char > utfBuffer( wordSize * 4 );

  int wordsAdded = 0; // Number of stored parts

  for( ; ; )
  {
    // Skip any whitespace/punctuation
    for( ; ; ++nextChar )
    {
      if ( !*nextChar ) // End of string ends everything
      {
          if( wordsAdded == 0)
          {
              wstring folded = Folding::applyWhitespaceOnly( wstring( wordBegin, wordSize ) );
              if( !folded.empty() )
              {
                  iterator i = insert(
                    IndexedWords::value_type(
                      string( &utfBuffer.front(),
                              Utf8::encode( folded.data(), folded.size(), &utfBuffer.front() ) ),
                      vector< WordArticleLink >() ) ).first;

                  // Try to conserve memory somewhat -- slow insertions are ok
                  i->second.reserve( i->second.size() + 1 );

                  string utfWord( &utfBuffer.front(),
                                  Utf8::encode( wordBegin, wordSize, &utfBuffer.front() ) );
                  string utfPrefix;
                  i->second.push_back( WordArticleLink( utfWord, articleOffset, utfPrefix ) );
              }
          }
          return;
      }
  
      if ( !Folding::isWhitespace( *nextChar ) && !Folding::isPunct( *nextChar ) )
        break;
    }

    // Insert this word
    wstring folded = Folding::apply( nextChar );
    
    iterator i = insert(
      IndexedWords::value_type(
        string( &utfBuffer.front(),
                Utf8::encode( folded.data(), folded.size(), &utfBuffer.front() ) ),
        vector< WordArticleLink >() ) ).first;

    if ( ( i->second.size() < 1024 ) || ( nextChar == wordBegin ) ) // Don't overpopulate chains with middle matches
    {
      // Try to conserve memory somewhat -- slow insertions are ok
      i->second.reserve( i->second.size() + 1 );
  
      string utfWord( &utfBuffer.front(),
                      Utf8::encode( nextChar, wordSize - ( nextChar - wordBegin ), &utfBuffer.front() ) );
  
      string utfPrefix( &utfBuffer.front(),
                        Utf8::encode( wordBegin, nextChar - wordBegin, &utfBuffer.front() ) );
  
      i->second.push_back( WordArticleLink( utfWord, articleOffset, utfPrefix ) );
    }

    wordsAdded += 1;

    // Skip all non-whitespace/punctuation
    for( ++nextChar; ; ++nextChar )
    {
      if ( !*nextChar )
        return; // End of string ends everything

      if ( Folding::isWhitespace( *nextChar ) || Folding::isPunct( *nextChar ) )
        break;
    }
  }
}

void IndexedWords::addSingleWord( wstring const & word, uint32_t articleOffset )
{
  wstring folded = Folding::apply( word );
  if( folded.empty() )
      folded = Folding::applyWhitespaceOnly( word );
  operator []( Utf8::encode( folded ) ).push_back(
    WordArticleLink( Utf8::encode( word ), articleOffset ) );
}

IndexInfo buildIndex( IndexedWords const & indexedWords, File::Class & file )
{
  size_t indexSize = indexedWords.size();
  IndexedWords::const_iterator nextIndex = indexedWords.begin();

  // Skip any empty words. No point in indexing those, and some dictionaries
  // are known to have buggy empty-word entries (Stardict's jargon for instance).

  while( indexSize && nextIndex->first.empty() )
  {
    indexSize--;
    ++nextIndex;
  }

  // We try to stick to two-level tree for most dictionaries. Try finding
  // the right size for it.

  size_t btreeMaxElements = ( (size_t) sqrt( (double) indexSize ) ) + 1;

  if ( btreeMaxElements < BtreeMinElements )
    btreeMaxElements = BtreeMinElements;
  else
  if ( btreeMaxElements > BtreeMaxElements )
    btreeMaxElements = BtreeMaxElements;

  DPRINTF( "Building a tree of %u elements\n", (unsigned) btreeMaxElements );


  uint32_t lastLeafOffset = 0;

  uint32_t rootOffset = buildBtreeNode( nextIndex, indexSize,
                                        file, btreeMaxElements,
                                        lastLeafOffset );

  return IndexInfo( btreeMaxElements, rootOffset );
}

}
