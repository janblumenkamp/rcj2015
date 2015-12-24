/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/             >>> RP6 CONTROL
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6ControlLib.h
 * Version: 1.1
 * Target: RP6 CONTROL - ATMEGA32 @16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * The RP6 Robot Base function Library header file. Detailled description
 * of each function can be found in the RP6RobotBaseLib.c file!
 * 
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

#ifndef RP6CONTROLLIB_H
#define RP6CONTROLLIB_H

/*****************************************************************************/
// Includes:

#include "RP6Control.h"		// General RP6 Robot Base definitions
#include "RP6uart.h"		// RP6 UART function lib
#include "RP6Config.h"      // Configuration for RP6BaseLibrary - Constants for 
							// speed and distance calculation etc.
							
#include <avr/sleep.h>		// Power saving functions
#include <util/delay.h>		// Some delay loops

/*****************************************************************************/

// You can use this define to identify the RP6Library version:
#define RP6LIB_VERSION 13
// (this does not mean file version - each file has its own
// sub-version but the complete library has an overall version)

/*****************************************************************************/
// Init:

extern void initRP6Control(void);

/*****************************************************************************/
// ADC

uint16_t readADC(uint8_t channel);


/*****************************************************************************/
// External Interrupts:

void INT0_setEventHandler(void (*i0eventHandler)(void));
void INT1_setEventHandler(void (*i1eventHandler)(void));
void INT2_setEventHandler(void (*i2eventHandler)(void));

/*****************************************************************************/
// SPI:

void writeSPI(uint8_t data);
uint8_t readSPI(void);
uint16_t readWordSPI(void);
void writeWordSPI(uint16_t data);
void writeBufferSPI(uint8_t *buffer, uint8_t length);
void readBufferSPI(uint8_t *buffer, uint8_t length);

/*****************************************************************************/
// SPI EEPROM:

#define SPI_EEPROM_READ		0x03	// Read byte(s)
#define SPI_EEPROM_WRITE	0x02	// Write byte(s)
#define SPI_EEPROM_WREN		0x06	// Write Enable
#define SPI_EEPROM_WRDI		0x04	// Write Disable
#define SPI_EEPROM_RDSR		0x05	// Read Status Register
#define SPI_EEPROM_WRSR		0x01	// Write Status Register

#define SPI_EEPROM_STAT_WIP	1	 // Write in Progress Bit
#define SPI_EEPROM_STAT_WEL	2	 // Write Enable Latch Bit
#define SPI_EEPROM_STAT_BP0	4	 // Block Protect 0 Bit
#define SPI_EEPROM_STAT_BP1	8	 // Block Protect 1 Bit
#define SPI_EEPROM_STAT_SRWD 128 // Status Register Write Protect

#define SPI_EEPROM_PAGESIZE 64

uint8_t SPI_EEPROM_readByte(uint16_t memAddr);
void SPI_EEPROM_writeByte(uint16_t memAddr, uint8_t data);
void SPI_EEPROM_enableWrite(void);
void SPI_EEPROM_disableWrite(void);
uint8_t SPI_EEPROM_getStatus(void);

void SPI_EEPROM_writeBytes(uint16_t startAddr, uint8_t *buffer, uint8_t length);
void SPI_EEPROM_readBytes(uint16_t startAddr, uint8_t *buffer, uint8_t length);

/*****************************************************************************/
// Status LEDs

// A shadow register that simplifies usage of status LEDs:
typedef union {
 	uint8_t byte;
	struct {
		uint8_t LEDS:4;
		uint8_t LCDD:4;
	};
	struct {
		uint8_t LED4:1;
		uint8_t LED3:1;
		uint8_t LED2:1;
		uint8_t LED1:1;
		uint8_t D0:1;
		uint8_t D1:1;
		uint8_t D2:1;
		uint8_t D3:1;
	};
} externalPort_t;
extern externalPort_t externalPort;

void outputExt(void);
void setLEDs(uint8_t leds);

/*****************************************************************************/
// LCD:

void setLCDD(uint8_t lcdd);
void write4BitLCDData(uint8_t data);
void writeLCDCommand(uint8_t cmd);
void initLCD(void);

void clearLCD(void);
void clearPosLCD(uint8_t line, uint8_t pos, uint8_t length);
void writeCharLCD(uint8_t ch);
#define writeStringLCD_P(__pstr) writeNStringLCD_P((PSTR(__pstr)))
void writeStringLengthLCD(char *string, uint8_t length, uint8_t offset);
void writeStringLCD(char *string);
void writeNStringLCD_P(const char *pstring);

void _showScreenLCD_P(const char *line1, const char *line2);

#define showScreenLCD(__line1,__line2); ({_showScreenLCD_P((PSTR(__line1)),(PSTR(__line2)));})

#ifndef HEX
	#define HEX 16
#endif
#ifndef DEC 
	#define DEC 10
#endif
#ifndef OCT
	#define OCT 8
#endif
#ifndef BIN
	#define BIN 2
#endif
void writeIntegerLCD(int16_t number, uint8_t base);
void writeIntegerLengthLCD(int16_t number, uint8_t base, uint8_t length);

void setCursorPosLCD(uint8_t line, uint8_t pos);


/*****************************************************************************/
// Keys:

uint8_t getPressedKeyNumber(void);
uint8_t checkPressedKeyEvent(void);
uint8_t checkReleasedKeyEvent(void);

