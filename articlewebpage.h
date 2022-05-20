#ifndef ARTICLEWEBPAGE_H
#define ARTICLEWEBPAGE_H

#include <QWebEnginePage>

class ArticleWebPage : public QWebEnginePage
{
  Q_OBJECT
public:
  explicit ArticleWebPage( QObject * parent = nullptr );
signals:
    void linkClicked( const QUrl & url );
protected:
  virtual bool acceptNavigationRequest( const QUrl & url, NavigationType type, bool isMainFrame );
};

#endif // ARTICLEWEBPAGE_H
