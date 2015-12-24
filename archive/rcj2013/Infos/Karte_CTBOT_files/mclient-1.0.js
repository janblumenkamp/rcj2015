/* 
 * $Id: mclient-1.0.js 20286 2009-03-17 13:08:09Z pmb $
 */

var szm_mccookie = "sso_id2";

var szm_prefix='http://mclient.ivwbox.de/cgi-bin/ivw/CP/';
var szm_prefixssl='https://mclientssl2.ivwbox.de/cgi-bin/ivw/CP/';

function szm_client(szm_site, szm_hcode, szm_isssl) {
	if (szm_getcookie(szm_mccookie) != szm_hcode) {
		szm_getmclient(szm_site,szm_isssl,szm_hcode);
		szm_setcookie(szm_hcode);
	}
}

function current_url_is_ssl() {
	if( document.location.toString().indexOf( 'https://' ) != -1 ) {
		return 1;
	}
	return 0;
}

function szm_getcookie(cookie_name) {
	var split = new Array();
	split = document.cookie.split(";");
	for (var i=0; i<split.length;i++) {
		if(split[i].match(".*"+cookie_name+".*")) {
			return (split[i].split("="))[1];
		}
	}
	return "";
}

function szm_getmclient(szm_site,szm_isssl,szm_hcode) {
	var img = new Image();

	//alert("requesting mclient tag: " + szm_hcode);

	if(szm_isssl) {
		img.src = szm_prefixssl+szm_site+'/'+szm_hcode;
	}
	else {
		img.src = szm_prefix+szm_site+'/'+szm_hcode;
	}
}

function szm_setcookie(code) {
    document.cookie = szm_mccookie+"="+code+"; path=/";
}

function __create_szm_pixel() {
	var sso_id_cookie = szm_getcookie('sso_id');

	if ( sso_id_cookie.length > 0 ) {
		szm_client('0903/heise', sso_id_cookie, current_url_is_ssl());
	}

	return '';
}

__create_szm_pixel();

