/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fulltextsearch.hh"
#include "ftshelpers.hh"
#include "wstring_qt.hh"
#include "file.hh"
#include "gddebug.hh"
#include "folding.hh"
#include "utils.hh"

#include <vector>
#include <string>

#include <QVector>

#include <QRegularExpression>

#include "wildcard.hh"
#include <QtConcurrent>
#include "base/globalregex.hh"
#include <QFutureSynchronizer>
#include <QSemaphoreReleaser>

using std::vector;
using std::string;

DEF_EX( exUserAbort, "User abort", Dictionary::Ex )

namespace FtsHelpers
{

bool ftsIndexIsOldOrBad( string const & indexFile,
                         BtreeIndexing::BtreeDictionary * dict )
{
  File::Class idx( indexFile, "rb" );

  FtsIdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != FtsSignature ||
         header.formatVersion != CurrentFtsFormatVersion + dict->getFtsIndexVersion();
}

static QString makeHiliteRegExpString( QStringList const & words,
                                       int searchMode, int distanceBetweenWords, bool hasCJK = false, bool ignoreWordsOrder = false )
{
  QString searchString( "(" );

  QString stripWords( "(?:\\W+\\w+){0," );

  if( hasCJK )
  {
    stripWords = "(?:[\\W\\w]){0,";
  }

  if( distanceBetweenWords >= 0 )
    stripWords += QString::number( distanceBetweenWords );
  stripWords += "}";

  if(!hasCJK)
  {
    stripWords += "\\W+";
  }

  QString boundWord( searchMode == FTS::WholeWords ? "\\b" : "(?:\\w*)");
  if(hasCJK)
  {
    //no boundary for CJK
    boundWord.clear();
  }

  for( int x = 0; x < words.size(); x++ )
  {
    if( x )
    {
      searchString += stripWords;
      if(ignoreWordsOrder)
        searchString += "(";
    }

    searchString += boundWord + words[ x ] + boundWord;

    if( x )
    {
      if( ignoreWordsOrder )
        searchString += ")?";
    }

  }

  searchString += ")";
  return searchString;
}

void tokenizeCJK( QStringList & indexWords, QRegularExpression wordRegExp, QStringList list )
{
  QStringList wordList, hieroglyphList;
  for( int i = 0; i < list.size(); i ++ )
  {
    QString word = list.at( i );

    // Check for CJK symbols in word
    bool parsed = false;
    QString hieroglyph;
    for( int x = 0; x < word.size(); x++ )
      if( isCJKChar( word.at( x ).unicode() ) )
      {
        parsed = true;
        hieroglyph.append( word[ x ] );

        if( QChar( word.at( x ) ).isHighSurrogate()
            &&  QChar( word[ x + 1 ] ).isLowSurrogate() )
          hieroglyph.append( word[ ++x ] );

        hieroglyphList.append( hieroglyph );
        hieroglyph.clear();
      }

    // If word don't contains CJK symbols put it in list as is
    if( !parsed )
      wordList.append( word );
  }

  indexWords = wordList.filter( wordRegExp );
  indexWords.removeDuplicates();

  hieroglyphList.removeDuplicates();
  indexWords += hieroglyphList;
}

bool containCJK( QString const & str)
{
  bool hasCJK = false;
  for( int x = 0; x < str.size(); x++ )
    if( isCJKChar( str.at( x ).unicode() ) )
    {
      hasCJK = true;
      break;
    }
  return hasCJK;
}

bool parseSearchString( QString const & str, QStringList & indexWords,
                        QStringList & searchWords,
                        QRegExp & searchRegExp, int searchMode,
                        bool matchCase,
                        int distanceBetweenWords,
                        bool & hasCJK,
                        bool ignoreWordsOrder )
{
  searchWords.clear();
  indexWords.clear();
  // QRegularExpression spacesRegExp( "\\W+", QRegularExpression::UseUnicodePropertiesOption );
  // QRegularExpression wordRegExp( QString( "\\w{" ) + QString::number( FTS::MinimumWordSize ) + ",}", QRegularExpression::UseUnicodePropertiesOption );
  // QRegularExpression setsRegExp( "\\[[^\\]]+\\]", QRegularExpression::CaseInsensitiveOption );
  // QRegularExpression regexRegExp( "\\\\[afnrtvdDwWsSbB]|\\\\x([0-9A-Fa-f]{4})|\\\\0([0-7]{3})", QRegularExpression::CaseInsensitiveOption);

  hasCJK = containCJK( str );

  if( searchMode == FTS::WholeWords || searchMode == FTS::PlainText )
  {
    // Make words list for search in article text
    searchWords = str.normalized( QString::NormalizationForm_C ).split( RX::Ftx::spacesRegExp, Qt::SkipEmptyParts );
    // Make words list for index search
    QStringList list =
      str.normalized( QString::NormalizationForm_C ).toLower().split( RX::Ftx::spacesRegExp, Qt::SkipEmptyParts );

    QString searchString;
    if( hasCJK )
    {
      tokenizeCJK( indexWords, RX::Ftx::wordRegExp, list );
      // QStringList allWords = str.split( spacesRegExp, Qt::SkipEmptyParts );
      searchString         = makeHiliteRegExpString( list, searchMode, distanceBetweenWords, hasCJK , ignoreWordsOrder);
    }
    else
    {
      indexWords = list.filter( RX::Ftx::wordRegExp );
      indexWords.removeDuplicates();

      // Make regexp for results hilite

      QStringList allWords = str.split( RX::Ftx::spacesRegExp, Qt::SkipEmptyParts );
      searchString = makeHiliteRegExpString( allWords, searchMode, distanceBetweenWords,false, ignoreWordsOrder );
    }
    searchRegExp = QRegExp( searchString, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
    searchRegExp.setMinimal( true );
    return !indexWords.isEmpty();
  }
  else
  {
    // Make words list for index search

    QString tmp = str;

    // Remove RegExp commands
    if( searchMode == FTS::RegExp )
      tmp.replace( RX::Ftx::regexRegExp, " " );

    // Remove all symbol sets
    tmp.replace( RX::Ftx::setsRegExp, " " );

    QStringList list = tmp.normalized( QString::NormalizationForm_C )
                          .toLower().split( RX::Ftx::spacesRegExp, Qt::SkipEmptyParts );

    if( hasCJK )
    {
      tokenizeCJK( indexWords, RX::Ftx::wordRegExp, list );
    }
    else
    {
      indexWords = list.filter( RX::Ftx::wordRegExp );
      indexWords.removeDuplicates();
    }

    searchRegExp = QRegExp( str, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            searchMode == FTS::Wildcards ? QRegExp::WildcardUnix : QRegExp::RegExp2 );
    searchRegExp.setMinimal( true );
  }

  return true;
}
//definition;
Mutex lockMutex;

void parseArticleForFts( uint32_t articleAddress, QString & articleText,
                         QMap< QString, QVector< uint32_t > > & words,
                         bool handleRoundBrackets )
{
  if( articleText.isEmpty() )
    return;

  QStringList articleWords = articleText.normalized( QString::NormalizationForm_C )
      .split( handleRoundBrackets ? RX::Ftx::handleRoundBracket : RX::Ftx::noRoundBracket,
                                                Qt::SkipEmptyParts );

  QVector< QString > setOfWords;
  setOfWords.reserve( articleWords.size() );

  for( int x = 0; x < articleWords.size(); x++ )
  {
    QString word = articleWords.at( x ).toLower();

    bool hasCJK = false;
    QString hieroglyph;

    // If word contains CJK symbols we add to index only these symbols separately
    for( int y = 0; y < word.size(); y++ )
      if( isCJKChar( word.at( y ).unicode() ) )
      {
        hasCJK = true;
        hieroglyph.append( word[ y ] );

        if( QChar( word.at( y ) ).isHighSurrogate()
            &&  QChar( word[ y + 1 ] ).isLowSurrogate() )
          hieroglyph.append( word[ ++y ] );

        //if( !setOfWords.contains( hieroglyph ) )
        {
          setOfWords.push_back( hieroglyph );
          /*Mutex::Lock _( _mapLock );
          words[ hieroglyph ].push_back( articleAddress );*/
        }

        hieroglyph.clear();
      }

    if( !hasCJK )
    {
      // Else we add word to index as is
      if( word.size() < FTS::MinimumWordSize )
        continue;

      if( handleRoundBrackets && ( word.indexOf( '(' ) >= 0 || word.indexOf( ')' ) >= 0 ) )
      {
        // Special handle for words with round brackets - DSL feature
        QStringList list;

        QStringList oldVariant = word.split( RX::Ftx::regSplit, Qt::SkipEmptyParts );
        for( QStringList::iterator it = oldVariant.begin(); it != oldVariant.end(); ++it )
          if( it->size() >= FTS::MinimumWordSize && !list.contains( *it ) )
            list.append( *it );

        QRegularExpressionMatch match = RX::Ftx::regBrackets.match( word );
        if( match.hasMatch() )
        {
          QStringList parts = match.capturedTexts();
          // Add empty strings for compatibility with QRegExp behaviour
          for( int i = match.lastCapturedIndex() + 1; i < 6; i++ )
            parts.append( QString() );
          QString parsedWord = parts[ 2 ] + parts[ 4 ]; // Brackets removed

          if( parsedWord.size() >= FTS::MinimumWordSize && !list.contains( parsedWord ) )
            list.append( parsedWord );

          parsedWord = parts[ 1 ].remove( '(' ).remove( ')' )
                       + parts[ 2 ]
                       + parts[ 3 ].remove( '(' ).remove( ')' )
                       + parts[ 4 ]
                       + parts[ 5 ].remove( '(' ).remove( ')' ); // Brackets expansed

          if( parsedWord.size() >= FTS::MinimumWordSize && !list.contains( parsedWord ) )
            list.append( parsedWord );
        }

        for( QStringList::iterator it = list.begin(); it != list.end(); ++it )
        {
          //if( !setOfWords.contains( *it ) )
          {
            setOfWords.push_back( *it );
            /*Mutex::Lock _( _mapLock );
            words[ *it ].push_back( articleAddress );*/
          }
        }
      }
      else
      //if( !setOfWords.contains( word ) )
      {
        setOfWords.push_back( word );
        /*Mutex::Lock _( _mapLock );
        words[ word ].push_back( articleAddress );*/
      }
    }


  }

  {
    Mutex::Lock _( lockMutex );

    for( const QString & word : setOfWords )
    {
      words[ word ].push_back( articleAddress );
    }
  }
}

void makeFTSIndex( BtreeIndexing::BtreeDictionary * dict, QAtomicInt & isCancelled )
{
  Mutex::Lock _( dict->getFtsMutex() );

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    throw exUserAbort();

  File::Class ftsIdx( dict->ftsIndexName(), "wb" );

  FtsIdxHeader ftsIdxHeader;
  memset( &ftsIdxHeader, 0, sizeof( ftsIdxHeader ) );

  // We write a dummy header first. At the end of the process the header
  // will be rewritten with the right values.

  ftsIdx.write( ftsIdxHeader );

  ChunkedStorage::Writer chunks( ftsIdx );

  BtreeIndexing::IndexedWords indexedWords;

  QSet< uint32_t > setOfOffsets;
  setOfOffsets.reserve( dict->getArticleCount() );

  dict->findArticleLinks( 0, &setOfOffsets, 0, &isCancelled );

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    throw exUserAbort();

  QVector< uint32_t > offsets;
  offsets.resize( setOfOffsets.size() );
  uint32_t * ptr = &offsets.front();

  for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
       it != setOfOffsets.constEnd(); ++it )
  {
    *ptr = *it;
    ptr++;
  }

  // Free memory
  setOfOffsets.clear();

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    throw exUserAbort();

  dict->sortArticlesOffsetsForFTS( offsets, isCancelled );

  QMap< QString, QVector< uint32_t > > ftsWords;

  bool needHandleBrackets;
  {
    QString name = QString::fromUtf8( dict->getDictionaryFilenames()[ 0 ].c_str() ).toLower();
    needHandleBrackets = name.endsWith( ".dsl" ) || name.endsWith( "dsl.dz" );
  }

  QSemaphore sem( QThread::idealThreadCount() );
  //QFutureSynchronizer< void > synchronizer;

  for( auto & address : offsets )
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    {
        //wait the future to be finished.
      sem.acquire( QThread::idealThreadCount() );
      return;
    }

    sem.acquire();
    QFuture< void > f = QtConcurrent::run(
      [ & ]()
      {
        QSemaphoreReleaser releaser( sem );
        if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        {
          return;
        }

        QString headword, articleStr;

        dict->getArticleText( address, headword, articleStr );

        parseArticleForFts( address, articleStr, ftsWords, needHandleBrackets );
      } );
    //synchronizer.addFuture( f );
  }
  sem.acquire( QThread::idealThreadCount() );
  // Free memory
  offsets.clear();

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    throw exUserAbort();

