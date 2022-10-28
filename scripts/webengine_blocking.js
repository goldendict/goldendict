// See the comment at the top of blocking.js.

let gdArticleView;

let gdCurrentArticle;

// The topmost article on the page or undefined if there are no articles (yet).
let gdFirstArticle;

// If truthy, contains a fragment string that points to an element within gdCurrentArticle.
// This fragment string is appended to location.href once gdCurrentArticle is loaded.
let gdCurrentArticleHash;

// The timestamps prevent timing issues when a current article is passed to/from C++.
// More details in the comment above the corresponding ArticleView's timestamp data members.
const gdPageTimestamp = new Date();
let gdCurrentArticleTimestamp = gdPageTimestamp;

// These 3 variables are temporary and are permanently set to null once gdArticleView becomes available.
let gdPendingArticles = [];
let gdPendingAudioLinks = [];
let gdHasPageInitFinished = false;

// Name the unused gdWebChannel variable to silence Qt Creator's M127 Warning:
// Expression statements should be assignments, calls or delete expressions only
const gdWebChannel = new QWebChannel(qt.webChannelTransport, function(channel) {
    gdArticleView = channel.objects.gdArticleView;

    gdArticleView.onJsPageInitStarted(gdPendingArticles, gdPendingAudioLinks,
        gdPendingArticles.indexOf(gdCurrentArticle), gdHasPageInitFinished, gdPageTimestamp);
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
        // Note: the web engine's automatic vertical scroll position restoration is unreliable. Some positions are
        // stable and get restored correctly, others shift slightly when restored. This is not much of a problem though.
        // Should probably be left as is, because manual scroll restoration comes with its own set of problems:
        // 1) scroll restoration for fragment navigation on the same page has to be handled separately (also manually);
        // 2) automatic scroll restoration occurs as soon as the necessary amount of content is loaded, while manual
        //    scroll restoration has to be done at some fixed point of time. If that point is before a 'load' event
        //    handler (in a deferred script), sometimes a wrong position is restored. A 'load' event handler can be
        //    invoked much later than the automatic scroll restoration, which decreases perceived responsiveness.
        return;
    }

    const searchParams = new URLSearchParams(location.search);

    // Initialize gdCurrentArticle to the scrollto query item value of the page's URL.
    gdCurrentArticle = searchParams.get('scrollto');

    if (location.hash && location.hash !== '#') {
        // The page's URL has a non-empty fragment, which overrides scrolling to scrollto or gdanchor.

        // Leaving location.hash as is scrolls to it only after the entire page is loaded.
        // Furthermore, the scrolling cannot be canceled if the user activates an article
        // before the page is loaded. Remove the fragment for now; restore it (unless canceled)
        // when the current article, which contains the element with the fragment id, is loaded.

        // Replacing location.hash with an empty string in location has no effect.
        // So location.hash has to be replaced with '#' to (almost) clear the fragment.
        gdCurrentArticleHash = location.hash.substring(1); // exclude the '#' character
        const urlWithoutFragment = location.href.replace(location.hash, '#');
        location.replace(urlWithoutFragment);
        // Now location.href ends with '#', gdCurrentArticleHash does not start with '#'
        // => there is a single '#' character in (location.href + gdCurrentArticleHash).
        return;
    }

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

function gdMakeArticleActive(newId) {
    gdCurrentArticleTimestamp = new Date();
    gdSetActiveArticle('gdfrom-' + newId);
    // Call gdArticleView.onJsActiveArticleChanged() even if gdSetActiveArticle returns false and gdCurrentArticle
    // is unchanged. This is necessary to send the updated gdCurrentArticleTimestamp value and possibly overwrite
    // an obsolete current article value on the C++ side, which hasn't been received by JavaScript yet.
    if (gdArticleView)
        gdArticleView.onJsActiveArticleChanged(gdCurrentArticle, gdCurrentArticleTimestamp);
    // else: the updated gdCurrentArticle will be passed in gdArticleView.onJsPageInitStarted().
}

function gdOnCppActiveArticleChanged(articleId, moveToIt, pageTimestampString, currentArticleTimestampString) {
    const pageTimestamp = new Date(pageTimestampString);
    const currentArticleTimestamp = new Date(currentArticleTimestampString);
    // operator=== compares objects and would always compare page timestamps as not equal. To offer protection
    // against timing attacks and fingerprinting, the precision of new Date().getTime() might get rounded
    // depending on browser settings, so it cannot be used to compare page timestamps either. operator< works
    // here, because a next page's JavaScript code couldn't have sent its newer page timestamp to the C++ code.
    // Compare the received and stored current article timestamps using operator<= here and using
    // operator< in the C++ code to ensure that the current article values are always consistent, even
    // if the user manages to activate different articles in JavaScript and C++ at the same time.
    if (pageTimestamp < gdPageTimestamp || currentArticleTimestamp <= gdCurrentArticleTimestamp)
        return; // This current article update is meant for a previous page or is stale => ignore it.
    gdCurrentArticleTimestamp = currentArticleTimestamp;

    gdOnCppActiveArticleChangedNoTimestamps(articleId, moveToIt);
}

function gdBodyMouseDown(event) {
    if (gdSelectWordBySingleClick && gdArticleView && event.button === 0 && event.detail === 1)
        gdArticleView.onJsFirstLeftButtonMouseDown();
}

function gdBodyMouseUp(event) {
    // When "Select word by single click" option is off and the user double-clicks a word, it becomes selected some time
    // after QWebEngineView::mouseDoubleClickEvent(), not during this event handler as in the Qt WebKit version. The
    // implementation of "Double-click translates the word clicked" option translates a currently selected word and
    // needs up-to-date selection. The selection is empty at the time of a second JavaScript mousedown event. Send the
    // double-clicked message to ArticleView on a second mouseup JavaScript event, at which time the selection is
    // up-to-date. This later translation is useful: the user can double-click a word, extend selection to one or more
    // neighboring words, then release the left mouse button. Such a prolonged double-click translates the selected
    // multi-word phrase, while only a single word can be translated by a double-click in the Qt WebKit version.

    // Normally a second mouseup event (with event.detail === 2) signifies a double click. But when
    // "Select word by single click" option is on, a second mouseup event occurs after a single click due to
    // an artificial double click synthesized by ArticleView. Fortunately, when this option is on, ArticleView also
    // filters out a second non-synthesized mousedown event (MouseButtonDblClick) in a user's double click before
    // it reaches JavaScript, which makes event.detail of a second non-synthesized mouseup event unique - equal to 0.
    const targetClickCount = gdSelectWordBySingleClick ? 0 : 2;
    if (gdArticleView && event.button === 0 && event.detail === targetClickCount) {
        let imageUrl = null;
        if (event.target.tagName.toLowerCase() === 'img')
            imageUrl = event.target.currentSrc;
        gdArticleView.onJsDoubleClicked(imageUrl);
    }
}
