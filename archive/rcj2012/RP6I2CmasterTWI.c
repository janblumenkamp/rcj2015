/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/         >>> BASE CONTROLLER
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6I2CmasterTWI.c
 * Version: 1.0
 * Target: RP6 Base & Processor Expansion - ATMEGA32 @8.00 or 16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * This is the I2C Bus Master Library. 
 *
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */
 
/*****************************************************************************/
// Includes:

#include "RP6I2CmasterTWI.h"

/*
 * This function initializes the TWI interface! You need
 * to call this first before you use the TWI interface!
 * You should better use the macro I2CTWI_initMaster without __ at the
 * beginning. There you can specify the SCL frequency in kHz!
 * Example: 
 * I2CTWI_initMaster(100); // I2C Master mode with 100kHz SCL frequency
 *						   // This calculates TWBR value automatically.
 *
 * __I2CTWI_initMaster(32); // I2C Master mode also with 100kHz SCL frequency
 * 							// but directly calculated with the formula in the
 *							// MEGA32 datasheet.
 */
void __I2CTWI_initMaster(uint8_t twi_bitrate)
{
	cli();
	TWBR = twi_bitrate;
	TWSR = 0x00;      // DO NOT USE PRESCALER! Otherwise you need to mask the
	TWDR = 0xFF;      // TWSR Prescaler bits everywhere TWSR is read!  	      
	TWCR = (1<<TWEN);
	TWI_statusReg.lastTransOK = 1;
    sei();                      
}

/*****************************************************************************/
// TWI Event handlers
// These functions are used to receive Data or react on errors.

void I2CTWI_requestedDataReady_DUMMY(uint8_t requestID){}
static void (*I2CTWI_requestedDataReadyHandler)(uint8_t) = I2CTWI_requestedDataReady_DUMMY;
void I2CTWI_setRequestedDataReadyHandler(void (*requestedDataReadyHandler)(uint8_t))
{
	I2CTWI_requestedDataReadyHandler = requestedDataReadyHandler;
}

void I2CTWI_transmissionError_DUMMY(uint8_t requestID){}
static void (*I2CTWI_transmissionErrorHandler)(uint8_t) = I2CTWI_transmissionError_DUMMY;
void I2CTWI_setTransmissionErrorHandler(void (*transmissionErrorHandler)(uint8_t))
{
	I2CTWI_transmissionErrorHandler = transmissionErrorHandler;
}

/*****************************************************************************/
// Delay

/**
 * A small delay that is required between some transfers. Without this delay
 * the transmission may fail.
 */
void I2CTWI_delay(void)
{
	volatile uint8_t dly = 150;
	while(dly--);
}

/*****************************************************************************/
// Control task

static uint8_t I2CTWI_buf[I2CTWI_BUFFER_SIZE];
static uint8_t I2CTWI_recbuf[I2CTWI_BUFFER_REC_SIZE];
static uint8_t TWI_msgSize;
volatile union TWI_statusReg TWI_statusReg = {0}; 
uint8_t TWI_TWSR_state = 0;

uint8_t I2CTWI_request_adr = 0;
uint8_t I2CTWI_request_reg = 0;
uint8_t I2CTWI_request_size = 0;
int16_t I2CTWI_requestID = 0;
uint8_t TWI_operation = I2CTWI_NO_OPERATION;


/**
 * You have to call this functions frequently out of the
 * main loop. It calls the event handlers above automatically if data has been received.
 */
