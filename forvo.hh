/* This file is (c) 2008-2010 Konstantin Isakov <ikm@users.berlios.de>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __FORVO_HH_INCLUDED__
#define __FORVO_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"
#include <QNetworkAccessManager>

/// Support for Forvo pronunciations, based on its API.
namespace Forvo {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries(
                                      Dictionary::Initializing &,
                                      Config::Forvo const &,
                                      QNetworkAccessManager & )
    throw( std::exception );

/// Exposed here for moc
class ForvoDataRequestSlots: public Dictionary::DataRequest
{
  Q_OBJECT

protected slots:

  virtual void requestFinished( QNetworkReply * )
  {}
};

}

#endif
