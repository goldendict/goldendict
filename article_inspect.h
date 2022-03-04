#ifndef ARTICLE_INSPECT_H
#define ARTICLE_INSPECT_H

#include <QDialog>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>

class article_inspect : public QDialog
{
  Q_OBJECT
  QWebEngineView * inspectView   = nullptr;
  QWebEnginePage * inspectedPage = nullptr;
public:
  article_inspect( QWidget * parent = nullptr );

  void setInspectPage( QWebEnginePage * page );

private:

  virtual void closeEvent( QCloseEvent * );
};

#endif // ARTICLE_INSPECT_H
