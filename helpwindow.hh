#ifndef __HELPWINDOW_HH_INCLUDED__
#define __HELPWINDOW_HH_INCLUDED__

#include <QDialog>
#include <QToolBar>
#include <QtHelp/QHelpEngine>
#include <QString>
#include <QTextBrowser>
#include <QTableWidget>
#include <QAction>
#include <QSplitter>

#include <exception>

#include "config.hh"

namespace Help {

class HelpBrowser : public QTextBrowser
{
Q_OBJECT
public:
  HelpBrowser( QHelpEngineCore * engine, QWidget *parent );
  void showHelpForKeyword( QString const & id );
private:
  QVariant loadResource( int type, QUrl const & name );
  QHelpEngineCore * helpEngine;

private slots:
  void linkClicked( QUrl const & url );
};


class HelpWindow : public QDialog
{
Q_OBJECT

  Config::Class & cfg;
  QToolBar * navToolBar;
  QHelpEngine * helpEngine;
  QTabWidget * tabWidget;
  HelpBrowser * helpBrowser;
  QAction * navForward, * navBack, * navHome;
  QAction * zoomInAction, * zoomOutAction, * zoomBaseAction;
  QSplitter * splitter;

  QString helpFile, helpCollectionFile;

  int fontSize;

  void applyZoomFactor();

public:
  HelpWindow( QWidget * parent, Config::Class & cfg_ );
  ~HelpWindow();

  QHelpEngine const * getHelpEngine()
  { return helpEngine; }

  void showHelpFor( QString const & keyword );

public slots:
  virtual void reject();
  virtual void accept();
  void forwardEnabled( bool enabled );
  void backwardEnabled( bool enabled );
  void contentsItemClicked( QModelIndex const & index );

  void zoomIn();
  void zoomOut();
  void zoomBase();

signals:
  void needClose();
};

} // namespace Help

#endif // __HELPWINDOW_HH_INCLUDED__
