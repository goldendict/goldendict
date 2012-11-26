/* This file is (c) 2008-2012 Konstantin Isakov <ikm@goldendict.org>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef __WEBSITE_HH_INCLUDED__
#define __WEBSITE_HH_INCLUDED__

#include "dictionary.hh"
#include "config.hh"
#include <QWebPage>

/// Support for any web sites via a templated url.
namespace WebSite {

using std::vector;
using std::string;

vector< sptr< Dictionary::Class > > makeDictionaries( Config::WebSites const & )
    throw( std::exception );

class WebDictHttpRequest: public Dictionary::DataRequest
{
  Q_OBJECT

public:
    WebDictHttpRequest( QUrl const &url_,
                        QString const &word_,
                        QString const & resultselectors_,
                        QString const & noresulttext_,
                        QString const & filter_,
                        QString const & customcss_,
                        bool usePost_ = false ) ;
    ~WebDictHttpRequest(){ delete m_webpage;}
    virtual void cancel();

protected slots:
  virtual void loaded( bool );
private:
    QWebPage * m_webpage;
    QString word, resultselectors, noresulttext, filter, icon, customcss;
};

}

#endif
