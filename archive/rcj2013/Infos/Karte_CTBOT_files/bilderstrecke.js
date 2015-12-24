(function($) {
    "use strict";
    
    // JSON-URL ermitteln
    function get_json_url(id,mode) {
        if(mode == 'ir') {
            if(document.domain == 'hmg-vorschau.heise-cms.de') {
                return '//www.heise-medien.de/bilderstrecke/bilderstrecke_'+id+'.html?callback=?&view=json';
            }
            else {
                return '//www.heise.de/bilderstrecke/bilderstrecke_'+id+'.html?callback=?&view=json';
            }
        }
        else if(mode == 'autos') {
            return $('#bilderstrecke-static-'+id).attr('data-autos-json-url')+'?callback=?&view=json&type=bilderstrecke';
        }
        else if(mode == 'swvz') {
            // SWVZ
            return $('#bilderstrecke-'+id).attr('data-json-url')+'?callback=?';
        }
        else if(mode == 'techstage') {
            // TechStage
            return $('#bilderstrecke-'+id).attr('data-json-url');
        }
    }

    var language = {
        "de" : {
            "prev"  : "voriges",
            "next"  : "nächstes",
            "close" : "schließen",
            "source": "Quelle"
        },
        "en" : {
            "prev"  : "previous",
            "next"  : "next",
            "close" : "close",
            "source": "Source"
        }
    }

    $.fn.bilderstrecke = function(settings) {
        // Standardwerte setzen
        var defaults = {
            "url"      : get_json_url,
            "type"     : "",
            "start"    : 1,
            "mode"     : "inline-full",
            "width"    : '520',
            "zaehlpixel_reload_url" : "/js/ho/ivw_reload.js",
            "css_url"  : "/stil/bilderstrecke.css",
            "language" : "de"
        };

        // Standardwerte durch evtl. gesetzte Parameter ueberschreiben
        var options = $.extend(true, defaults, settings);
        
        // CSS einbinden; if/else ist hier um einen IE-Bug zu umschiffen
        if(document.createStyleSheet) {
            document.createStyleSheet(options.css_url);
        } 
        else {
            $('head').append('<link rel="stylesheet" type="text/css" href="'+options.css_url+'" />');
        }

        function trackingReloadIntern(num) {
            var ivw_intern_tag = document.getElementById('ivw_pixel_intern'), src_val, matches_array, insert_pos;
            if (ivw_intern_tag) {
                src_val = ivw_intern_tag.src;

                matches_array = src_val.match(/\/(\d+)\?/);
                if (matches_array == null) {
                    insert_pos = src_val.length;
                    if (src_val.indexOf('?') > -1) {
                        insert_pos = src_val.indexOf('?');
                    }
                    src_val = src_val.substring(0, insert_pos) + '/' + num + src_val.substring(insert_pos);
                    src_val = trackingReload.replace_random_param(src_val, "d");
                }
                src_val = src_val.replace(/\/\d+\?/, "/" + num + "?");
                src_val = trackingReload.replace_random_param(src_val, "d");

                ivw_intern_tag.src = src_val;
            }
        }

        function reloadTracking(num) {
            if ( ! options.zaehlpixel_reload_url ) {
                // keine URL, kein Reload
                return;
            }
            if (window.trackingReload === undefined) {
                $.ajax({
                    url: options.zaehlpixel_reload_url,
                    dataType: "script",
                    cache: "false",
                    success: function() {
                        trackingReload.intern = function(){};
//                        trackingReload.reload_all();
//                        trackingReloadIntern(num);
                    }
                });
            }
            else {
                trackingReload.reload_all();
                trackingReloadIntern(num);
            }
        }

        // Funktion wird aufgerufen, wenn beim Initialisieren keine Breite angegeben wurde
        function setKnownChannelWidth(context) {
            // Bekannte Channel-Breiten
            var channel_list = { 
                "ho"                : "520",
                "ct"                : "526",
                "ix"                : "496",
                "tr"                : "503",
                "apple"             : "506",
                "mobil"             : "496",
                "security"          : "540",
                "netze"             : "496",
                "open"              : "530",
                "developer"         : "558",
                "resale"            : "516",
                "foto"              : "530",
                "jobs"              : "531",
                "hardware_hacks"    : "369",
                "techstage"         : "584"
            };

            // ermitteln ob der Channel - so gesetzt - bekannt ist
            var channel = $(context).data('channel');
            if(channel_list[channel] !== undefined) { 
                options.width = channel_list[channel];
            }
        }

        // Rahmenfunktion
        function do_the_stuff(data, context, type) {
            
            // Es wird ermittelt, ob die Bilderstrecke aus nur einem Bild besteht, da dann diverse Funktionen nicht geladen werden muessen
            var single_image = false;
            if($(data._bilder).length == 1) {
                single_image = true;
            }

            // Inline-Funktionen
            // direkt das mittels "num" uebergebene Bild der Bilderstrecke ansteuern
            var gotoImage = function(num) {

                // Verschiebung des ul-Elements berechnen
                var calcOffset = function(elem) {
                    var left_offset = 0;
                    var elem = parseInt($('figure ul li[data-item-no="'+elem+'"]', context).index());
                    $('figure ul li', context).each(function(index){
                        if(index < elem){
                            left_offset = left_offset - parseInt($(this).css('width'));
                        }
                    });
                    return left_offset += 'px';
                }

                // Update der Rahmenelemente (<figure>, Verschiebung der <ul> an richtige Position, Update des Counters)
                var updateFrame = function(width,height,num) {
                    $('figure ul', context).css('left',calcOffset(num));
                    $('figure ul li', context).removeClass('pic_shown');
                    $('figure ul li[data-item-no='+num+']', context).addClass('pic_shown').attr('data-status','loaded');
                    $('.pic', context).html(num);

                    var height = parseInt($('figure ul li[data-item-no='+num+'] img', context).height());
                    if($('figure ul li[data-item-no='+num+'] span.creator', context).length > 0) {
                        height = height + parseInt($('figure ul li[data-item-no='+num+'] span.creator', context).outerHeight());
                    }
                    if($('figure ul li[data-item-no='+num+'] div.bild_titel', context).length > 0) {
                        height = height + parseInt($('figure ul li[data-item-no='+num+'] div.bild_titel', context).outerHeight());
                    }
                    if($('figure ul li[data-item-no='+num+'] div.bu', context).length > 0) {
                        height = height + parseInt($('figure ul li[data-item-no='+num+'] div.bu', context).outerHeight());
                    }
                    height = height + 5;
                    $('figure, figure ul li[data-item-no='+num+']', context).css('height',height+'px');

                    // Links mit rel=external in der BU bekommen ein target=_blank (provisorium, bis external.js angepasst ist)
                    $('figure ul li[data-item-no='+num+'] a[rel="external"]', context).attr('target','_blank');

                    // Zaehlpixel-Reload
                    reloadTracking(num);
                }
                
                // voriges und naechstes Bild vorladen
                var loadSiblings = function(num) {
                    var prev_sibl, next_sibl;
                    var sum = parseInt($('figure nav .pic_count', context).html());

                    // voriges und naechstes Element werden ermittelt und Position im DOM (Zaehlung startet bei 0) festgelegt
                    switch(num) {
                        case 1:
                            prev_sibl = sum - 1;
                            next_sibl = num;
                            break;
                        case sum:
                            prev_sibl = num - 2;
                            next_sibl = 0;
                            break;
                        default: 
                            prev_sibl = num - 2;
                            next_sibl = num;
                    }

                    // Gibt es eine BU wird diese angezeigt
                    var bu_prev = '', bu_next = '';
                    if(data._bilder[prev_sibl].text_html != '' || data._bilder[prev_sibl].bildquelle_html != '' || data._bilder[prev_sibl].titel != '') {
                        // gibt es eine BU, Bildtitel oder Bildquelle wird das HTML generiert
                        // ist eine Bildquelle hinterlegt wird sie ausgegeben
                        if(data._bilder[prev_sibl].bildquelle_html != '') {
                            bu_prev += '<span class="creator" style="max-width: '+data._bilder[prev_sibl].previewbild_width+'px">'+language[options.language]['source']+': '+data._bilder[prev_sibl].bildquelle_html+'</span>';
                        }
                        if(data._bilder[prev_sibl].titel != '') {
                            bu_prev += '<div class="bild_titel" style="max-width: '+data._bilder[prev_sibl].previewbild_width+'px">'+data._bilder[prev_sibl].titel+'</div>';
                        }
                        if(data._bilder[prev_sibl].text_html != '' && data._bilder[prev_sibl].text_html != null) {
                            bu_prev += '<div class="bu" style="max-width: '+data._bilder[prev_sibl].previewbild_width+'px">'+data._bilder[prev_sibl].text_html+'</div>';
                        }
                    }
                    if(data._bilder[next_sibl].text_html != '' || data._bilder[next_sibl].bildquelle_html != '' || data._bilder[next_sibl].titel != '') {
                        // gibt es eine BU oder Bildquelle wird das HTML generiert
                        // ist eine Bildquelle hinterlegt wird sie ausgegeben
                        if(data._bilder[next_sibl].bildquelle_html != '') {
                            bu_next += '<span class="creator" style="max-width: '+data._bilder[next_sibl].previewbild_width+'px">'+language[options.language]['source']+': '+data._bilder[next_sibl].bildquelle_html+'</span>';
                        }
                        if(data._bilder[next_sibl].titel != '') {
                            bu_next += '<div class="bild_titel" style="max-width: '+data._bilder[next_sibl].previewbild_width+'px">'+data._bilder[next_sibl].titel+'</div>';
                        }
                        if(data._bilder[next_sibl].text_html != '' && data._bilder[next_sibl].text_html != null) {
                            bu_next += '<div class="bu" style="max-width: '+data._bilder[next_sibl].previewbild_width+'px">'+data._bilder[next_sibl].text_html+'</div>';
                        }
                    }

                    // Bilder werden geladen und eingebunden, sofern sie noch nicht geladen wurden
                    if($('li:eq('+prev_sibl+')', context).attr('data-status') == 'unloaded') {
                        $('<img />').attr('src',data._bilder[prev_sibl].previewbild_url).load(function(){
                            $('figure ul li:eq('+prev_sibl+')', context).append( $(this) ).attr('data-status','loaded').append(bu_prev);
                        });
                    }
                    // if-Abfrage hier, da bei einer Bilderstrecke mit nur zwei Bildern prev_sibl == next_sibl ist
                    // Wegen des asynchronen Ladens bei prev_sibl (.load()) ist data-status noch nicht auf "loaded" gesetzt und 
                    // das Laden von next_sibl wuerde ebenfalls nochmal ausgefuehrt, sodass das Bild 2x eingefuegt werden wuerde
                    if(prev_sibl != next_sibl) {
                        if($('li:eq('+next_sibl+')', context).attr('data-status') == 'unloaded') {
                            $('<img />').attr('src',data._bilder[next_sibl].previewbild_url).load(function(){
                                $('figure ul li:eq('+next_sibl+')', context).append( $(this) ).attr('data-status','loaded').append(bu_next);;
                            });
                        }
                    }
                }

                if($('figure ul li[data-item-no='+num+'][data-status=loaded]', context).length == 1) {
                    // das Bild wurde schon angezeigt/ist bereits in der Liste, muss also nur noch ins Sichtfeld geholt werden
                    updateFrame(parseInt($('figure ul li[data-item-no="'+num+'"]', context).css('width')),parseInt($('figure ul li[data-item-no="'+num+'"]', context).css('height')),num);
                    // umliegende Bilder werden vorgeladen, wenn die Bilderstrecke nicht nur aus einem Bild besteht
                    if(!single_image) {
                        loadSiblings(num);
                    }
                }
                else {
                    // das Bild wurde noch nicht angezeigt, muss also geladen werden
                    var index_count_num = num - 1;
                    $('<img />').attr({
                        'src'    : data._bilder[index_count_num].previewbild_url,
                        'alt'    : data._bilder[index_count_num].titel
                    }).load(function() {
                        // neues Bild ist geladen, Einbindung folgt
                        var bu = '';
                        if(data._bilder[index_count_num].text_html != '' || data._bilder[index_count_num].bildquelle_html != '' || data._bilder[0].titel != '') {
                            // gibt es eine BU, Bildtitel oder Bildquelle wird das HTML generiert
                            // ist eine Bildquelle hinterlegt wird sie ausgegeben
                            if(data._bilder[index_count_num].bildquelle_html != '') {
                                bu += '<span class="creator" style="max-width: '+data._bilder[index_count_num].previewbild_width+'px">'+language[options.language]['source']+': '+data._bilder[index_count_num].bildquelle_html+'</span>';
                            }
                            if(data._bilder[index_count_num].titel != '') {
                                bu += '<div class="bild_titel" style="max-width: '+data._bilder[index_count_num].previewbild_width+'px">'+data._bilder[index_count_num].titel+'</div>';
                            }
                            if(data._bilder[index_count_num].text_html != '' && data._bilder[index_count_num].text_html != null) {
                                bu += '<div class="bu" style="max-width: '+data._bilder[index_count_num].previewbild_width+'px">'+data._bilder[index_count_num].text_html+'</div>';
                            }
                        }
                        $('figure ul li[data-item-no="'+num+'"]', context).append( $(this) ).append(bu);
                        updateFrame(parseInt($('li[data-item-no="'+num+'"]', context).css('width')),parseInt($('li[data-item-no="'+num+'"]', context).css('height')),num);
                        // umliegende Bilder werden vorgeladen, wenn die Bilderstrecke nicht nur aus einem Bild besteht
                        if(!single_image) {
                            loadSiblings(num);
                        }
                    });
                }
            }

            // Inline-Funktion
            // generelles HTML und erstes Bild der Inline-Galerie werden eingebunden
            var init = function() {
                $(context).empty();
                $(context).prepend('<h4>'+data.titel+'</h4>');
                $(context).append('<figure style="width: '+options.width+'px;" />');
                var bu = '';
                if(data._bilder[0].text_html != '' || data._bilder[0].bildquelle_html != '' || data._bilder[0].titel != '') {
                    if(data._bilder[0].bildquelle_html != '') {
                        bu += '<span class="creator" style="max-width: '+data._bilder[0].previewbild_width+'px">'+language[options.language]['source']+': '+data._bilder[0].bildquelle_html+'</span>';
                    }
                    if(data._bilder[0].titel != '') {
                        bu += '<div class="bild_titel" style="max-width: '+data._bilder[0].previewbild_width+'px">'+data._bilder[0].titel+'</div>';
                    }
                    if(data._bilder[0].text_html != '' && data._bilder[0].text_html != null) {
                        bu += '<div class="bu" style="max-width: '+data._bilder[0].previewbild_width+'px">'+data._bilder[0].text_html+'</div>';
                    }
                }
                $('figure', context).append('<nav></nav><ul><li class="pic_shown" data-item-no="1" data-status="loaded" style="width: '+options.width+'px; height: '+data._bilder[0].previewbild_height+'px;"></li></ul>');
                $('figure nav', context).append('<span class="pic_prev">&lt;</span> <span class="pic">1</span>/<span class="pic_count">'+$(data._bilder).length+'</span> <span class="pic_next">&gt;</span>');
                var i;
                var list_width = parseInt(options.width);
                for(i = 1; i < $(data._bilder).length; i++){
                    var item_no = i + 1;
                    $('figure ul', context).append('<li data-item-no="'+item_no+'" data-status="unloaded" style="width: '+options.width+'px; height: '+data._bilder[i].previewbild_height+'px;"></li>');
                    list_width = list_width + parseInt(options.width);
                }
                $('figure ul', context).css('width',list_width+'px');
                
                // gibt es nur ein Bild in der Bilderstrecke, braucht keine vor-/zurueck-Navigation angezeigt werden, dafuer wird eine Klasse gesetzt
                if(single_image) {
                    $(context).addClass('single_image');
                }

                /* Zum Konstrukt "erster_durchlauf":
                 * Es gibt ein Problem auf einigen Systemen mit dem Add On "https everywhere", das versucht das Bild immer wieder per https abzurufen
                 * und wir es per Redirect via http zurueckliefern und das Add On kurioserweise in eine Endlosschleife laeuft und dann immer wieder 
                 * die Funktion gotoImage(); aufruft, was zu Fehlverhalten fuehrt.
                 * Leider funktioniert eine Abfrage mittels des data-Attributs "status" nicht - der Bug bleibt bestehen. Zu einer Loesung brachte uns 
                 * bis dato leider nur das hier vorliegende Konstrukt mit einer zusaetzlichen Variable. Auch die Abfrage erst im .load()-Teil ist Absicht.
                 */
                var erster_durchlauf = 1;
                // Das erste Bild wird geladen, wenn der Ladevorgang beendet ist, wird es eingebaut
                $('<img />').attr({
                    'src'    : data._bilder[0].previewbild_url,
                    'alt'    : data._bilder[0].titel
                }).load(function() {
                    if(erster_durchlauf == 1) {
                        // Nachdem das Bild geladen ist wird es eingebunden und die BU - so vorhanden - angehaengt
                        $('figure ul li[data-item-no="1"]', context).append($(this)).append(bu);

                        // es wird zum - via Optionen uebergebenen - Startbild der Galerie gewechselt (Standard: 1)
                        gotoImage(options.start);

                        // je nach Channel, wird das Event fuer das equalHeights.js noch unterschiedlich getriggert
                        if($(context).data('channel') == 'ho' || $(context).data('channel') == 'ix' || $(context).data('channel') == 'security' || $(context).data('channel') == 'open') {
                            $("#mitte_links").trigger("HeiseContentChanged", "#mitte_links, #mitte_rechts");
                        }
                    }
                    erster_durchlauf = 0;
                });
            }

            // Fullscreen-Funktion
            // Bild laden, Position, Breite, Abstaende etc. anpassen
            var updateFullscreen = function(num,num_index,context_full) {
                var full_loaded = $(context).data('full_loaded') || {};
                
                $(context_full).addClass('loading');
                
                if(single_image) {
                    $(context_full).addClass('single_image');
                }

                // "Bild wird geladen..."-Grafik einblenden
                if (!full_loaded.hasOwnProperty(num)) {
                    $('figure', context_full).prepend('<span class="loader">Bild wird geladen...</span>');
                    var loader_top = (parseInt($('figure', context_full).css('height')) / 2) - (parseInt($('figure .loader', context_full).height()) / 2)+'px';
                    var loader_left = (parseInt($('figure', context_full).css('width')) / 2) - (parseInt($('figure .loader', context_full).width()) / 2)+'px';
                    $('figure .loader', context_full).css({ 'top' : loader_top, 'left' : loader_left });
                }

                $('<img />').attr({
                    'src'    : data._bilder[num_index].bild_url,
                    'alt'    : data._bilder[num_index].bild_alttext_html, 
                    'title'  : language[options.language]['next']
                }).load(function() {
                    // Bild ist geladen und wird nun eingebunden
                    $('figcaption', context_full).remove();
                    $('figure .loader', context_full).remove();

                    // Falls das Grossbild breiter ist als der Browser des Users
                    if(parseInt($('body').width()) > parseInt(data._bilder[num_index].bild_width)) {
                        $('figure', context_full).css('width',data._bilder[num_index].bild_width+'px');
                    }
                    else {
                        $('figure', context_full).css('width',(parseInt($('body').width()) - 50)+'px');
                    }
                    $('figure img', context_full).replaceWith( $(this) );

                    var figure_height = parseInt($('nav', context_full).outerHeight());
                    var caption_height = 0;

                    // Wenn BU oder Bildquelle vorhanden sind, werden diese eingebaut
                    if(data._bilder[num_index].text_html != '' || data._bilder[num_index].bildquelle_html != '') {
                        $('figure', context_full).append('<figcaption />');
                        if(data._bilder[num_index].text_html != '' && data._bilder[num_index].text_html != null) {
                            if($(context).data('channel') != '') {
                                // graue Seiten koennen nur Plaintext-BUs enthalten
                                $('figure figcaption', context_full).append('<div><p>'+data._bilder[num_index].text_html+'</p></div>');
                            }
                            else {
                                // SWVZ kann RTE
                                $('figure figcaption', context_full).append('<div>'+data._bilder[num_index].text_html+'</div>');
                            }
                            $('figure figcaption div > p', context_full).each(function(){
                                caption_height = caption_height + parseInt($(this).outerHeight());
                            });
                        }
                        if(data._bilder[num_index].bildquelle_html != '') {
                            $('figure figcaption', context_full).prepend('<span class="source">'+data._bilder[num_index].bildquelle_html+'</span>');
                            caption_height = caption_height + parseInt($('figure figcaption span', context_full).height());
                        }
                        $('figure figcaption', context_full).css('height',caption_height+'px');
                        caption_height = parseInt($('figure figcaption', context_full).outerHeight());
                    }
                    figure_height = figure_height + caption_height;

                    // Falls das Grossbild breiter ist als der Browser des Users, muss auch die Hoehe des figure-Elements seperat berechnet werden
                    if(parseInt($('body').width()) < parseInt(data._bilder[num_index].bild_width)) {
                        figure_height = figure_height + parseInt(Math.round(parseInt(data._bilder[num_index].bild_height) * parseFloat((parseInt($('body').width()) - 50) / parseInt(data._bilder[num_index].bild_width))));
                    }
                    else {
                        figure_height = figure_height + parseInt(data._bilder[num_index].bild_height);
                    }

                    $('figure', context_full).css('height',figure_height+'px');
                    $('figure nav .pic', context_full).html(num);
                    $(context_full).removeClass('loading');
                    
                    // Falls das Grossbild breiter ist als der Browser des Users
                    if(parseInt($('body').width()) > parseInt(data._bilder[num_index].bild_width)) {
                        $(context_full).css({
                            'width' : parseInt(data._bilder[num_index].bild_width)+'px',
                            'left'  : (parseInt($('body').width()) / 2) - (parseInt(data._bilder[num_index].bild_width) / 2)+'px'
                        });
                    }
                    else {
                        $(context_full).css({
                            'width' : (parseInt($('body').width()) - 50)+'px',
                            'left'  : (parseInt($('body').width()) / 2) - ((parseInt($('body').width()) - 50) / 2)+'px'
                        });
                    }
                    
                    // Links mit rel=external in der BU bekommen ein target=_blank (provisorium, bis external.js angepasst ist)
                    $('figure figcaption a[rel="external"]').attr('target','_blank');

                    // Um bei erneutem Anzeigen des Bildes nicht wieder das "Laden"-Element anzuzeigen, wird sich hier gemerkt, welche Bilder bereits gross angezeigt wurden und damit im Cache liegen sollten
                    if(!full_loaded.hasOwnProperty(num)) {
                        full_loaded[num] = true;
                        $(context).data('full_loaded', full_loaded);
                    }
                });

                // Zaehlpixel-Reload
                reloadTracking(num);
            }

            // Fullscreen-Funktion
            // generelles HTML und erstes Bild der Fullscreen-Galerie werden eingebunden
            var init_fullscreen = function(num,sum) {
                var num_index = num - 1;
                var unique = Math.round(Math.random()*10000);

                $('body').prepend('<div id="white_canvas">&nbsp;</div> <div class="bilderstrecke_full" id="full-'+unique+'" />');
                $('#white_canvas').animate({ 'opacity' : '1.0' }, 300, function() {
                    var canvas_offset_top = '';
                    if($.browser.mozilla || $.browser.msie || $.browser.opera) {
                        canvas_offset_top = $('html').scrollTop();
                    }
                    else {
                        canvas_offset_top = $('body').scrollTop();
                    }

                    var context_full = $('body div#full-'+unique).css('top',canvas_offset_top+'px');
                    $(context_full).prepend('<span class="close_x">'+language[options.language]['close']+' <em>X</em></span><figure style="width: '+data._bilder[num_index].bild_width+'px;"><nav /></figure>');
                    $('figure nav', context_full).prepend('<span class="pic_prev">'+language[options.language]['prev']+'</span><span class="orientation"><span class="pic">'+num+'</span>/<span class="pic_count">'+sum+'</span></span><span class="pic_next">'+language[options.language]['next']+'</span>');

                    var figure_height = parseInt($('figure nav', context_full).outerHeight()) + parseInt(data._bilder[num_index].bild_height);
                    $('figure', context_full).append('<img src="'+data._bilder[num_index].previewbild_url+'" />').css('height',figure_height+'px');

                    updateFullscreen(num,num_index,context_full);

                    // voriges Bild im Fullscreen anzeigen
                    $('.pic_prev', context_full).live('click', function(){
                        if(parseInt($('.pic', context_full).html()) == 1) {
                            var newFullImg = parseInt($('.pic_count', context_full).html());
                            var newFullImg_index = newFullImg - 1;
                        }
                        else {
                            var newFullImg = parseInt($('.pic', context_full).html()) - 1;
                            var newFullImg_index = newFullImg - 1;
                        }

                        // Webtrekk-Action 'zurueck'
                        if($(context).data('cid')) {
                            $(context).triggerHandler({
                                type : "webtrekk_bilderstrecke",
                                action : "zurueck",
                                bilderstrecken_id : $(context).data('cid'),
                                source : parseInt($('.pic', context_full).html())
                            });
                        }

                        updateFullscreen(newFullImg,newFullImg_index,context_full);
                    });

                    // naechstes Bild im Fullscreen anzeigen
                    $('.pic_next', context_full).live('click', function(){
                        if(parseInt($('.pic', context_full).html()) == parseInt($('.pic_count', context_full).html())) {
                            var newFullImg = 1;
                            var newFullImg_index = 0;
                        }
                        else {
                            var newFullImg = parseInt($('.pic', context_full).html()) + 1;
                            var newFullImg_index = newFullImg - 1;
                        }

                        // Webtrekk-Action 'vor'
                        if($(context).data('cid')) {
                            $(context).triggerHandler({
                                type : "webtrekk_bilderstrecke",
                                action : "vor",
                                bilderstrecken_id : $(context).data('cid'),
                                source : parseInt($('.pic', context_full).html())
                            });
                        }

                        updateFullscreen(newFullImg,newFullImg_index,context_full);
                    });

                    // naechstes Bild durch Klick aufs Bild aufrufen
                    $('figure > img', context_full).live('click', function(){
                        $('.pic_next', context_full).click();
                    });

                    // Detail-Ansicht/Fullscreen schliessen
                    $('.close_x', context_full).add('#white_canvas').bind('click', function(){
                        if(options.mode != 'inline') {
                            var full_num = parseInt($('figure nav .pic', context_full).html());
                            
                            // Webtrekk-Action 'zoom_out'
                            if($(context).data('cid')) {
                                $(context).triggerHandler({
                                    type : "webtrekk_bilderstrecke",
                                    action : "zoom_out",
                                    bilderstrecken_id : $(context).data('cid'),
                                    source : parseInt($('.pic', context_full).html())
                                });
                            }
                            
                            gotoImage(full_num);
                        }
                        $(context_full).remove();
                        $('#white_canvas').remove();
                    });

                    // Tastatur-Navigation im Fullscreen
                    $(document.documentElement).keyup(function(event) {
                        if($('input:focus').length == 0 && $('textarea:focus').length == 0 && $('body .bilderstrecke_full').length == 1) {
                            switch(event.which) {
                                case 37:
                                    // Pfeil links - Klick auf "voriges"-Link wird ausgefuehrt
                                    $('.pic_prev', context_full).click();
                                    break;
                                case 39:
                                    // Pfeil rechts - Klick auf "naechstes"-Link wird ausgefuehrt
                                    $('.pic_next', context_full).click();
                                    break;
                                case 27:
                                    // ESC - Klick auf "schliessen"-Link wird ausgefuehrt
                                    $('body .bilderstrecke_full .close_x').click();
                                    break;
                            }
                        }
                    });
                });
            }

            // initialisieren: grundlegendes HTML und erstes Bild laden
            // mode == inline-full bedeutet, Einbindung der Inline-Galerie, aus der die Fullscreen-Galerie geoeffnet wird
            // mode == inline bedeutet, lediglich die Einbindung der Inline-Galerie ohne Fullscreen
            // mode == full bedeutet, dass keine Inline-Galerie eingebunden wird, ein Klick auf das Object startet aber die Fullscreen-Galerie
            if(options.mode == 'inline-full' || options.mode == 'inline') {
                init();
            }

            if(options.mode != 'full') {
                // voriges Bild
                $('.pic_prev', context).live('click', function() {
                    if(parseInt($('.pic', context).html()) == 1) {
                        var goto_num = parseInt($('.pic_count', context).html());
                    }
                    else {
                        var goto_num = parseInt($('.pic', context).html()) - 1;
                    }
                    
                    // Webtrekk-Action 'zurueck'
                    if($(context).data('cid')) {
                        $(context).triggerHandler({
                            type : "webtrekk_bilderstrecke",
                            action : "zurueck",
                            bilderstrecken_id : $(context).data('cid'),
                            source : parseInt($('.pic', context).html())
                        });
                    }
                    
                    gotoImage(goto_num);
                });

                // naechstes Bild
                $('.pic_next', context).live('click', function(){
                    if(parseInt($('.pic', context).html()) == parseInt($('.pic_count', context).html())) {
                        var goto_num = 1;
                    }
                    else {
                        var goto_num = parseInt($('.pic', context).html()) + 1;
                    }
                    
                    // Webtrekk-Action 'vor'
                    if($(context).data('cid')) {
                        $(context).triggerHandler({
                            type : "webtrekk_bilderstrecke",
                            action : "vor",
                            bilderstrecken_id : $(context).data('cid'),
                            source : parseInt($('.pic', context).html())
                        });
                    }
                    
                    gotoImage(goto_num);
                });

                // Tastatur-Navigation in der Vorschau, sofern nur eine Bilderstrecken-Vorschau im Artikel ist
                $(document.documentElement).keyup(function(event) {
                    if($('input:focus').length == 0 && $('textarea:focus').length == 0 && $('.bilderstrecke_vorschau.js').length == 1 && $('body .bilderstrecke_full').length == 0) {
                        switch(event.which) {
                            case 37:
                                // Pfeil links - Klick auf "voriges"-Link wird ausgefuehrt
                                $('.pic_prev', context).click();
                                break;
                            case 39:
                                // Pfeil rechts - Klick auf "naechstes"-Link wird ausgefuehrt
                                $('.pic_next', context).click();
                                break;
                        }
                    }
                });
            }

            if(options.mode == 'inline-full') {
                // Detail-Ansicht/Fullscreen oeffnen
                $('figure ul li img', context).live('click', function(){
                    init_fullscreen(parseInt($('.pic', context).html()),parseInt($('.pic_count', context).html()));
                    
                    // Webtrekk-Action 'zoom_in'
                    if($(context).data('cid')) {
                        $(context).triggerHandler({
                            type : "webtrekk_bilderstrecke",
                            action : "zoom_in",
                            bilderstrecken_id : $(context).data('cid'),
                            source : parseInt($('.pic', context).html())
                        });
                    }
                });
            }
            
            if(options.mode == 'full') {
                // keine Inline-Galerie: Detail-Ansicht/Fullscreen oeffnen
                $(type == 'autos' ? 'a' : 'a:has(img)', context).live('click', function(e){
                    e.preventDefault();
                    init_fullscreen(parseInt(options.start),parseInt($(data._bilder).length));
                });
            }
        }

        return this.each(function () {
            var id = null;
            var url = null;

            // Es muss zwischen Integration in IR und SWVZ unterschieden werden
            if($(this).attr('data-cid')) {
                // IR
                id = $(this).attr('data-cid');
                if(options.mode != 'full') {
                    $(this).attr('id','bilderstrecke-'+id).addClass('js');
                }
                else {
                    $(this).attr('id','bilderstrecke-static-'+id);
                }
                url = get_json_url(id,'ir');
            }
            // Extra fuer heise Autos, da die Bilderstrecken anders sind
            else if($(this).attr('data-autos-json-url')) {
                id = Math.round(Math.random()*10000);
                if(options.mode != 'full') {
                    $(this).attr('id','bilderstrecke-'+id).addClass('js');
                }
                else {
                    $(this).attr('id','bilderstrecke-static-'+id);
                }
                url = get_json_url(id,'autos');
            }
            else if($(this).attr('data-json-url')) {
                // SWVZ oder TechStage
                id = Math.round(Math.random()*10000);
                if(options.mode != 'full') {
                    $(this).attr('id','bilderstrecke-'+id).addClass('js');
                }
                else {
                    $(this).attr('id','bilderstrecke-static-'+id);
                }
                if($(this).data('channel') == 'techstage') {
                    url = get_json_url(id,'techstage');
                }
                else {
                    url = get_json_url(id,'swvz');
                }
            }

            // Klasse setzen, damit der Zoom-Cursor angezeigt werden kann
            if(options.mode == 'inline-full') {
                $(this).addClass('inline-full');
            }

            // Legacy-Bilderstrecken-BU entfernen
            $('#bilderstrecke-'+id+' + .bilderstrecke_unterschrift').remove();

            // Kontext setzen
            if(options.mode != 'full') {
                var context = $('#bilderstrecke-'+id);
            }
            else {
                var context = $('#bilderstrecke-static-'+id);
            }

            // wurde beim Initialisieren keine Breite angegeben (settings ist nicht vorhanden oder settings.width wurde nur nicht gesetzt)
            // wird die fuer diesen Channel bekannte Breite genommen, statt der Breite der default-Werte
            if(settings === undefined || settings.width === undefined) {
                setKnownChannelWidth(context);
            }

            $(this).css('width',options.width+'px');

            // JSON abrufen und bei Erfolg die eigentlichen Funktionen in Gang setzen
            if($(context).data('channel') != '' && $(context).data('channel') != undefined) {
                $.getJSON(url, { "channel" : $(context).data('channel') }, function(data) {
                    do_the_stuff(data, context, options.type);
                });
            }
            else {
                $.getJSON(url, function(data) {
                    do_the_stuff(data, context, options.type);
                });
            }
        });
    }
}(jQuery));
