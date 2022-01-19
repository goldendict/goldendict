#include "articleviewagent.hh"

ArticleViewAgent::ArticleViewAgent(QObject *parent)
  : QObject{parent}
{

}
ArticleViewAgent::ArticleViewAgent(ArticleView *articleView)
  : articleView(articleView)
{

}

void ArticleViewAgent::onJsActiveArticleChanged(QString const & id){
    articleView->onJsActiveArticleChanged(id);
}

void ArticleViewAgent::linkClickedInHtml(QUrl const & url){
    articleView->linkClickedInHtml(url);
}
