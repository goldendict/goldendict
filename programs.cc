/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QUrl>
#include "programs.hh"
#include "audiolink.hh"
#include "htmlescape.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include "parsecmdline.hh"
#include <QDir>
#include <QFileInfo>

namespace Programs {

using namespace Dictionary;

namespace {

class ProgramsDictionary: public Dictionary::Class
{
  Config::Program prg;
public:

  ProgramsDictionary( Config::Program const & prg_ ):
    Dictionary::Class( prg_.id.toStdString(), vector< string >() ),
    prg( prg_ )
  {
  }

  virtual string getName() throw()
  { return prg.name.toUtf8().data(); }

  virtual map< Property, string > getProperties() throw()
  { return map< Property, string >(); }

  virtual unsigned long getArticleCount() throw()
  { return 0; }

  virtual unsigned long getWordCount() throw()
  { return 0; }

  virtual sptr< WordSearchRequest > prefixMatch( wstring const & word,
                                                 unsigned long maxResults )
    throw( std::exception );

  virtual sptr< DataRequest > getArticle( wstring const &,
                                          vector< wstring > const & alts,
                                          wstring const & )
    throw( std::exception );

protected:

  virtual void loadIcon() throw();
};

sptr< WordSearchRequest > ProgramsDictionary::prefixMatch( wstring const & word,
                                                           unsigned long /*maxResults*/ )
  throw( std::exception )
{
  if ( prg.type == Config::Program::PrefixMatch )
    return new ProgramWordSearchRequest( gd::toQString( word ), prg );
  else
  {
    sptr< WordSearchRequestInstant > sr = new WordSearchRequestInstant;

    sr->setUncertain( true );

    return sr;
  }
}

sptr< Dictionary::DataRequest > ProgramsDictionary::getArticle(
  wstring const & word, vector< wstring > const &, wstring const & )
  throw( std::exception )
{
  switch( prg.type )
  {
    case Config::Program::Audio:
    {
      // Audio results are instantaneous
      string result;

      string wordUtf8( Utf8::encode( word ) );

      result += "<table class=\"programs_play\"><tr>";

      QUrl url;
      url.setScheme( "gdprg" );
      url.setHost( QString::fromUtf8( getId().c_str() ) );
      url.setPath( QString::fromUtf8( wordUtf8.c_str() ) );

      string ref = string( "\"" ) + url.toEncoded().data() + "\"";

      result += addAudioLink( ref, getId() );

      result += "<td><a href=" + ref + "><img src=\"qrcx://localhost/icons/playsound.png\" border=\"0\" alt=\"Play\"/></a></td>";
      result += "<td><a href=" + ref + ">" +
                Html::escape( wordUtf8 ) + "</a></td>";
      result += "</tr></table>";

      sptr< DataRequestInstant > ret = new DataRequestInstant( true );

      ret->getData().resize( result.size() );

      memcpy( &(ret->getData().front()), result.data(), result.size() );
      return ret;
    }

    case Config::Program::Html:
    case Config::Program::PlainText:
      return new ProgramDataRequest( gd::toQString( word ), prg );

    default:
      return new DataRequestInstant( false );
  }
}

void ProgramsDictionary::loadIcon() throw()
{
  if ( dictionaryIconLoaded )
    return;

  if( !prg.iconFilename.isEmpty() )
  {
    QFileInfo fInfo(  QDir( Config::getConfigDir() ), prg.iconFilename );
    if( fInfo.isFile() )
      loadIconFromFile( fInfo.absoluteFilePath(), true );
  }
  if( dictionaryIcon.isNull() )
    dictionaryIcon = dictionaryNativeIcon = QIcon(":/icons/programs.png");
  dictionaryIconLoaded = true;
}

}

RunInstance::RunInstance(): process( this )
{
  connect( this, SIGNAL(processFinished()), this,
           SLOT(handleProcessFinished()), Qt::QueuedConnection );
  connect( &process, SIGNAL(finished(int)), this, SIGNAL(processFinished()));
  connect( &process, SIGNAL(error(QProcess::ProcessError)), this,
           SIGNAL(processFinished()) );
}

bool RunInstance::start( Config::Program const & prg, QString const & word,
                         QString & error )
{
  QStringList args = parseCommandLine( prg.commandLine );

  if ( !args.empty() )
  {
    QString programName = args.first();
    args.pop_front();

    bool writeToStdInput = true;

    for( int x = 0; x < args.size(); ++x )
      if( args[ x ].indexOf( "%GDWORD%" ) >= 0 )
      {
        writeToStdInput = false;
        args[ x ].replace( "%GDWORD%", word );
      }

    process.start( programName, args );
    if( writeToStdInput )
    {
      process.write( word.toLocal8Bit() );
      process.closeWriteChannel();
    }

    return true;
  }
  else
  {
    error = tr( "No program name was given." );
    return false;
  }
}

void RunInstance::handleProcessFinished()
{
  // It seems that sometimes the process isn't finished yet despite being
  // signalled as such. So we wait for it here, which should hopefully be
  // nearly instant.
  process.waitForFinished();

  QByteArray output = process.readAllStandardOutput();

  QString error;
  if ( process.exitStatus() != QProcess::NormalExit )
    error = tr( "The program has crashed." );
  else
  if ( int code = process.exitCode() )
    error = tr( "The program has returned exit code %1." ).arg( code );

  if ( !error.isEmpty() )
  {
    QByteArray err = process.readAllStandardError();

    if ( !err.isEmpty() )
      error += "\n\n" + QString::fromLocal8Bit( err );
  }

  emit finished( output, error );
}

ProgramDataRequest::ProgramDataRequest( QString const & word,
                                        Config::Program const & prg_ ):
  prg( prg_ )
{
  connect( &instance, SIGNAL(finished(QByteArray,QString)),
           this, SLOT(instanceFinished(QByteArray,QString)) );

  QString error;
  if ( !instance.start( prg, word, error ) )
  {
    setErrorString( error );
    finish();
  }
}

void ProgramDataRequest::instanceFinished( QByteArray output, QString error )
{
  if ( !isFinished() )
  {
    if ( !output.isEmpty() )
    {
      string result = "<div class='programs_";

      switch( prg.type )
      {
      case Config::Program::PlainText:
        result += "plaintext'>";
        result += Html::preformat( QString::fromLocal8Bit( output ).toUtf8().data() );
        break;
      default:
        result += "html'>";
        // We assume html data is in utf8 encoding already.
        result += output.data();
      }

      result += "</div>";

      Mutex::Lock _( dataMutex );
      data.resize( result.size() );
      memcpy( data.data(), result.data(), data.size() );
      hasAnyData = true;
    }

    if ( !error.isEmpty() )
      setErrorString( error );

    finish();
  }
}

void ProgramDataRequest::cancel()
{
  finish();
}

ProgramWordSearchRequest::ProgramWordSearchRequest( QString const & word,
                                                    Config::Program const & prg_ ):
  prg( prg_ )
{
  connect( &instance, SIGNAL(finished(QByteArray,QString)),
           this, SLOT(instanceFinished(QByteArray,QString)) );

  QString error;
  if ( !instance.start( prg, word, error ) )
  {
    setErrorString( error );
    finish();
  }
}

void ProgramWordSearchRequest::instanceFinished( QByteArray output, QString error )
{
  if ( !isFinished() )
  {
    // Handle any Windows artifacts
    output.replace( "\r\n", "\n" );
    QStringList result =
      QString::fromUtf8( output ).split( "\n", QString::SkipEmptyParts );

    for( int x = 0; x < result.size(); ++x )
      matches.push_back( Dictionary::WordMatch( gd::toWString( result[ x ] ) ) );

    if ( !error.isEmpty() )
      setErrorString( error );

    finish();
  }
}

void ProgramWordSearchRequest::cancel()
{
  finish();
}

vector< sptr< Dictionary::Class > > makeDictionaries(
  Config::Programs const & programs )
  throw( std::exception )
{
  vector< sptr< Dictionary::Class > > result;

  for( Config::Programs::const_iterator i = programs.begin();
       i != programs.end(); ++i )
    if ( i->enabled )
      result.push_back( new ProgramsDictionary( *i ) );

  return result;
}

}
