/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include <QUrl>
#include "programs.hh"
#include "audiolink.hh"
#include "htmlescape.hh"
#include "utf8.hh"
#include "wstring_qt.hh"
#include "parsecmdline.hh"

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

  virtual QIcon getIcon() throw();

  virtual sptr< WordSearchRequest > prefixMatch( wstring const & /*word*/,
                                                 unsigned long /*maxResults*/ ) throw( std::exception )
  {
    sptr< WordSearchRequestInstant > sr = new WordSearchRequestInstant;

    sr->setUncertain( true );

    return sr;
  }

  virtual sptr< DataRequest > getArticle( wstring const &, vector< wstring > const & alts,
                                          wstring const & )
    throw( std::exception );
};

sptr< DataRequest > ProgramsDictionary::getArticle( wstring const & word,
                                                 vector< wstring > const &,
                                                 wstring const & )
  throw( std::exception )
{
  if ( prg.type == Config::Program::Audio )
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

    sptr< Dictionary::DataRequestInstant > ret =
      new Dictionary::DataRequestInstant( true );

    ret->getData().resize( result.size() );

    memcpy( &(ret->getData().front()), result.data(), result.size() );
    return ret;
  }
  else
    return new ArticleRequest( gd::toQString( word ), prg );
}

QIcon ProgramsDictionary::getIcon() throw()
{
  return QIcon( ":/icons/programs.png" );
}

}

ArticleRequest::ArticleRequest( QString const & word,
                                Config::Program const & prg_ ):
  prg( prg_ ), process( this )
{
  QStringList args = parseCommandLine( prg.commandLine );

  if ( !args.empty() )
  {
    QString programName = args.first();
    args.pop_front();

    for( int x = 0; x < args.size(); ++x )
      args[ x ].replace( "%GDWORD%", word );

    connect( this, SIGNAL(processFinished()), this,
             SLOT(handleProcessFinished()), Qt::QueuedConnection );
    connect( &process, SIGNAL(finished(int)), this, SIGNAL(processFinished()));
    connect( &process, SIGNAL(error(QProcess::ProcessError)), this,
             SIGNAL(processFinished()) );

    process.start( programName, args );
    process.write( word.toLocal8Bit() );
    process.closeWriteChannel();
  }
  else
  {
    setErrorString( tr( "No program name was given." ) );
    finish();
  }
}

void ArticleRequest::handleProcessFinished()
{
  if ( !isFinished() )
  {
    // It seems that sometimes the process isn't finished yet despite being
    // signalled as such. So we wait for it here, which should hopefully be
    // nearly instant.
    process.waitForFinished();

    QByteArray output = process.readAllStandardOutput();

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

      data.resize( result.size() );
      memcpy( data.data(), result.data(), data.size() );
      hasAnyData = true;
    }

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

      setErrorString( error );
    }

    finish();
  }
}

void ArticleRequest::cancel()
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
    result.push_back( new ProgramsDictionary( *i ) );

  return result;
}

}
