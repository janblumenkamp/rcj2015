/*
 * (c) Heise Zeitschriften Verlag GmbH & Co. KG
 * $Revision: 19115 $
 */

// Fall <a rel="external nofollow"> beachten => $("a[rel~=external]")
if (window.externalLinks === undefined) {

    var externalLinks = (function() {

        function external_blank() {
            if (typeof jQuery != "undefined") {
                jQuery( function($) {
                    $(document).on('click', "a[rel~=external]", function(e) {
                        e.preventDefault();
                        var url = $(this).attr("href");
                        window.open(url, "_blank");
                    });
                });
            }
        }
    
        var my = {};
    
        my.openBlank = function() {
            external_blank();
        };
    
        return my;
    })();
    
    externalLinks.openBlank();
}
