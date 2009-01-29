/* This file is (c) 2008-2009 Konstantin Isakov <ikm@users.sf.net>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "wordfinder.hh"
#include "dictlock.hh"
#include "folding.hh"
#include <QMetaType>
#include <map>

using std::vector;
using std::wstring;
using std::map;
using std::pair;

namespace
{
  struct MetaTypeRegister
  {
    MetaTypeRegister()
    {
      qRegisterMetaType< WordFinderResults >( "WordFinderResults" );
    }
  };
}

WordFinder::WordFinder( QObject * parent ): QThread( parent ), op( NoOp )
{
  static MetaTypeRegister _;

  start();
}

WordFinder::~WordFinder()
{
  // Request termination and wait for it to happen
  opMutex.lock();

  op = QuitOp;

  opCondition.wakeOne();

  opMutex.unlock();

  wait();
}

void WordFinder::prefixMatch( QString const & str,
                              std::vector< sptr< Dictionary::Class > > const * dicts )
{
  opMutex.lock();

  op = DoPrefixMatch;
  prefixMatchString = str;
  prefixMatchDicts = dicts;

  opCondition.wakeOne();

  opMutex.unlock();
}

void WordFinder::run()
{
  opMutex.lock();

  for( ; ; )
  {
    // Check the operation requested

    if ( op == NoOp )
    {
      opCondition.wait( &opMutex );
      continue;
    }

    if ( op == QuitOp )
      break;

    // The only one op value left is DoPrefixMatch

    Q_ASSERT( op == DoPrefixMatch );

    QString prefixMatchReq = prefixMatchString;
    vector< sptr< Dictionary::Class > > const & activeDicts = *prefixMatchDicts;

    op = NoOp;

    opMutex.unlock();

    map< wstring, wstring > exactResults, prefixResults;

    {
      wstring word = prefixMatchReq.toStdWString();

      DictLock _;
  
      // Maps lowercased string to the original one. This catches all duplicates
      // without case sensitivity
  
      bool cancel = false;
  
      for( unsigned x = 0; x < activeDicts.size(); ++x )
      {
        vector< wstring > exactMatches, prefixMatches;
  
        activeDicts[ x ]->findExact( word, exactMatches, prefixMatches, 40 );
  
        for( unsigned y = 0; y < exactMatches.size(); ++y )
        {
          wstring lowerCased = Folding::applySimpleCaseOnly( exactMatches[ y ] );
  
          pair< map< wstring, wstring >::iterator, bool > insertResult =
            exactResults.insert( pair< wstring, wstring >( lowerCased,
                                                           exactMatches[ y ] ) );
  
          if ( !insertResult.second )
          {
            // Wasn't inserted since there was already an item -- check the case
            if ( insertResult.first->second != exactMatches[ y ] )
            {
              // The case is different -- agree on a lowercase version
              insertResult.first->second = lowerCased;
            }
          }
        }
  
        for( unsigned y = 0; y < prefixMatches.size(); ++y )
        {
          wstring lowerCased = Folding::applySimpleCaseOnly( prefixMatches[ y ] );
  
          pair< map< wstring, wstring >::iterator, bool > insertResult =
            prefixResults.insert( pair< wstring, wstring >( lowerCased,
                                                            prefixMatches[ y ] ) );
  
          if ( !insertResult.second )
          {
            // Wasn't inserted since there was already an item -- check the case
            if ( insertResult.first->second != prefixMatches[ y ] )
            {
              // The case is different -- agree on a lowercase version
              insertResult.first->second = lowerCased;
            }
          }
        }
  
        // Check if we've got an op request -- abort the query then
  
        opMutex.lock();
  
        if ( op != NoOp )
        {
          cancel = true;
          break;
        }
  
        opMutex.unlock();
      }
  
      if ( cancel )
        continue;
    }

    // Do any sort of collation here in the future. For now we just put the
    // strings sorted by the map.

    WordFinderResults r( prefixMatchReq, &activeDicts );

    r.results.reserve( exactResults.size() + prefixResults.size() );

    for( map< wstring, wstring >::const_iterator i = exactResults.begin();
         i != exactResults.end(); ++i )
      r.results.push_back( QString::fromStdWString( i->second ) );

    for( map< wstring, wstring >::const_iterator i = prefixResults.begin();
         i != prefixResults.end(); ++i )
      r.results.push_back( QString::fromStdWString( i->second ) );

    emit prefixMatchComplete( r );

    // Continue serving op requests

    opMutex.lock();
  }

  opMutex.unlock();
}

