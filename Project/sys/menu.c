////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////menu.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///	Menu (based on m2tklib)
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "menu.h"
#include "u8g.h"
//#include "m2.h"
//#include "m2utl.h"
//#include "m2ghu8g.h"

/*
M2_EXTERN_ALIGN(top_menu);

uint8_t select_color = 0;

uint8_t n1 = 0;
uint8_t n2 = 0;

M2_LABEL(el_l1, NULL, "value 1:");
M2_U8NUM(el_u1, "c2", 0, 99, &n1);
M2_LABEL(el_l2, NULL, "value 2:");
M2_U8NUM(el_u2, "c2", 0, 99, &n2);

M2_LIST(list) = { &el_l1, &el_u1, &el_l2, &el_u2 };
M2_GRIDLIST(el_gridlist, "c2", list);
M2_ALIGN(top_menu, "-1|1W64H64", &el_gridlist);

void init_m2(void)
{
	m2_Init(&top_menu, m2_es_avr_rotary_encoder_u8g, m2_eh_4bd, m2_gh_u8g_bfs);
	m2_SetU8g(&u8g, m2_u8g_box_icon);
	m2_SetFont(0, (const void *)u8g_font_5x8r);

	m2_SetPin(M2_KEY_SELECT, PN(2, 0));
	m2_SetPin(M2_KEY_ROT_ENC_A, PN(2, 1));
	m2_SetPin(M2_KEY_ROT_ENC_B, PN(2, 2));
}*/

void m2_drawSetup(void)
{
	//m2_Draw();
	//m2_CheckKey();
}
