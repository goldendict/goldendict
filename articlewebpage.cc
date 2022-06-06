/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebpage.hh"
#include "gddebug.hh"

ArticleWebPage::ArticleWebPage( QObject * parent ):
  QWebPage( parent )
{
}

void ArticleWebPage::javaScriptConsoleMessage( QString const & message, int lineNumber, QString const & sourceID )
{
  gdWarning( "JS: %s (at %s:%d)", message.toUtf8().constData(), sourceID.toUtf8().constData(), lineNumber );
}
