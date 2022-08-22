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

  void setInspectPage( QWebEnginePage * page);
private:

  virtual void closeEvent( QCloseEvent * );
};

#endif // ARTICLE_INSPECT_H
