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

function gdMakeArticleActive(newId) {
    if (gdSetActiveArticle('gdfrom-' + newId))
        gdArticleView.onJsActiveArticleChanged(gdCurrentArticle);
}

function gdOnCppActiveArticleChanged(articleId, moveToIt) {
    gdOnCppActiveArticleChangedNoTimestamps(articleId, moveToIt);
}
