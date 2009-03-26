/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "wordfinder.hh"
#include "folding.hh"
#include <QThreadPool>
#include <map>

using std::vector;
using std::list;
using std::wstring;
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
           this, SLOT( updateResults() ) );
}

WordFinder::~WordFinder()
{
  clear();
}

void WordFinder::prefixMatch( QString const & str,
                              std::vector< sptr< Dictionary::Class > > const & dicts )
{
  cancel();

  searchQueued = true;
  inputWord = str;
  inputDicts = &dicts;

  results.clear();
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

void WordFinder::startSearch()
{
  if ( !searchQueued )
    return; // Search was probably cancelled

  // Clear the requests just in case
  queuedRequests.clear();
  finishedRequests.clear();

  searchErrorString.clear();

  searchQueued = false;
  searchInProgress = true;

  wstring word = inputWord.toStdWString();
  
  for( size_t x = 0; x < inputDicts->size(); ++x )
  {
    sptr< Dictionary::WordSearchRequest > sr = (*inputDicts)[ x ]->prefixMatch( word, 40 );

    connect( sr.get(), SIGNAL( finished() ),
             this, SLOT( requestFinished() ), Qt::QueuedConnection );

    queuedRequests.push_back( sr );
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

void WordFinder::updateResults()
{
  if ( !searchInProgress )
    return; // Old queued signal

  if ( updateResultsTimer.isActive() )
    updateResultsTimer.stop(); // Can happen when we were done before it'd expire

  for( list< sptr< Dictionary::WordSearchRequest > >::iterator i =
         finishedRequests.begin(); i != finishedRequests.end(); )
  {
    for( size_t count = (*i)->matchesCount(), x = 0; x < count; ++x )
    {
      wstring match = (**i)[ x ].word;
      wstring lowerCased = Folding::applySimpleCaseOnly( match );

      pair< map< wstring, wstring >::iterator, bool > insertResult =
        results.insert( pair< wstring, wstring >( lowerCased, match ) );

      if ( !insertResult.second )
      {
        // Wasn't inserted since there was already an item -- check the case
        if ( insertResult.first->second != match )
        {
          // The case is different -- agree on a lowercase version
          insertResult.first->second = lowerCased;
        }
      }
    }
    finishedRequests.erase( i++ );
  }

  // Do any sort of collation here in the future. For now we just put the
  // strings sorted by the map.

  searchResults.clear();
  searchResults.reserve( results.size() );

  for( map< wstring, wstring >::const_iterator i = results.begin();
       i != results.end(); ++i )
  {
    if ( searchResults.size() < 500 )
      searchResults.push_back( QString::fromStdWString( i->second ) );
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

