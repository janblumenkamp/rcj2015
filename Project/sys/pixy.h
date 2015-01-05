////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////pixy.h////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe pixy.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef PIXY_H
#define PIXY_H

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUMBER_OF_PIXY_BLOCKS 7 //HAS TO BE CONFIGURED IN PIXYMON!

#define PIXY_I2C_ADR 0x54<<1

#define PIXY_START_WORD             0xaa55
#define PIXY_START_WORDX            0x55aa

enum PIXY_SM {GET_START_1, GET_START_2, GET_BLOCKS};

struct _PIXY_BLOCK {
	uint8_t signature;
	uint8_t x;
	uint8_t y;
	uint8_t width;
	uint8_t height;
};

extern struct _PIXY_BLOCK pixy_blocks[NUMBER_OF_PIXY_BLOCKS];

extern uint8_t pixy_number_of_blocks;


extern uint16_t pixy_getWord(uint8_t adr);

extern uint8_t pixy_get(void);

#endif // PIXY_H
