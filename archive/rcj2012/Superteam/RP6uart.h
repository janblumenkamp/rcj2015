/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/         		 >>> COMMON
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6uart.h
 * Version: 1.1
 * Target: RP6 Base & Processor Expansion - ATMEGA32 @8.00 or 16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * The RP6 uart function Library header file. Detailled description
 * of each function can be found in the RP6uart.c file!
 *
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

#ifndef RP6UART_H
#define RP6UART_H

/*****************************************************************************/
// Includes:

#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <stdlib.h>			// C standard functions (e.g. itoa...)
#include <string.h>
#include <avr/io.h>			// I/O Port definitions
#include <avr/interrupt.h>	// Interrupt macros (e.g. cli(), sei())

/*****************************************************************************/
// UART


// TX:

void writeChar(char ch);
void writeStringLength(char *data, uint8_t length, uint8_t offset);
void writeString(char *data);
void writeNStringP(const char *pstring);
#define writeString_P(__pstr) writeNStringP((PSTR(__pstr)))

#define HEX 16
#define DEC 10
#define OCT 8
#define BIN 2
void writeInteger(int16_t number, uint8_t base);
void writeIntegerLength(int16_t number, uint8_t base, uint8_t length);


// RX:
extern volatile uint8_t uart_status;

#define UART_RECEIVE_BUFFER_SIZE 32 // Default buffer size is 32!
#define UART_BUFFER_OK 0
#define UART_BUFFER_OVERFLOW 1

char readChar(void);
uint8_t readChars(char *buf, uint8_t numberOfChars);
uint8_t getBufferLength(void);
void clearReceptionBuffer(void);

#endif

/******************************************************************************
 * Additional info
 * ****************************************************************************
 * Changelog:
 * 
 *  ---> changes are documented in the file "RP6uart.c"
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
