/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FORVO_HH_INCLUDED__
#define __FORVO_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"
#include <QNetworkAccessManager>
#include "wstring.hh"
#include <QNetworkReply>

/// Support for Forvo pronunciations, based on its API.
namespace Forvo {

using std::vector;
using std::string;
using gd::wstring;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Dictionary::Initializing &,
                                      Config::Forvo const &,
                                      QNetworkAccessManager & )
    ;

/// Exposed here for moc
class ForvoArticleRequest: public Dictionary::DataRequest
{
  Q_OBJECT

  struct NetReply
  {
    sptr< QNetworkReply > reply;
    string word;
    bool finished;

    NetReply( sptr< QNetworkReply > const & reply_, string const & word_ ):
      reply( reply_ ), word( word_ ), finished( false )
    {}
  };

  typedef std::list< NetReply > NetReplies;
  NetReplies netReplies;
  QString apiKey, languageCode;
  string dictionaryId;

public:

  ForvoArticleRequest( wstring const & word, vector< wstring > const & alts,
                       QString const & apiKey_,
                       QString const & languageCode_,
                       string const & dictionaryId_,
                       QNetworkAccessManager & mgr );

  virtual void cancel();

private:

  void addQuery( QNetworkAccessManager & mgr, wstring const & word );

private slots:
  virtual void requestFinished( QNetworkReply * );
};

}

#endif
