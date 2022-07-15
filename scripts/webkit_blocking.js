// See the comment at the top of blocking.js.

gdArticleView.onJsPageInitStarted();

var gdCurrentArticle;

function gdArticleLoaded(articleId) {
    const isCurrent = !gdCurrentArticle;
    if (isCurrent)
        gdCurrentArticle = articleId; // This is the first article. It becomes current when loaded.

    gdArticleView.onJsArticleLoaded(articleId, gdJustLoadedAudioLink, isCurrent);
    gdJustLoadedAudioLink = null;
}
