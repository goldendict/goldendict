/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "wordfinder.hh"
#include "folding.hh"
#include "wstring_qt.hh"
#include <QThreadPool>
#include <map>
#include "gddebug.hh"

using std::vector;
using std::list;
using gd::wstring;
using gd::wchar;
using std::map;
using std::pair;

WordFinder::WordFinder( QObject * parent ):
  QObject( parent ), searchInProgress( false ),
  updateResultsTimer( this ),
  searchQueued( false )
{
  updateResultsTimer.setInterval( 1000 ); // We use a one second update timer
  updateResultsTimer.setSingleShot( true );

  connect( &updateResultsTimer, SIGNAL( timeout() ),
           this, SLOT( updateResults() ), Qt::QueuedConnection );
}

WordFinder::~WordFinder()
{
  clear();
}

void WordFinder::prefixMatch( QString const & str,
                              std::vector< sptr< Dictionary::Class > > const & dicts,
                              unsigned long maxResults,
                              Dictionary::Features features )
{
  cancel();

  searchQueued = true;
  searchType = PrefixMatch;
  inputWord = str;
  inputDicts = &dicts;
  requestedMaxResults = maxResults;
  requestedFeatures = features;

  resultsArray.clear();
  resultsIndex.clear();
  searchResults.clear();

  if ( queuedRequests.empty() )
  {
    // No requests are queued, no need to wait for them to finish.
    startSearch();
  }

  // Else some requests are still queued, last one to finish would trigger
  // new search. This shouldn't take a lot of time, since they were all
  // cancelled, but still it could take some time.
}
void WordFinder::stemmedMatch( QString const & str,
                               std::vector< sptr< Dictionary::Class > > const & dicts,
                               unsigned minLength,
                               unsigned maxSuffixVariation,
                               unsigned long maxResults,
                               Dictionary::Features features )
{
  cancel();

  searchQueued = true;
  searchType = StemmedMatch;
  inputWord = str;
  inputDicts = &dicts;
  requestedMaxResults = maxResults;
  requestedFeatures = features;
  stemmedMinLength = minLength;
  stemmedMaxSuffixVariation = maxSuffixVariation;

  resultsArray.clear();
  resultsIndex.clear();
  searchResults.clear();

  if ( queuedRequests.empty() )
    startSearch();
}

void WordFinder::expressionMatch( QString const & str,
                                  std::vector< sptr< Dictionary::Class > > const & dicts,
                                  unsigned long maxResults,
                                  Dictionary::Features features )
{
  cancel();

  searchQueued = true;
  searchType = ExpressionMatch;
  inputWord = str;
  inputDicts = &dicts;
  requestedMaxResults = maxResults;
  requestedFeatures = features;

  resultsArray.clear();
  resultsIndex.clear();
  searchResults.clear();

  if ( queuedRequests.empty() )
  {
    // No requests are queued, no need to wait for them to finish.
    startSearch();
  }
}

void WordFinder::startSearch()
{
  if ( !searchQueued )
    return; // Search was probably cancelled

  // Clear the requests just in case
  queuedRequests.clear();
  finishedRequests.clear();

  searchErrorString.clear();
  searchResultsUncertain = false;

  searchQueued = false;
  searchInProgress = true;

  // Gather all writings of the word

  if ( allWordWritings.size() != 1 )
    allWordWritings.resize( 1 );
  
  allWordWritings[ 0 ] = gd::toWString( inputWord );

  for( size_t x = 0; x < inputDicts->size(); ++x )
  {
    vector< wstring > writings = (*inputDicts)[ x ]->getAlternateWritings( allWordWritings[ 0 ] );

    allWordWritings.insert( allWordWritings.end(), writings.begin(), writings.end() );
  }

  // Query each dictionary for all word writings

  for( size_t x = 0; x < inputDicts->size(); ++x )
  {
    if ( ( (*inputDicts)[ x ]->getFeatures() & requestedFeatures ) != requestedFeatures )
      continue;

    for( size_t y = 0; y < allWordWritings.size(); ++y )
    {
      try
      {
        sptr< Dictionary::WordSearchRequest > sr =
          ( searchType == PrefixMatch || searchType == ExpressionMatch ) ?
            (*inputDicts)[ x ]->prefixMatch( allWordWritings[ y ], requestedMaxResults ) :
            (*inputDicts)[ x ]->stemmedMatch( allWordWritings[ y ], stemmedMinLength, stemmedMaxSuffixVariation, requestedMaxResults );

        connect( sr.get(), SIGNAL( finished() ),
                 this, SLOT( requestFinished() ), Qt::QueuedConnection );

        queuedRequests.push_back( sr );
      }
      catch( std::exception & e )
      {
        gdWarning( "Word \"%s\" search error (%s) in \"%s\"\n",
                   inputWord.toUtf8().data(), e.what(), (*inputDicts)[ x ]->getName().c_str() );
      }
    }
  }

  // Handle any requests finished already

  requestFinished();
}