  QMap< QString, QVector< uint32_t > >::iterator it = ftsWords.begin();
  while( it != ftsWords.end() )
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      throw exUserAbort();

    uint32_t offset = chunks.startNewBlock();
    uint32_t size = it.value().size();

    chunks.addToBlock( &size, sizeof(uint32_t) );
    chunks.addToBlock( it.value().data(), size * sizeof(uint32_t) );

    indexedWords.addSingleWord( gd::toWString( it.key() ), offset );

    it = ftsWords.erase( it );
  }

  ftsIdxHeader.chunksOffset = chunks.finish();
  ftsIdxHeader.wordCount = indexedWords.size();

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    throw exUserAbort();

  BtreeIndexing::IndexInfo ftsIdxInfo = BtreeIndexing::buildIndex( indexedWords, ftsIdx );

  // Free memory
  indexedWords.clear();

  ftsIdxHeader.indexBtreeMaxElements = ftsIdxInfo.btreeMaxElements;
  ftsIdxHeader.indexRootOffset = ftsIdxInfo.rootOffset;

  ftsIdxHeader.signature = FtsHelpers::FtsSignature;
  ftsIdxHeader.formatVersion = FtsHelpers::CurrentFtsFormatVersion + dict->getFtsIndexVersion();

  ftsIdx.rewind();
  ftsIdx.writeRecords( &ftsIdxHeader, sizeof(ftsIdxHeader), 1 );
}