/*****************************************************************************/
// Microphone:

void dischargePeakDetector(void);
uint16_t getMicrophonePeak(void);

/*****************************************************************************/
// Stopwatches:
/*****************************************************************************/
// Stopwatches:

#define STOPWATCH1 1
#define STOPWATCH2 2
#define STOPWATCH3 4
#define STOPWATCH4 8
#define STOPWATCH5 16
#define STOPWATCH6 32
#define STOPWATCH7 64
#define STOPWATCH8 128

typedef struct {
	uint8_t watches;
	volatile uint16_t watch1;
	volatile uint16_t watch2;
	volatile uint16_t watch3;
	volatile uint16_t watch4;
	volatile uint16_t watch5;
	volatile uint16_t watch6;
	volatile uint16_t watch7;
	volatile uint16_t watch8;
} stopwatches_t;
extern volatile stopwatches_t stopwatches;

#define stopStopwatch1() stopwatches.watches &= ~STOPWATCH1
#define stopStopwatch2() stopwatches.watches &= ~STOPWATCH2
#define stopStopwatch3() stopwatches.watches &= ~STOPWATCH3
#define stopStopwatch4() stopwatches.watches &= ~STOPWATCH4
#define stopStopwatch5() stopwatches.watches &= ~STOPWATCH5
#define stopStopwatch6() stopwatches.watches &= ~STOPWATCH6
#define stopStopwatch7() stopwatches.watches &= ~STOPWATCH7
#define stopStopwatch8() stopwatches.watches &= ~STOPWATCH8

#define startStopwatch1() stopwatches.watches |= STOPWATCH1
#define startStopwatch2() stopwatches.watches |= STOPWATCH2
#define startStopwatch3() stopwatches.watches |= STOPWATCH3
#define startStopwatch4() stopwatches.watches |= STOPWATCH4
#define startStopwatch5() stopwatches.watches |= STOPWATCH5
#define startStopwatch6() stopwatches.watches |= STOPWATCH6
#define startStopwatch7() stopwatches.watches |= STOPWATCH7
#define startStopwatch8() stopwatches.watches |= STOPWATCH8

#define isStopwatch1Running() (stopwatches.watches & STOPWATCH1)
#define isStopwatch2Running() (stopwatches.watches & STOPWATCH2)
#define isStopwatch3Running() (stopwatches.watches & STOPWATCH3)
#define isStopwatch4Running() (stopwatches.watches & STOPWATCH4)
#define isStopwatch5Running() (stopwatches.watches & STOPWATCH5)
#define isStopwatch6Running() (stopwatches.watches & STOPWATCH6)
#define isStopwatch7Running() (stopwatches.watches & STOPWATCH7)
#define isStopwatch8Running() (stopwatches.watches & STOPWATCH8)

#define getStopwatch1() stopwatches.watch1
#define getStopwatch2() stopwatches.watch2
#define getStopwatch3() stopwatches.watch3
#define getStopwatch4() stopwatches.watch4
#define getStopwatch5() stopwatches.watch5
#define getStopwatch6() stopwatches.watch6
#define getStopwatch7() stopwatches.watch7
#define getStopwatch8() stopwatches.watch8

#define setStopwatch1(__VALUE__) stopwatches.watch1 = __VALUE__
#define setStopwatch2(__VALUE__) stopwatches.watch2 = __VALUE__
#define setStopwatch3(__VALUE__) stopwatches.watch3 = __VALUE__
#define setStopwatch4(__VALUE__) stopwatches.watch4 = __VALUE__
#define setStopwatch5(__VALUE__) stopwatches.watch5 = __VALUE__
#define setStopwatch6(__VALUE__) stopwatches.watch6 = __VALUE__
#define setStopwatch7(__VALUE__) stopwatches.watch7 = __VALUE__
#define setStopwatch8(__VALUE__) stopwatches.watch8 = __VALUE__

/*****************************************************************************/
// Universal timer:

// This timer variable:
volatile uint32_t timer;  
// is incremented each 100µs. It is suited for performing time
// measurements where the resolution of the stopwatches (1ms) is not enough. 

/*****************************************************************************/
// Delays:

void sleep(uint8_t time);
void mSleep(uint16_t time);
void delayCycles(uint16_t dly);
#define delay_us(us)  _delay_loop_2(( ( ( 1*(F_CPU/4000) )*us )/1000 ))

/*****************************************************************************/
// Beeper:

void beep(unsigned char pitch, unsigned int time);
void setBeeperPitch(uint8_t pitch);
#define sound(_pitch_,_time_,_delay_) {beep(_pitch_,_time_);mSleep(_delay_ + _time_);}

#endif

/******************************************************************************
 * Additional info
 * ****************************************************************************
 * Changelog:
 * 
 *  ---> changes are documented in the file "RP6ControlLib.c"
 *
 * ****************************************************************************
 * Bugs, feedback, questions and modifications can be posted on the AREXX Forum
 * on http://www.arexx.com/forum/ !
 * Of course you can also write us an e-mail to: info@arexx.nl
 * AREXX Engineering may publish updates from time to time on AREXX.com!
 * ****************************************************************************
 * - LICENSE -
 * GNU GPL v2 (http://www.gnu.org/licenses/gpl.txt, a local copy can be found
 * on the RP6 CD in the RP6 sorce code folders!)
 * This program is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 * ****************************************************************************
 */

/*****************************************************************************/
// EOF
