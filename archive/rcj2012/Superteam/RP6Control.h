/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/             >>> RP6 CONTROL
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * --------------------------- All rights reserved ----------------------------
 * ****************************************************************************
 * File: RP6Control.h
 * Version: 1.0
 * Target: RP6 CONTROL - ATMEGA32 @16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * The RP6 CONTROL M32 header file with general definitions. If you don't want
 * to include the complete RP6ControlLib because it is too large - then at least
 * include this file! It is already included in the RP6ControlLib, but can be
 * used without it!
 *
 * This file contains helpful definitions that simplify reading the sourcecode.
 * Most important are the default settings for Port and Direction registers!
 * Hint: You should better leave all this as it is, but it is a good idea to
 * read the comments, it will help you to understand C programming for AVR
 * better!
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

#ifndef RP6CONTROL_H
#define RP6CONTROL_H

/*****************************************************************************/
// Useful definition for common routines that are used on several
// different devices (you can switch register settings to match the target!):

#define DEVICE_RP6CONTROL

/*****************************************************************************/
// CPU Clock:

#ifndef F_CPU
#define F_CPU 16000000 //Expansion: 16.00MHz  - DO NOT CHANGE!
#endif

/*****************************************************************************/
// Includes:

#include <stdint.h>			// Standard Integer definitions
#include <avr/io.h>			// I/O Port definitions
#include <avr/interrupt.h>	// Signal and other Interrupt macros

/*****************************************************************************/
// I/O PORT pin definitions
// These definitions simplify reading and understanding the source code.
//
// ATTENTION: Initial value of port and direction registers should not
// be changed, if you do not exactly know what you are doing!
//
// Hints for DDRx and PORTx Registers:
// DDRxy = 0 and PORTxy = 0 ==> Input without internal Pullup
// DDRxy = 0 and PORTxy = 1 ==> Input with internal Pullup
// DDRxy = 1 and PORTxy = 0 ==> Output low
// DDRxy = 1 and PORTxy = 1 ==> Output high
// "=1" indicates that the appropriate bit is set.
//
// Example:
// #define INIT_DDRA 0b00010000
// #define INIT_PRTA 0b00000000
//
// This means that ALL ports on PortA are inputs without internal pullups
// except for PortA4, which is an output (E_INT1 signal in this case) and
// initial value is low.
//
// Binary value explanation:
// 0b00010000     = 16 in decimal system
//   ^      ^
// MSB      LSB      (MSB = Most Significant Bit, LSB = Least Significant Bit)
//
// The program should always call the macro "portInit();" FIRST! You can find
// it a bit below. Correct port initialisation is the most important step
// after a hardware reset!

// ---------------------------------------------------
// PORTA

#define ADC7 		(1 << PINA7) // ADC7 (Input)
#define ADC6		(1 << PINA6) // ADC6 (Input)
#define ADC5 		(1 << PINA5) // ADC5 (Input)
#define ADC4 		(1 << PINA4) // ADC4 (Input)
#define ADC3 		(1 << PINA3) // ADC3 (Input)
#define ADC2 		(1 << PINA2) // ADC2 (Input)
#define KEYPAD 		(1 << PINA1) // ADC1 (Input)
#define MIC 		(1 << PINA0) // ADC0 (Input)

// Initial value of port and direction registers.
#define INIT_DDRA 0b00000000
#define INIT_PRTA 0b00000000

// ---------------------------------------------------
// PORTA A/D Convertor channels

#define ADC_7			7
#define ADC_6 			6
#define ADC_5 			5
#define ADC_4 			4
#define ADC_3 			3
#define ADC_2 			2
#define ADC_KEYPAD 		1
#define ADC_MIC 		0

// ---------------------------------------------------
// PORTB

#define SCK 	 (1 << PINB7) 	// Output
#define MISO 	 (1 << PINB6) 	// Input
#define MOSI 	 (1 << PINB5) 	// Output
#define LCD_EN 	 (1 << PINB4)	// Output
#define LCD_RS 	 (1 << PINB3) 	// Output
#define EINT3 	 (1 << PINB2)   // INT2 (Input - Pullup)
#define MEM_CS2  (1 << PINB1)	// Output
#define MEM_CS   (1 << PINB0)   // Output


// Initial value of port and direction registers.
#define INIT_DDRB 0b10111011
#define INIT_PRTB 0b00000111

// ---------------------------------------------------
// PORTC

#define IO_PC7 	(1 << PINC7)	// I/O
#define IO_PC6 	(1 << PINC6)	// I/O
#define IO_PC5 	(1 << PINC5)	// I/O (Optional JTAG: TDI)
#define IO_PC4 	(1 << PINC4)	// I/O (JTAG: TDO)
#define IO_PC3 	(1 << PINC3)	// I/O (JTAG: TMS)
#define IO_PC2 	(1 << PINC2)	// I/O (JTAG: TCK)
#define SDA 	(1 << PINC1)	// I2C Data (I/O)
#define SCL 	(1 << PINC0)	// I2C Clock (Output as Master, Input as Slave)

// Initial value of port and direction registers.
#define INIT_DDRC 0b00000000
#define INIT_PRTC 0b11111100

// ---------------------------------------------------
// PORTD

#define BUZ 	(1 << PIND7)	// Output
#define IO_PD6 	(1 << PIND6)	// I/O
#define IO_PD5 	(1 << PIND5)	// I/O
#define STR 	(1 << PIND4)	// Output
#define EINT2 	(1 << PIND3)	// INT1 (Input - Pullup)
#define EINT1 	(1 << PIND2)	// INT0 (Input - Pullup)
#define TX 		(1 << PIND1)	// USART TX (Output)
#define RX 		(1 << PIND0)	// USART RX (Input)

// Initial value of port and direction registers.
#define INIT_DDRD 0b10010010
#define INIT_PRTD 0b01100001

/*****************************************************************************/
// I/O Port init macro - always call this first! It is called first from
// initRP6Control() in the RP6ControlLib!
//
// Example:
// int main(void)
// {
// 		portInit();
// 		// ...
//		// your application
//		while(true);
//		return 0;
// }

#define portInit();	\
PORTA = INIT_PRTA;	\
PORTB = INIT_PRTB;	\
PORTC = INIT_PRTC;	\
PORTD = INIT_PRTD;	\
DDRA = INIT_DDRA;	\
DDRB = INIT_DDRB;	\
DDRC = INIT_DDRC;	\
DDRD = INIT_DDRD;

/*****************************************************************************/
// Some additional definitions/macros

// Boolean:
#define true 1
#define false 0
#define TRUE 1
#define FALSE 0

// Assembly and system macros:
#define nop() asm volatile("nop\n\t")
#define sysSleep() asm volatile("sleep\n\t")

/*****************************************************************************/
// Standard Baudrates for RP6:

#define BAUD_LOW		38400UL  // Low speed - 38.4 kBaud
#define UBRR_BAUD_LOW	((F_CPU/(16*BAUD_LOW))-1)

#define BAUD_HIGH		500000UL // High speed - 500 kBaud
#define UBRR_BAUD_HIGH	((F_CPU/(16*BAUD_HIGH))-1)

/******************************************************************************
 * Additional info
 * ****************************************************************************
 * Changelog:
 * - v. 1.0 (initial release) 16.05.2007 by Dominik S. Herwald
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

#endif

/*****************************************************************************/
// EOF
