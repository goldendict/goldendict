#ifndef ARTICLEINSPECTOR_HH
#define ARTICLEINSPECTOR_HH

// TODO (Qt WebKit): drop ArticleInspector and code that uses it in ArticleWebView, share the cleaner
// alternative implementation in ArticleWebPage with the Qt WebEngine version once Qt 4 is no longer
// supported. The problem with Qt 4 is: destroying an article inspector when it is closed, then
// creating another one for the same page causes a crash in QWebInspector::setPage().

#ifdef USE_QTWEBKIT

#include <QWebInspector>
#include <list>
#include "config.hh"
#include "ex.hh"

class ArticleInspector : public QWebInspector
{
Q_OBJECT

public:
  DEF_EX( exInit, "Article inspector failed to init", std::exception )

  explicit ArticleInspector( Config::Class * cfg,  QWidget* parent = 0 );
  ~ArticleInspector();

public slots:
  void beforeClosed();

protected:
  void showEvent( QShowEvent *event );

private:
  Config::Class * cfg;

  static std::list< ArticleInspector * > openedInspectors;
};

#endif // USE_QTWEBKIT

#endif // ARTICLEINSPECTOR_HH
