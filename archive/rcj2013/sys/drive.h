////////////////////////////////////////////////////////////////////////////////
//////////////////////////Fahrfunktionen////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "system.h"
#include "bluetooth.h"

extern const uint8_t FRONT;
extern const uint8_t BACK;
extern const uint8_t LEFT;
extern const uint8_t RIGHT;
extern uint16_t wallsensor[4][4];
extern uint16_t wallsensor_top[4];

extern int8_t impasse; //Sackgasse ja nein
extern int8_t ramp; //Sackgasse ja nein
extern int8_t endmaze; //Labyrinth zuende

extern volatile int8_t timer_align_dead;
extern volatile int8_t timer_rotate_dead;
extern volatile uint32_t timer;

extern int16_t speed_l;
extern int16_t speed_r;

extern uint16_t srf10[4];

extern uint16_t check_srf;
extern uint8_t check_mlx;
extern uint8_t check_um6;
extern uint16_t check_sir;

extern int32_t enc_l; //Encoder
extern int32_t enc_r;
extern int32_t enc_lr; //Nur in drive_oneTile aktualisiert

extern int16_t um6_psi;
extern int16_t um6_theta;
extern int16_t um6_phi;
extern int32_t um6_psi_t; //total

extern uint8_t sm_dot;
//////////////////////////////////

extern uint16_t check_srf;
extern uint8_t check_mlx;
extern uint8_t check_um6;

/////////////////////////////////

extern int32_t displayvar[3];
///////////////////////

//Return: 0: fertig
//Return: 1: in Arbeit
//Return: 2: //fährt, aber entfernung groß, deshalb nur auf enc achten
//Return: 3: eigentlich geht es nicht weiter nach vorne, es wird sich nur auf mitte gerichtet
extern uint8_t drive_oneTile(void);

extern uint8_t drive_ramp(int8_t speed);

extern uint8_t drive_turn(int16_t angle, uint8_t align);

extern uint8_t drive_lr(uint8_t left, uint8_t width);

extern uint8_t drive_dist(int8_t motor, int8_t dist_cm); //motor: <0 links, 0 beide, >0 rechts

extern void drive_reset(void);
