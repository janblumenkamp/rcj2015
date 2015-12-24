#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



///////////////////////////
extern int32_t enc_l;
extern int32_t enc_r;
extern int16_t speed_l_raw;
extern int16_t speed_r_raw;

extern int16_t speed_l;
extern int16_t speed_r;

extern int32_t displayvar[3];

//Timer///
extern volatile int8_t timer_motor_speed;
extern volatile uint32_t timer;

///////////////////////////
extern void init_sys(void);

extern void init_adc(void);

extern uint16_t adc_read(uint8_t channel);

extern void init_pwm(void);

extern void init_timer(void);

extern void init_display(uint8_t ledstate);

extern uint8_t get_t1(void);

extern uint8_t get_incrOk(void);

extern uint8_t get_bumpL(void);

extern uint8_t get_bumpR(void);

extern void set_speed(void);

extern void display_setBGLED(uint8_t state);

extern void victim_setLED(uint8_t brightness);

extern uint8_t getLS(void);

extern void controlSpeed(void);
