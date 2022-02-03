/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __MEDIAWIKI_HH_INCLUDED__
#define __MEDIAWIKI_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"
#include <QNetworkAccessManager>

/// Support for MediaWiki-based wikis, based on its public API.
namespace MediaWiki {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Dictionary::Initializing &,
                                      Config::MediaWikis const & wikis,
                                      QNetworkAccessManager & )
    ;

/// Exposed here for moc
class MediaWikiWordSearchRequestSlots: public Dictionary::WordSearchRequest
{
  Q_OBJECT

protected slots:

  virtual void downloadFinished()
  {}
};

/// Exposed here for moc
class MediaWikiDataRequestSlots: public Dictionary::DataRequest
{
  Q_OBJECT

protected slots:

  virtual void requestFinished( QNetworkReply * )
  {}
};

}

#endif