void WordFinder::cancel()
{
  searchQueued = false;
  searchInProgress = false;
  
  cancelSearches();
}

void WordFinder::clear()
{
  cancel();
  queuedRequests.clear();
  finishedRequests.clear();
}

void WordFinder::requestFinished()
{
  bool newResults = false;

  // See how many new requests have finished, and if we have any new results
  for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
         queuedRequests.begin(); i != queuedRequests.end(); )
  {
    if ( (*i)->isFinished() )
    {
      if ( searchInProgress && !(*i)->getErrorString().isEmpty() )
        searchErrorString = tr( "Failed to query some dictionaries." );

      if ( (*i)->isUncertain() )
        searchResultsUncertain = true;

      if ( (*i)->matchesCount() )
      {
        newResults = true;

        // This list is handled by updateResults()
        finishedRequests.splice( finishedRequests.end(), queuedRequests, i++ );
      }
      else // We won't do anything with it anymore, so we erase it
        queuedRequests.erase( i++ );
    }
    else
      ++i;
  }

  if ( !searchInProgress )
  {
    // There is no search in progress, so we just wait until there's
    // no requests left
    
    if ( queuedRequests.empty() )
    {
      // We got rid of all queries, queued search can now start
      finishedRequests.clear();
  
      if ( searchQueued )
        startSearch();
    }

    return;
  }

  if ( newResults && queuedRequests.size() && !updateResultsTimer.isActive() )
  {
    // If we have got some new results, but not all of them, we would start a
    // timer to update a user some time in the future
    updateResultsTimer.start();
  }

  if ( queuedRequests.empty() )
  {
    // Search is finished.
    updateResults();
  }
}

namespace {


unsigned saturated( unsigned x )
{
  return x < 255 ? x : 255;
}

/// Checks whether the first string has the second one inside, surrounded from
/// both sides by either whitespace, punctuation or begin/end of string.
/// If true is returned, pos holds the offset in the haystack. If the offset
/// is larger than 255, it is set to 255.
bool hasSurroundedWithWs( wstring const & haystack, wstring const & needle,
                          wstring::size_type & pos )
{
  if ( haystack.size() < needle.size() )
    return false; // Needle won't even fit into a haystack

  for( pos = 0; ; ++pos )
  {
    pos = haystack.find( needle, pos );
  
    if ( pos == wstring::npos )
      return false; // Not found
  
    if ( ( !pos || Folding::isWhitespace( haystack[ pos - 1 ] ) ||
           Folding::isPunct( haystack[ pos - 1 ] ) ) &&
         ( ( pos + needle.size() == haystack.size() ) ||
           Folding::isWhitespace( haystack[ pos + needle.size() ] ) ||
           Folding::isPunct( haystack[ pos + needle.size() ] ) ) )
    {
      pos = saturated( pos );

      return true;
    }
  }
}

}

