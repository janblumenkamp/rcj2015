////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////display.c//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Fast alles, was mit dem DIsplay zu tun hat:
//	- Zahlenkonvertierung (Zahl zu string)
//	- Bitmaps (Logo etc.)
//	- Anzeige und Management der Kopfzeile
//	- Pfeil
//	- Komplettes Setup UI
//	Nicht enthalten ist nur die Anzeige der Karte!
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#include "main.h"
#include "display.h"
#include "maze.h"
#include "funktionen.h"
#include "u8g.h"
#include "um6.h"
#include "bluetooth.h"
#include "system.h"
#include "i2cdev.h"
#include "pixy.h"
#include "victim.h"

//////////////////////////////////////////////////////////////////////////////

void u8g_DrawLong(uint8_t koord_x, uint8_t koord_y, int32_t num)
{
  char buffer[15];
	ltoa( num , buffer, 10);
  u8g_DrawStr(&u8g, koord_x, koord_y, buffer);
}

//128 x 29
const uint8_t logo[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xc0, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xe0, 0xff, 0x40,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x20, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x80, 0x40,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x20, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x40, 0x20, 0x80, 0x40, 0x04, 0x00, 0x00, 0x00,
   0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xe0, 0xff, 0x40,
   0x04, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x00, 0x00, 0x40, 0x0e, 0xc3, 0xd1, 0x0c, 0x8e, 0x46, 0x07, 0xa3,
   0xc3, 0x69, 0x0c, 0x06, 0x5c, 0x0c, 0x00, 0x40, 0x84, 0x24, 0xb2, 0x13,
   0x91, 0xc9, 0x88, 0x64, 0x24, 0xda, 0x13, 0x09, 0x62, 0x12, 0x00, 0x40,
   0x44, 0x08, 0x12, 0xa1, 0xa0, 0x50, 0x50, 0x28, 0x08, 0x0a, 0xa1, 0x10,
   0x41, 0x21, 0x00, 0x40, 0xc4, 0x8f, 0x13, 0xa1, 0xa0, 0x50, 0xd0, 0x2f,
   0x88, 0x0b, 0xa1, 0x1f, 0x41, 0x3f, 0x00, 0x40, 0x44, 0x40, 0x12, 0xa1,
   0xa0, 0x50, 0x50, 0x20, 0x48, 0x0a, 0xa1, 0x00, 0x41, 0x01, 0x00, 0x40,
   0x44, 0x20, 0x12, 0xa1, 0xa0, 0x50, 0x50, 0x20, 0x28, 0x0a, 0xa1, 0x00,
   0x41, 0x01, 0x00, 0x40, 0x54, 0x28, 0x13, 0x21, 0x91, 0x50, 0x50, 0x28,
   0x28, 0x0b, 0xa1, 0x10, 0x61, 0x21, 0x00, 0x60, 0x88, 0xe7, 0x12, 0x21,
   0x8e, 0x50, 0x90, 0x27, 0xe8, 0x0a, 0x21, 0x4f, 0x5e, 0x9e, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//41 x 41
const uint8_t logo_a1[] PROGMEM = {
	0xfc, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x06, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x03, 0x00, 0x00, 0x00, 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0xf0, 0xff, 0x1f, 0x00, 0x01,
   0x01, 0x10, 0x00, 0x10, 0x00, 0x01, 0x01, 0x10, 0x00, 0x10, 0x00, 0x01,
   0x01, 0x10, 0x00, 0x10, 0x00, 0x01, 0x01, 0x10, 0x00, 0x10, 0x00, 0x01,
   0x01, 0x10, 0x00, 0x10, 0x00, 0x01, 0x01, 0x10, 0x00, 0x10, 0x00, 0x01,
   0x01, 0x10, 0x00, 0x10, 0x00, 0x01, 0x01, 0x10, 0x00, 0x10, 0x00, 0x01,
   0x01, 0xf0, 0xff, 0x1f, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x03, 0x00, 0x00, 0x00, 0x80, 0x01, 0x06, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0xfc, 0xff, 0xff, 0xff, 0x7f, 0x00
};

//37 x 37
const uint8_t logo_a2[] PROGMEM = {
	0xfc, 0xff, 0xff, 0xff, 0x07, 0x06, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00,
   0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00,
   0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01,
   0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00,
   0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0xf8,
   0xff, 0x03, 0x10, 0x01, 0x08, 0x00, 0x02, 0x10, 0x01, 0x08, 0x00, 0x02,
   0x10, 0x01, 0x08, 0x00, 0x02, 0x10, 0x01, 0x08, 0x00, 0x02, 0x10, 0x01,
   0x08, 0x00, 0x02, 0x10, 0x01, 0x08, 0x00, 0x02, 0x10, 0x01, 0x08, 0x00,
   0x02, 0x10, 0x01, 0xf8, 0xff, 0x03, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00,
   0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00,
   0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01,
   0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00,
   0x00, 0x10, 0x03, 0x00, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x00, 0x0c,
   0xfc, 0xff, 0xff, 0xff, 0x07
};

//33 x 33
const uint8_t logo_a3[] PROGMEM = {
	0xfc, 0xff, 0xff, 0x7f, 0x00, 0x06, 0x00, 0x00, 0xc0, 0x00, 0x03, 0x00,
   0x00, 0x80, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
   0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
   0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
   0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01,
   0x01, 0xfc, 0x7f, 0x00, 0x01, 0x01, 0x04, 0x40, 0x00, 0x01, 0x01, 0x04,
   0x40, 0x00, 0x01, 0x01, 0x04, 0x40, 0x00, 0x01, 0x01, 0x04, 0x40, 0x00,
   0x01, 0x01, 0x04, 0x40, 0x00, 0x01, 0x01, 0x04, 0x40, 0x00, 0x01, 0x01,
   0x04, 0x40, 0x00, 0x01, 0x01, 0xfc, 0x7f, 0x00, 0x01, 0x01, 0x00, 0x00,
   0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01,
   0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
   0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
   0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x80, 0x01, 0x06,
   0x00, 0x00, 0xc0, 0x00, 0xfc, 0xff, 0xff, 0x7f, 0x00 
};

//29 x 29
const uint8_t logo_a4[] PROGMEM = {
	0xfc, 0xff, 0xff, 0x07, 0x06, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x18,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x01, 0xfe, 0x0f, 0x10,
   0x01, 0x02, 0x08, 0x10, 0x01, 0x02, 0x08, 0x10, 0x01, 0x02, 0x08, 0x10,
   0x01, 0x02, 0x08, 0x10, 0x01, 0x02, 0x08, 0x10, 0x01, 0xfe, 0x0f, 0x10,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10,
   0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x10, 0x03, 0x00, 0x00, 0x18,
   0x06, 0x00, 0x00, 0x0c, 0xfc, 0xff, 0xff, 0x07
};

//60 x 60
const uint8_t warning[] PROGMEM = {
   0x00, 0x00, 0x80, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff,
   0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0x00, 0x00,
   0x00, 0x80, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff,
   0xff, 0x3f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
   0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0xfc, 0xff, 0xff,
   0xff, 0xff, 0x03, 0x00, 0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00,
   0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0x3f, 0x00,
   0xe0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0x7f, 0x00, 0xe0, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0x7f, 0x00, 0xf0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x00,
   0xf8, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x00, 0xf8, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0xff, 0x01, 0xf8, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x01,
   0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0xff, 0x03, 0xfe, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x03,
   0xfe, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x07,
   0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07,
   0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07,
   0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07,
   0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07,
   0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x07, 0xfe, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07,
   0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03,
   0xf8, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x01, 0xf8, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x00,
   0xf0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0xff, 0x00, 0xe0, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0x7f, 0x00, 0xe0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0x3f, 0x00,
   0xc0, 0xff, 0xff, 0x03, 0xfc, 0xff, 0x3f, 0x00, 0x80, 0xff, 0xff, 0x03,
   0xfc, 0xff, 0x1f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0xfc, 0xff, 0xff,
   0xff, 0xff, 0x03, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00,
   0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xff,
   0xff, 0x3f, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0xfe, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff,
   0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x1f, 0x00, 0x00, 0x00
};

//118 x 29
const uint8_t logo_rios[] PROGMEM = {
	 0xff, 0xff, 0x3f, 0x00, 0xf8, 0x0f, 0x00, 0xf8, 0xff, 0xff, 0x00, 0x00,
   0xfe, 0xff, 0x03, 0xff, 0xff, 0xff, 0x01, 0xf8, 0x0f, 0x00, 0xff, 0xff,
   0xff, 0x03, 0x80, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x07, 0xf8, 0x0f,
   0xc0, 0xff, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff,
   0x0f, 0xf8, 0x0f, 0xe0, 0xff, 0x87, 0xff, 0x1f, 0xf0, 0x7f, 0xe0, 0x0f,
   0xff, 0x01, 0xfc, 0x0f, 0xf8, 0x0f, 0xf8, 0x3f, 0x00, 0xf0, 0x7f, 0xf0,
   0x3f, 0x00, 0x00, 0xff, 0x01, 0xfc, 0x0f, 0xf8, 0x0f, 0xfc, 0x1f, 0x00,
   0xe0, 0xff, 0xf0, 0x3f, 0x00, 0x00, 0xff, 0x01, 0xfc, 0x0f, 0xf8, 0x0f,
   0xfc, 0x0f, 0x00, 0xc0, 0xff, 0xf0, 0xff, 0x01, 0x00, 0xff, 0x81, 0xff,
   0x07, 0xf8, 0x0f, 0xfc, 0x0f, 0x00, 0xc0, 0xff, 0xe0, 0xff, 0x3f, 0x00,
   0xff, 0xff, 0xff, 0x01, 0xf8, 0x0f, 0xfc, 0x0f, 0x00, 0xc0, 0xff, 0x80,
   0xff, 0xff, 0x03, 0xff, 0xff, 0x3f, 0x00, 0xf8, 0x0f, 0xfc, 0x0f, 0x00,
   0xc0, 0xff, 0x00, 0xfe, 0xff, 0x0f, 0xff, 0xff, 0x7f, 0x00, 0xf8, 0x0f,
   0xfc, 0x0f, 0x00, 0xc0, 0xff, 0x00, 0xf0, 0xff, 0x1f, 0xff, 0x81, 0xff,
   0x01, 0xf8, 0x0f, 0xfc, 0x0f, 0x00, 0xc0, 0xff, 0x00, 0x00, 0xf8, 0x3f,
   0xff, 0x01, 0xff, 0x03, 0xf8, 0x0f, 0xfc, 0x0f, 0x00, 0xe0, 0x7f, 0x00,
   0x00, 0xf0, 0x3f, 0xff, 0x01, 0xfe, 0x07, 0xf8, 0x0f, 0xf8, 0x3f, 0x00,
   0xf0, 0x3f, 0x00, 0x00, 0xf0, 0x3f, 0xff, 0x01, 0xfc, 0x0f, 0xf8, 0x0f,
   0xf0, 0xff, 0x00, 0xfe, 0x1f, 0xf8, 0x01, 0xf8, 0x1f, 0xff, 0x01, 0xfc,
   0x1f, 0xf8, 0x0f, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f,
   0xff, 0x01, 0xf8, 0x3f, 0xf8, 0x0f, 0x80, 0xff, 0xff, 0xff, 0x03, 0xf0,
   0xff, 0xff, 0x03, 0xff, 0x01, 0xf0, 0x3f, 0xf8, 0x0f, 0x00, 0xfc, 0xff,
   0x3f, 0x00, 0xc0, 0xff, 0xff, 0x00, 0xff, 0x01, 0xe0, 0x3f, 0xe0, 0x07,
   0x00, 0xf8, 0xff, 0x3f, 0x00, 0xc0, 0xff, 0xff, 0x00, 0xff, 0x01, 0xf0,
   0x3f, 0xf8, 0x0f, 0x00, 0xff, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03,
   0xff, 0x01, 0xf8, 0x1f, 0xf8, 0x0f, 0xc0, 0xff, 0xff, 0xff, 0x07, 0xf8,
   0xff, 0xff, 0x0f, 0xff, 0x01, 0xfc, 0x0f, 0xf8, 0x0f, 0xf0, 0xff, 0x87,
   0xff, 0x1f, 0xf8, 0x0f, 0xfc, 0x1f, 0xff, 0x01, 0xfe, 0x07, 0xf8, 0x0f,
   0xf8, 0x1f, 0x00, 0xf0, 0x3d, 0x30, 0x00, 0xf0, 0x3f, 0xff, 0x01, 0xff,
   0x03, 0xf8, 0x0f, 0xb8, 0x0f, 0x00, 0xe0, 0x4f, 0x00, 0x00, 0xf0, 0x3f,
   0xdb, 0x81, 0xff, 0x01, 0x68, 0x0e, 0x54, 0x0c, 0x00, 0x80, 0x77, 0x00,
   0x00, 0xf8, 0x3f, 0x1f, 0xbc, 0x79, 0x00, 0x00, 0x08, 0x2c, 0x01, 0x00,
   0x00, 0x41, 0x00, 0x00, 0xf0, 0x1d, 0x06, 0x10, 0x12, 0x00, 0x00, 0x0c,
   0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x81, 0x01, 0x27, 0x00, 0x16,
   0x00, 0x18, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,
   0x00, 0x00, 0x00
};

//63 x 40
const uint8_t logo_rjgo[] PROGMEM = {
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8,
   0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x7c, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xc0, 0x01, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x70, 0xf0,
   0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x38, 0xfb, 0x6f, 0x0e, 0x00, 0x00,
   0x00, 0x00, 0xcc, 0xfd, 0xdf, 0x19, 0x00, 0x00, 0x00, 0x00, 0x06, 0xfc,
   0x3f, 0x30, 0x00, 0x00, 0x00, 0xfe, 0xe3, 0xfd, 0xdf, 0xe3, 0x3f, 0x00,
   0x80, 0x03, 0xf8, 0xf9, 0xdf, 0x0f, 0xe0, 0x00, 0xc0, 0x00, 0xfc, 0xfb,
   0xef, 0x1f, 0x80, 0x01, 0xc0, 0xfe, 0xfc, 0xf3, 0xe7, 0x9f, 0x3f, 0x01,
   0x40, 0x7e, 0xfe, 0x03, 0xe0, 0x3f, 0x3f, 0x01, 0x60, 0x7f, 0xfe, 0xfb,
   0xef, 0x3f, 0x7f, 0x03, 0x60, 0x7f, 0xff, 0xf9, 0xcf, 0x7f, 0x7e, 0x03,
   0x20, 0x3f, 0xff, 0xfd, 0xdf, 0x7f, 0x7e, 0x02, 0xa0, 0x3f, 0xfe, 0xfe,
   0xbf, 0x3f, 0xfe, 0x06, 0xb0, 0xbf, 0xfe, 0xfe, 0x3f, 0xbf, 0xfe, 0x06,
   0xd0, 0xbf, 0x7e, 0xff, 0x7f, 0xbf, 0xfe, 0x05, 0xd0, 0xbf, 0x01, 0xff,
   0x7f, 0xc0, 0xfe, 0x0d, 0xd8, 0xbf, 0x7d, 0xff, 0x7f, 0xde, 0xfe, 0x0d,
   0xd8, 0xbf, 0x7e, 0xff, 0x7f, 0xbf, 0xfe, 0x0b, 0xe8, 0xbf, 0xfe, 0xfe,
   0xbf, 0xbf, 0xfe, 0x0b, 0xec, 0x3f, 0xfe, 0xfc, 0x9f, 0x3f, 0xfe, 0x1b,
   0xec, 0x3f, 0xff, 0xfd, 0xdf, 0x7f, 0xfe, 0x1b, 0xe4, 0x7f, 0xff, 0xfb,
   0xef, 0x7f, 0xff, 0x13, 0xe4, 0x7f, 0xfe, 0xf3, 0xe7, 0x3f, 0xff, 0x17,
   0xf6, 0xff, 0xfe, 0x03, 0xe0, 0x3f, 0xff, 0x37, 0xf6, 0xff, 0xfc, 0xfb,
   0xef, 0x9f, 0xff, 0x37, 0x04, 0x00, 0xf8, 0xfb, 0xcf, 0x0f, 0x00, 0x10,
   0xfc, 0xff, 0xfb, 0xfd, 0xdf, 0xcf, 0xff, 0x1f, 0xf8, 0xff, 0xe7, 0xfc,
   0x9f, 0xf3, 0xff, 0x0f, 0x00, 0x00, 0x0c, 0xfc, 0x1f, 0x18, 0x00, 0x00,
   0x00, 0x00, 0x9c, 0xf9, 0xdf, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x30, 0xf9,
   0x4f, 0x0e, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xe0, 0x83, 0x03, 0x00, 0x00,
   0x00, 0x00, 0x80, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//99x42
const uint8_t logo_rcj[] PROGMEM = {
	0x02, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x02, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x00, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0f, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xde, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0xde, 0x01, 0xfb, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x7e, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0xe0, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x3d, 0xc0,
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9f, 0x1f,
	0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x0e, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x20, 0x9e, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x44,
	0x04, 0x00, 0x60, 0xbf, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x6a, 0x06, 0x00, 0x80, 0x71, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xc0, 0x4a, 0x07, 0x00, 0x80, 0x78, 0x0e, 0xf0, 0x07, 0x80, 0x01, 0x00,
	0xf8, 0x43, 0x4a, 0x04, 0x00, 0xc0, 0x6c, 0x14, 0x38, 0x0f, 0x80, 0x01,
	0x00, 0xfc, 0xc1, 0x44, 0x04, 0x00, 0xc0, 0xde, 0x2c, 0x18, 0x0c, 0x80,
	0x01, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x80, 0xaf, 0x68, 0x18, 0x0c,
	0x80, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb, 0x58, 0x38,
	0x8e, 0x8f, 0x1f, 0x3e, 0x06, 0x18, 0xe3, 0x03, 0x00, 0x00, 0x7c, 0x70,
	0xf8, 0xcf, 0x99, 0x33, 0x67, 0x06, 0x1c, 0x73, 0x07, 0x00, 0x00, 0x7c,
	0x6c, 0xf8, 0xc7, 0xb8, 0x73, 0xe3, 0x06, 0x1c, 0x33, 0x06, 0x00, 0x00,
	0xe8, 0x4f, 0x18, 0xc7, 0xb8, 0x71, 0xe3, 0x06, 0x1c, 0x3b, 0x06, 0x00,
	0x00, 0xfc, 0x3a, 0x18, 0xce, 0xb8, 0x71, 0xe3, 0x0e, 0x1c, 0x3b, 0x06,
	0x00, 0x00, 0xff, 0x2e, 0x18, 0xce, 0xb9, 0x73, 0x67, 0x1c, 0x18, 0x3b,
	0x06, 0x00, 0x80, 0xcf, 0x15, 0x18, 0x9c, 0x1f, 0x3f, 0x7e, 0xf8, 0xfb,
	0xfb, 0x03, 0x00, 0xe0, 0x80, 0x0f, 0x10, 0x18, 0x0f, 0x1e, 0x3c, 0xf0,
	0xf1, 0xf9, 0x01, 0x00, 0xb0, 0x81, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x38, 0x00, 0x00, 0x78, 0x81, 0x05, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0xd8, 0x81, 0x07, 0x20, 0x00, 0xe0,
	0x00, 0x00, 0x00, 0x0e, 0x00, 0x05, 0x00, 0xb4, 0x01, 0x07, 0x20, 0x30,
	0xb0, 0x01, 0x00, 0x00, 0x1a, 0x00, 0x04, 0x00, 0xec, 0x8d, 0x03, 0x20,
	0x01, 0xb1, 0x6d, 0x23, 0x00, 0xde, 0x60, 0x05, 0x00, 0x78, 0xff, 0x02,
	0xa0, 0xba, 0xf2, 0x2e, 0x51, 0x07, 0x52, 0x4e, 0x05, 0x00, 0x58, 0xdc,
	0x02, 0xb0, 0xaa, 0x32, 0xc6, 0x56, 0x05, 0x56, 0x2a, 0x05, 0x00, 0x38,
	0x88, 0x02, 0x18, 0x19, 0x31, 0x6c, 0x23, 0x43, 0x5e, 0x66, 0x05, 0x00,
	0x3c, 0x18, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1c, 0x10, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x1c, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x14, 0xc8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x28, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x28, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t sm_logo = 0; //Logo fertig? StateMachine

void u8g_DrawStartUp(void) //,,Verschönerung" zum Zeit ,,schinden" am Anfang für UM6
{
	if((mcusr_mirror & (1<<0)) ||
		 check_res) //Nur bei Power-off Neustart (nur dann kalibriert sich UM6 neu) und bei Reset (Warnung anzeigen)
	{
		while(sm_logo < 7)
		{
			u8g_FirstPage(&u8g);
		 	do
			{
			 	if(!check_res) //Normaler Start
				{
					/////Logo (Animation)
					switch(sm_logo)
					{
						case 0:	u8g_DrawXBMP(&u8g, 5, 5, 118, 29, logo_rios); //500ms
										break;
						case 1:	u8g_DrawXBMP(&u8g, 14, 2, 99, 42, logo_rcj); //500ms
										break;
						case 2:	u8g_DrawXBMP(&u8g, 44, 2, 41, 41, logo_a1);		//500ms
										break;
						case 3:	u8g_DrawXBMP(&u8g, 60, 2, 37, 37, logo_a2);		//20ms
										break;
						case 4: u8g_DrawXBMP(&u8g, 76, 2, 33, 33, logo_a3);		//20ms
										break;
						case 5:	u8g_DrawXBMP(&u8g, 91, 2, 29, 29, logo_a4);		//20ms
										break;
						case 6:	u8g_DrawXBMP(&u8g, 0, 0, 128, 29, logo);			//1000ms
										break;
						default: 	foutf(&str_error, "%i: ERR:sw[disp.01]:DEF\n\r", timer);
									fatal_err = 1;
					}
					///////Text/////
					u8g_SetFont(&u8g, u8g_font_6x10); //Schriftart
					u8g_DrawStr(&u8g, 20, 	55, "Calibrate IMU...");
	
					u8g_SetFont(&u8g, u8g_font_4x6); //Schriftart

					if(mcusr_mirror & (1<<0))
						u8g_DrawStr(&u8g, 40, 	64, "ResSRC: PORF"); //Power-on Reset Flag
					else if(mcusr_mirror & (1<<1))
						u8g_DrawStr(&u8g, 40, 	64, "ResSRC: EXTRF"); //External Reset Flag
					else if(mcusr_mirror & (1<<2))
						u8g_DrawStr(&u8g, 40, 	64, "ResSRC: BORF"); //Brown-out Reset Flag
					else if(mcusr_mirror & (1<<4))
						u8g_DrawStr(&u8g, 40, 	64, "ResSRC: JTRF"); //JTAG Reset Flag

					u8g_DrawStr(&u8g, 0, 	57, "V3.1");
					u8g_DrawStr(&u8g, 0, 	63, "RCJ 2014");
				}
				else	//Watchdog Start!
				{
					u8g_DrawXBMP(&u8g, 2, 2, 60, 60, warning);
					u8g_SetFont(&u8g, u8g_font_8x13Br); //Schriftart
					u8g_DrawStr(&u8g, 65, 	10, "WARNING");
					u8g_SetFont(&u8g, u8g_font_6x10);
					u8g_DrawStr(&u8g, 65, 	20, "Recovered");
					u8g_DrawStr(&u8g, 65, 	30, "after an");
					u8g_DrawStr(&u8g, 65, 	40, "unexpected");
					u8g_DrawStr(&u8g, 65, 	50, "shutdown!");
					//u8g_SetFont(&u8g, u8g_font_4x6); //Schriftart
					//u8g_DrawStr(&u8g, 60, 	60, "Calibrate IMU...");
				}
			} while(u8g_NextPage(&u8g));

			while(get_t1()); //Animation pausieren, während Pausetaste gedückt gehalten wird
			if((sm_logo == 0) || (sm_logo == 1) || (sm_logo == 2))
				_delay_ms(500);
			else if(sm_logo == 6)
			{
				_delay_ms(1000);
			}
			else
				_delay_ms(20);
			
			if(sm_logo < 7)
				sm_logo++;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
uint8_t disp_msg_i = 0;
uint8_t which_msg = 0;
uint8_t is_msg = 0;

#define DISP_MSG_I_TOP 14

const char *disp_msg[DISP_MSG_I_TOP+1] = {
	"teamohnename.de - ok",
	"WARNING 01: Battery low",
	"WARNING 02: Reset",
	"FATAL ERROR: CHECK LOG",
	"ERROR 10: MLX90614 L",
	"ERROR 11: MLX90614 R",
	"ERROR 20: UM6 Comm.",
	"ERROR 21: UM6 Drift",
	"ERROR 30: SRF10 L",
	"ERROR 31: SRF10 R",
	"INFO 01: Motor off",
	"INFO 02: Bluetooth on",
	"INFO 03: Setup",
	"INFO 04: Restart...",
	"INFO 05: LOP..."
};

void set_msg(void) //Hilfsfunktion
{
	which_msg = disp_msg_i+1;
	if(!(is_msg & (1<<0)))
	{
		is_msg |= (1<<0);
		is_msg |= (1<<1);
		timer_disp_msg = TIMER_DISP_MSG;
	}
}

enum DISP_MSG {WNG_01, WNG_02, ERR_FA, ERR_10, ERR_11, ERR_20, ERR_21, ERR_30, ERR_31, INF_01, INF_02, INF_03, INF_04, INF_05}; //Ok-Meldung nicht enthalten!

void u8g_DrawInfo(void) //Malen der Batterie, Anzeige in %
{
	///////Infos////  X  /////////
	//  Karte   //Pfeil// Frei  //
	//  akt.    //     // hier: //
	//  Raum    /////////       //
	//          //Info //       //
	//          //Karte//       //
	//////////////////////////////
		
	u8g_DrawHLine(&u8g, 0, 6, 128); //Horizontale Abgrenzungslinie

	u8g_SetFont(&u8g, u8g_font_4x6); //Schriftart
	
	//////Batterie////////////
	if(batt_percent > -1)
	{
		u8g_DrawFrame(&u8g, 0, 0, 10, 5); //Batteriekörper
		u8g_DrawPixel(&u8g, 10, 2);
		int8_t batt_volumeVar = batt_percent/11; //Berechnung Füllung
		if(batt_volumeVar > 8)
			batt_volumeVar = 8;
		if(batt_volumeVar < 0)
			batt_volumeVar = 0;	
		u8g_DrawBox(&u8g, 1, 1, batt_volumeVar, 3); //Füllung

		//Anzeige daneben
		if(batt_percent < 99)
			u8g_DrawLong(15,	5, batt_percent);
		else
			u8g_DrawLong(15,	5, 99);

		u8g_DrawStr(&u8g, 25, 	5, "%");
	}
	else
	{
		u8g_DrawStr(&u8g, 0, 	5, "ERR:BAT");
	}
	
	//Fehleranzeige daneben:
	if(timer_disp_msg == 0)
	{
		disp_msg_i ++;
		if(disp_msg_i == DISP_MSG_I_TOP)
		{
			disp_msg_i = 0;
			if(!(is_msg & (1<<1)))
				which_msg = 0;
			else
				is_msg &= ~ (1<<1);
		}
		is_msg &= ~ (1<<0);
	}

	switch(disp_msg_i)
	{
		case WNG_01:		if(batt_percent < 20)															set_msg();		break;
		case WNG_02:		if(check_res)																	set_msg();		break;
		case ERR_FA:		if(fatal_err)																	set_msg();		break;
		case ERR_10:		if(check_mlx & (1<<0))															set_msg();		break;
		case ERR_11:		if(check_mlx & (1<<1))															set_msg();		break;
		case ERR_20:		if((check_um6 & (1<<0)) || (check_um6 & (1<<1)))								set_msg();		break;
		case ERR_21:		if(check_um6 & (1<<2))															set_msg();		break;
		case ERR_30:		if(check_srf & (1<<0))															set_msg();		break;
		case ERR_31:		if(check_srf & (1<<1))															set_msg();		break;
		case INF_01:		if(mot.off)																		set_msg();		break;
		case INF_02:		if(timer_bt_is_busy > 0)														set_msg();		break;
		case INF_03: 		if(setup != 0)																	set_msg();		break;
		case INF_04: 		if(timer_rdy_restart > -1)														set_msg();		break;
		case INF_05:		if((maze_solve_state_path == LOP_INIT) || (maze_solve_state_path == LOP_WAIT))	set_msg();		break;
		default:			foutf(&str_error, "%i: ERR:sw[disp.02]:DEF\n\r", timer);
							fatal_err = 1;
	}
	
	u8g_DrawStr(&u8g, 35, 	5, disp_msg[which_msg]);
}

void u8g_drawArrow(uint8_t size, uint8_t pos_x, uint8_t pos_y, uint8_t dir, int8_t mode)
{
	uint8_t arrowlines_angle_pc = 2; //PC PerCent, logisch nicht korrekt, aber ok

	if(size > 10)
		arrowlines_angle_pc = 3;

	switch(dir)
	{
		case 1:	//NORTH
							if(mode) u8g_DrawLine(&u8g, pos_x+(size/2), pos_y, pos_x-(size/2), pos_y);
							u8g_DrawLine(&u8g, pos_x+(size/2), pos_y, pos_x, pos_y-(size/arrowlines_angle_pc));  // -
							u8g_DrawLine(&u8g, pos_x+(size/2), pos_y, pos_x, pos_y+(size/arrowlines_angle_pc));  // -
						break;
		case 2:	//EAST
							if(mode) u8g_DrawLine(&u8g, pos_x, pos_y+(size/2), pos_x, pos_y-(size/2));
							u8g_DrawLine(&u8g, pos_x, pos_y+(size/2), pos_x-(size/arrowlines_angle_pc),pos_y);  // /
							u8g_DrawLine(&u8g, pos_x, pos_y+(size/2), pos_x+(size/arrowlines_angle_pc), pos_y); //  \ //
						break;
		case 3:	//SOUTH
							if(mode) u8g_DrawLine(&u8g, pos_x-(size/2), pos_y, pos_x+(size/2), pos_y); // –
							u8g_DrawLine(&u8g, pos_x-(size/2), pos_y, pos_x, pos_y-(size/arrowlines_angle_pc));  // -
							u8g_DrawLine(&u8g, pos_x-(size/2), pos_y, pos_x, pos_y+(size/arrowlines_angle_pc));  // -
						break;
		case 4:	//WEST
							if(mode) u8g_DrawLine(&u8g, pos_x, pos_y-(size/2), pos_x, pos_y+(size/2)); // |
							u8g_DrawLine(&u8g, pos_x, pos_y-(size/2), pos_x-(size/arrowlines_angle_pc),pos_y);  // /
							u8g_DrawLine(&u8g, pos_x, pos_y-(size/2), pos_x+(size/arrowlines_angle_pc), pos_y); //  \ //
						break;
		default:  u8g_DrawStr(&u8g, pos_x-(size/2), pos_y, "ERROR");
						break;
	}
}

//////////////////////////////

#define SETUP_MODE_TOP 4 //Über der Zahl muss mode von vorne (bei 0) anfangen
	//Temp
	//Victim
	//View
	//Cam
	//ok

#define SETUP_STEP_TOP_MLX 2
	//Cursor
	//1000er
	//100er

#define SETUP_STEP_TOP_BIN 1
	//1er

int8_t setup_mode = 0; //Temp, view, ground?
int8_t setup_step = 0;

struct {
	unsigned temp:1;
	unsigned tarry:1;
	unsigned cam:1;
	unsigned cam_ground:1;
	unsigned ground:1;
	//1 bit left
} eepr_value_changed;
	
///////Hilfsfunktionen:
int32_t incremental_old_setup = 0;

void setupStep_Fac(int16_t fac)
{
	switch(setup_mode)
	{
		case 0: 	mlx90614[RIGHT].th += ((incremental - incremental_old_setup) * fac);
					if(mlx90614[RIGHT].th < 0)
						mlx90614[RIGHT].th = 4000;
					eepr_value_changed.temp = 1;
					break;
		case 1: 	setup = 4;
						break;
		case 2: 	setup = 2; //Modus 2
						break;
		case 3: 	setup = 3; //Modus 3
						break;
		case 4: 	/*if(eepr_value_changed.temp)
					{
						eeprom_update_word((uint16_t*)0, mlx90614[LEFT].th);
						eepr_value_changed.temp = 0;
					}*/
					if(eepr_value_changed.temp)
					{
						eeprom_update_word((uint16_t*)2, mlx90614[RIGHT].th);
						eepr_value_changed.temp = 0;
					}
					/*if(eepr_value_changed.tarry)
					{
						eeprom_update_byte((uint8_t*)4, use_tarry);
						eepr_value_changed.tarry = 0;
					}*/
					if(eepr_value_changed.cam)
					{
						eeprom_update_word((uint16_t*)6, tsl_th);
						eepr_value_changed.cam = 0;
					}
				/*	if(eepr_value_changed.ground)
					{
						//eeprom_update_word((uint16_t*)8, ground_th);
						eepr_value_changed.ground = 0;
					}
					if(eepr_value_changed.cam_ground)
					{
						//eeprom_update_word((uint16_t*)10, tsl_th_ground);
						//tsl_th_ground = tsl_th + 30;
						eepr_value_changed.cam_ground = 0;
					}*/

					setup = 0;
					motor_activate(1); //Activate motor driver

						break;
		default:			foutf(&str_error, "%i: ERR:sw[disp.03]:DEF\n\r", timer);
							fatal_err = 1;
	}
}

///////


uint8_t init_setup = 0;
void u8g_DrawSetUp(void)
{
	u8g_SetFont(&u8g, u8g_font_4x6);

	if(!get_incrOk())
		init_setup = 1;

	if(get_incrOk() && (timer_incr_entpr == 0) && init_setup)
	{
		switch(setup_mode)
		{
			case 1:		setup_step ++;
						if(setup_step > SETUP_STEP_TOP_MLX)
							setup_step = 0;
						break;
			case 0:
			case 2:
			case 3:
			case 4: 	setup_step ++;
						if(setup_step > SETUP_STEP_TOP_BIN)
							setup_step = 0;
						break;
			default: 	foutf(&str_error, "%i: ERR:sw[disp.04]:DEF\n\r", timer);
						fatal_err = 1;
		}

		timer_incr_entpr = TIMER_ENTPR_INCR;
	}

	switch(setup_step)
	{
		case 0:		setup_mode += (incremental - incremental_old_setup);
					if(setup_mode > SETUP_MODE_TOP)
						setup_mode = 0;
					else if(setup_mode < 0)
						setup_mode = SETUP_MODE_TOP;
						break;
		case 1:		setupStep_Fac(100);
						break;
		case 2:		setupStep_Fac(100);
						break;
		case 3:		setupStep_Fac(10);
						break;
		case 4:		setupStep_Fac(1);
						break;
		default: 	foutf(&str_error, "%i: ERR:sw[disp.05]:DEF\n\r", timer);
					fatal_err = 1;
	}
	incremental_old_setup = incremental;

	switch(setup_mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4: 	u8g_drawArrow(5, 60, 10+(setup_mode*7), NORTH, 0);
						break;
		default: 	foutf(&str_error, "%i: ERR:sw[disp.06]:DEF\n\r", timer);
					fatal_err = 1;
	}

	u8g_DrawStr(&u8g, 0, 13, "IR:"); /*u8g_DrawLong(35, 20,victim_BufGetMaxDiff(RIGHT));*/		u8g_DrawLong(65, 13, mlx90614[RIGHT].th);
	u8g_DrawStr(&u8g, 0, 20, "Victim");
	u8g_DrawStr(&u8g, 0, 27, "View");
	u8g_DrawStr(&u8g, 0, 34, "Ground");

	u8g_DrawStr(&u8g, 65,41, "ok");

	u8g_DrawStr(&u8g, 0, 48, "IR l:"); u8g_DrawLong(65, 48, mlx90614[LEFT].is);
	u8g_DrawStr(&u8g, 0, 55, "IR r:"); u8g_DrawLong(65, 55, mlx90614[RIGHT].is);

}

/////////////////////////////
void u8g_DrawView(void)
{
	if(get_incrOk() && (timer_incr_entpr == 0))
	{
		setup = 1;
		setup_step = 0;

		timer_incr_entpr = TIMER_ENTPR_INCR;
	}

	u8g_SetFont(&u8g, u8g_font_4x6);

	int16_t sensorvar = 0;
	
	u8g_DrawFrame(&u8g, 59, 27, 11, 11); //Roboter
	if(um6.psi > 0 && um6.psi <= 90)
		u8g_drawArrow(7, 64, 32, NORTH, 0);
	else if(um6.psi > 90 && um6.psi <= 180)
		u8g_drawArrow(7, 64, 32, WEST, 0);
	else if(um6.psi > 180 && um6.psi <= 270)
		u8g_drawArrow(7, 64, 32, SOUTH, 0);
	else
		u8g_drawArrow(7, 64, 32, EAST, 0);

	sensorvar = dist[LIN][RIGHT][FRONT] / 18;
	u8g_DrawHLine(&u8g, 71, 28, sensorvar);
	sensorvar = dist[LIN][RIGHT][BACK] / 18;
	u8g_DrawHLine(&u8g, 71, 36, sensorvar);

	sensorvar = (dist[LIN][BACK][RIGHT]/18);
	u8g_DrawVLine(&u8g, 68, 39, sensorvar);
	sensorvar = (dist[LIN][BACK][BACK]/30);
	u8g_DrawVLine(&u8g, 64, 39, sensorvar);
	sensorvar = (dist[LIN][BACK][LEFT]/18);
	u8g_DrawVLine(&u8g, 60, 39, sensorvar);

	sensorvar = 57 - (dist[LIN][LEFT][FRONT] / 18);
	u8g_DrawLine(&u8g, 57, 28, sensorvar, 28);
	sensorvar = 57 - (dist[LIN][LEFT][BACK] / 18);
	u8g_DrawLine(&u8g, 57, 36, sensorvar, 36);

	sensorvar = 25 - (dist[LIN][FRONT][LEFT] / 18);
	u8g_DrawLine(&u8g, 60, 25, 60, sensorvar);
	sensorvar = 25 - (dist[LIN][FRONT][FRONT] / 30);
	u8g_DrawLine(&u8g, 64, 25, 64, sensorvar);
	sensorvar = 25 - (dist[LIN][FRONT][RIGHT] / 18);
	u8g_DrawLine(&u8g, 68, 25, 68, sensorvar);



	u8g_DrawLong(75,	13, dist[LIN][FRONT][LEFT]);
	u8g_DrawLong(75,	20, dist[LIN][FRONT][FRONT]);
	u8g_DrawLong(75,	27, dist[LIN][FRONT][RIGHT]);

	u8g_DrawLine(&u8g, 90, 20, 106, 10);

	u8g_DrawLong(110,	13, dist[LIN][RIGHT][FRONT]);
	u8g_DrawLong(110,	20, dist[LIN][RIGHT][LEFT]);
	u8g_DrawLong(110,	27, dist[LIN][RIGHT][BACK]);

	u8g_DrawLong(8,	48, dist[LIN][LEFT][FRONT]);
	u8g_DrawLong(8,	55, dist[LIN][LEFT][LEFT]);
	u8g_DrawLong(8,	62, dist[LIN][LEFT][BACK]);

	u8g_DrawLine(&u8g, 20, 60, 36, 50);

	u8g_DrawLong(43,	48, dist[LIN][BACK][LEFT]);
	u8g_DrawLong(43,	55, dist[LIN][BACK][BACK]);
	u8g_DrawLong(43,	62, dist[LIN][BACK][RIGHT]);

	u8g_DrawStr(&u8g,		0, 	13, "UM6:");
	u8g_DrawStr(&u8g,		17, 	13, "x:"); u8g_DrawLong(25,	13, um6.phi);
	u8g_DrawStr(&u8g,		17, 	20, "y:"); u8g_DrawLong(25,	20, um6.theta);
	u8g_DrawStr(&u8g,		17, 	27, "z:"); u8g_DrawLong(25,	27, um6.psi);

	u8g_DrawStr(&u8g,		75, 	48, "Temp:");
	u8g_DrawStr(&u8g,		95, 	48, "l:"); u8g_DrawLong(105,	48, mlx90614[LEFT].is);
	u8g_DrawStr(&u8g,		95, 	55, "r:"); u8g_DrawLong(105,	55, mlx90614[RIGHT].is);
}

/////////////////////////////
uint8_t viewCam_mode = 0;
uint8_t viewCam_sorted = 0; //Sollen die Daten sortiert dargestellt werden?
uint32_t incremental_old_cam = 0;

uint8_t incrOk_pushDuration = 0; //Langer/Kurzer Tastendruck?
uint8_t pushed = 0;

void u8g_DrawCamRaw(void)
{
	if(get_incrOk() && (timer_incr_entpr == 0))
	{
		pushed = 1;
		if(incrOk_pushDuration < 0xff)
			incrOk_pushDuration ++;
	}
	else
	{
		if(pushed == 1)
		{
			if(incrOk_pushDuration < 100)
				timer_incr_entpr = TIMER_ENTPR_INCR;
	
			if((incrOk_pushDuration < 100) &&
				 (viewCam_mode == 0))
			{
				setup = 1;
				setup_step = 0;
			}
			else if((incrOk_pushDuration < 100) &&
							(viewCam_mode == 1))
			{
				viewCam_mode = 0;
			}
		
			if(incrOk_pushDuration > 100)
				viewCam_sorted ^= 1;
			
			incrOk_pushDuration = 0;
			pushed = 0;
		}
	}
	
	if(viewCam_mode == 0)
	{
		viewCam_mode += (incremental-incremental_old_cam);
		if(viewCam_mode > 1)
			viewCam_mode = 0;
		if(viewCam_mode < 0)
			viewCam_mode = 1;
	}
	else if(viewCam_mode == 1)
	{
		u8g_drawArrow(5, 125, 24, SOUTH, 0);
		tsl_th += (incremental-incremental_old_cam)*10;
		if(tsl_th < 0)
			tsl_th = 0;
		tsl_th_ground = tsl_th + TSL_GROUNDSENS_DIFF;
		if((int8_t)(incremental-incremental_old_cam) != 0) //etwas wurde geändert
			eepr_value_changed.cam = 1;
	}
	
	
	u8g_SetFont(&u8g, u8g_font_4x6);

	u8g_DrawStr(&u8g,		65, 	34, "Ground l:"); u8g_DrawLong(110,	34, groundsens_l);
	u8g_DrawStr(&u8g,		65, 	41, "Ground r:"); u8g_DrawLong(110,	41, groundsens_r);

	u8g_DrawStr(&u8g,		65, 	55, "TH Ground:"); 	u8g_DrawLong(110,	55, ground_th);
	//u8g_DrawStr(&u8g,		65, 	62, "View:"); 			if(viewCam_sorted)	u8g_DrawStr(&u8g,	110, 	62, "srt");
	//																							else								u8g_DrawStr(&u8g,	110, 	62, "raw");
	
	incremental_old_cam = incremental;
}

/////////////////////////////Victim/////////////////////////////////////////

void u8g_DrawVictim(void)
{
	if(get_incrOk())
		setup = 1;

	uint8_t victimDir;

	if(incremental % 2 == 0)
	{
		u8g_DrawStr(&u8g,		65, 	13, "LEFT");
		victimDir = LEFT;
	}
	else
	{
		u8g_DrawStr(&u8g,		65, 	13, "RIGHT");
		victimDir = RIGHT;
	}

	u8g_DrawTempScan(victimDir);

	u8g_DrawStr(&u8g,		65, 	21, "Max Diff:"); 	u8g_DrawLong(100,	21, victim_BufGetMaxDiff(victimDir));
}

/////////////////////////////Pixy///////////////////////////////////////////

void u8g_DrawPixy(void)
{
	for(uint8_t i = 0; i < pixy_number_of_blocks; i++)
	{
		u8g_DrawBox(&u8g, pixy_blocks[i].x/3, pixy_blocks[i].y/3+7, pixy_blocks[i].width/3, pixy_blocks[i].height/3);
	}
}

/////////////////////////////////Frontscan/////////////////////////////////////

void u8g_DrawFrontScan(void)
{
	uint8_t angle = data_scanFront[DATA_SCANFRONT_REG_ANGLE];

	for(uint8_t i = DATA_SCANFRONT_START; i < (DATA_SCANFRONT_SIZE); i++) //For every saved distance
	{
		int8_t alpha = ((-(angle/2)) + (i * (angle / DATA_SCANFRONT_SIZE))); //
		uint8_t alpha_abs = 90 - abs(alpha);
		uint8_t beta = 90 - alpha_abs;

		int16_t x = ((data_scanFront[i]+incremental*10)/12) * sin(beta * (M_PI/180));
		if(alpha < 0)
			x *= -1;
		x += 40;

		int16_t y = 63 - ((data_scanFront[i]+incremental*10)/12) * sin(alpha_abs * (M_PI/180));

		if(data_scanFront[i] < DIST_MAX_SRP_NEW)
			//u8g_DrawLine(&u8g, 40, 63, x, y);
			u8g_DrawPixel(&u8g, x, y);
		//u8g_DrawVLine(&u8g,i,64-(data_scanFront[i]/10),(data_scanFront[i]/10));
	}
}

///////////////////////////Tempscan//////////////////////////////////////////////

void u8g_DrawTempScan(uint8_t dir)
{
	for(int8_t i = 0; i < VICTIMBUFFER_SIZE; i++)
		if((victimBuf[dir].value[i] != VICTIMBUF_UNUSED) && ((victimBuf[dir].value[i]-victimBuf[dir].lowest)/50 > 0))
			u8g_DrawVLine(&u8g,i,63-(victimBuf[dir].value[i] - victimBuf[dir].lowest)/50,
									(victimBuf[dir].value[i] - victimBuf[dir].lowest)/50);
}
