#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uart.h"

extern int32_t displayvar[3];

extern int16_t speed_l_raw;
extern int16_t speed_r_raw;

extern int16_t um6_phi;
extern int16_t um6_theta;
extern int16_t um6_psi;
extern int32_t um6_psi_t; //total

extern int16_t um6_gyr_z;
/////////////////////////////

extern uint8_t um6_getUM6(void);

extern uint8_t um6_gyroZeroRate(void);
