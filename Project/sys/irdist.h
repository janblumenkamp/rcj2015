////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////i2cdev.h///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//    Siehe i2cdev.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef IRDIST_H
#define IRDIST_H

#include <avr/io.h>
#include <avr/pgmspace.h>     // Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define IRDIST_I2CDEV_NUM 10

#define IRDIST_MAX 500
#define IRDIST_OFFSET 44 //Make the vorder of the robot 0mm

//Not used: 0x40, 0x80, 0xD0, 0xF0

#define IRDIST_I2CADR_B_B 0x20
#define IRDIST_I2CADR_L_B 0xC0
#define IRDIST_I2CADR_B_L 0xB0
#define IRDIST_I2CADR_F_L 0xA0
#define IRDIST_I2CADR_L_F 0x90
#define IRDIST_I2CADR_F_F 0x70
#define IRDIST_I2CADR_R_F 0x50
#define IRDIST_I2CADR_F_R 0x30
#define IRDIST_I2CADR_B_R 0x60
#define IRDIST_I2CADR_R_B 0x10

/////////////////////////////////////////////////////
//Prototypes

extern void irDist_setSensorStandby(uint8_t adr, uint8_t state);

extern uint8_t irDist_get(void);

#endif // IRDIST_H
