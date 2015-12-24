$(document).ready( function() {
  // Grundvariante fuer fast alle Anrisse
  var collection;
  if (collection) {collection = $('.anriss_mit_bild_links').add(collection);} else {collection = $('.anriss_mit_bild_links')}
  if (collection) {collection = $('.anriss').add(collection);} else {collection = $('.anriss');}
  if (collection) {collection = $('.teaser_mit_bild_und_text').add(collection);} else {collection = $('.teaser_mit_bild_und_text');}

  // Mac-and-i Blog-Index und Blog-Teaser
  if (collection) {collection = $('.blogeintrag').add(collection);} else {collection = $('.blogeintrag');}
  if (collection) {collection = $('.blogteaser_eintrag').add(collection);} else {collection = $('.blogteaser_eintrag');}

  // Developer
  if (collection) {collection = $('.anriss_blog_eintrag').add(collection);} else {collection = $('.anriss_blog_eintrag');}
  if (collection) {collection = $('.teaser_frei').add(collection);} else {collection = $('.teaser_frei');}

  // Open
  if (collection) {collection = $('#left_teaser').add(collection);} else {collection = $('#left_teaser');}
  if (collection) {collection = $('#right_teaser').add(collection);} else {collection = $('#right_teaser');}

  // Security
  if (collection) {collection = $('.position_1').add(collection);} else {collection = $('.position_1');}
  if (collection) {collection = $('.position_2').add(collection);} else {collection = $('.position_2');}
  if (collection) {collection = $('.topteaser_basic').add(collection);} else {collection = $('.topteaser_basic');}
  if (collection) {collection = $('.topteaser_einzel').add(collection);} else {collection = $('.topteaser_einzel');}
  if (collection) {collection = $('.fiver_teaser_linker').add(collection);} else {collection = $('.fiver_teaser_linker');}
 
  // Mobil
  if (collection) {collection = $('.anriss_bild_left').add(collection);} else {collection = $('.anriss_bild_left');}
  if (collection) {collection = $('.anriss_bild_right').add(collection);} else {collection = $('.anriss_bild_right');}

  // TR brauchte keine Ausnahmen

  // c't-TV wurde nicht behandelt

  // c't
  if (collection) {collection = $('.position_3').add(collection);} else {collection = $('.position_3');}
  if (collection) {collection = $('.teaser_artikel').add(collection);} else {collection = $('.teaser_artikel');}
  if (collection) {collection = $('.teaser_linker').add(collection);} else {collection = $('.teaser_linker');}
  if (collection) {collection = $('.teaser_fiver_element').add(collection);} else {collection = $('.teaser_fiver_element');}

  // Autos
  if (collection) {collection = $('.promo_teaser').add(collection);} else {collection = $('.promo_teaser');}
  if (collection) {collection = $('.top_promo').add(collection);} else {collection = $('.top_promo');}
  if (collection) {collection = $('.promo').add(collection);} else {collection = $('.promo');}

  // Foto
  if (collection) {collection = $('.anriss_rubrik').add(collection);} else {collection = $('.anriss_rubrik');}
  if (collection) {collection = $('.top_promo2').add(collection);} else {collection = $('.top_promo2');}

  // Jobs brauchte keine Ausnahmen

  // Resale brauchte keine Ausnahmen

  // ho
  if (collection) {collection = $('.teaser_left').add(collection);} else {collection = $('.teaser_left');}
  if (collection) {collection = $('.teaser_right').add(collection);} else {collection = $('.teaser_right');}

  if (collection && collection.size() > 0) {
    collection.each( function() {
       // die explizit ausgezeichnete (class="the_content_url") URL suchen und verwenden
      var url = $(this).find('a.the_content_url').attr('href');
  
      // Fallback-Verhalten - die erste URL im Teaser nehmen
      if (!url) {
          url = $(this).find('a').attr('href');
      }
  
      // folgende Elemente (img,p,span,dd) werden zusaetzlich verlinkt
      $(this).find('img,p,span,dd').each ( function() {
        if ( $(this).parent().hasClass('fiver_teaser_linker') ) { 
            // Ausnahme fuer den Fiver-Teaser
            if ( $(this).hasClass('textzeile') ) { 
                if ($(this).is('p')) {
                    $(this).wrapInner('<a href="'+url+'" />');
                } else {
                    $(this).wrap('<a href="'+url+'" />');
                }
            }   
        } else if ( $(this).parent().hasClass('promo_teaser') ) { 
            // Ausnahme fuer die Auto Promo-Teaser
            url = $(this).parent().find('h4').find('a').attr('href');
            if ($(this).is('p')) {
                $(this).wrapInner('<a href="'+url+'" />');
            } else {
                $(this).wrap('<a href="'+url+'" />');
            }
        } else if ( $(this).parent().hasClass('blogteaser_eintrag') ) { 
          // Mac-and-i Blogteaser sollen keine verlinkte Autoren-Angaben haben
          if (
            !$(this).hasClass('autor_datum') &&
            !$(this).is('img')
          ) { 
            if ($(this).is('p')) {
                $(this).wrapInner('<a href="'+url+'" />');
            } else {
                $(this).wrap('<a href="'+url+'" />');
            }
          }   
        } else if ( $(this).parent().hasClass('teaser_right') || $(this).parent().hasClass('teaser_left') || $(this).parent().hasClass('teaser') ) { 
  
  		// Damit die Teaser-Ueberschrift ('heise Resale', 'Technology Review',
  		// 'ct') nicht verlinkt werden ignorieren wir das umschliessende <p>
  		// und verlinken es somit nicht.
  
        } else {
          // DEFAULT: Treffen keine der o.g. Ausnahmen zu, dann wird alles mit 
          // dem href des Beitrags gewrapped.
          if ($(this).is('p') || $(this).is('dd')) {
              $(this).wrapInner('<a href="'+url+'" />');
          } else {
              $(this).wrap('<a href="'+url+'" />');
          }
        }   
      }); // img,p each 
    }); // collection.each
  }
}); // document.ready

