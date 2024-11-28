/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef ARTICLE_URLSCHEMEHANDLER_HH_INCLUDED
#define ARTICLE_URLSCHEMEHANDLER_HH_INCLUDED

#ifndef USE_QTWEBKIT

#include <QWebEngineUrlSchemeHandler>

class ArticleNetworkAccessManager;
class QWebEngineProfile;

/// Register custom article URL schemes.
/// This function should be called early at application startup, before creating any
/// Qt WebEngine classes, i.e. before a QGuiApplication or QApplication instance is created.
/// @sa QWebEngineUrlScheme
void registerArticleUrlSchemes();

class ArticleUrlSchemeHandler: public QWebEngineUrlSchemeHandler
{
  Q_OBJECT
public:
  /// @note netMgr becomes the parent of this handler object.
  explicit ArticleUrlSchemeHandler( ArticleNetworkAccessManager & netMgr );

  /// Register this handler for custom article URL schemes in @p profile.
  void install( QWebEngineProfile & profile );

  void requestStarted( QWebEngineUrlRequestJob * job ) override;

private:
  ArticleNetworkAccessManager & articleNetMgr;
};

#endif // USE_QTWEBKIT

#endif // ARTICLE_URLSCHEMEHANDLER_HH_INCLUDED
