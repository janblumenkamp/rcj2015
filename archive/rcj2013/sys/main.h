#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <avr/wdt.h> //watchdog
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

extern int32_t displayvar[3];

extern const uint8_t FRONT;
extern const uint8_t BACK;
extern const uint8_t LEFT;
extern const uint8_t RIGHT;
extern uint16_t wallsensor[4][4];

extern int8_t batt_percent;

extern uint16_t batt_mV;

extern uint8_t motor_off;

extern volatile uint32_t timer;

extern int16_t incremental;
extern int8_t incr_delta;
extern int32_t enc_l;
extern int32_t enc_r;
