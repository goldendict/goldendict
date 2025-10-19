// See the comment at the top of deferred.js.

// If gdCurrentArticle is invalid, activate the first (topmost) article on the page:
(function() {
    if (!gdCurrentArticle) {
        console.assert(!gdFirstArticle, "gdArticleLoaded() must have assigned gdFirstArticle to gdCurrentArticle");
        return;
    }

    if (!gdFirstArticle) {
        // Zero articles on the page => no current article.
        gdCurrentArticle = null;
        return;
    }

    if (document.getElementById(gdCurrentArticle))
        return; // gdCurrentArticle is valid => nothing to do.

    if (gdCurrentArticleHash === gdCurrentArticle || gdCurrentArticleHash === '#' + gdCurrentArticle) {
        // Don't try to scroll to the absent HTML element with id=gdCurrentArticle.
        gdCurrentArticleHash = null;
    }
    // else: if gdCurrentArticleHash is truthy, its target HTML element probably does not exist on this page since the
    // target article is missing. Try to scroll to it anyway, in case it somehow refers to another article's element.
    gdCurrentArticle = gdFirstArticle;
    gdCurrentArticleLoaded();

    // When gdCurrentArticle is invalid, ArticleView::currentArticle is empty, in which case
    // ArticleView::onJsPageInitFinished() makes the first article current. So there is no need
    // to notify gdArticleView about this function's deterministic assignment to gdCurrentArticle.
})();

if (gdArticleView)
    gdArticleView.onJsPageInitFinished();
else
    gdHasPageInitFinished = true;
