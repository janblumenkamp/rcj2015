/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/         >>> BASE CONTROLLER
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6Config.h
 * Version: 1.2
 * Target: RP6 Base - ATMEGA32 @8.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 *
 * This file contains the calibration values for Encoder resolution and
 * the DIST_MM macros.
 * You need to do some experiments to get good encoder resolution values!
 *
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

#ifndef RP6CONFIG_H
#define RP6CONFIG_H

/*****************************************************************************/


/**
 * You can change the Encoder resolution setting of 0.25mm to another 
 * value if you measured different resolution for your encoders.
 * 
 */
//#define ENCODER_RESOLUTION 0.23
//#define ENCODER_RESOLUTION 0.235
#define ENCODER_RESOLUTION 0.24
//#define ENCODER_RESOLUTION 0.25


#define DIST_MM(__DST__) (uint16_t)(__DST__/ENCODER_RESOLUTION)
#define DIST_CM(__DST__) (uint16_t)(__DST__/(ENCODER_RESOLUTION/10))
#define DIST_M(__DST__) (uint16_t)(__DST__/(ENCODER_RESOLUTION/1000))


/**
 * Defines a Factor for the rotation error. 
 * As you may have read in the Manual, the rotation is unprecise
 * because of wheel/track slippery. This Factor allows you to
 * make it roughly precise. 
 * The Problem is: You need to use different rotation factor
 * for different surfaces, as the Robot behaves different on
 * different surfaces (e.g. carpet vs. wooden floor).
 *
 * You need to do some experiments with this until you get
 * good results. Let the Robot rotate by 90°, 180° and 360°
 * and change the rotation factor depending on if it works
 * good or not. 
 *
 * If you want to make it really precise, you will have to
 * use a Mouse sensor, electronic compass or gyroscope. 
 */
#define ROTATION_FACTOR 688 


// ------------------
/**
 * This defines the speed measurement interval. By default it is 200 (= 200ms), 
 * so each second the speed value is updated 5 times and you get "encoder segments" 
 * per 1/5 second. This also sets the rate at which the task_motorSpeedControl routine
 * adjusts the PWM values.
 * ATTENTION: SPEED_TIMER_BASE can not be set higher than 255ms by default!
 * You need to change "volatile uint8_t speed_timer;" to uint16_t if you want 
 * longer delays!
 *
 * Speed is calculated like follows: 
 *	speed = mSpeed*0.25mm*5Hz = mSpeed*1.25mm*1/s = mSpeed * 0.125cm/s.
 * when mSpeed is the speed value measured by the encoders and updated with 5Hz
 * and encoder resolution of 0.25mm
 *
 * Example: Measured speed is 20 Encoder Segments --> mSpeed =20
 * Real speed is: 20 * 0.125cm/s = 2.5cm/s
 *
 * BETTER DO NOT CHANGE THE UPDATE INTERVAL IF YOU DO NOT KNOW WHAT YOU ARE DOING!
 *
 */
#define SPEED_TIMER_BASE 200


/*****************************************************************************/
// Power on warning:

/**
 * If you don't need the flashing LED that shows that the Robot is still turned
 * on - outcomment this line and the code will not be compiled anymore
 * (saves some programspace and execution time):
 */
#define POWER_ON_WARNING
#define POWER_ON_WAIT_TIME 12000
#define POWER_ON_SHOW_TIME 4000


/*****************************************************************************/
// ACS Config:

// You can only uncomment ONE of these two defines at a time. 
// use ACS_CONFIG_CUSTOM for your own ACS settings!
#define ACS_CONFIG_DEFAULT
//#define ACS_CONFIG_CUSTOM

// General hints:
// Only values > 0 will work for ALL values!
// Settings with very different values from those that are in here
// by default will maybe not work! You have to do some experiments...


