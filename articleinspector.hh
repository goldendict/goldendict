#ifndef ARTICLEINSPECTOR_HH
#define ARTICLEINSPECTOR_HH

// TODO (Qt WebEngine): implement equivalent article inspector functionality using Qt WebEngine API.
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