bool isCJKChar( ushort ch )
{
  return Utils::isCJKChar(ch);
}

void FTSResultsRequest::checkArticles( QVector< uint32_t > const & offsets,
                                       QStringList const & words,
                                       QRegExp const & searchRegexp )
{
  // const int parallel_count = QThread::idealThreadCount()/2;
  // QSemaphore sem( parallel_count  < 1 ? 1 : parallel_count  );
  //
  // QFutureSynchronizer< void > synchronizer;
  const auto searchRegularExpression = createMatchRegex( searchRegexp );

  for( auto & address : offsets )
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    {
      return;
    }
    checkSingleArticle( address, words, searchRegularExpression );
  }
}

QRegularExpression FTSResultsRequest::createMatchRegex( QRegExp const & searchRegexp )
{
  QRegularExpression searchRegularExpression;

  if( searchMode == FTS::Wildcards )
    searchRegularExpression.setPattern( wildcardsToRegexp( searchRegexp.pattern() ) );
  else
    searchRegularExpression.setPattern( searchRegexp.pattern() );
  QRegularExpression::PatternOptions patternOptions =
    QRegularExpression::DotMatchesEverythingOption | QRegularExpression::UseUnicodePropertiesOption
    | QRegularExpression::MultilineOption | QRegularExpression::InvertedGreedinessOption;
  if( searchRegexp.caseSensitivity() == Qt::CaseInsensitive )
    patternOptions |= QRegularExpression::CaseInsensitiveOption;
  searchRegularExpression.setPatternOptions( patternOptions );
  if( !searchRegularExpression.isValid() )
    searchRegularExpression.setPattern( "" );
  return searchRegularExpression;
}

