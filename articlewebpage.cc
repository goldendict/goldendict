/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#include "articlewebpage.hh"
#ifndef USE_QTWEBKIT
#include "config.hh"
#endif
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
ArticleWebPage::ArticleWebPage( Config::Class & cfg_, QWebEngineProfile * profile, QObject * parent ):
  QWebEnginePage( profile, parent ), cfg( cfg_ )
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

namespace {

/// Creates a dev tools view and starts inspecting @p inspectedPage in it.
/// @p geometry a dev tools view geometry to restore.
/// @return the created dev tools view.
QWidget * createDevToolsView( QWebEnginePage & inspectedPage, QByteArray const & geometry )
{
  // Making inspectedPage->view() the parent of devToolsView embeds devToolsView into
  // inspectedPage->view() => delete devToolsView in inspectedPage's destructor instead.
  auto * const devToolsView = new QWebEngineView();

  // Destroy devToolsView when closed, because otherwise the cursor over the inspected page remains
  // a circle, hovering over links does not show a tooltip, selecting text is impossible. Furthermore,
  // if closing devToolsView does not destroy it, there is no way for the user to free up the resources
  // it uses, other than destroy the inspected page (e.g. close the tab that contains it).
  devToolsView->setAttribute( Qt::WA_DeleteOnClose, true );

  devToolsView->restoreGeometry( geometry );

  // Use the inspected page's QWebEngineProfile. This might improve performance. On the other hand,
  // if the article profile is customized further in the future, this sharing could cause issues. In that
  // case, an off-the-record profile should be used instead (probably shared between all dev tools pages).
  auto * const devToolsPage = new QWebEnginePage( inspectedPage.profile(), devToolsView );
  devToolsView->setPage( devToolsPage );

  inspectedPage.setDevToolsPage( devToolsPage );

  return devToolsView;
}

/// Unminimizes, raises and gives focus to @p devToolsView.
void resumeWebPageInspection( QWidget & devToolsView )
{
  Q_ASSERT( devToolsView.isVisible() ); // a dev tools view is destroyed when closed
  devToolsView.activateWindow();
  devToolsView.raise();
}

} // unnamed namespace

ArticleWebPage::~ArticleWebPage()
{
  // Calling deleteLater() in place of delete here prints the following error message on GoldenDict exit:
  // Release of profile requested but WebEnginePage still not deleted. Expect troubles !
  delete devToolsView();
}

void ArticleWebPage::saveConfigData() const
{
  if( auto const * view = devToolsView() )
    cfg.inspectorGeometry = view->saveGeometry();
}

void ArticleWebPage::triggerAction( WebAction action, bool checked )
{
  if( action == InspectElement )
  {
    if( auto * view = devToolsView() )
      resumeWebPageInspection( *view );
    else
    {
      QWidget * const devToolsView = createDevToolsView( *this, cfg.inspectorGeometry );

      // Connecting &ArticleWebPage::saveConfigData to &QWidget::destroyed leads to a crash in saveConfigData(), because
      // the dev tools page and the the dev tools view are no longer bound and page->view() returns nullptr then.
      connect( devToolsView, &QWidget::destroyed, this, [ devToolsView, this ] {
        cfg.inspectorGeometry = devToolsView->saveGeometry();
      } );

      devToolsView->show();
    }
  }

  QWebEnginePage::triggerAction( action, checked );
}

QWidget * ArticleWebPage::devToolsView() const
{
  if( auto const * page = devToolsPage() )
    return page->view();
  return nullptr;
}
#endif
