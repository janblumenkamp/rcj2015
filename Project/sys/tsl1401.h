////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
////////////////////////////////tsl1401.h///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	siehe tsl1401.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define TSL_A0_CH 13 //ADC Channel of the cam

#define TSL_CK_PORT			PORTA
#define TSL_CK_PIN			PA5

#define TSL_SI_PORT			PORTA
#define TSL_SI_PIN			PA7

////////////Reading out///////////////////////////////////////////
#define TSL_RESOLUTION 4 //Scale Resolution to optimize performance (only evenry nth pixel is read out). Every Pixelreading will be in this new unit, so when you read out the nth Pixel you actually read the cams n*TSL_RESOLUTIONth Pixel.

#define TSL_REG_PIX				5	//Referencepixel for aparture
#define TSL_PREG_EXP_SOLL	500 //The Referencepixels regulating to-value
#define TSL_REG_EXP_FAC		70 //Integrationfactor (I-Regulator of the aparture)

#define TSL_EXPOSURE_MAX	40 //ms
#define TSL_EXPOSURE_MIN	1	//ms
#define TSL_EXPOSURE_MAX_LED	TSL_EXPOSURE_MAX-2 //When the aparture rises above this level, the LED will power up.
#define TSL_EXPOSURE_MIN_LED	10 //Above this aparture the LED will power down again.

#define TSL_RES_CNT_START	TSL_REG_PIX //Offset; first pixel to read out in the normal resolution
#define TSL_CNT_OFFSET_ROB	10 //Above this pixel the robot and the reference ends and the underground begins

enum TASK_ADC_STATES {ADC_CAM_IDLE, ADC_CAM_INIT, MEASURE_1, MEASURE_2, MEASURE_3, MEASURE_4, MEASURE_5_RES, ADC_CAM_DONE };
enum TASK_CAM_STATES { TSL_INIT, TSL_EXPOSE, TSL_WAIT_GET, TSL_GET, TSL_PROCESS };

extern int16_t tslData[128/TSL_RESOLUTION];

extern int16_t tslData_med[(128/TSL_RESOLUTION)-TSL_CNT_OFFSET_ROB];

extern int16_t tsl_exposure;

extern uint8_t tsl_led_brightness;

extern int16_t tsl_res;

extern int8_t tsl_adc_state;

extern uint8_t tsl_state;
//////////////////////////////////////////////

extern int8_t tsl1401(int8_t state);
