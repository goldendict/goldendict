/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ARTICLEWEBPAGE_HH_INCLUDED
#define ARTICLEWEBPAGE_HH_INCLUDED

#include "webkit_or_webengine.hh"

/// Note: this class always delegates all links in the Qt WebEngine version.
class ArticleWebPage: public WebPage
{
  Q_OBJECT
public:
#ifdef USE_QTWEBKIT
  explicit ArticleWebPage( QObject * parent = 0 );
#else
  explicit ArticleWebPage( QWebEngineProfile * profile, QObject * parent = nullptr );

signals:
  /// This signal is emitted whenever the user clicks on a link.
  void linkClicked( QUrl const & url );
#endif

protected:
#ifdef USE_QTWEBKIT
  virtual void javaScriptConsoleMessage( QString const & message, int lineNumber, QString const & sourceID );
#else
  void javaScriptConsoleMessage( JavaScriptConsoleMessageLevel level, QString const & message,
                                 int lineNumber, QString const & sourceID ) override;
  bool acceptNavigationRequest( QUrl const & url, NavigationType type, bool isMainFrame ) override;
#endif
};

#endif // ARTICLEWEBPAGE_HH_INCLUDED