void FTSResultsRequest::checkSingleArticle( uint32_t  offset,
                                            QStringList const & words,
                                            QRegularExpression const & searchRegularExpression )
{
  // int results = 0;
  QString headword, articleText;
  QList< uint32_t > offsetsForHeadwords;
  QVector< QStringList > hiliteRegExps;

  QString id = QString::fromUtf8( dict.getId().c_str() );

  // RegExp mode

  if( searchMode == FTS::Wildcards || searchMode == FTS::RegExp )
  {
    // for( int i = 0; i < offsets.size(); i++ )
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        return;

      // auto article_address = offsets.at( i );
      dict.getArticleText( offset, headword, articleText );
      articleText = articleText.normalized( QString::NormalizationForm_C );

      if( ignoreDiacritics )
        articleText = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( articleText ) ) );

      if( articleText.contains( searchRegularExpression ) )
      {
        if( headword.isEmpty() )
          offsetsForHeadwords.append( offset );
        else
        {
          Mutex::Lock _( dataMutex );
          foundHeadwords->append( FTS::FtsHeadword( headword, id, QStringList(), matchCase ) );
        }

        ++results;
        if( maxResults > 0 && results >= maxResults )
          return;
      }
  }
  else
  {
    // Words mode

    QVector< QPair< QString, bool > > wordsList;
    if( ignoreWordsOrder )
    {
      for( QStringList::const_iterator it = words.begin(); it != words.end(); ++it )
        wordsList.append( QPair< QString, bool >( *it, true ) );
    }

    // for( int i = 0; i < offsets.size(); i++ )
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        return;

      if( ignoreWordsOrder )
      {
        for( int i = 0; i < wordsList.size(); i++ )
          wordsList[ i ].second = true;
      }

      dict.getArticleText( offset, headword, articleText );

      articleText = articleText.normalized( QString::NormalizationForm_C );

      if( ignoreDiacritics )
        articleText = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( articleText ) ) );

      if( ignoreWordsOrder )
      {
        bool allMatch = true;
        foreach( QString word, words )
        {
          if( containCJK( word ) || searchMode == FTS::PlainText )
          {
            if( !articleText.contains( word ) )
            {
              allMatch = false;
              break;
            }
          }
          else if( searchMode == FTS::WholeWords )
          {
            QRegularExpression tmpReg( QString( "\b%1\b" ).arg( word ),
                                       QRegularExpression::CaseInsensitiveOption
                                         | QRegularExpression::UseUnicodePropertiesOption );
            if( !articleText.contains( tmpReg ) )
            {
              allMatch = false;
              break;
            }
          }
        }

        if( !allMatch )
        {
          return;
        }

        if( distanceBetweenWords >= 0 )
        {
          // the article text contains all the needed words.
          // determine if distance restriction is meet
          const QRegularExpression replaceReg( QString( "(%1)" ).arg( words.join( '|' ) ),
                                               QRegularExpression::CaseInsensitiveOption
                                               | QRegularExpression::UseUnicodePropertiesOption );
          // use a string that could not be presented in the article.
          articleText = articleText.replace( replaceReg, "=@XXXXX@=" );

          auto hasCJK = false;
          foreach( QString word, words )
          {
            if( containCJK( word ) )
            {
              hasCJK = true;
              break;
            }
          }

          // hascjk value ,perhaps should depend on each word
          const auto searchRegStr = makeHiliteRegExpString( Utils::repeat( "=@XXXXX@=", words.size() ),
                                                            searchMode,
                                                            distanceBetweenWords,
                                                            hasCJK );
          const QRegularExpression distanceOrderReg( searchRegStr,
                                                     QRegularExpression::CaseInsensitiveOption
                                                     | QRegularExpression::UseUnicodePropertiesOption );
          // use a string that could not be presented in the article.
          if( articleText.contains( distanceOrderReg ) )
          {
            if( headword.isEmpty() )
              offsetsForHeadwords.append( offset );
            else
            {
              Mutex::Lock _( dataMutex );
              foundHeadwords->append( FTS::FtsHeadword( headword, id, QStringList(), matchCase ) );
            }

            ++results;
            if( maxResults > 0 && results >= maxResults )
              return;
          }
        }
      }
      else
      {
        if( articleText.contains( searchRegularExpression ) )
        {
          if( headword.isEmpty() )
            offsetsForHeadwords.append( offset );
          else
          {
            Mutex::Lock _( dataMutex );
            foundHeadwords->append( FTS::FtsHeadword( headword, id, QStringList(), matchCase ) );
          }

          ++results;
          if( maxResults > 0 && results >= maxResults )
            return;
        }
      }
  }
  if( !offsetsForHeadwords.isEmpty() )
  {
    QVector< QString > headwords;
    Mutex::Lock _( dataMutex );

    dict.getHeadwordsFromOffsets( offsetsForHeadwords, headwords, &isCancelled );
    for( int x = 0; x < headwords.size(); x++ )
    {
      foundHeadwords->append( FTS::FtsHeadword( headwords.at( x ),
                                                id,
                                                x < hiliteRegExps.size() ? hiliteRegExps.at( x ) : QStringList(),
                                                matchCase ) );
    }
  }
}

