/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebpage.hh"
#include "gddebug.hh"

namespace {

#ifndef USE_QTWEBKIT
char const * messageLevelToString( QWebEnginePage::JavaScriptConsoleMessageLevel level )
{
  switch( level )
  {
    case QWebEnginePage::InfoMessageLevel:
      return "info";
    case QWebEnginePage::WarningMessageLevel:
      return "warning";
    case QWebEnginePage::ErrorMessageLevel:
      return "error";
  }
  return "unknown";
}
#endif

} // unnamed namespace

ArticleWebPage::ArticleWebPage( QObject * parent ):
  WebPage( parent )
{
}

#ifdef USE_QTWEBKIT
void ArticleWebPage::javaScriptConsoleMessage( QString const & message, int lineNumber, QString const & sourceID )
{
  gdWarning( "JS: %s (at %s:%d)", message.toUtf8().constData(), sourceID.toUtf8().constData(), lineNumber );
}
#else
void ArticleWebPage::javaScriptConsoleMessage( JavaScriptConsoleMessageLevel level, QString const & message,
                                               int lineNumber, QString const & sourceID )
{
  gdWarning( "JS %s: %s (at %s:%d)", messageLevelToString( level ),
             qUtf8Printable( message ), qUtf8Printable( sourceID ), lineNumber );
}

bool ArticleWebPage::acceptNavigationRequest( QUrl const & url, NavigationType type, bool isMainFrame )
{
  Q_UNUSED( isMainFrame )
  if( type != NavigationTypeLinkClicked )
    return true;
  emit linkClicked( url );
  return false;
}
#endif
