/* This file is (c) 2022 Igor Kushnir <igorkuo@gmail.com>
 * Part of GoldenDict. Licensed under GPLv3 or later, see the LICENSE file */

#ifndef WEBKIT_OR_WEBENGINE_HH_INCLUDED
#define WEBKIT_OR_WEBENGINE_HH_INCLUDED

#ifdef USE_QTWEBKIT
#include <QWebPage>
#include <QWebView>
typedef QWebPage WebPage;
typedef QWebView WebView;
#else
#include <QWebEnginePage>
#include <QWebEngineView>
typedef QWebEnginePage WebPage;
typedef QWebEngineView WebView;
#endif

#endif // WEBKIT_OR_WEBENGINE_HH_INCLUDED