void FTSResultsRequest::indexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                                     sptr< ChunkedStorage::Reader > chunks,
                                     QStringList & indexWords,
                                     QStringList & searchWords, QRegExp & regexp )
{
  // Find articles which contains all requested words

  QSet< uint32_t > setOfOffsets;

  if( indexWords.isEmpty() )
    return;

  QList< QSet< uint32_t > > addressLists;

  auto findLinks = [ & ]( const QString & word )
  {
    QSet< uint32_t > tmp;
    uint32_t size;

    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    {
      addressLists << tmp;
      return;
    }

    vector< BtreeIndexing::WordArticleLink > links =
      ftsIndex.findArticles( gd::toWString( word ), ignoreDiacritics );
    for( unsigned x = 0; x < links.size(); x++ )
    {
      if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      {
        addressLists << tmp;
        return;
      }

      vector< char > chunk;
      char * linksPtr;
      {
        // Mutex::Lock _( dict.getFtsMutex() );
        linksPtr = chunks->getBlock( links[ x ].articleOffset, chunk );
      }

      memcpy( &size, linksPtr, sizeof( uint32_t ) );
      linksPtr += sizeof( uint32_t );
      for( uint32_t y = 0; y < size; y++ )
      {
        tmp.insert( *( reinterpret_cast< uint32_t * >( linksPtr ) ) );
        linksPtr += sizeof( uint32_t );
      }
    }

    links.clear();
    {
      Mutex::Lock _( dataMutex );
      addressLists << tmp;
    }
  };
  // int n = indexWords.length();
  // QtConcurrent::blockingMap( indexWords, findLinks );

  for(QString word:indexWords)
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    {
      return;
    }
    findLinks( word );
  }

  // blocked execution.

  int i = 0;
  for( auto & elem : addressLists )
  {
    if( i++ == 0 )
      setOfOffsets = elem;
    else
      setOfOffsets = setOfOffsets.intersect( elem );
  }


  if( setOfOffsets.isEmpty() )
    return;

  QVector< uint32_t > offsets;
  offsets.resize( setOfOffsets.size() );
  uint32_t * ptr = &offsets.front();

  for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
       it != setOfOffsets.constEnd(); ++it )
  {
    *ptr = *it;
    ptr++;
  }

  setOfOffsets.clear();

  dict.sortArticlesOffsetsForFTS( offsets, isCancelled );

  checkArticles( offsets, searchWords, regexp );
}