void task_I2CTWI(void)
{
	if (!I2CTWI_isBusy()) {
		if (TWI_statusReg.lastTransOK) {
			if(TWI_operation) {
				no_rep_start = 1;
				if(TWI_operation == I2CTWI_SEND_REGISTER) {
					I2CTWI_delay();
					TWI_msgSize = 2;
					I2CTWI_buf[0] = I2CTWI_request_adr;
					I2CTWI_buf[1] = I2CTWI_request_reg;
					TWI_statusReg.all = 0;
					TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
					TWI_operation = I2CTWI_REQUEST_BYTES;
				}
				else if (TWI_operation == I2CTWI_REQUEST_BYTES) {
					I2CTWI_delay();
					TWI_msgSize = I2CTWI_request_size + 1;
					I2CTWI_request_adr = I2CTWI_request_adr | TWI_READ;
					I2CTWI_buf[0]  = I2CTWI_request_adr | TWI_READ; 
					TWI_statusReg.all = 0;
					TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);	
					TWI_operation = I2CTWI_READ_BYTES_FROM_BUFFER;
				}
				else if (TWI_operation == I2CTWI_READ_BYTES_FROM_BUFFER) { 
					TWI_operation = I2CTWI_NO_OPERATION;
					if(I2CTWI_requestID!=-1)
						I2CTWI_requestedDataReadyHandler(I2CTWI_requestID);
				}
			}
		}
		else {
			uint8_t errState = I2CTWI_getState();
			if(errState != 0) {
				TWI_operation = I2CTWI_NO_OPERATION;
				TWI_statusReg.lastTransOK = 1;
				I2CTWI_request_adr = 0;
				I2CTWI_requestID = 0;
				I2CTWI_request_size = 0;
				I2CTWI_transmissionErrorHandler(errState);
			}
		}
	}
}


/*****************************************************************************/
// Request functions - you need to use requestedDataReadyHandler to receive 
// the requested data...

/**
 * Requests a number of Bytes from the target device. You need to set a requestID 
 * to be able to identify the request after the data has been received and the 
 * "requestedDataReady" event handler is called.
 */
void I2CTWI_requestDataFromDevice(uint8_t targetAdr, uint8_t requestID, uint8_t numberOfBytes)
{	
	no_rep_start = 1;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	TWI_operation = I2CTWI_REQUEST_BYTES;
	I2CTWI_request_adr = targetAdr;
	I2CTWI_requestID = (int16_t)requestID;
	I2CTWI_request_size = numberOfBytes;
}

/**
 * Same as requestDataFromDevice, but this function first sets the register
 * that has to be read and transmits the register number before! 
 * This is neccessary for the reaction on interrupt requests of slave devices as this
 * function is non-blocking and thus very well suited for calls directly from ISRs. 
 */
void I2CTWI_requestRegisterFromDevice(uint8_t targetAdr, uint8_t requestID, uint8_t reg, uint8_t numberOfBytes)
{	
	no_rep_start = 1;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	TWI_operation = I2CTWI_SEND_REGISTER;
	I2CTWI_requestID = (int16_t)requestID;
	I2CTWI_request_adr = targetAdr;
	I2CTWI_request_reg = reg;
	I2CTWI_request_size = numberOfBytes;
}

/**
 * This function can be used in the requestedDataReady Handler to get the
 * received data from the TWI Buffer. You need to provide a pointer to a 
 * buffer which is large enough to hold all received data. 
 * You can specify the number of bytes you want to read out of the buffer
 * with the msgSize parameter.
 * It does not make sense to use this function anywhere else as you 
 * can not guarantee what is in the reception buffer...
 */
void I2CTWI_getReceivedData(uint8_t *msg, uint8_t msgSize)
{
	no_rep_start = 1;
	while(I2CTWI_isBusy());
	uint8_t i = 0;
	if(TWI_statusReg.lastTransOK)
		for(; i < msgSize; i++)
			msg[i] = I2CTWI_recbuf[i+1];
}

/**
 * This function returns the last TWI State / Error State. It waits until
 * TWI Module has completed last operation! 
 */
uint8_t I2CTWI_getState(void)
{
	no_rep_start = 1;
	while(I2CTWI_isBusy()); // Wait until TWI has completed the transmission.
	return (TWI_TWSR_state); // Return error state.
}

/*****************************************************************************/
// Read functions:

/**
 * Reads "numberOfBytes" from "targetAdr" Register "reg" into "messageBuffer". 
 * If the slave device supports auto increment, then it reads all subsequent registers of course! 
 */