#ifdef ACS_CONFIG_DEFAULT	// ACS General update interval:
	#define ACS_UPDATE_INTERVAL 2  // ms
	
	// ACS Wait for IRCOMM transmissions time:
	#define ACS_IRCOMM_WAIT_TIME 20  // ACS_IRCOMM_WAIT_TIME * ACS_UPDATE_INTERVAL ms
	
	// ------------------
	// ACS Left Channel:
	#define ACS_SEND_PULSES_LEFT 40     // Number of pulses to send must be much higher than...
	#define ACS_REC_PULSES_LEFT 6       // ... number of pulses that must be received!
	#define ACS_REC_PULSES_LEFT_THRESHOLD 2 
										// Once an object has been detected, it is 
										// required to get lower than this threshold value
										// to turn of the "obstacle_left" flag.
										// If you set this equal to ACS_REC_PULSES_LEFT
										// it will have no influence on the behaviour.
	#define ACS_TIMEOUT_LEFT 14		    // ACS_TIMEOUT_LEFT * ACS_UPDATE_INTERVAL ms
	
	// ------------------
	// ACS Right Channel:
	#define ACS_SEND_PULSES_RIGHT 40 // Number of pulses to send must be much higher than...
	#define ACS_REC_PULSES_RIGHT 6   // ... number of pulses that must be received! 
	#define ACS_REC_PULSES_RIGHT_THRESHOLD 2
										// Once an object has been detected, it is 
										// required to get lower than this threshold value
										// to turn of the "obstacle_right" flag.
										// If you set this equal to ACS_REC_PULSES_RIGHT
										// it will have no influence on the behaviour.
	#define ACS_TIMEOUT_RIGHT 14     // ACS_TIMEOUT_RIGHT * ACS_UPDATE_INTERVAL ms
#endif

// Use this for your own ACS settings! 
#ifdef ACS_CONFIG_CUSTOM
	// ACS General update interval:
	#define ACS_UPDATE_INTERVAL 2  // ms
	
	// ACS Wait for IRCOMM transmissions time:
	#define ACS_IRCOMM_WAIT_TIME 20  // ACS_IRCOMM_WAIT_TIME * ACS_UPDATE_INTERVAL ms
	
	// ------------------
	// ACS Left Channel:
	#define ACS_SEND_PULSES_LEFT 36     // Number of pulses to send must be much higher than...
	#define ACS_REC_PULSES_LEFT 5       // ... number of pulses that must be received!
	#define ACS_REC_PULSES_LEFT_THRESHOLD 1 
										// Once an object has been detected, it is 
										// required to get lower than this threshold value
										// to turn of the "obstacle_left" flag.
										// If you set this equal to ACS_REC_PULSES_LEFT
										// it will have no influence on the behaviour.
	#define ACS_TIMEOUT_LEFT 16		    // ACS_TIMEOUT_LEFT * ACS_UPDATE_INTERVAL ms
	
	// ------------------
	// ACS Right Channel:
	#define ACS_SEND_PULSES_RIGHT 36 // Number of pulses to send must be much higher than...
	#define ACS_REC_PULSES_RIGHT 5   // ... number of pulses that must be received! 
	#define ACS_REC_PULSES_RIGHT_THRESHOLD 1 
										// Once an object has been detected, it is 
										// required to get lower than this threshold value
										// to turn of the "obstacle_right" flag.
										// If you set this equal to ACS_REC_PULSES_RIGHT
										// it will have no influence on the behaviour.
	#define ACS_TIMEOUT_RIGHT 16     // ACS_TIMEOUT_RIGHT * ACS_UPDATE_INTERVAL ms
#endif



/*****************************************************************************/

#endif

/******************************************************************************
 * Additional info
 * ****************************************************************************
 * Changelog:
 * - v. 1.2 07.08.2007 by Dominik S. Herwald
 *		- added ACS settings
 * - v. 1.1 27.07.2007 by Dominik S. Herwald
 *		- added define POWER_ON_WARNING
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
