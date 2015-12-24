$(document).ready(function() {
    var webcode_input = $('#webcode_input')[0];
    
    if (!webcode_input) { return; }
    if (webcode_input.value == "") {
        webcode_input.value="c't-Link";
    }

    $(webcode_input).focus(function() {
        if (this.value == "c't-Link") { this.value=""; }
    });
    $(webcode_input).blur(function() {
        if (this.value == "") { this.value="c't-Link"; }
    });
});
