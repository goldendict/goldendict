//document ready ,
$(function() {
        $("a").click(function(event) {
            var link = $(this).attr("href");
            if (link.indexOf("://") < 0) {
                var newLink = window.location.href + "/" + link;
                $(this).attr("href", newLink);
            }
        });

    }

);