#ifndef ARTICLEWEBPAGE_H
#define ARTICLEWEBPAGE_H

#include <QWebEnginePage>

struct LastReqInfo{
  QString group;
  QString mutedDicts;
};

class ArticleWebPage : public QWebEnginePage
{
  Q_OBJECT
public:
  explicit ArticleWebPage( QObject * parent = nullptr );
signals:
    void linkClicked( const QUrl & url );
protected:
  virtual bool acceptNavigationRequest( const QUrl & url, NavigationType type, bool isMainFrame );
private:
  LastReqInfo lastReq;
};

#endif // ARTICLEWEBPAGE_H
