/* 
 * ****************************************************************************
 * RP6 ROBOT SYSTEM - RP6 CONTROL M32 Examples
 * ****************************************************************************
 * Example: I2C Master 10 - Lib 2
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * Header file for new library.
 *
 * ****************************************************************************
 */

#ifndef RP6CONTROL_I2CMASTERLIB_H
#define RP6CONTROL_I2CMASTERLIB_H


/*****************************************************************************/
// Includes:

#include "RP6ControlLib.h"

// define the RP6 Base address here:
#define I2C_RP6_BASE_ADR 10

//other I2C adresses:
#define I2C_3DBS_ADR 0x70
#define I2C_MLX90614_L 0x50<<1
#define I2C_MLX90614_R 0x5A<<1
#define I2C_SRF02 0xE0

/*****************************************************************************/
// These are the same command definitions as you can find them in the 
// I2C Bus Slave Example program for RP6Base:

#define I2C_REG_STATUS1 		 0
#define I2C_REG_STATUS2 		 1
#define I2C_REG_MOTION_STATUS 	 2
#define I2C_REG_POWER_LEFT 		 3
#define I2C_REG_POWER_RIGHT 	 4
#define I2C_REG_SPEED_LEFT 		 5
#define I2C_REG_DES_SPEED_LEFT 	 7
#define I2C_REG_DES_SPEED_RIGHT  8
#define I2C_REG_DIST_LEFT_L 	 9
#define I2C_REG_DIST_LEFT_H 	 10
#define I2C_REG_DIST_RIGHT_L     11
#define I2C_REG_DIST_RIGHT_H 	 12
#define I2C_REG_ADC_LSL_L 		 13
#define I2C_REG_ADC_LSL_H 		 14
#define I2C_REG_ADC_LSR_L 		 15
#define I2C_REG_ADC_LSR_H 		 16
#define I2C_REG_ADC_MOTOR_CURL_L 17
#define I2C_REG_ADC_MOTOR_CURL_H 18
#define I2C_REG_ADC_MOTOR_CURR_L 19
#define I2C_REG_ADC_MOTOR_CURR_H 20
#define I2C_REG_ADC_UBAT_L 		 21
#define I2C_REG_ADC_UBAT_H 		 22
#define I2C_REG_ADC_ADC0_L 		 23
#define I2C_REG_ADC_ADC0_H 		 24
#define I2C_REG_ADC_ADC1_L 		 25
#define I2C_REG_ADC_ADC1_H 		 26
#define I2C_REG_RC5_ADR	 		 27
#define I2C_REG_RC5_DATA	 	 28
#define I2C_REG_LEDS	 		 29

//Commands od other I2C devices:
#define I2C_REG_3DBS	 0x02
#define I2C_REG_MLX90614 0x07
#define I2C_REG_SRF02_MEASERUEMENT 0x52 //Zeit in Mikrosekunden

#define CMD_POWER_OFF 		0
#define CMD_POWER_ON 		1
#define CMD_CONFIG 			2
#define CMD_SETLEDS 		3
#define CMD_STOP   			4
#define CMD_MOVE_AT_SPEED   5
#define CMD_CHANGE_DIR	    6
#define CMD_MOVE 			7
#define CMD_ROTATE 			8
#define CMD_SET_ACS_POWER	9 
#define CMD_SEND_RC5		10 
#define CMD_SET_WDT			11
#define CMD_SET_WDT_RQ		12
#define CMD_SET_WDT_RESET 13

/*****************************************************************************/

#define getMotorRight() (OCR1A)
#define getMotorLeft() (OCR1B)

//Encoders:
extern uint8_t mleft_speed;
extern uint8_t mright_speed;

// Distance
extern uint16_t mleft_dist;
extern uint16_t mright_dist;

// Desired Speed:
extern uint8_t mleft_des_speed;
extern uint8_t mright_des_speed;

// Power
extern uint8_t mleft_power;
extern uint8_t mright_power;

#define getLeftSpeed() (mleft_speed)
#define getRightSpeed() (mright_speed)
#define getLeftDistance() (mleft_dist)
#define getRightDistance() (mright_dist)

#define getDesSpeedLeft() (mleft_des_speed)
#define getDesSpeedRight() (mright_des_speed)

//Direction:

#define FWD 0
#define BWD 1
#define LEFT 2
#define RIGHT 3

#define getDirection() (drive_status.direction)


#define INT0_STATUS_CHECK 0



/*****************************************************************************/
// 
void rotate(uint8_t desired_speed, uint8_t dir, uint16_t angle, uint8_t blocking);
void move(uint8_t desired_speed, uint8_t dir, uint16_t distance, uint8_t blocking);
void moveAtSpeed(uint8_t desired_speed_left, uint8_t desired_speed_right);
void changeDirection(uint8_t dir);
void stop(void);

void baseWatchdog_reset(void);

void getADCs(void);
void getDists(void);
void get3DBS(void);
void getIR(void);

#define isMovementComplete() (drive_status.movementComplete)
	
extern uint8_t bumper_left;
extern uint8_t bumper_right;

extern uint8_t obstacle_left;
extern uint8_t obstacle_right;
#define isObstacleLeft() (obstacle_left)
#define isObstacleRight() (obstacle_right)

extern uint16_t adcBat;
extern uint16_t adcMotorCurrentLeft;
extern uint16_t adcMotorCurrentRight;
extern uint16_t adcLSL;
extern uint16_t adcLSR;
extern uint16_t adc0;
extern uint16_t adc1;

//Accelerometer:
extern int16_t threedbs_x;
extern int16_t threedbs_y;
extern int16_t threedbs_z;

//IR (MLX90614)
extern int16_t mlx90614_l;
extern int16_t mlx90614_r;

//Ultraschall SRF02
extern uint16_t srf_02;
	//==> State Machine
	uint8_t srf_02_state = 0;
	//Timer
	extern volatile int8_t timer_sm_srf;
	extern const int8_t TIMER_SM_SRF;

#define TOGGLEBIT 32

typedef union {
	uint16_t data;
	struct {
		uint8_t key_code:6;
		uint8_t device:5;
		uint8_t toggle_bit:1;
		uint8_t reserved:3;
	};
} RC5data_t;


void IRCOMM_sendRC5(uint8_t adr, uint8_t data);

void IRCOMM_setRC5DataReadyHandler(void (*rc5Handler)(RC5data_t));
void ACS_setStateChangedHandler(void (*acsHandler)(void));
void MOTIONCONTROL_setStateChangedHandler(void (*motionControlHandler)(void));
void BUMPERS_setStateChangedHandler(void (*bumperHandler)(void));
void WDT_setRequestHandler(void (*requestHandler)(void));

void getAllSensors(void);
void getLightSensors(void);
void getSRF(void);

#endif