void WordFinder::updateResults()
{
  if ( !searchInProgress )
    return; // Old queued signal

  if ( updateResultsTimer.isActive() )
    updateResultsTimer.stop(); // Can happen when we were done before it'd expire

  wstring original = Folding::applySimpleCaseOnly( allWordWritings[ 0 ] );

  for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
         finishedRequests.begin(); i != finishedRequests.end(); )
  {
    for( size_t count = (*i)->matchesCount(), x = 0; x < count; ++x )
    {
      wstring match = (**i)[ x ].word;
      int weight = (**i)[ x ].weight;
      wstring lowerCased = Folding::applySimpleCaseOnly( match );

      if( searchType == ExpressionMatch )
      {
        unsigned ws;

        for( ws = 0; ws < allWordWritings.size(); ws++ )
        {
          if( ws == 0 )
          {
            // Check for prefix match with original expression
            if( lowerCased.compare( 0, original.size(), original ) == 0 )
              break;
          }
          else
          if( lowerCased == Folding::applySimpleCaseOnly( allWordWritings[ ws ] ) )
            break;
        }

        if( ws >= allWordWritings.size() )
        {
          // No exact matches found
          continue;
        }
        weight = ws;
      }
      pair< ResultsIndex::iterator, bool > insertResult =
        resultsIndex.insert( pair< wstring, ResultsArray::iterator >( lowerCased,
                                                                      resultsArray.end() ) );

      if ( !insertResult.second )
      {
        // Wasn't inserted since there was already an item -- check the case
        if ( insertResult.first->second->word != match )
        {
          // The case is different -- agree on a lowercase version
          insertResult.first->second->word = lowerCased;
        }
        if ( !weight && insertResult.first->second->wasSuggested )
          insertResult.first->second->wasSuggested = false;
      }
      else
      {
        resultsArray.push_back( OneResult() );

        resultsArray.back().word = match;
        resultsArray.back().rank = INT_MAX;
        resultsArray.back().wasSuggested = ( weight != 0 );

        insertResult.first->second = --resultsArray.end();
      }
    }
    finishedRequests.erase( i++ );
  }

  size_t maxSearchResults = 500;

  if ( resultsArray.size() )
  {
    if ( searchType == PrefixMatch )
    {
      /// Assign each result a category, storing it in the rank's field
  
      enum Category
      {
        ExactMatch,
        ExactNoFullCaseMatch,
        ExactNoDiaMatch,
        ExactNoPunctMatch,
        ExactNoWsMatch,
        ExactInsideMatch,
        ExactNoDiaInsideMatch,
        ExactNoPunctInsideMatch,
        PrefixMatch,
        PrefixNoDiaMatch,
        PrefixNoPunctMatch,
        PrefixNoWsMatch,
        WorstMatch,
        Multiplier = 256 // Categories should be multiplied by Multiplier
      };

      for( unsigned wr = 0; wr < allWordWritings.size(); ++wr )
      {
        wstring target = Folding::applySimpleCaseOnly( allWordWritings[ wr ] );
        wstring targetNoFullCase = Folding::applyFullCaseOnly( target );
        wstring targetNoDia = Folding::applyDiacriticsOnly( targetNoFullCase );
        wstring targetNoPunct = Folding::applyPunctOnly( targetNoDia );
        wstring targetNoWs = Folding::applyWhitespaceOnly( targetNoPunct );
    
        wstring::size_type matchPos = 0;
    
        for( ResultsIndex::const_iterator i = resultsIndex.begin(), j = resultsIndex.end();
             i != j; ++i )
        {
          wstring resultNoFullCase, resultNoDia, resultNoPunct, resultNoWs;

          int rank;
          
          if ( i->first == target )
            rank = ExactMatch * Multiplier;
          else
          if ( ( resultNoFullCase = Folding::applyFullCaseOnly( i->first ) ) == targetNoFullCase )
            rank = ExactNoFullCaseMatch * Multiplier;
          else
          if ( ( resultNoDia = Folding::applyDiacriticsOnly( resultNoFullCase ) ) == targetNoDia )
            rank = ExactNoDiaMatch * Multiplier;
          else
          if ( ( resultNoPunct = Folding::applyPunctOnly( resultNoDia ) ) == targetNoPunct )
            rank = ExactNoPunctMatch * Multiplier;
          else
          if ( ( resultNoWs = Folding::applyWhitespaceOnly( resultNoPunct ) ) == targetNoWs )
            rank = ExactNoWsMatch * Multiplier;
          else
          if ( hasSurroundedWithWs( i->first, target, matchPos ) )
            rank = ExactInsideMatch * Multiplier + matchPos;
          else
          if ( hasSurroundedWithWs( resultNoDia, targetNoDia, matchPos ) )
            rank = ExactNoDiaInsideMatch * Multiplier + matchPos;
          else
          if ( hasSurroundedWithWs( resultNoPunct, targetNoPunct, matchPos ) )
            rank = ExactNoPunctInsideMatch * Multiplier + matchPos;
          else
          if ( i->first.size() > target.size() && i->first.compare( 0, target.size(), target ) == 0 )
            rank = PrefixMatch * Multiplier + saturated( i->first.size() );
          else
          if ( resultNoDia.size() > targetNoDia.size() && resultNoDia.compare( 0, targetNoDia.size(), targetNoDia ) == 0 )
            rank = PrefixNoDiaMatch * Multiplier + saturated( i->first.size() );
          else
          if ( resultNoPunct.size() > targetNoPunct.size() && resultNoPunct.compare( 0, targetNoPunct.size(), targetNoPunct ) == 0 )
            rank = PrefixNoPunctMatch * Multiplier + saturated( i->first.size() );
          else
          if ( resultNoWs.size() > targetNoWs.size() && resultNoWs.compare( 0, targetNoWs.size(), targetNoWs ) == 0 )
            rank = PrefixNoWsMatch * Multiplier + saturated( i->first.size() );
          else
            rank = WorstMatch * Multiplier;

          if ( i->second->rank > rank )
            i->second->rank = rank; // We store the best rank of any writing
        }
      }
  
      resultsArray.sort( SortByRank() );
    }
    else
    if( searchType == StemmedMatch )
    {
      // Handling stemmed matches

      // We use two factors -- first is the number of characters strings share
      // in their beginnings, and second, the length of the strings. Here we assign
      // only the first one, storing it in rank. Then we sort the results using
      // SortByRankAndLength.
      for( unsigned wr = 0; wr < allWordWritings.size(); ++wr )
      {
        wstring target = Folding::apply( allWordWritings[ wr ] );
  
        for( ResultsIndex::const_iterator i = resultsIndex.begin(), j = resultsIndex.end();
             i != j; ++i )
        {
          wstring resultFolded = Folding::apply( i->first );
  
          int charsInCommon = 0;
  
          for( wchar const * t = target.c_str(), * r = resultFolded.c_str();
               *t && *t == *r; ++t, ++r, ++charsInCommon ) ;
  
          int rank = -charsInCommon; // Negated so the lesser-than
                                     // comparison would yield right
                                     // results.

          if ( i->second->rank > rank )
            i->second->rank = rank; // We store the best rank of any writing
        }
      }
      
      resultsArray.sort( SortByRankAndLength() );

      maxSearchResults = 15;
    }
  }

  searchResults.clear();
  searchResults.reserve( resultsArray.size() < maxSearchResults ? resultsArray.size() : maxSearchResults );

  for( ResultsArray::const_iterator i = resultsArray.begin(), j = resultsArray.end();
       i != j; ++i )
  {
    //DPRINTF( "%d: %ls\n", i->second, i->first.c_str() );

    if ( searchResults.size() < maxSearchResults )
      searchResults.push_back( std::pair< QString, bool >( gd::toQString( i->word ), i->wasSuggested ) );
    else
      break;
  }

  if ( queuedRequests.size() )
  {
    // There are still some unhandled results.
    emit updated();
  }
  else
  {
    // That were all of them.
    searchInProgress = false;
    emit finished();
  }
}

void WordFinder::cancelSearches()
{
  for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
         queuedRequests.begin(); i != queuedRequests.end(); ++i )
    (*i)->cancel();
}

