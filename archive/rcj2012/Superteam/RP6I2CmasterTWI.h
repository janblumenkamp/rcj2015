/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/         >>> BASE CONTROLLER
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6I2CmasterTWI.h
 * Version: 1.0
 * Target: RP6 Base & Processor Expansion - ATMEGA32 @8.00 or 16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * For functional description of the TWI Master lib s. RP6I2CmasterTWI.c!
 *
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

#ifdef RP6I2C_SLAVE_TWI_H
	#error YOU CAN NOT INCLUDE TWI I2C MASTER AND SLAVE ROUTINES AT THE SAME TIME!
#else
#ifndef RP6I2C_MASTER_TWI_H
#define RP6I2C_MASTER_TWI_H

/*****************************************************************************/
// Includes:

#include <stdint.h>					
#include <avr/interrupt.h>
#include <avr/io.h>

/*****************************************************************************/

union TWI_statusReg {
    uint8_t all;
    struct {
        volatile unsigned lastTransOK:1;
        unsigned unusedBits:7;
    };
};

extern volatile union TWI_statusReg TWI_statusReg;

extern uint8_t i2c_req_adr;
extern uint8_t TWI_operation;

extern uint8_t no_rep_start;

#define I2CTWI_isBusy() ((TWCR & (1<<TWIE))) 


// Sample TWI transmission states, used in the main application.
#define I2CTWI_NO_OPERATION			       0
#define I2CTWI_SEND_REGISTER           1
#define I2CTWI_REQUEST_BYTES           2
#define I2CTWI_READ_BYTES_FROM_BUFFER  3

#define I2CTWI_BUFFER_SIZE 16   // Set this to the largest message size that will be sent including address byte.
#define I2CTWI_BUFFER_REC_SIZE 48 // Set this to the largest message size that will be received including address byte.

#define I2CTWI_initMaster(__FREQ__) __I2CTWI_initMaster((uint8_t)((F_CPU/(2000UL*__FREQ__))-8))
void __I2CTWI_initMaster(uint8_t twi_bitrate);

void I2CTWI_setRequestedDataReadyHandler(void (*requestedDataReadyHandler)(uint8_t));
void I2CTWI_setTransmissionErrorHandler(void (*transmissionErrorHandler)(uint8_t));

void task_I2CTWI(void);
uint8_t I2CTWI_getState(void);

void I2CTWI_requestDataFromDevice(uint8_t requestAdr, uint8_t requestID, uint8_t numberOfBytes);
void I2CTWI_requestRegisterFromDevice(uint8_t targetAdr, uint8_t requestID, uint8_t reg, uint8_t numberOfBytes);
void I2CTWI_getReceivedData(uint8_t *msg, uint8_t msgSize);

void I2CTWI_readBytes(uint8_t targetAdr, uint8_t * messageBuffer, uint8_t numberOfBytes);
uint8_t I2CTWI_readByte(uint8_t targetAdr);
void I2CTWI_readRegisters(uint8_t targetAdr, uint8_t reg, uint8_t * messageBuffer, uint8_t numberOfBytes);

void I2CTWI_transmitByte(uint8_t adr, uint8_t data);
void I2CTWI_transmitByte_RepeatedStart(uint8_t adr, uint8_t data);
void I2CTWI_transmit2Bytes(uint8_t adr, uint8_t data1, uint8_t data2);
void I2CTWI_transmit3Bytes(uint8_t targetAdr, uint8_t data1, uint8_t data2, uint8_t data3);
void I2CTWI_transmit4Bytes(uint8_t targetAdr, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);
void I2CTWI_transmitBytes(uint8_t targetAdr, uint8_t *msg, uint8_t numberOfBytes);

#define TWI_READ  1
#define TWI_GEN_CALL 0 

/*****************************************************************************/
// TWI Status Codes:
// The TWI status codes were taken from ATMEL AN315!

// General TWI Master staus codes
#define TWI_START                  0x08  // START has been transmitted
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been transmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been transmitted and NACK received
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been transmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been transmitted and NACK received

// TWI Master Receiver staus codes
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been transmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been transmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK transmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK transmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = “0”
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition


#endif
#endif

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

/*****************************************************************************/
// EOF