void I2CTWI_readRegisters(uint8_t targetAdr, uint8_t reg, uint8_t * messageBuffer, uint8_t numberOfBytes)
{
	no_rep_start = 1;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	TWI_operation = I2CTWI_SEND_REGISTER;
	I2CTWI_request_adr = targetAdr;
	I2CTWI_requestID = -1;
	I2CTWI_request_reg = reg;
	I2CTWI_request_size = numberOfBytes;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	if (TWI_statusReg.lastTransOK) 
		I2CTWI_getReceivedData(&messageBuffer[0], numberOfBytes+1);
}

/**
 * Same as readRegisters, but you need to make sure which register to read yourself - if there
 * are any registers at all in your slave device.  
 * 
 */
void I2CTWI_readBytes(uint8_t targetAdr, uint8_t * messageBuffer, uint8_t numberOfBytes)
{
	no_rep_start = 1;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_operation = I2CTWI_REQUEST_BYTES;
	I2CTWI_request_adr = targetAdr;
	I2CTWI_requestID = -1;
	I2CTWI_request_size = numberOfBytes;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	if (TWI_statusReg.lastTransOK) 
		I2CTWI_getReceivedData(&messageBuffer[0], numberOfBytes+1);
}

/**
 * Reads a single byte from the slave device. 
 */
uint8_t I2CTWI_readByte(uint8_t targetAdr)
{
	no_rep_start = 1;
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_operation = I2CTWI_REQUEST_BYTES;
	I2CTWI_request_adr = targetAdr;
	I2CTWI_requestID = -1;
	I2CTWI_request_size = 1;
	while(TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	if (TWI_statusReg.lastTransOK)
		return I2CTWI_recbuf[1];
	else
		return 0;
}
/*****************************************************************************/
// Transmission functions

/**
 * Transmits a single byte to a slave device. It waits until the last 
 * TWI operation is finished (it blocks the normal program flow!) but
 * it does NOT wait until this transmission is finished! 
 * This allows you to perform other things while the transmission is 
 * in progress! 
 */
void I2CTWI_transmitByte(uint8_t targetAdr, uint8_t data)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_msgSize = 2;
	I2CTWI_buf[0] = targetAdr;
	I2CTWI_buf[1] = data;
	TWI_statusReg.all = 0;
	no_rep_start = 1;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}

//With Sending Repeated Start
void I2CTWI_transmitByte_RepeatedStart(uint8_t targetAdr, uint8_t data)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_msgSize = 2;
	I2CTWI_buf[0] = targetAdr;
	I2CTWI_buf[1] = data;
	TWI_statusReg.all = 0;
	no_rep_start = 0;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}

/**
 * This is just the same as transmitByte, but you can pass 2 Bytes to
 * this function which are then transferred. 
 */
void I2CTWI_transmit2Bytes(uint8_t targetAdr, uint8_t data1, uint8_t data2)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_msgSize = 3;
	I2CTWI_buf[0] = targetAdr;
	I2CTWI_buf[1] = data1;
	I2CTWI_buf[2] = data2;
	TWI_statusReg.all = 0;
	no_rep_start = 1;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}

/**
 * Transmits 3 Bytes to the slave.
 */
void I2CTWI_transmit3Bytes(uint8_t targetAdr, uint8_t data1, uint8_t data2, uint8_t data3)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_msgSize = 4;
	I2CTWI_buf[0] = targetAdr;
	I2CTWI_buf[1] = data1;
	I2CTWI_buf[2] = data2;
	I2CTWI_buf[3] = data3;
	TWI_statusReg.all = 0;
	no_rep_start = 1;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}

/**
 * Transmits 4 Bytes to the slave.
 */
void I2CTWI_transmit4Bytes(uint8_t targetAdr, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	TWI_msgSize = 5;
	I2CTWI_buf[0] = targetAdr;
	I2CTWI_buf[1] = data1;
	I2CTWI_buf[2] = data2;
	I2CTWI_buf[3] = data3;
	I2CTWI_buf[4] = data4;
	TWI_statusReg.all = 0;
	no_rep_start = 1;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}


