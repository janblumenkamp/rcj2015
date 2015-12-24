#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>

#include "system.h"

extern int32_t displayvar[3];

extern const uint8_t NONE; //Richtung (Orientierung), siehe auch DRIVE.C!!!!!
extern const uint8_t NORTH;
extern const uint8_t EAST;
extern const uint8_t SOUTH;
extern const uint8_t WEST;

extern const uint8_t FRONT;
extern const uint8_t BACK;
extern const uint8_t LEFT;
extern const uint8_t RIGHT;

extern uint16_t wallsensor[4][4];
extern uint16_t wallsensor_top[4];

////////////////////

extern void led_hsvToRgb (uint8_t hue, uint8_t  saturation, uint8_t value); //Farbe, Sättigung, Helligkeit

extern void led_rgb(uint16_t heartbeat_color, uint16_t led_error);

extern uint16_t get_sharpIR(void);
