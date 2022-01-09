/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <vector>
#include <algorithm>
#include <cstdio>
#include "dictionary.hh"

#include <QCryptographicHash>

// For needToRebuildIndex(), read below
#include <QFileInfo>
#include <QDateTime>

#include "config.hh"
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QDateTime>
#include "fsencoding.hh"
#include "langcoder.hh"

#include <QImage>
#include <QPainter>
#include <QRegularExpression>
#include "utils.hh"
#include "zipfile.hh"

namespace Dictionary {

bool Request::isFinished()
{
  return Utils::AtomicInt::loadAcquire( isFinishedFlag );
}

void Request::update()
{
  if ( !Utils::AtomicInt::loadAcquire( isFinishedFlag ) )
    emit updated();
}

void Request::finish()
{
  if ( !Utils::AtomicInt::loadAcquire( isFinishedFlag ) )
  {
    isFinishedFlag.ref();

    emit finished();
  }
}

void Request::setErrorString( QString const & str )
{
  Mutex::Lock _( errorStringMutex );

  errorString = str;
}

QString Request::getErrorString()
{
  Mutex::Lock _( errorStringMutex );

  return errorString;
}


///////// WordSearchRequest
  
size_t WordSearchRequest::matchesCount()
{
  Mutex::Lock _( dataMutex );
  
  return matches.size();
}

WordMatch WordSearchRequest::operator [] ( size_t index ) 
{
  Mutex::Lock _( dataMutex );
  
  if ( index >= matches.size() )
    throw exIndexOutOfRange();
  
  return matches[ index ];
}

vector< WordMatch > & WordSearchRequest::getAllMatches() 
{
  if ( !isFinished() )
    throw exRequestUnfinished();

  return matches;
}

void WordSearchRequest::addMatch( WordMatch const & match )
{
  unsigned n;
  for( n = 0; n < matches.size(); n++ )
    if( matches[ n ].word.compare( match.word ) == 0 )
      break;

  if( n >= matches.size() )
    matches.push_back( match );
}

////////////// DataRequest

long DataRequest::dataSize()
{
  Mutex::Lock _( dataMutex );
  
  return hasAnyData ? (long)data.size() : -1;
}

void DataRequest::getDataSlice( size_t offset, size_t size, void * buffer )
  
{
  if ( size == 0 )
    return;

  Mutex::Lock _( dataMutex );

  if ( offset + size > data.size() || !hasAnyData )
    throw exSliceOutOfRange();

  memcpy( buffer, &data[ offset ], size );
}

vector< char > & DataRequest::getFullData() 
{
  if ( !isFinished() )
    throw exRequestUnfinished();

  return data;
}

Class::Class( string const & id_, vector< string > const & dictionaryFiles_ ):
  id( id_ ), dictionaryFiles( dictionaryFiles_ ), dictionaryIconLoaded( false )
  , can_FTS( false), FTS_index_completed( false )
{
}

void Class::deferredInit()
{
}

sptr< WordSearchRequest > Class::stemmedMatch( wstring const & /*str*/,
                                               unsigned /*minLength*/,
                                               unsigned /*maxSuffixVariation*/,
                                               unsigned long /*maxResults*/ )
  
{
  return new WordSearchRequestInstant();
}

sptr< WordSearchRequest > Class::findHeadwordsForSynonym( wstring const & )
  
{
  return new WordSearchRequestInstant();
}

vector< wstring > Class::getAlternateWritings( wstring const & )
  throw()
{
  return vector< wstring >();
}

sptr< DataRequest > Class::getResource( string const & /*name*/ )
  
{
  return new DataRequestInstant( false );
}

sptr< DataRequest > Class::getSearchResults(const QString &, int, bool, int, int, bool, bool )
{
  return new DataRequestInstant( false );
}

QString const& Class::getDescription()
{
    return dictionaryDescription;
}

QString Class::getMainFilename()
{
  return QString();
}

QIcon const & Class::getIcon() throw()
{
  if( !dictionaryIconLoaded )
    loadIcon();
  return dictionaryIcon;
}

QIcon const & Class::getNativeIcon() throw()
{
  if( !dictionaryIconLoaded )
    loadIcon();
  return dictionaryNativeIcon;
}

void Class::loadIcon() throw()
{
  dictionaryIconLoaded = true;
}

bool Class::loadIconFromFile( QString const & _filename, bool isFullName )
{
  QFileInfo info;
  QString fileName( _filename );

  if( isFullName )
    info = QFileInfo( fileName );
  else
  {
    fileName += "bmp";
    info = QFileInfo( fileName );
    if ( !info.isFile() )
    {
      fileName.chop( 3 );
      fileName += "png";
      info = QFileInfo( fileName );
    }
    if ( !info.isFile() )
    {
      fileName.chop( 3 );
      fileName += "jpg";
      info = QFileInfo( fileName );
    }
    if ( !info.isFile() )
    {
      fileName.chop( 3 );
      fileName += "ico";
      info = QFileInfo( fileName );
    }
  }

  if ( info.isFile() )
  {
    QImage img( fileName );

    if ( !img.isNull() )
    {
      // Load successful

      // Apply the color key

      img.setAlphaChannel( img.createMaskFromColor( QColor( 192, 192, 192 ).rgb(),
                                                    Qt::MaskOutColor ) );

      dictionaryNativeIcon = QIcon( QPixmap::fromImage( img ) );

      // Transform it to be square
      int max = img.width() > img.height() ? img.width() : img.height();

      QImage result( max, max, QImage::Format_ARGB32 );
      result.fill( 0 ); // Black transparent

      QPainter painter( &result );

      painter.drawImage( QPoint( img.width() == max ? 0 : ( max - img.width() ) / 2,
                                 img.height() == max ? 0 : ( max - img.height() ) / 2 ),
                         img );

      painter.end();

      dictionaryIcon = QIcon( QPixmap::fromImage( result ) );

      return !dictionaryIcon.isNull();
    }
  }
  return false;
}

void Class::isolateCSS( QString & css, QString const & wrapperSelector )
{
  if( css.isEmpty() )
    return;

  QRegularExpression reg1( "\\/\\*(?:.(?!\\*\\/))*.?\\*\\/",
                           QRegularExpression::DotMatchesEverythingOption );
  QRegularExpression reg2( "[ \\*\\>\\+,;:\\[\\{\\]]" );
  QRegularExpression reg3( "[,;\\{]" );


  int currentPos = 0;
  QString newCSS;
  QString prefix( "#gdfrom-" );
  prefix += QString::fromLatin1( getId().c_str() );
  if ( !wrapperSelector.isEmpty() )
    prefix += " " + wrapperSelector;

  // Strip comments
  css.replace( reg1, QString() );

  for( ; ; )
  {
    if( currentPos >= css.length() )
      break;
    QChar ch = css[ currentPos ];

    if( ch == '@' )
    {
      // @ rules

      int n = currentPos;
      if( css.mid( currentPos, 7 ).compare( "@import", Qt::CaseInsensitive ) == 0
          || css.mid( currentPos, 10 ).compare( "@font-face", Qt::CaseInsensitive ) == 0
          || css.mid( currentPos, 10 ).compare( "@namespace", Qt::CaseInsensitive ) == 0
          || css.mid( currentPos, 8 ).compare( "@charset", Qt::CaseInsensitive ) == 0 )
      {
        // Copy rule as is.
        n = css.indexOf( ';', currentPos );
        int n2 = css.indexOf( '{', currentPos );
        if( n2 > 0 && n > n2 )
          n = n2 - 1;
      }
      else
      if( css.mid( currentPos, 6 ).compare( "@media", Qt::CaseInsensitive ) == 0 )
      {
        // We must to parse it content to isolate it.
        // Copy all up to '{' and continue parse inside.
        n = css.indexOf( '{', currentPos );
      }
      else
      if( css.mid( currentPos, 5 ).compare( "@page", Qt::CaseInsensitive ) == 0 )
      {
        // Don't copy rule. GD use own page layout.
        n = css.indexOf( '}', currentPos );
        if( n < 0 )
          break;
        currentPos = n + 1;
        continue;
      }
      else
      {
        // Copy rule as is.
        n = css.indexOf( '}', currentPos );
      }

      newCSS.append( css.mid( currentPos, n < 0 ? n : n - currentPos + 1 ) );

      if( n < 0 )
        break;

      currentPos = n + 1;
      continue;
    }

    if( ch == '{' )
    {
      // Selector declaration block.
      // We copy it up to '}' as is.

      int n = css.indexOf( '}', currentPos );
      newCSS.append( css.mid( currentPos, n == -1 ? n : n - currentPos + 1 ) );
      if( n < 0 )
        break;
      currentPos = n + 1;
      continue;
    }

    if( ch.isLetter() || ch == '.' || ch == '#' || ch == '*' || ch == '\\' || ch == ':' )
    {
      if( ch.isLetter() || ch == '*' )
      {
        // Check for namespace prefix
        QChar chr;
        for( int i = currentPos; i < css.length(); i++ )
        {
          chr = css[ i ];
          if( chr.isLetterOrNumber() || chr.isMark() || chr == '_' || chr == '-'
              || ( chr == '*' && i == currentPos ) )
            continue;

          if( chr == '|' )
          {
            // Namespace prefix found, copy it as is
            newCSS.append( css.mid( currentPos, i - currentPos + 1 ) );
            currentPos = i + 1;
          }
          break;
        }
        if ( chr == '|' )
          continue;
      }

      // This is some selector.
      // We must to add the isolate prefix to it.

      int n = css.indexOf( reg2, currentPos + 1 );
      QString s = css.mid( currentPos, n < 0 ? n : n - currentPos );
      if( n < 0 )
      {
        newCSS.append( s );
        break;
      }
      QString trimmed = s.trimmed();
      if( trimmed.compare( "body", Qt::CaseInsensitive ) == 0
          || trimmed.compare( "html", Qt::CaseInsensitive ) == 0 )
      {
        newCSS.append( s + " " + prefix + " " );
        currentPos += 4;
      }
      else
      {
        newCSS.append( prefix + " " );
      }

      n = css.indexOf( reg3, currentPos );
      s = css.mid( currentPos, n < 0 ? n : n - currentPos );
      newCSS.append( s );
      if( n < 0 )
        break;
      currentPos = n;
      continue;
    }

    newCSS.append( ch );
    ++currentPos;
  }
  css = newCSS;
}

string makeDictionaryId( vector< string > const & dictionaryFiles ) throw()
{
  std::vector< string > sortedList;

  if ( Config::isPortableVersion() )
  {
    // For portable version, we use relative paths
    sortedList.reserve( dictionaryFiles.size() );

    QDir dictionariesDir( Config::getPortableVersionDictionaryDir() );

    for( unsigned x = 0; x < dictionaryFiles.size(); ++x )
    {
      string const & full( dictionaryFiles[ x ] );

      QFileInfo fileInfo( FsEncoding::decode( full.c_str() ) );

      if ( fileInfo.isAbsolute() )
        sortedList.push_back( FsEncoding::encode( dictionariesDir.relativeFilePath( fileInfo.filePath() ) ) );
      else
      {
        // Well, it's relative. We don't technically support those, but
        // what the heck
        sortedList.push_back( full );
      }
    }
  }
  else
    sortedList = dictionaryFiles;

  std::sort( sortedList.begin(), sortedList.end() );

  QCryptographicHash hash( QCryptographicHash::Md5 );

  for( std::vector< string >::const_iterator i = sortedList.begin();
       i != sortedList.end(); ++i )
    hash.addData( i->c_str(), i->size() + 1 );

  return hash.result().toHex().data();
}

// While this file is not supposed to have any Qt stuff since it's used by
// the dictionary backends, there's no platform-independent way to get hold
// of a timestamp of the file, so we use here Qt anyway. It is supposed to
// be fixed in the future when it's needed.
bool needToRebuildIndex( vector< string > const & dictionaryFiles,
                         string const & indexFile ) throw()
{
  unsigned long lastModified = 0;

  for( std::vector< string >::const_iterator i = dictionaryFiles.begin();
       i != dictionaryFiles.end(); ++i )
  {
    QString name = FsEncoding::decode( i->c_str() );
    QFileInfo fileInfo( name );
    unsigned long ts;

    if( fileInfo.isDir() )
      continue;

    if( name.toLower().endsWith( ".zip" ) )
    {
      ZipFile::SplitZipFile zf( name );
      if( !zf.exists() )
        return true;
      ts = zf.lastModified().toTime_t();
    }
    else
    {
      if ( !fileInfo.exists() )
        return true;
      ts = fileInfo.lastModified().toTime_t();
    }

    if ( ts > lastModified )
      lastModified = ts;
  }

  QFileInfo fileInfo( FsEncoding::decode( indexFile.c_str() ) );

  if ( !fileInfo.exists() )
    return true;

  return fileInfo.lastModified().toTime_t() < lastModified;
}

QString generateRandomDictionaryId()
{
  return QString(
    QCryptographicHash::hash(
      QDateTime::currentDateTime().toString( "\"Random\"dd.MM.yyyy hh:mm:ss.zzz" ).toUtf8(),
      QCryptographicHash::Md5 ).toHex() );
}


}