void FTSResultsRequest::combinedIndexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                                             sptr< ChunkedStorage::Reader > chunks,
                                             QStringList & indexWords,
                                             QStringList & searchWords,
                                             QRegExp & regexp )
{
  // Special case - combination of index search for hieroglyphs
  // and full index search for other words

  QSet< uint32_t > setOfOffsets;
  uint32_t size;

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  if( indexWords.isEmpty() )
    return;

  QStringList wordsList, hieroglyphsList;

  for( int x = 0; x < indexWords.size(); x++ )
  {
    QString const & word = indexWords.at( x );
    if( isCJKChar( word[ 0 ].unicode() ) )
      hieroglyphsList.append( word );
    else
      wordsList.append( word );
  }

  QVector< QSet< uint32_t > > allWordsLinks;

  int n = wordsList.size();
  if( !hieroglyphsList.isEmpty() )
  {
    wordsList += hieroglyphsList;
    n += 1;
  }

  allWordsLinks.resize( n );

  if( !wordsList.empty() )
  {
    QList< QSet< uint32_t > > sets;
    auto fn_wordLink = [ & ](const QString & word )
    {
      QSet< uint32_t > tmp;
      vector< BtreeIndexing::WordArticleLink > links = ftsIndex.findArticles( gd::toWString( word ) );
      for( unsigned x = 0; x < links.size(); x++ )
      {
        if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        {
          Mutex::Lock _( dataMutex );
          sets << tmp;
          return;
        }

        vector< char > chunk;
        char * linksPtr;
        {
          // Mutex::Lock _( dict.getFtsMutex() );
          linksPtr = chunks->getBlock( links[ x ].articleOffset, chunk );
        }

        memcpy( &size, linksPtr, sizeof( uint32_t ) );
        linksPtr += sizeof( uint32_t );
        // across chunks, need further investigation
        uint32_t max = ( chunk.size() - ( linksPtr - &chunk.front() )) / 4;

        tmp.reserve( size );
        uint32_t q_max = qMin(size,max);
        for( uint32_t y = 0; y < q_max; y++ )
        {
          tmp.insert( *( reinterpret_cast< uint32_t * >( linksPtr ) ) );
          linksPtr += sizeof( uint32_t );
        }
      }

      links.clear();

      {
        Mutex::Lock _( dataMutex );
        sets << tmp;
      }
    };
    // QtConcurrent::blockingMap( wordsList, fn_wordLink );

    {

      for(const auto & word : wordsList )
      {
        if( Utils::AtomicInt::loadAcquire( isCancelled ) )
        {
          return;
        }
        fn_wordLink( word );
      }
    }
    //blocked execution.

    int i = 0;
    for( auto & elem : sets )
    {
      if( i++ == 0 )
        setOfOffsets = elem;
      else
        setOfOffsets = setOfOffsets.intersect( elem );
    }

    // allWordsLinks[ wordNom ] = setOfOffsets;
    // setOfOffsets.clear();
    // wordNom += 1;
  }

  if( setOfOffsets.isEmpty() )
    return;

  allWordsLinks.clear();

  QVector< uint32_t > offsets( setOfOffsets.begin(),setOfOffsets.end() );
  // offsets.resize( setOfOffsets.size() );
  // uint32_t * ptr = &offsets.front();
  //
  // for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
  //      it != setOfOffsets.constEnd(); ++it )
  // {
  //   *ptr = *it;
  //   ptr++;
  // }

  setOfOffsets.clear();

  dict.sortArticlesOffsetsForFTS( offsets, isCancelled );

  checkArticles( offsets, searchWords, regexp );
}

