#include "u8g.h"
	extern u8g_t u8g; //Pointer für u8g muss hier eingebunden werden
#include "system.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/eeprom.h>

extern int32_t displayvar[3];

extern int8_t batt_percent;
extern uint16_t batt_mV;

extern uint8_t motor_off;

extern volatile int8_t timer_bt_is_busy;
extern volatile int8_t timer_disp_msg;

extern uint16_t check_srf;
extern uint8_t check_mlx;
extern uint8_t check_um6;
extern uint16_t check_sir;
extern uint8_t use_map;
extern uint8_t setup;
	extern int16_t mlx90614_l_th;
	extern int16_t mlx90614_r_th;
	extern int16_t mlx90614_l;
	extern int16_t mlx90614_r;
	extern uint8_t use_tarry;
	extern volatile int8_t timer_incr_entpr;
extern int8_t incr_delta;

extern const uint8_t NONE; //Richtung (Orientierung), siehe auch DRIVE.C!!!!!
extern const uint8_t NORTH;
extern const uint8_t EAST;
extern const uint8_t SOUTH;
extern const uint8_t WEST;

/////////////////////////////////

extern void u8g_DrawLong(uint8_t koord_x, uint8_t koord_y, int32_t num);

extern void u8g_DrawLogo(void);

extern void u8g_drawArrow(uint8_t size, uint8_t pos_x, uint8_t pos_y, uint8_t dir);

extern void u8g_DrawInfo(void);

extern void u8g_DrawSetUp(void);
