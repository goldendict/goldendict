/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ARTICLEWEBPAGE_HH_INCLUDED
#define ARTICLEWEBPAGE_HH_INCLUDED

#include <QWebPage>

class ArticleWebPage: public QWebPage
{
  Q_OBJECT
public:
  explicit ArticleWebPage( QObject * parent = 0 );

protected:
  virtual void javaScriptConsoleMessage( QString const & message, int lineNumber, QString const & sourceID );
};

#endif // ARTICLEWEBPAGE_HH_INCLUDED
