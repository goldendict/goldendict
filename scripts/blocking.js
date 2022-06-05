// This file contains variables and functions, loading and evaluating which blocks parsing.
// This parser blocking is not much of a problem because the scripts are local and
// instantly available. Furthermore, article requests are running in parallel to this
// script and usually take more time.

// Objects that are referenced by JavaScript code embedded in the page must be defined here.
// Functions that can be called from C++ code at any time should be defined here to make the
// functionality work (more or less) and prevent ReferenceError while the page is being loaded.

const gdAudioLinks = {
    first: null,
    current: null
};

function gdMakeArticleActive(newId) {
    const articleId = 'gdfrom-' + newId;
    if (gdCurrentArticle !== articleId) {
        var el = document.getElementById(gdCurrentArticle);
        el.className = el.className.replace(' gdactivearticle', '');
        el = document.getElementById(articleId);
        el.className = el.className + ' gdactivearticle';
        gdCurrentArticle = articleId;
        gdAudioLinks.current = newId;
        articleview.onJsActiveArticleChanged(gdCurrentArticle);
    }
}

function gdSelectArticle(id) {
    const selection = window.getSelection();
    const range = document.createRange();
    range.selectNodeContents(document.getElementById('gdfrom-' + id));
    selection.removeAllRanges();
    selection.addRange(range);
}

var overIframeId = null;

function processIframeMouseOut() {
    overIframeId = null;
    top.focus();
}

function processIframeMouseOver(newId) {
    overIframeId = newId;
}

function processIframeClick() {
    if (overIframeId != null) {
        overIframeId = overIframeId.replace('gdexpandframe-', '');
        gdMakeArticleActive(overIframeId);
    }
}

function init() {
    window.addEventListener('blur', processIframeClick, false);
}
window.addEventListener('load', init, false);

function gdExpandOptPart(expanderId, optionalId) {
    const d1 = document.getElementById(expanderId);

    function gdToggleExpanded(alt, iconFilePath, display) {
        d1.alt = alt;
        d1.src = iconFilePath;
        for (var i = 0;; i++) {
            const d2 = document.getElementById(optionalId + i);
            if (!d2)
                break;
            d2.style.display = display;
        }
    }

    if (d1.alt === '[+]')
        gdToggleExpanded('[-]', 'qrcx://localhost/icons/collapse_opt.png', 'inline');
    else
        gdToggleExpanded('[+]', 'qrcx://localhost/icons/expand_opt.png', 'none');
}

function gdExpandArticle(id) {
    const elem = document.getElementById('gdarticlefrom-' + id);
    const ico = document.getElementById('expandicon-' + id);
    const art = document.getElementById('gdfrom-' + id);
    const ev = window.event;
    var t = null;
    if (ev)
        t = ev.target || ev.srcElement;
    if (elem.style.display === 'inline' && t === ico) {
        elem.style.display = 'none';
        ico.className = 'gdexpandicon';
        art.className = art.className + ' gdcollapsedarticle';
        const nm = document.getElementById('gddictname-' + id);
        nm.style.cursor = 'pointer';
        if (ev)
            ev.stopPropagation();
        ico.title = '';
        nm.title = gdExpandArticleTitle;
    } else if (elem.style.display === 'none') {
        elem.style.display = 'inline';
        ico.className = 'gdcollapseicon';
        art.className = art.className.replace(' gdcollapsedarticle', '');
        const nm = document.getElementById('gddictname-' + id);
        nm.style.cursor = 'default';
        nm.title = '';
        ico.title = gdCollapseArticleTitle;
    }
}

function gdCheckArticlesNumber() {
    const elems = document.getElementsByClassName('gddictname');
    if (elems.length === 1) {
        var el = elems.item(0);
        const s = el.id.replace('gddictname-', '');
        el = document.getElementById('gdfrom-' + s);
        if (el && el.className.search('gdcollapsedarticle') > 0)
            gdExpandArticle(s);
    }
}
