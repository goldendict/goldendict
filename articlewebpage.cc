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

#ifdef USE_QTWEBKIT
ArticleWebPage::ArticleWebPage( QObject * parent ):
  WebPage( parent )
{
}

void ArticleWebPage::javaScriptConsoleMessage( QString const & message, int lineNumber, QString const & sourceID )
{
  gdWarning( "JS: %s (at %s:%d)", message.toUtf8().constData(), sourceID.toUtf8().constData(), lineNumber );
}
#else
ArticleWebPage::ArticleWebPage( QWebEngineProfile * profile, QObject * parent ):
  QWebEnginePage( profile, parent )
{
}

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

  // TODO (Qt WebEngine): when a link that does not load a new page is clicked before the current page is fully loaded,
  // the view stops updating because of QTBUG-106580. And then only the beginning of the last displayed article may be
  // visible, a few bottommost articles may be altogether missing from the page. A possible workaround is to replace the
  // anchor element (<a>) of each such link with a button (possibly image button) and pass the link's URL to ArticleView
  // via Qt WebChannel: onclick="gdArticleView.onJsLinkClicked('<url>');". Such a replacement is very time-consuming for
  // a workaround: each audio, video, other resource or external link in every dictionary format has to be replaced.
  // One more downside of this workaround is that the buttons won't work in saved articles (this in turn can be worked
  // around by falling back to assigning the URL to window.location.href if gdArticleView is falsy in a new
  // gdLinkClicked(url) helper function). Clicking a link <a href="javascript: gdLinkClicked('<url>'); void(0);"> also
  // does not stop web view updates. This alternative javascript link replacement workaround is just as time-consuming
  // to implement as the button workaround, but does not change the appearance of the links, provided
  // ArticleView::linkHovered() is adjusted accordingly.
  emit linkClicked( url );
  return false;
}
#endif
