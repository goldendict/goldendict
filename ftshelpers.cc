/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "fulltextsearch.hh"
#include "ftshelpers.hh"
#include "wstring_qt.hh"
#include "file.hh"
#include "gddebug.hh"

#include <vector>
#include <string>

using std::vector;
using std::string;

DEF_EX( exUserAbort, "User abort", Dictionary::Ex )

namespace FtsHelpers
{

bool ftsIndexIsOldOrBad( string const & indexFile )
{
  File::Class idx( indexFile, "rb" );

  FtsIdxHeader header;

  return idx.readRecords( &header, sizeof( header ), 1 ) != 1 ||
         header.signature != FtsSignature ||
         header.formatVersion != CurrentFtsFormatVersion;
}

bool parseSearchString( QString const & str, QStringList & indexWords,
                        QStringList & searchWords,
                        QRegExp & searchRegExp, int searchMode,
                        bool matchCase,
                        int distanceBetweenWords )
{
  searchWords.clear();
  indexWords.clear();
  QRegExp spacesRegExp( "\\W+" );
  QRegExp wordRegExp( QString( "\\w{" ) + QString::number( FTS::MinimumWordSize ) + ",}" );
  QRegExp setsRegExp( "\\[[^\\]]+\\]", Qt::CaseInsensitive, QRegExp::RegExp2 );

  if( searchMode == FTS::WholeWords || searchMode == FTS::PlainText )
  {
    // Make words list for search in article text
    searchWords = str.normalized( QString::NormalizationForm_C )
                     .split( spacesRegExp, QString::SkipEmptyParts );

    // Make words list for index search
    QStringList list = str.normalized( QString::NormalizationForm_C )
                          .toLower().split( spacesRegExp, QString::SkipEmptyParts );
    indexWords = list.filter( wordRegExp );
    indexWords.removeDuplicates();

    // Make regexp for results hilite

    QStringList allWords = str.split( spacesRegExp, QString::SkipEmptyParts );
    QString searchString( "(" );

    QString stripWords( "(?:\\W+\\w+){0," );
    if( distanceBetweenWords >= 0 )
      stripWords += QString::number( distanceBetweenWords );
    stripWords += "}\\W+";

    QString boundWord( searchMode == FTS::WholeWords ? "\\b" : "(?:\\w*)");

    for( int x = 0; x < allWords.size(); x++ )
    {
      if( x )
        searchString += stripWords;

      searchString += boundWord + allWords[ x ] + boundWord;
    }

    searchString += ")";

    searchRegExp = QRegExp( searchString, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            QRegExp::RegExp2 );
    searchRegExp.setMinimal( true );

    return !indexWords.isEmpty();
  }
  else if( searchMode == FTS::Enumeration )
  {
    // Make a pattern for search in article text

//    QString spacesInExpression = QString::number(str.simplified().count(" "));
    QString wordsForEnum = str.simplified().replace(" ","|");
    QString patternForEnum;

    if( wordsForEnum.count("|") > 0 )patternForEnum = "(?:\\b(?:" + wordsForEnum + ")\\b((?:\\W+\\w+){0,"
        + QString::number( distanceBetweenWords ) + "}\\W+\\b(?:" + wordsForEnum + ")\\b){"
        + QString::number(str.simplified().count(" ")) + "})";
//        + spacesInExpression + "," + spacesInExpression + "})";

    else patternForEnum = "\\b" + wordsForEnum + "\\b";

    searchRegExp  = QRegExp( patternForEnum, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::RegExp2 );
    searchRegExp.setMinimal(true);

    // Make words list for index search

    QStringList list = str.normalized( QString::NormalizationForm_C )
        .toLower().split( spacesRegExp, QString::SkipEmptyParts );
    indexWords = list.filter( wordRegExp );
    indexWords.removeDuplicates();

    return !indexWords.isEmpty();
  }
  else
  {
    // Make words list for index search

    QString tmp = str;

    // Remove all symbol sets
    tmp.replace( setsRegExp, " " );

    QStringList list = tmp.normalized( QString::NormalizationForm_C )
                          .toLower().split( spacesRegExp, QString::SkipEmptyParts );
    indexWords = list.filter( wordRegExp );
    indexWords.removeDuplicates();

    searchRegExp = QRegExp( str, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive,
                            searchMode == FTS::Wildcards ? QRegExp::WildcardUnix : QRegExp::RegExp2 );
    searchRegExp.setMinimal( true );
  }

  return true;
}

void parseArticleForFts( uint32_t articleAddress, QString & articleText,
                         QMap< QString, QVector< uint32_t > > & words )
{
  if( articleText.isEmpty() )
    return;

  QStringList articleWords = articleText.normalized( QString::NormalizationForm_C )
                                        .split( QRegExp( "\\W+" ), QString::SkipEmptyParts );
  QSet< QString > setOfWords;

  for( int x = 0; x < articleWords.size(); x++ )
  {
    QString word = articleWords.at( x ).toLower();

    if( word.size() < FTS::MinimumWordSize )
      continue;

    if( !setOfWords.contains( word ) )
    {
      setOfWords.insert( word );
      words[ word ].push_back( articleAddress );
    }
  }
}

void makeFTSIndex( BtreeIndexing::BtreeDictionary * dict, QAtomicInt & isCancelled )
{
  Mutex::Lock _( dict->getFtsMutex() );

  if( isCancelled )
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

  dict->findArticleLinks( 0, &setOfOffsets, 0, &isCancelled );

  if( isCancelled )
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

  if( isCancelled )
    throw exUserAbort();

  qSort( offsets );

  QMap< QString, QVector< uint32_t > > ftsWords;

  // index articles for full-text search
  for( int i = 0; i < offsets.size(); i++ )
  {
    if( isCancelled )
      throw exUserAbort();

    QString headword, articleStr;

    dict->getArticleText( offsets.at( i ), headword, articleStr );

    parseArticleForFts( offsets.at( i ), articleStr, ftsWords );
  }

  // Free memory
  offsets.clear();

  QMap< QString, QVector< uint32_t > >::iterator it = ftsWords.begin();
  while( it != ftsWords.end() )
  {
    if( isCancelled )
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

  if( isCancelled )
    throw exUserAbort();

  BtreeIndexing::IndexInfo ftsIdxInfo = BtreeIndexing::buildIndex( indexedWords, ftsIdx );

  // Free memory
  indexedWords.clear();

  ftsIdxHeader.indexBtreeMaxElements = ftsIdxInfo.btreeMaxElements;
  ftsIdxHeader.indexRootOffset = ftsIdxInfo.rootOffset;

  ftsIdxHeader.signature = FtsHelpers::FtsSignature;
  ftsIdxHeader.formatVersion = FtsHelpers::CurrentFtsFormatVersion;

  ftsIdx.rewind();
  ftsIdx.writeRecords( &ftsIdxHeader, sizeof(ftsIdxHeader), 1 );
}

void FTSResultsRequestRunnable::run()
{
  r.run();
}

void FTSResultsRequest::checkArticles( QVector< uint32_t > const & offsets,
                                       QStringList const & words,
                                       QRegExp const & searchRegexp )
{
  int results = 0;
  QString headword, articleText;
  QList< uint32_t > offsetsForHeadwords;

  QString id = QString::fromUtf8( dict.getId().c_str() );

  if( searchMode == FTS::Wildcards || searchMode == FTS::RegExp )
  {
    // RegExp mode

    for( int i = 0; i < offsets.size(); i++ )
    {
      if( isCancelled )
        break;

      dict.getArticleText( offsets.at( i ), headword, articleText );

      if( articleText.contains( searchRegexp ) )
      {
        if( headword.isEmpty() )
          offsetsForHeadwords.append( offsets.at( i ) );
        else
          foundHeadwords->append( FTS::FtsHeadword( headword, id ) );

        results++;
        if( maxResults > 0 && results >= maxResults )
          break;
      }
    }
  }
  else if( searchMode == FTS::Enumeration )
  {
    // Enumeration mode

    QStringList searchListForEnum = QString( searchRegexp.pattern().remove(0,8).remove(QRegExp("\\).*")) ).split("|");

    for( int i = 0; i < offsets.size(); i++ )
    {
      if( isCancelled )
        break;

      dict.getArticleText( offsets.at( i ), headword, articleText );

      if ( articleText.simplified().contains(searchRegexp) )
      {
        bool ourCase;
        int pos = 0;

        while ((pos = searchRegexp.indexIn(articleText, pos)) != -1)
        {
          QString foundExpression = articleText.mid(pos, searchRegexp.matchedLength()).simplified().trimmed();

          if (foundExpression.count(" ") <= distanceBetweenWords + 1)
          {
            ourCase = true;

            foreach (QString nextWord, searchListForEnum)
            {
              if ( !foundExpression.contains(nextWord, matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive) )
              {
                ourCase = false;
                break;
              }
            }
            if ( ourCase )
            {
              if( headword.isEmpty() )offsetsForHeadwords.append( offsets.at( i ) );
              else foundHeadwords->append( FTS::FtsHeadword( headword, id ) );

              results++;
              break;
            }
          }
          pos += searchRegexp.matchedLength();
        }

//        qDebug() << "results..." << results;

        if( maxResults > 0 && results >= maxResults )
          break;
      }
    }
  }
  else
  {
    // Words mode

    QRegExp regex( "\\b\\w+\\b" );
    Qt::CaseSensitivity cs = matchCase ? Qt::CaseSensitive : Qt::CaseInsensitive;

    for( int i = 0; i < offsets.size(); i++ )
    {
      if( isCancelled )
        break;

      int pos = 0;
      int matchWordNom = 0;
      int unmatchWordNom = 0;

      dict.getArticleText( offsets.at( i ), headword, articleText );

      int articleLength = articleText.length();
      while ( pos >= 0 && pos < articleLength )
      {
        pos = articleText.indexOf( regex, pos );
        if( pos >= 0 )
        {
          QString s = regex.cap().normalized( QString::NormalizationForm_C );
          if( ( searchMode == FTS::WholeWords && s.compare( words.at( matchWordNom ), cs ) == 0 )
              || ( searchMode == FTS::PlainText && s.contains( words.at( matchWordNom ), cs ) ) )
          {
            matchWordNom += 1;

            if( matchWordNom >= words.size() )
              break;  // All words are found

            unmatchWordNom = 0;
          }
          else
          if( matchWordNom > 0 )
          {
            unmatchWordNom += 1;
            if( distanceBetweenWords >= 0 && unmatchWordNom > distanceBetweenWords )
            {
              matchWordNom = 0;
              unmatchWordNom = 0;
            }
          }
          pos += s.isEmpty() ? 1 : s.length();
        }
      }

      if( matchWordNom >= words.size() )
      {
        if( headword.isEmpty() )
          offsetsForHeadwords.append( offsets.at( i ) );
        else
          foundHeadwords->append( FTS::FtsHeadword( headword, id ) );

        results++;
        if( maxResults > 0 && results >= maxResults )
          break;
      }
    }
  }
  if( !offsetsForHeadwords.isEmpty() )
  {
    QVector< QString > headwords;
    dict.getHeadwordsFromOffsets( offsetsForHeadwords, headwords, &isCancelled );
    for( int x = 0; x < headwords.size(); x++ )
      foundHeadwords->append( FTS::FtsHeadword( headwords.at( x ), id ) );
  }
}

void FTSResultsRequest::indexSearch( BtreeIndexing::BtreeIndex & ftsIndex,
                                     sptr< ChunkedStorage::Reader > chunks,
                                     QStringList & indexWords,
                                     QStringList & searchWords )
{
  // Find articles which contains all requested words

  vector< BtreeIndexing::WordArticleLink > links;
  QSet< uint32_t > setOfOffsets, tmp;
  uint32_t size;

  if( indexWords.isEmpty() )
    return;

  int n = indexWords.length();
  for( int i = 0; i < n; i++ )
  {
    if( isCancelled )
      return;

    tmp.clear();

    links = ftsIndex.findArticles( gd::toWString( indexWords.at( i ) ) );
    for( unsigned x = 0; x < links.size(); x++ )
    {

      if( isCancelled )
        return;

      vector< char > chunk;
      char * linksPtr;
      {
        Mutex::Lock _( dict.getFtsMutex() );
        linksPtr = chunks->getBlock( links[ x ].articleOffset, chunk );
      }

      memcpy( &size, linksPtr, sizeof(uint32_t) );
      linksPtr += sizeof(uint32_t);
      for( uint32_t y = 0; y < size; y++ )
      {
        tmp.insert( *( reinterpret_cast< uint32_t * >( linksPtr ) ) );
        linksPtr += sizeof(uint32_t);
      }
    }

    links.clear();

    if( i == 0 )
      setOfOffsets = tmp;
    else
      setOfOffsets = setOfOffsets.intersect( tmp );
  }

  tmp.clear();

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

  qSort( offsets );

  checkArticles( offsets, searchWords );
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

  if( isCancelled )
    return;

  if( indexWords.isEmpty() )
    return;

  ftsIndex.findArticleLinks( &links, 0, 0 );

  QVector< QSet< uint32_t > > allWordsLinks;
  allWordsLinks.resize( indexWords.size() );

  for( int x = 0; x < links.size(); x++ )
  {
    if( isCancelled )
      return;

    QString word = QString::fromUtf8( links[ x ].word.data(), links[ x ].word.size() );
    for( int i = 0; i < indexWords.size(); i++ )
    {
      if( word.length() >= indexWords.at( i ).length() && word.contains( indexWords.at( i ) ) )
      {
        vector< char > chunk;
        char * linksPtr;
        {
          Mutex::Lock _( dict.getFtsMutex() );
          linksPtr = chunks->getBlock( links[ x ].articleOffset, chunk );
        }

        memcpy( &size, linksPtr, sizeof(uint32_t) );
        linksPtr += sizeof(uint32_t);
        for( uint32_t y = 0; y < size; y++ )
        {
          allWordsLinks[ i ].insert( *( reinterpret_cast< uint32_t * >( linksPtr ) ) );
          linksPtr += sizeof(uint32_t);
        }
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

  qSort( offsets );

  checkArticles( offsets, searchWords, regexp );
}

void FTSResultsRequest::fullSearch( QStringList & searchWords, QRegExp & regexp )
{
  // Whole file survey

  if( isCancelled )
    return;

  QSet< uint32_t > setOfOffsets;
  setOfOffsets.reserve( dict.getArticleCount() );
  dict.findArticleLinks( 0, &setOfOffsets, 0 );

  if( isCancelled )
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

  if( isCancelled )
    return;

  setOfOffsets.clear();

  qSort( offsets );

  if( isCancelled )
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
                                        searchMode, matchCase, distanceBetweenWords ) )
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

        ftsIndex.openIndex( BtreeIndexing::IndexInfo( ftsIdxHeader.indexBtreeMaxElements,
                                                      ftsIdxHeader.indexRootOffset ),
                            ftsIdx, dict.getFtsMutex() );

        chunks = new ChunkedStorage::Reader( ftsIdx, ftsIdxHeader.chunksOffset );
      }

      if( searchMode == FTS::WholeWords )
        indexSearch( ftsIndex, chunks, indexWords, searchWords );
      else
        fullIndexSearch( ftsIndex, chunks, indexWords, searchWords, searchRegExp );
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

