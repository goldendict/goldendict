/* This file is (c) 2014 Abs62
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "dictserver.hh"
#include "wstring_qt.hh"
#include <QUrl>
#include <QTcpSocket>
#include <QString>
#include <list>
#include "gddebug.hh"
#include "htmlescape.hh"

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
#include <QRegularExpression>
#endif

namespace DictServer {

using namespace Dictionary;

enum {
  DefaultPort = 2628
};

namespace {

#define MAX_MATCHES_COUNT 60

bool readLine( QTcpSocket & socket, QString & line,
               QString & errorString, QAtomicInt & isCancelled )
{
  line.clear();
  errorString.clear();
  if( socket.state() != QTcpSocket::ConnectedState )
    return false;

  for( ; ; )
  {
    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      return false;

    if( socket.canReadLine() )
    {
      QByteArray reply = socket.readLine();
      line = QString::fromUtf8( reply.data(), reply.size() );
      return true;
    }

    if( !socket.waitForReadyRead( 2000 ) )
    {
      errorString = "Data reading error: socket error " + QString::number( socket.error() )
                    + ": \"" + socket.errorString() + "\"";
      break;
    }
  }
  return false;
}

bool connectToServer( QTcpSocket & socket, QString const & url,
                      QString & errorString, QAtomicInt & isCancelled )
{
  QUrl serverUrl( url );
  quint16 port = serverUrl.port( DefaultPort );
  QString reply;

  for( ; ; )
  {
    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      return false;

    socket.connectToHost( serverUrl.host(), port );

    if( socket.state() != QTcpSocket::ConnectedState )
    {
      if( !socket.waitForConnected( 5000 ) )
        break;
    }

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      return false;

    if( !readLine( socket, reply, errorString, isCancelled ) )
      break;

    if( !reply.isEmpty() && reply.left( 3 ) != "220" )
    {
      errorString = "Server refuse connection: " + reply;
      return false;
    }

    QString msgId = reply.mid( reply.lastIndexOf(" ") ).trimmed();

    socket.write( QByteArray( "CLIENT GoldenDict\r\n") );
    if( !socket.waitForBytesWritten( 1000 ) )
      break;

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      return false;

    if( !readLine( socket, reply, errorString, isCancelled ) )
      break;

    if( !serverUrl.userInfo().isEmpty() )
    {
      QString authCommand = QString( "AUTH " );
      QString authString = msgId;

      int pos = serverUrl.userInfo().indexOf( QRegExp( "[:;]" ) );
      if( pos > 0 )
      {
        authCommand += serverUrl.userInfo().left( pos );
        authString += serverUrl.userInfo().mid( pos + 1 );
      }
      else
        authCommand += serverUrl.userInfo();

      authCommand += " ";
      authCommand += QCryptographicHash::hash( authString.toUtf8(), QCryptographicHash::Md5 ).toHex();
      authCommand += "\r\n";

      socket.write( authCommand.toUtf8() );

      if( !socket.waitForBytesWritten( 1000 ) )
        break;

      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        return false;

      if( !readLine( socket, reply, errorString, isCancelled ) )
        break;

      if( reply.left( 3 ) != "230" )
      {
        errorString = "Authentication error: " + reply;
        return false;
      }
    }

    socket.write( QByteArray( "OPTION MIME\r\n" ) );

    if( !socket.waitForBytesWritten( 1000 ) )
      break;

    if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
      return false;

    if( !readLine( socket, reply, errorString, isCancelled ) )
      break;

    if( reply.left( 3 ) != "250" )
    {
      // RFC 2229, 3.10.1.1:
      // OPTION MIME is a REQUIRED server capability,
      // all DICT servers MUST implement this command.
      errorString = "Server doesn't support mime capability: " + reply;
      return false;
    }

    return true;
  }

  if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    errorString = QString( "Server connection fault, socket error %1: \"%2\"" )
                  .arg( QString::number( socket.error() ) )
                  .arg( socket.errorString() );
  return false;
}

void disconnectFromServer( QTcpSocket & socket )
{
  if( socket.state() == QTcpSocket::ConnectedState )
    socket.write( QByteArray( "QUIT\r\n" ) );

  socket.disconnectFromHost();
}

class DictServerDictionary: public Dictionary::Class
{
  string name;
  QString url, icon;
  quint32 langId;
  QString errorString;
  QTcpSocket socket;
  QStringList databases;
  QStringList strategies;
  QStringList serverDatabases;

public:

  DictServerDictionary( string const & id, string const & name_,
                        QString const & url_,
                        QString const & database_,
                        QString const & strategies_,
                        QString const & icon_ ):
    Dictionary::Class( id, vector< string >() ),
    name( name_ ),
    url( url_ ),
    icon( icon_ ),
    langId( 0 )
  {
    int pos = url.indexOf( "://" );
    if( pos < 0 )
      url = "dict://" + url;

    databases = database_.split( QRegExp( "[ ,;]" ), QString::SkipEmptyParts );
    if( databases.isEmpty() )
      databases.append( "*" );

    strategies = strategies_.split( QRegExp( "[ ,;]" ), QString::SkipEmptyParts );
    if( strategies.isEmpty() )
      strategies.append( "prefix" );
  }

  virtual string getName() throw()
  { return name; }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const &,
                                                 unsigned long maxResults ) THROW_SPEC( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &, vector< wstring > const & alts,
                                          wstring const &, bool )
    THROW_SPEC( std::exception );

  virtual quint32 getLangFrom() const
  { return langId; }

  virtual quint32 getLangTo() const
  { return langId; }

  virtual QString const & getDescription();
protected:

  virtual void loadIcon() throw();

  void getServerDatabases();

  friend class DictServerWordSearchRequest;
  friend class DictServerArticleRequest;
};

void DictServerDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  if( !icon.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), icon );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/network.png");
  dictionaryIconLoaded = true;
}

QString const & DictServerDictionary::getDescription()
{
  if( serverDatabases.isEmpty() )
  {
    dictionaryDescription.clear();
    getServerDatabases();
  }

  if( dictionaryDescription.isEmpty() )
  {
    dictionaryDescription = QCoreApplication::translate( "DictServer", "Url: " ) + url + "\n";
    dictionaryDescription += QCoreApplication::translate( "DictServer", "Databases: " ) + databases.join( ", " ) + "\n";
    dictionaryDescription += QCoreApplication::translate( "DictServer", "Search strategies: " ) + strategies.join( ", " );
    if( !serverDatabases.isEmpty() )
    {
      dictionaryDescription += "\n\n";
      dictionaryDescription += QCoreApplication::translate( "DictServer", "Server databases" )
                               + " (" + QString::number( serverDatabases.size()) + "):";
      for( QStringList::const_iterator i = serverDatabases.begin(); i != serverDatabases.end(); ++i )
        dictionaryDescription += "\n" + *i;
    }
  }
  return dictionaryDescription;
}

void DictServerDictionary::getServerDatabases()
{
  QAtomicInt isCancelled;
  QTcpSocket * socket = new QTcpSocket;

  if( !socket )
    return;

  if( connectToServer( *socket, url, errorString, isCancelled ) )
  {
    for( ; ; )
    {
      QString req = QString( "SHOW DB\r\n" );
      socket->write( req.toUtf8() );
      socket->waitForBytesWritten( 1000 );

      QString reply;

      if( !readLine( *socket, reply, errorString, isCancelled ) )
        return;

      if( reply.left( 3 ) == "110" )
      {
        int countPos = reply.indexOf( ' ', 4 );
        // Get databases count
        int count = reply.mid( 4, countPos > 4 ? countPos - 4 : -1 ).toInt();

        // Read databases
        for( int x = 0; x < count; x++ )
        {
          if( !readLine( *socket, reply, errorString, isCancelled ) )
             break;

          if( reply[ 0 ] == '.' )
            break;

          while( reply.endsWith( '\r' ) || reply.endsWith( '\n' ) )
            reply.chop( 1 );

          if( !reply.isEmpty() )
            serverDatabases.append( reply );
        }

        break;
      }
      else
      {
        gdWarning( "Retrieving databases from \"%s\" fault: %s\n",
                   getName().c_str(), reply.toUtf8().data() );
        break;
      }
    }
    disconnectFromServer( *socket );
  }

  if( !errorString.isEmpty() )
    gdWarning( "Retrieving databases from \"%s\" fault: %s\n",
               getName().c_str(), errorString.toUtf8().data() );
  delete socket;
}

class DictServerWordSearchRequest;

class DictServerWordSearchRequestRunnable: public QRunnable
{
  DictServerWordSearchRequest & r;
  QSemaphore & hasExited;

public:

  DictServerWordSearchRequestRunnable( DictServerWordSearchRequest & r_,
                                       QSemaphore & hasExited_ ): r( r_ ),
                                       hasExited( hasExited_ )
  {}

  ~DictServerWordSearchRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class DictServerWordSearchRequest: public Dictionary::WordSearchRequest
{
  QAtomicInt isCancelled;
  wstring word;
  QString errorString;
  QSemaphore hasExited;
  DictServerDictionary & dict;
  QTcpSocket * socket;

public:

  DictServerWordSearchRequest( wstring const & word_,
                               DictServerDictionary & dict_ ) :
    word( word_ ),
    dict( dict_ ),
    socket( 0 )
  {
    QThreadPool::globalInstance()->start(
      new DictServerWordSearchRequestRunnable( *this, hasExited ) );
  }

  void run();

  ~DictServerWordSearchRequest()
  {
    hasExited.acquire();
  }

  virtual void cancel();

};

void DictServerWordSearchRequestRunnable::run()
{
  r.run();
}

void DictServerWordSearchRequest::run()
{
  if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
  {
    finish();
    return;
  }

  socket = new QTcpSocket;

  if( !socket )
  {
    finish();
    return;
  }

  if( connectToServer( *socket, dict.url, errorString, isCancelled ) )
  {
    QStringList matchesList;

    for( int ns = 0; ns < dict.strategies.size(); ns++ )
    {
      for( int i = 0; i < dict.databases.size(); i++ )
      {
        QString matchReq = QString( "MATCH " )
                           + dict.databases.at( i )
                           + " " + dict.strategies.at( ns )
                           + " \"" + gd::toQString( word )
                           + "\"\r\n";
        socket->write( matchReq.toUtf8() );
        socket->waitForBytesWritten( 1000 );

        if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
          break;

        QString reply;

        if( !readLine( *socket, reply, errorString, isCancelled ) )
          break;

        if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
          break;

        if( reply.left( 3 ) == "250" )
        {
          // "OK" reply - matches info will be later
          if( !readLine( *socket, reply, errorString, isCancelled ) )
            break;

          if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
            break;
        }

        if( reply.left( 3 ) == "552" )
        {
          // No matches
          continue;
        }

        if( reply[ 0 ] == '5' || reply[ 0 ] == '4' )
        {
          // Database error
          gdWarning( "Find matches in \"%s\", database \"%s\", strategy \"%s\" fault: %s\n",
                     dict.getName().c_str(), dict.databases.at( i ).toUtf8().data(),
                     dict.strategies.at( ns ).toUtf8().data(), reply.toUtf8().data() );
          continue;
        }

        if( reply.left( 3 ) == "152" )
        {
          // Matches found
          int countPos = reply.indexOf( ' ', 4 );

          // Get matches count
          int count = reply.mid( 4, countPos > 4 ? countPos - 4 : -1 ).toInt();

          // Read matches
          for( int x = 0; x <= count; x++ )
          {
            if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
              break;

            if( !readLine( *socket, reply, errorString, isCancelled ) )
              break;

            if( reply[ 0 ] == '.' )
              break;

            while( reply.endsWith( '\r' ) || reply.endsWith( '\n' ) )
              reply.chop( 1 );

            int pos = reply.indexOf( ' ' );
            if( pos >= 0 )
            {
              QString word = reply.mid( pos + 1 );
              if( word.endsWith( '\"') )
                word.chop( 1 );
              if( word[ 0 ] ==  '\"' )
                word = word.mid( 1 );

              matchesList.append( word );
            }
          }
          if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) || !errorString.isEmpty() )
            break;
        }
      }

      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) || !errorString.isEmpty() )
        break;

      matchesList.removeDuplicates();
      if( matchesList.size() >= MAX_MATCHES_COUNT )
        break;
    }

    if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) && errorString.isEmpty() )
    {
      matchesList.removeDuplicates();

      int count = matchesList.size();
      if( count > MAX_MATCHES_COUNT )
        count = MAX_MATCHES_COUNT;

      if( count )
      {
        Mutex::Lock _( dataMutex );
        for( int x = 0; x < count; x++ )
          matches.push_back( gd::toWString( matchesList.at( x ) ) );
      }
    }
  }

  if( !errorString.isEmpty() )
    gdWarning( "Prefix find in \"%s\" fault: %s\n", dict.getName().c_str(),
                errorString.toUtf8().data() );

  if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    socket->abort();
  else
    disconnectFromServer( *socket );

  delete socket;
  if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    finish();
}

void DictServerWordSearchRequest::cancel()
{
  isCancelled.ref();

  Mutex::Lock _( dataMutex );
  finish();
}

class DictServerArticleRequest;

class DictServerArticleRequestRunnable: public QRunnable
{
  DictServerArticleRequest & r;
  QSemaphore & hasExited;

public:

  DictServerArticleRequestRunnable( DictServerArticleRequest & r_,
                                    QSemaphore & hasExited_ ): r( r_ ),
                                    hasExited( hasExited_ )
  {}

  ~DictServerArticleRequestRunnable()
  {
    hasExited.release();
  }

  virtual void run();
};

class DictServerArticleRequest: public Dictionary::DataRequest
{
  QAtomicInt isCancelled;
  wstring word;
  QString errorString;
  QSemaphore hasExited;
  DictServerDictionary & dict;
  QTcpSocket * socket;

public:

  DictServerArticleRequest( wstring const & word_,
                            DictServerDictionary & dict_ ) :
    word( word_ ),
    dict( dict_ ),
    socket( 0 )
  {
    QThreadPool::globalInstance()->start(
      new DictServerArticleRequestRunnable( *this, hasExited ) );
  }

  void run();

  ~DictServerArticleRequest()
  {
    hasExited.acquire();
  }

  virtual void cancel();

};

void DictServerArticleRequestRunnable::run()
{
  r.run();
}

void DictServerArticleRequest::run()
{
  if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
  {
    finish();
    return;
  }

  socket = new QTcpSocket;

  if( !socket )
  {
    finish();
    return;
  }

  if( connectToServer( *socket, dict.url, errorString, isCancelled ) )
  {
    string articleData;

    for( int i = 0; i < dict.databases.size(); i++ )
    {
      QString defineReq = QString( "DEFINE " )
                         + dict.databases.at( i )
                         + " \"" + gd::toQString( word ) + "\"\r\n";
      socket->write( defineReq.toUtf8() );
      socket->waitForBytesWritten( 1000 );

      QString reply;

      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        break;

      if( !readLine( *socket, reply, errorString, isCancelled ) )
        break;

      if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
        break;

      if( reply.left( 3 ) == "250" )
      {
        // "OK" reply - matches info will be later
        if( !readLine( *socket, reply, errorString, isCancelled ) )
          break;

        if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
          break;
      }

      if( reply.left( 3 ) == "552" )
      {
        // No matches found
        continue;
      }

      if( reply[ 0 ] == '5' || reply[ 0 ] == '4' )
      {
        // Database error
        gdWarning( "Articles request from \"%s\", database \"%s\" fault: %s\n",
                   dict.getName().c_str(), dict.databases.at( i ).toUtf8().data(),
                   reply.toUtf8().data() );
        continue;
      }

      if( reply.left( 3 ) == "150" )
      {
        // Articles found
        int countPos = reply.indexOf( ' ', 4 );

        QString articleText;

        // Get articles count
        int count = reply.mid( 4, countPos > 4 ? countPos - 4 : -1 ).toInt();

        // Read articles
        for( int x = 0; x < count; x++ )
        {
          if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
            break;

          if( !readLine( *socket, reply, errorString, isCancelled ) )
            break;

          if( reply.left( 3 ) == "250" )
            break;

          if( reply.left( 3 ) == "151" )
          {
            int pos = 4;
            int endPos;

            // Skip requested word
            if( reply[ pos ] == '\"' )
              endPos = reply.indexOf( '\"', pos + 1 ) + 1;
            else
              endPos = reply.indexOf( ' ', pos );

            if( endPos < pos )
            {
              // It seems mailformed string
              break;
            }

            pos = endPos + 1;

            QString dbID, dbName;

            // Retrieve database ID
            endPos = reply.indexOf( ' ', pos );

            if( endPos < pos )
            {
              // It seems mailformed string
              break;
            }

            dbID = reply.mid( pos, endPos - pos );

            // Retrieve database ID
            pos = endPos + 1;
            endPos = reply.indexOf( ' ', pos );
            if( reply[ pos ] == '\"' )
              endPos = reply.indexOf( '\"', pos + 1 ) + 1;
            else
              endPos = reply.indexOf( ' ', pos );

            if( endPos < pos )
            {
              // It seems mailformed string
              break;
            }

            dbName = reply.mid( pos, endPos - pos );
            if( dbName.endsWith( '\"' ) )
              dbName.chop( 1 );
            if( dbName[ 0 ] == '\"' )
              dbName = dbName.mid( 1 );

            articleData += string( "<div class=\"dictserver_from\">From " )
                           + dbName.toUtf8().data()
                           + " [" + dbID.toUtf8().data() + "]:"
                           + "</div>";

            // Retreive MIME headers if any

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
            static QRegularExpression contentTypeExpr( "Content-Type\\s*:\\s*text/html",
                                                       QRegularExpression::CaseInsensitiveOption );
#else
            QRegExp contentTypeExpr( "Content-Type\\s*:\\s*text/html", Qt::CaseInsensitive );
#endif
            bool contentInHtml = false;
            for( ; ; )
            {
              if( !readLine( *socket, reply, errorString, isCancelled ) )
                break;

              if( reply == "\r\n" )
                break;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
              QRegularExpressionMatch match = contentTypeExpr.match( reply );
              if( match.hasMatch() )
                contentInHtml = true;
#else
              if( contentTypeExpr.indexIn( reply ) >= 0 )
                contentInHtml = true;
#endif
            }

            // Retrieve article text

            articleText.clear();
            for( ; ; )
            {
              if( !readLine( *socket, reply, errorString, isCancelled ) )
                break;

              if( reply == ".\r\n" )
                break;

              articleText += reply;
            }

            if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) || !errorString.isEmpty() )
              break;

#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
            static QRegularExpression phonetic( "\\\\([^\\\\]+)\\\\",
                                                QRegularExpression::CaseInsensitiveOption ); // phonetics: \stuff\ ...
            static QRegularExpression divs_inside_phonetic( "</div([^>]*)><div([^>]*)>",
                                                            QRegularExpression::CaseInsensitiveOption );
            static QRegularExpression refs( "\\{([^\\{\\}]+)\\}",
                                            QRegularExpression::CaseInsensitiveOption );     // links: {stuff}
            static QRegularExpression links( "<a href=\"gdlookup://localhost/([^\"]*)\">",
                                             QRegularExpression::CaseInsensitiveOption );
            static QRegularExpression tags( "<[^>]*>",
                                            QRegularExpression::CaseInsensitiveOption );
#else
            QRegExp phonetic( "\\\\([^\\\\]+)\\\\", Qt::CaseInsensitive ); // phonetics: \stuff\ ...
            QRegExp divs_inside_phonetic( "</div([^>]*)><div([^>]*)>", Qt::CaseInsensitive );
            QRegExp refs( "\\{([^\\{\\}]+)\\}", Qt::CaseInsensitive );     // links: {stuff}
            QRegExp links( "<a href=\"gdlookup://localhost/([^\"]*)\">", Qt::CaseInsensitive );
            QRegExp tags( "<[^>]*>", Qt::CaseInsensitive );
#endif
            string articleStr;
            if( contentInHtml )
              articleStr = articleText.toUtf8().data();
            else
              articleStr = Html::preformat( articleText.toUtf8().data() );

            articleText = QString::fromUtf8( articleStr.c_str(), articleStr.size() );
            if( !contentInHtml )
            {
              articleText = articleText.replace(refs, "<a href=\"gdlookup://localhost/\\1\">\\1</a>" );

              pos = 0;
#if QT_VERSION >= QT_VERSION_CHECK( 5, 0, 0 )
              QString articleNewText;

              // Handle phonetics

              QRegularExpressionMatchIterator it = phonetic.globalMatch( articleText );
              while( it.hasNext() )
              {
                QRegularExpressionMatch match = it.next();
                articleNewText += articleText.midRef( pos, match.capturedStart() - pos );
                pos = match.capturedEnd();

                QString phonetic_text = match.captured( 1 );
                phonetic_text.replace( divs_inside_phonetic, "</span></div\\1><div\\2><span class=\"dictd_phonetic\">" );

                articleNewText += "<span class=\"dictd_phonetic\">" + phonetic_text + "</span>";
              }
              if( pos )
              {
                articleNewText += articleText.midRef( pos );
                articleText = articleNewText;
                articleNewText.clear();
              }

              // Handle links

              pos = 0;
              it = links.globalMatch( articleText );
              while( it.hasNext() )
              {
                QRegularExpressionMatch match = it.next();
                articleNewText += articleText.midRef( pos, match.capturedStart() - pos );
                pos = match.capturedEnd();

                QString link = match.captured( 1 );
                link.replace( tags, " " );
                link.replace( "&nbsp;", " " );

                QString newLink = match.captured();
                newLink.replace( 30, match.capturedLength( 1 ),
                                 QString::fromUtf8( QUrl::toPercentEncoding( link.simplified() ) ) );
                articleNewText += newLink;
              }
              if( pos )
              {
                articleNewText += articleText.midRef( pos );
                articleText = articleNewText;
                articleNewText.clear();
              }
#else
              // Handle phonetics

              for( ; ; )
              {
                pos = articleText.indexOf( phonetic, pos );
                if( pos < 0 )
                  break;

                QString phonetic_text = phonetic.cap( 1 );
                phonetic_text.replace( divs_inside_phonetic, "</span></div\\1><div\\2><span class=\"dictd_phonetic\">" );
                phonetic_text = "<span class=\"dictd_phonetic\">" + phonetic_text + "</span>";
                articleText.replace( pos, phonetic.cap().length(), phonetic_text );
                pos += phonetic_text.length();
              }

              // Handle links

              pos = 0;
              for( ; ; )
              {
                pos = articleText.indexOf( links, pos );
                if( pos < 0 )
                  break;

                QString link = links.cap( 1 );
                link.replace( tags, " " );
                link.replace( "&nbsp;", " " );
                articleText.replace( pos + 30, links.cap( 1 ).length(),
                                     QString::fromUtf8( QUrl::toPercentEncoding( link.simplified() ) ) );
                pos += 30;
              }
#endif
            }

            articleData += string( "<div class=\"dictd_article\">" )
                           + articleText.toUtf8().data()
                           + "<br></div>";
          }

          if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) || !errorString.isEmpty() )
            break;
        }
      }
    }
    if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) && errorString.isEmpty() && !articleData.empty() )
    {
      Mutex::Lock _( dataMutex );

      data.resize( articleData.size() );
      memcpy( &data.front(), articleData.data(), articleData.size() );

      hasAnyData = true;
    }
  }

  if( !errorString.isEmpty() )
    gdWarning( "Articles request from \"%s\" fault: %s\n", dict.getName().c_str(),
                errorString.toUtf8().data() );

  if( Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    socket->abort();
  else
    disconnectFromServer( *socket );

  delete socket;
  if( !Qt4x5::AtomicInt::loadAcquire( isCancelled ) )
    finish();
}

void DictServerArticleRequest::cancel()
{
  isCancelled.ref();

  Mutex::Lock _( dataMutex );
  finish();
}

sptr< WordSearchRequest > DictServerDictionary::prefixMatch( wstring const & word,
                                                             unsigned long maxResults )
  THROW_SPEC( std::exception )
{
  (void) maxResults;
  if ( word.size() > 80 )
  {
    // Don't make excessively large queries -- they're fruitless anyway

    return new WordSearchRequestInstant();
  }
  else
    return new DictServerWordSearchRequest( word, *this );
}

sptr< DataRequest > DictServerDictionary::getArticle( wstring const & word,
                                                      vector< wstring > const &,
                                                      wstring const &, bool )
  THROW_SPEC( std::exception )
{
  if ( word.size() > 80 )
  {
    // Don't make excessively large queries -- they're fruitless anyway

    return new DataRequestInstant( false );
  }
  else
    return new DictServerArticleRequest( word, *this );
}

} // Anonimuos namespace

vector< sptr< Dictionary::Class > > makeDictionaries( Config::DictServers const & servers )
  THROW_SPEC( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  for( int x = 0; x < servers.size(); ++x )
  {
    if ( servers[ x ].enabled )
      result.push_back( new DictServerDictionary( servers[ x ].id.toStdString(),
                                                  servers[ x ].name.toUtf8().data(),
                                                  servers[ x ].url,
                                                  servers[ x ].databases,
                                                  servers[ x ].strategies,
                                                  servers[ x ].iconFilename  ) );
  }

  return result;
}

} // namespace DictServer
