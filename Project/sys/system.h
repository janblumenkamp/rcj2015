////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////system.h///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe system.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/eeprom.h>
#include <avr/wdt.h> //watchdog

///////////////////////////
extern uint8_t mcusr_mirror; //Für watchdogflag

struct SPEED_t {
	int16_t is; //Geschw. Istwert
	int16_t to; //Geschw. Sollwert
};

struct MOTOR_t {
	struct SPEED_t speed;
	int32_t enc; //Encoderwert
	int32_t enc_abs; //Encodervalue (absolute), counts only into one direction (up).
};

typedef struct {
	struct MOTOR_t d[2]; //d ~ direction (LEFT/RIGHT)
	int8_t off;
	int8_t off_invisible;
	int32_t enc;
}	MOTOR_ROB_t;

extern MOTOR_ROB_t mot;

#define SERVO_T_MIN 1800
#define SERVO_T_FUL 2000
#define SERVO_T_LOW 4000
#define SERVO_T_MAX 4100

//-> x = mot.d[LEFT].speed.is;
//-> mot.d[RIGHT].speed.to = y;
//-> _enc = motor.d[LEFT].enc;
///////////////////////////
extern void init_sys(void);

extern void init_adc(void);

extern void init_pwm(void);

extern void init_timer(void);

extern void init_display(uint8_t ledstate);

extern uint8_t get_t1(void);

extern uint8_t get_incrOk(void);

extern uint8_t get_bumpL(void);

extern uint8_t get_bumpR(void);

extern void servo_setPos(uint8_t angle);

extern void motor_activate(uint8_t activate);

extern void set_speed(void);

extern void display_setBGLED(uint8_t state);

extern void ui_setLED(int8_t led, uint8_t brightness);

extern void groundSens_setLED(uint8_t brightness);

extern void controlSpeed(void);
