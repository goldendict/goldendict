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

                if (link.indexOf("?gdanchor") > -1) {
                    newLink = "gdlookup://localhost/" + link;
                }
                else {
                    if (index > -1) {
                        newLink = href.substring(0, index) + "?word=" + link;
                    }
                    else {
                        newLink = href + "?word=" + link;
                    }
                }
            }
            $(this).attr("href", newLink);

        });

    //monitor iframe height.

        $( "iframe" ).on( "load", function() {
          var iframe = $( this );
          resizeIframe( iframe[ 0 ] );
        } );

    function resizeIframe(obj) {
        setInterval(function(){
            //in some cases ,the website in iframe will load result after document has been loaded. the height will continue to change.
            if($(obj).contents().height() <2000)
            {
                $(obj).height($(obj).contents().height());
            }
            else{
                $(obj).height(2000);
                obj.scrolling="yes";
            }
        },500);
    }

    });
})($_$);

function playSound(sound) {
    var a = new Audio(sound);
    a.play();
}


