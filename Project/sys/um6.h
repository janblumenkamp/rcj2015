////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
///////////////////////////////////um6.h////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	siehe um6.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef UM6_C
#define UM6_C

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


typedef struct _um6_t UM6_t;

struct _um6_t {
	void (*put_c)(unsigned char c);
	unsigned int (*get_c)(void);

	uint8_t gyr_drift_cnt;
	uint8_t isBusy;

	int16_t phi_old;
	int16_t psi_old;
	int16_t theta_old;

	int16_t phi;
	int32_t phi_t; //total
	int16_t theta;
	int16_t theta_t; //total
	int16_t psi;
	int32_t psi_t; //total
	int16_t accl_x;
	int16_t accl_y;
	int8_t isRamp;
	int16_t gyr_x, gyr_y, gyr_z;
};

/////////////////////////////

extern void um6_init(UM6_t *_um6, void (*put_c)(unsigned char c), unsigned int (*get_c)(void));

extern uint8_t um6_rwc(UM6_t *_um6, uint8_t um6_register, uint8_t batch, uint8_t r_w_c, uint16_t um6_result[], uint8_t data_size);

extern uint8_t um6_getUM6(UM6_t *_um6);

extern uint8_t um6_gyroZeroRate(UM6_t *um6);

extern int8_t um6_getPsiCorrection(void);

extern void um6_checkRamp(UM6_t *um6);

#endif
