setInterval(function () {
    //in some cases ,the website in iframe will load result after document has been loaded. the height will continue to change.
    const body = document.body;
    const html = document.documentElement;

    const height = Math.max(body.scrollHeight, body.offsetHeight,
    html.clientHeight, html.scrollHeight, html.offsetHeight);

    if ('parentIFrame' in window) {
        console.log("iframe set height to " + height);
        parentIFrame.size(height); 
    }
}, 500);