#ifndef ARTICLEVIEWAGENT_HH
#define ARTICLEVIEWAGENT_HH

#include <QObject>
#include "articleview.hh"

class ArticleViewAgent : public QObject
{
    Q_OBJECT
    ArticleView* articleView;
  public:
    explicit ArticleViewAgent(QObject *parent = nullptr);
    ArticleViewAgent(ArticleView* articleView);

  signals:

  public slots:
    Q_INVOKABLE void onJsActiveArticleChanged(QString const & id);
     Q_INVOKABLE void linkClickedInHtml( QUrl const & );

};

#endif // ARTICLEVIEWAGENT_HH
