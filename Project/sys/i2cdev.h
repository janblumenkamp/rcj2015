////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////i2cdev.h///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe i2cdev.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//I²C Adressen
#define I2C_MLX90614_L 0x5A<<1
#define I2C_MLX90614_R 0x5C<<1

#define I2C_SRF10 0xE6
//#define I2C_SRF10_L 0xF2

//Commands für I²C
#define I2C_REG_MLX90614 0x07
#define I2C_REG_SRF10_MEASERUEMENT_S 0x51 //Laufzeit in µS

struct _SRF {
	int8_t state;
	int16_t dist;
};

#define NUMBER_OF_SRF 1

extern struct _SRF srf[NUMBER_OF_SRF];

//IR (MLX90614)
struct MLX_INFO {
	int16_t is;
	int16_t th;
};


//typedef struct TEMP_ROB TEMP_ROB_t;
#define NUMBER_OF_MLX 2

extern struct MLX_INFO mlx90614[NUMBER_OF_MLX];

/////////////////////////////////////////////////////
extern void init_srf10(void);

extern uint8_t srf_writeByte(uint8_t adr, uint8_t reg, uint8_t val);

extern uint8_t getIR(void);

extern uint8_t getSRF(void);
