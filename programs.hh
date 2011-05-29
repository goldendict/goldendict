/* This file is (c) 2008-2011 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __PROGRAMS_HH_INCLUDED__
#define __PROGRAMS_HH_INCLUDED__

#include <QProcess>
#include "dictionary.hh"
#include "config.hh"
#include "wstring.hh"

/// Support for arbitrary programs.
namespace Programs {

using std::vector;
using std::string;
using gd::wstring;

vector< sptr< Dictionary::Class > > makeDictionaries( Config::Programs const & )
  throw( std::exception );

class ArticleRequest: public Dictionary::DataRequest
{
  Q_OBJECT
  Config::Program prg;
  QProcess process;

public:

  ArticleRequest( QString const & word, Config::Program const & );

  virtual void cancel();

signals:
  void processFinished();
private slots:

  void handleProcessFinished();
};

}

#endif
