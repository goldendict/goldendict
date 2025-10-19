// See the comment at the top of blocking.js.

gdArticleView.onJsPageInitStarted();

var gdCurrentArticle;

function gdArticleLoaded(articleId) {
    if (!gdCurrentArticle)
        gdCurrentArticle = articleId; // This is the first article. It becomes current when loaded.

    const isCurrent = articleId === gdCurrentArticle;
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