void FTSResultsRequest::fullIndexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                                         sptr< ChunkedStorage::Reader > chunks,
                                         QStringList & indexWords,
                                         QStringList & searchWords,
                                         QRegExp & regexp )
{
  QSet< uint32_t > setOfOffsets;
  uint32_t size;
  QVector< BtreeIndexing::WordArticleLink > links;

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  if( indexWords.isEmpty() )
    return;

  links.reserve( wordsInIndex );
  ftsIndex.findArticleLinks( &links, 0, 0, &isCancelled );

  QVector< QSet< uint32_t > > allWordsLinks;
  allWordsLinks.resize( indexWords.size() );

  for( int x = 0; x < links.size(); x++ )
  {
    if( Utils::AtomicInt::loadAcquire( isCancelled ) )
      return;

    QString word = QString::fromUtf8( links[ x ].word.data(), links[ x ].word.size() );

    if( ignoreDiacritics )
      word = gd::toQString( Folding::applyDiacriticsOnly( gd::toWString( word ) ) );

    for( int i = 0; i < indexWords.size(); i++ )
    {
      if( word.length() >= indexWords.at( i ).length() && word.contains( indexWords.at( i ) ) )
      {
        vector< char > chunk;
        char * linksPtr;
        {
          // Mutex::Lock _( dict.getFtsMutex() );
          linksPtr = chunks->getBlock( links[ x ].articleOffset, chunk );
        }

        memcpy( &size, linksPtr, sizeof(uint32_t) );
        linksPtr += sizeof(uint32_t);
        for( uint32_t y = 0; y < size; y++ )
        {
          allWordsLinks[ i ].insert( *( reinterpret_cast< uint32_t * >( linksPtr ) ) );
          linksPtr += sizeof(uint32_t);
        }
        if( searchMode == FTS::PlainText )
          break;
      }
    }
  }

  links.clear();

  for( int i = 0; i < allWordsLinks.size(); i++ )
  {
    if( i == 0 )
      setOfOffsets = allWordsLinks.at( i );
    else
      setOfOffsets = setOfOffsets.intersect( allWordsLinks.at( i ) );
  }

  if( setOfOffsets.isEmpty() )
    return;

  allWordsLinks.clear();

  QVector< uint32_t > offsets( setOfOffsets.begin(), setOfOffsets.end() );
  // offsets.resize( setOfOffsets.size() );
  // uint32_t * ptr = &offsets.front();
  //
  // for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
  //      it != setOfOffsets.constEnd(); ++it )
  // {
  //   *ptr = *it;
  //   ptr++;
  // }

  setOfOffsets.clear();

  dict.sortArticlesOffsetsForFTS( offsets, isCancelled );

  checkArticles( offsets, searchWords, regexp );
}

