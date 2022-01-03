//document ready ,
$(function() {
        $("a").click(function(event) {
            var link = $(this).attr("href");
            if(link.indexOf("://")>=0){
                return;
            }

            var newLink;
            if (link.startsWith("#")) {
                newLink = window.location.href + link;
            } else {
                var href = window.location.href;
                var index = href.indexOf("?");
                newLink = href.substring(0, index) + "?word=" + link;
            }
            $(this).attr("href", newLink);

        });

    }

);
function playSound(sound) {
            var a = new Audio(sound);
            a.play();
        }
