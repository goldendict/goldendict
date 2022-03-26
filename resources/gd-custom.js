//document ready
(function($){
$(function() {
        $(document).on("click","a",function(event) {
            var link = $(this).attr("href");
            if ('string' != typeof(link)) {
                return;
            }
            if(link.indexOf(":")>=0){
                emitClickedEvent(link);
                return false;
            }
            emitClickedEvent("");

            var newLink;
            var href = window.location.href;
            var index=-1;
            if (link.startsWith("#")) {
                //the href may contain # fragment already.remove them before append the new #fragment
                index = href.indexOf("#");
                if(index>-1)
                {
                    newLink = href.substring(0, index) + link;
                } 
                else{
                    newLink= href+link;
                }
            } else {
                index = href.indexOf("?");
                if(index>-1)
                {
                    newLink = href.substring(0, index) + "?word=" + link;
                }
                else{
                    newLink=href+"?word=" + link;
                }
            }
            $(this).attr("href", newLink);

        });

    });
})($_$);

function playSound(sound) {
    var a = new Audio(sound);
    a.play();
}

function resizeIframe(obj) {
    obj.style.height = obj.contentWindow.document.documentElement.scrollHeight + 'px';
}
