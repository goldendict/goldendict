#ifndef ARTICLE_INSPECT_H
#define ARTICLE_INSPECT_H

#include <QDialog>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>

class ArticleInspector : public QWidget
{
  Q_OBJECT
  QWebEngineView * viewContainer   = nullptr;
public:
  ArticleInspector( QWidget * parent = nullptr );

  void setInspectPage( QWebEngineView * view);
private:
  //used to record if the devtool was first time opened.
  //if right click on the webpage and open inspect page on the first time ,the application has great possiblity to hang forever.
  bool firstTimeOpened;
  virtual void closeEvent( QCloseEvent * );
};

#endif // ARTICLE_INSPECT_H
