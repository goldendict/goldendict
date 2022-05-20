#ifndef ARTICLE_INSPECT_H
#define ARTICLE_INSPECT_H

#include <QDialog>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>

class ArticleInspector : public QWidget
{
  Q_OBJECT
  QWebEngineView * inspectView   = nullptr;
  QWebEnginePage * inspectedPage = nullptr;
public:
  ArticleInspector( QWidget * parent = nullptr );

  void setInspectPage( QWebEngineView * view);
private:

  virtual void closeEvent( QCloseEvent * );
};

#endif // ARTICLE_INSPECT_H
