// See the comment at the top of blocking.js.

let gdArticleView;

let gdCurrentArticle;

// The topmost article on the page or undefined if there are no articles (yet).
let gdFirstArticle;

// If truthy, contains a fragment string that points to an element within gdCurrentArticle.
// This fragment string is appended to location.href once gdCurrentArticle is loaded.
let gdCurrentArticleHash;

// These 3 variables are temporary and are permanently set to null once gdArticleView becomes available.
let gdPendingArticles = [];
let gdPendingAudioLinks = [];
let gdHasPageInitFinished = false;

// Name the unused gdWebChannel variable to silence Qt Creator's M127 Warning:
// Expression statements should be assignments, calls or delete expressions only
const gdWebChannel = new QWebChannel(qt.webChannelTransport, function(channel) {
    gdArticleView = channel.objects.gdArticleView;

    gdArticleView.onJsPageInitStarted(gdPendingArticles, gdPendingAudioLinks,
        gdPendingArticles.indexOf(gdCurrentArticle), gdHasPageInitFinished);
    gdPendingArticles = null;
    gdPendingAudioLinks = null;
    gdHasPageInitFinished = null;
});

// Save the current article in web history before leaving the page. The current article
// is restored when the user navigates to a page that has been visited before.
window.addEventListener('pagehide', function(event) {
    history.replaceState(gdCurrentArticle, '');
});

// 1. Initialize gdCurrentArticle. 2. Set gdCurrentArticleHash or add a scrolling event listener.
(function() {
    // Returns true if this page is being reloaded; false otherwise.
    function isPageReloading() {
        return performance.getEntriesByType('navigation').some(nav => nav.type === 'reload');
    }

    // The pagehide event occurs just fine when a page starts reloading. However, for some reason
    // the current article saved in web history cannot be restored in this function upon page reloading
    // - either history.state is null, or, if the current article has been stored the previous
    // time this page was loaded, history.state contains an out-of-date current article value.
    // To work this issue around, the C++ side injects two variables on document creation:
    // 1) the current article to be restored when the page is reloaded - gdCurrentArticleBeforePageReloading;
    // 2) whether to scroll to this restored current article - gdScrollToCurrentArticleAfterPageReloading.
    if (gdCurrentArticleBeforePageReloading && isPageReloading()) {
        gdCurrentArticle = gdCurrentArticleBeforePageReloading;
        if (gdScrollToCurrentArticleAfterPageReloading) {
            // Only scrolling after (not even on) the load event overrides the web engine's
            // automatic window position restoration, which the C++ side declared undesirable
            // by setting gdScrollToCurrentArticleAfterPageReloading to true.
            window.addEventListener('load', function() {
                setTimeout(function() {
                    if (!gdWasCurrentArticleSetExplicitly)
                        document.getElementById(gdCurrentArticle).scrollIntoView();
                    // else: the user has managed to activate an article already => don't scroll to it.
                }, 0);
            });
        }
        return;
    }

    gdCurrentArticle = history.state;
    if (gdCurrentArticle) {
        // Restored current article from web history. The web engine will restore the window position automatically.
        // Scrolling the mouse wheel before the page is fully loaded prevents this window position restoration.
        return;
    }

    const searchParams = new URLSearchParams(location.search);

    // Initialize gdCurrentArticle to the scrollto query item value of the page's URL.
    gdCurrentArticle = searchParams.get('scrollto');

    if (location.hash && location.hash !== '#')
        return; // The page's URL has a non-empty fragment, which overrides scrolling to scrollto or gdanchor.

    // TODO (Qt WebEngine): port MDict gdanchor pattern support from Qt WebKit's scrollToGdAnchor() to JavaScript.
    // The answers to the following question can help to implement this:
    // https://stackoverflow.com/questions/16791527/can-i-use-a-regular-expression-in-queryselectorall
    // Hopefully the substring matching attribute selectors ([att^=val], [att$=val]) would suffice, and
    // the more complex DOMRegex GitHub Gist referenced in a comment to the second answer won't be needed.
    gdCurrentArticleHash = searchParams.get('gdanchor');

    if (!gdCurrentArticleHash)
        gdCurrentArticleHash = gdCurrentArticle; // Scroll to the target article if there is no fragment.

    if (gdCurrentArticleHash) {
        // Unless location.hash equals '#' and consequently location.href ends with '#', make gdCurrentArticleHash
        // start with '#'. This ensures a single '#' character in (location.href + gdCurrentArticleHash).
        if (!location.hash)
            gdCurrentArticleHash = '#' + gdCurrentArticleHash;
    }
})();

function gdCurrentArticleLoaded() {
    document.getElementById(gdCurrentArticle).className += ' gdactivearticle';

    // Unlike Element.scrollIntoView(), setting the URL fragment works correctly in background tabs.
    // When location.hash is set after the HTML element with the corresponding id has been loaded, the
    // scrolling is immediate. Another benefit of setting the fragment when the requisite article is
    // loaded: the scrolling is automatically canceled if the user activates another article earlier,
    // because that activated article must have been loaded already, and so won't be loaded again.
    if (gdCurrentArticleHash) {
        location.replace(location.href + gdCurrentArticleHash);
        gdCurrentArticleHash = null;
    }
}

function gdArticleLoaded(articleId) {
    if (!gdFirstArticle) {
        gdFirstArticle = articleId;
        if (!gdCurrentArticle)
            gdCurrentArticle = gdFirstArticle;
    }

    const isCurrent = articleId === gdCurrentArticle;
    if (isCurrent)
        gdCurrentArticleLoaded();

    if (gdArticleView)
        gdArticleView.onJsArticleLoaded(articleId, gdJustLoadedAudioLink, isCurrent);
    else {
        gdPendingArticles.push(articleId);
        gdPendingAudioLinks.push(gdJustLoadedAudioLink);
    }
    gdJustLoadedAudioLink = null;
}
