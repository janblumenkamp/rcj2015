////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////victim.h//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe victim.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef VICTIM_H
#define VICTIM_H

#endif // VICTIM_H

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct _victimbuffer VICTIMBUFFER;
#define VICTIMBUFFER_SIZE 40 //cm
#define VICTIMBUF_UNUSED 0xffff/2

struct _victimbuffer {
	int16_t value[VICTIMBUFFER_SIZE];
	uint8_t next;
	int16_t lowest;
	int16_t highest;
};

extern VICTIMBUFFER victimBuf[2];
enum VICTIM_CHECK {VIC_INIT, VIC_ALIGN_NEAR, DEPL_1, VIC_ALIGN_NORM, DEPL_2};

extern void victim_init(void);

extern void victim_scan(void);

extern int16_t victim_BufGetRaw(uint8_t dir);

extern int16_t victim_BufGetLastHighest(uint8_t index, uint8_t dir);

extern int16_t victim_BufGetMaxDiff(uint8_t dir);

extern uint8_t victim_BufIsVic(uint8_t dir);