void FTSResultsRequest::fullSearch( QStringList & searchWords, QRegExp & regexp )
{
  // Whole file survey

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  QSet< uint32_t > setOfOffsets;
  setOfOffsets.reserve( dict.getArticleCount() );
  dict.findArticleLinks( 0, &setOfOffsets, 0, &isCancelled );

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  QVector< uint32_t > offsets;
  offsets.resize( setOfOffsets.size() );
  uint32_t * ptr = &offsets.front();

  for( QSet< uint32_t >::ConstIterator it = setOfOffsets.constBegin();
       it != setOfOffsets.constEnd(); ++it )
  {
    *ptr = *it;
    ptr++;
  }

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  setOfOffsets.clear();

  dict.sortArticlesOffsetsForFTS( offsets, isCancelled );

  if( Utils::AtomicInt::loadAcquire( isCancelled ) )
    return;

  checkArticles( offsets, searchWords, regexp );
}

void FTSResultsRequest::run()
{
  if ( dict.ensureInitDone().size() )
  {
    setErrorString( QString::fromUtf8( dict.ensureInitDone().c_str() ) );
    finish();
    return;
  }

  try
  {
    QStringList indexWords, searchWords;
    QRegExp searchRegExp;

    if( !FtsHelpers::parseSearchString( searchString, indexWords, searchWords, searchRegExp,
                                        searchMode, matchCase, distanceBetweenWords, hasCJK, ignoreWordsOrder ) )
    {
      finish();
      return;
    }

    if( dict.haveFTSIndex() && !indexWords.isEmpty() )
    {
      FtsIdxHeader ftsIdxHeader;
      BtreeIndexing::BtreeIndex ftsIndex;
      sptr< ChunkedStorage::Reader > chunks;

      File::Class ftsIdx( dict.ftsIndexName(), "rb" );

      {
        Mutex::Lock _( dict.getFtsMutex() );

        ftsIdxHeader = ftsIdx.read< FtsIdxHeader >();

        wordsInIndex = ftsIdxHeader.wordCount;

        ftsIndex.openIndex( BtreeIndexing::IndexInfo( ftsIdxHeader.indexBtreeMaxElements,
                                                      ftsIdxHeader.indexRootOffset ),
                            ftsIdx, dict.getFtsMutex() );

        chunks = new ChunkedStorage::Reader( ftsIdx, ftsIdxHeader.chunksOffset );
      }

      if( hasCJK )
        combinedIndexSearch( ftsIndex, chunks, indexWords, searchWords, searchRegExp );
      else
      {
        if( searchMode == FTS::WholeWords )
          indexSearch( ftsIndex, chunks, indexWords, searchWords, searchRegExp );
        else
          fullIndexSearch( ftsIndex, chunks, indexWords, searchWords, searchRegExp );
      }
    }
    else
    {
      fullSearch( searchWords, searchRegExp );
    }

    if( foundHeadwords && foundHeadwords->size() > 0 )
    {
      Mutex::Lock _( dataMutex );
      data.resize( sizeof( foundHeadwords ) );
      memcpy( &data.front(), &foundHeadwords, sizeof( foundHeadwords ) );
      foundHeadwords = 0;
      hasAnyData = true;
    }
  }
  catch( std::exception &ex )
  {
    gdWarning( "FTS: Failed full-text search for \"%s\", reason: %s\n",
               dict.getName().c_str(), ex.what() );
    // Results not loaded -- we don't set the hasAnyData flag then
  }

  finish();
}

} // namespace

