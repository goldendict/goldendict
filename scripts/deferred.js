// This file contains code to be evaluated after the document has been parsed.

// Expand collapsed article if only one loaded:
(function() {
    const elems = document.getElementsByClassName('gddictname');
    if (elems.length === 1) {
        var el = elems.item(0);
        const s = el.id.replace('gddictname-', '');
        el = document.getElementById('gdfrom-' + s);
        if (el && el.className.search('gdcollapsedarticle') > 0)
            gdExpandArticle(s);
    }
})();

gdArticleView.onPageJsReady(gdAudioLinks, gdCurrentArticle);