/**
 * Transmits "numberOfBytes" Bytes to the Slave device. The Bytes need to be
 * in the Buffer "msg". Otherwise it is just the same as the other transmit functions.
 */
void I2CTWI_transmitBytes(uint8_t targetAdr, uint8_t *msg, uint8_t numberOfBytes)
{
	while(I2CTWI_isBusy() || TWI_operation != I2CTWI_NO_OPERATION) task_I2CTWI();
	I2CTWI_delay();
	numberOfBytes++; 
	TWI_msgSize = numberOfBytes;
	I2CTWI_buf[0]  = targetAdr;
	uint8_t i = 0;
	for(; i < numberOfBytes; i++)
		I2CTWI_buf[i+1] = msg[i];
	TWI_statusReg.all = 0;
	no_rep_start = 1;
	TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO);
}


/*****************************************************************************/
// ISR:

/*
 * TWI ISR
 */
uint8_t no_rep_start;  //Wird ein Repeated start vom Slave benötigt?

ISR (TWI_vect)
{
	static uint8_t TWI_bufPos = 0;
	switch (TWSR)
	{
		case TWI_START:           // START has been transmitted  
		case TWI_REP_START:         // Repeated START has been transmitted
		  no_rep_start = 1;
			TWI_bufPos = 0;          // Set buffer pointer to the TWI Address location
		case TWI_MTX_ADR_ACK:       // SLA+W has been transmitted and ACK received
		case TWI_MTX_DATA_ACK:      // Data byte has been transmitted and ACK received
		 if (TWI_bufPos < TWI_msgSize) {
			TWDR = I2CTWI_buf[TWI_bufPos++];  
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
				   (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           //
				   (0<<TWWC);                                 //  
		  } else {                   // Send STOP after last byte
				TWI_statusReg.lastTransOK = 1;                 // Set status bits to completed successfully. 
				if(no_rep_start == 1){
					TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				  	 (0<<TWIE)|(0<<TWINT)|                      // Disable TWI Interrupt and clear the flag
				  	 (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
				  	 (0<<TWWC);                                //
				}
				else{
					TWCR = (1<<TWEN)|                                 // TWI Interface enabled
					   (0<<TWIE)|(0<<TWINT)|                      // Disable TWI Interrupt and clear the flag
					   (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|           // Initiate a REPEATED START condition.
					   (0<<TWWC);                                //
		  	}
			}
		  break;
		case TWI_MRX_DATA_ACK:     // Data byte has been received and ACK transmitted
		  I2CTWI_recbuf[TWI_bufPos++] = TWDR;
		case TWI_MRX_ADR_ACK:      // SLA+R has been transmitted and ACK received
		  if (TWI_bufPos < (TWI_msgSize-1) ) {                 // Detect the last byte to NACK it.
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
				   (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after reception
				   (0<<TWWC);                                 //  
		  } else {                 // Send NACK after next reception
			TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				   (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
				   (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send NACK after reception
				   (0<<TWWC);                                 // 
		  }    
		  break; 
		case TWI_MRX_DATA_NACK:     // Data byte has been received and NACK transmitted
		  I2CTWI_recbuf[TWI_bufPos] = TWDR;
		  TWI_statusReg.lastTransOK = 1;                 	// Set status bits to completed successfully. 		 
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				 (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
				 (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
				 (0<<TWWC);                                 //
		  break;      
		case TWI_ARB_LOST:         // Arbitration lost
		  TWI_TWSR_state = TWSR;  							// Store TWSR 	
		  TWCR = (1<<TWEN)|                                 // TWI Interface enabled
				 (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag
				 (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|           // Initiate a (RE)START condition.
				 (0<<TWWC);                               //
		  break;
		default:     
		  TWI_TWSR_state = TWSR;                     		// Store TWSR 					
		  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
				 (0<<TWIE)|(1<<TWINT)|                      // Disable Interupt
				 (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // No Signal requests
				 (0<<TWWC);  
		break;
	}
}

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
