//document ready ,
$(function() {
        $("a").click(function(event) {
            var link = $(this).attr("href");
            if(link.indexOf(":")>=0){
                return;
            }

            var newLink;
            var href = window.location.href;
                
            if (link.startsWith("#")) {
                //the href may contain # fragment already.remove them before append the new #fragment
                var index = href.indexOf("#");
                if(index>-1)
                {
                    newLink = href.substring(0, index) + link;
                } 
                else{
                    newLink= href+link;
                }
            } else {
                var index = href.indexOf("?");
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

    }

);
function playSound(sound) {
            var a = new Audio(sound);
            a.play();
        }
