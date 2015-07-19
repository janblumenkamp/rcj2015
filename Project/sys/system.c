////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////system.c///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	„Tiefste“ Einheit des Roboters:
//	- Initialisierung der Hardware
//	- Geschwindigkeitsregelung
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#define SERVO_T_MIN 1800
#define SERVO_T_FUL 2000
#define SERVO_T_LOW 3800
#define SERVO_T_MAX 4100

#include "main.h"
#include "system.h"
#include "u8g.h"
#include "funktionen.h"
#include "bluetooth.h"
#include "maze.h"
#include "i2cdev.h"
#include "display.h"

////////////////////////////////////////////////////////////////////////////////
//System
uint8_t mcusr_mirror = 0;

void init_sys(void)
{
	mcusr_mirror = MCUSR;	//Watchdog
	MCUSR = 0;
	wdt_disable();


	//Pins bzw. Ports als Ein-/Ausgänge konfigurieren
	DDRA |= 0xfc; //Encoder (nur 2 Pins!), Zeilenkamera
		PORTA |= 0x03; //Pullups an
	DDRA &= ~(1<<PA5); //Bumper
	DDRA &= ~(1<<PA6); //Bumper
		PORTA |= (1<<PA5); //Pullups
		PORTA |= (1<<PA6);
	DDRB |= 0xff;	//SPI, PWM
	DDRC |= 0xf0;	//Taster (LCD), LCD Hintergund LED, SS1/SS2
		PORTC |= 0x0f; //Pullups für Eingänge
	DDRD |= 0xff;	//UART1, I²C, LED RNmega2560
	DDRE |= 0x18;	//00011000 => PWM, INT4, ENC M1/M2
		PORTE |= 0xe7; //Pullups für Eingänge
	DDRF |= 0x00; //herausgeführt auf Micromatch
	DDRG |= 0x20; //00100000 => NC, Opfer LED
	DDRH |= 0xff; //UART2 (Bluetooth), RGB LED, PWM (RGB LED)
	//PORT K ADC
	DDRL |= 0xff; //Motortreiber

	//Initialisierung der Datenstruktur für die Motoren:
	mot.off = 1;
	for(uint8_t i = 0; i < 2; i++)
	{
		mot.d[i].enc = 0;
		mot.d[i].speed.is = 0;
		mot.d[i].speed.to = 0;
	}
	
	//Initialisierung der Datenstruktur für die IR Sensoren:
	mlx90614[LEFT].is = 0;
	mlx90614[RIGHT].is = 0;
	
	//EEPROM
	mlx90614[LEFT].th = eeprom_read_word((uint16_t*)0); //Schwellwerte, werden aus EEPROM gelesen
	mlx90614[RIGHT].th = eeprom_read_word((uint16_t*)2);
	//use_tarry = eeprom_read_byte((uint8_t*)4); //Algo oder rechte Hand? Wird aus EEPROM gelesen
	//debug = eeprom_read_byte((uint8_t*)5);
	ground_th = eeprom_read_word((uint16_t*)6); //tsl_th...
	//ground_th = 900;//eeprom_read_word((uint16_t*)8);
//	tsl_th_ground = tsl_th + TSL_GROUNDSENS_DIFF;//eeprom_read_word((uint16_t*)10);
}

////////////////////////////////////////////////////////////////////////////////
//ADC

void init_adc(void)
{
	//http://bennthomsen.wordpress.com/embedded-design/peripherals/analogue-input/
	//16MHz/128 = 125kHz the ADC reference clock
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
	ADMUX |= (1<<REFS0);       //Set Voltage reference to Avcc (5v)
	ADCSRA |= (1<<ADEN);       //Turn on ADC
	ADCSRA |= (1<<ADSC);       //Do an initial conversion
}

//Auswertung in Timer!

////////////////////////////////////////////////////////////////////////////////

//Initialisierung der PWM/ des Timers
void init_pwm(void)	//Initialisierung des Timers für Erzeugung des PWM-Signals
{
  //OC0A/B:
	//normale 8-bit PWM aktivieren
  TCCR0A = (1<<COM0A1)
					|(1<<COM0B1) //nicht invertierend
					|(1<<WGM00);   //Fast PWM 8bit
		 			
  //Einstellen der PWM-Frequenz
  TCCR0B = (1<<CS02); //Clk/256 (122,5Hz)
	
	//OC4A/B/C:
	TCCR4A = (1<<COM4A1)
					|(1<<COM4B1)
					|(1<<COM4C1)
					|(1<<WGM40);

	TCCR4B = (1<<CS42); //Clk/256

	//OC3A (PWM/Servo)
	TCCR3A = (1<<COM3A1)
			|(1<<WGM31);

	TCCR3B = (1<<WGM33)
			|(1<<WGM32)
			|(1<<CS31);

	ICR3 = 40000;

	//OC5A/B:
	TCCR5A = (1<<COM5B1)
					|(1<<COM5C1)
					|(1<<WGM50);

	TCCR5B = (1<<CS50); //Clk/1

	///////////////////////////////////
	OCR0A = 0; //Zeilenkamera
	OCR0B = 0; //Opfer LED
	OCR3A = SERVO_T_LOW; //Servo (0)
	OCR5B = 0; //PWM B
	OCR5C = 0; //PWM A
	//Zuweisung von OCR4x in hsv zu rgb (RGB LED)
}

void init_timer(void)
{
	//OC1: Scheduler
	TCCR1B = (1<<WGM12) //CTC
			|(1<<CS10)
			|(1<<CS11); //Prescaler 64

	TIMSK1 |= (1<<OCIE1A);
	
	OCR1A = 250; //1kHz; Scheduler
	
	//OC2A: ADC
	/*TCCR2A = (1<<WGM21);
					
	TCCR2B = (1<<CS21); //Presc. 8

	TIMSK2 = (1<<OCIE2A);

	OCR2A = 250; //8kHz 250*/
}

///////////////////////////////////Display//////////////////////////////////////
void init_display(uint8_t ledstate)
{
	u8g_InitHWSPI(&u8g, &u8g_dev_st7920_128x64_hw_spi, PN(1, 0), U8G_PIN_NONE, U8G_PIN_NONE);
	u8g_SetRot180(&u8g);
	display_setBGLED(1);	  //Hintergrundbeleuchtung
}
////////////////////////////Taster, Bumper//////////////////////////////////////

uint8_t get_t1(void) //Taster neben Power
{
	return (PINC & (1<<PC3));
}

uint8_t get_incrOk(void) //incrementalOk
{
	return !(PINC & (1<<PC0));
}

uint8_t get_bumpL(void) //Bumper left
{
	return !(PINA & (1<<PA5));
}

uint8_t get_bumpR(void) //Bumper right
{
	return !(PINA & (1<<PA6));
}

//////////////////////////////Servo////////////


void servo_setPos(uint8_t angle)
{
	uint16_t servo_time = -21*angle+SERVO_T_LOW;
	if(servo_time < SERVO_T_MIN)
		servo_time = SERVO_T_MIN;
	else if(servo_time > SERVO_T_MAX)
		servo_time = SERVO_T_MAX;
	OCR3A = servo_time;
}

////////////////////////////////////////////////////////////////////////////////

void motor_activate(uint8_t activate)
{
	if(activate)
		PORTL |= (1<<6); //No Standby
	else
		PORTL &= ~(1<<6);
}

int16_t pwr[2];

void set_speed(void)
{
	int16_t pwrvar_left = pwr[LEFT];
	int16_t pwrvar_right = pwr[RIGHT];

	if(pwrvar_left > 0)
	{
		PORTL &= ~(1<<2); //DIRA1
		PORTL |= (1<<3); //DIRA2
	}
	else if(pwrvar_left == 0)
	{
		PORTL |= (1<<2); //DIRA1
		PORTL |= (1<<3); //DIRA2
	}
	else if(pwrvar_left < 0)
	{
		PORTL |= (1<<2); //DIRA1
		PORTL &= ~(1<<3); //DIRA2
	}

	if(pwrvar_right > 0)
	{
		PORTL |= (1<<1); //DIRB1 //rechter Motor rückwärts
		PORTL &= ~(1<<0); //DIRB2
	}
	else if(pwrvar_right == 0)
	{
		PORTL |= (1<<1); //DIRB1
		PORTL |= (1<<0); //DIRB2 Motor aus
	}
	else if(pwrvar_right < 0)
	{
		PORTL &= ~(1<<1); //DIRB1
		PORTL |= (1<<0); //DIRB2; //Rechter Motor vorwärts
	}

	abs_ptr(&pwrvar_left);
	abs_ptr(&pwrvar_right);
	
	if(pwrvar_left > 255)
		pwrvar_left = 255;
	if(pwrvar_right > 255)
		pwrvar_right = 255;

	if(pwrvar_left == 0)
		OCR5C = 0;
	else
		 OCR5C = ((0.84*pwrvar_left)+40); //PWMB (Geschwindigkeit) linker Motor

	if(pwrvar_right == 0)
		OCR5B = 0;
	else
		OCR5B = ((0.84*pwrvar_right)+40); //PWMA (Geschwindigkeit) rechter Motor
}

////////////////////////////////////////////////////////
//Einzelne IOs

//Hintergrundbeleuchtung Display
void display_setBGLED(uint8_t state)
{
	if(state == 1)	PORTC |= (1<<4);
	else						PORTC &= ~(1<<4);
}

//Signal LED
void ui_setLED(int8_t led, uint8_t brightness)
{
	if(led < 0) //untere LED (dimmbar)
		OCR0B = brightness;
	else if(led == 0) //beide aus
	{
		OCR0B = 0;
		PORTE &= ~(1<<PE4);
	}
	else //obere LED
	{
		if(brightness == 0)	PORTE &= ~(1<<PE4);
		else								PORTE |= (1<<PE4);
	}
}

//TSL LED
void groundSens_setLED(uint8_t brightness)
{
	OCR0A = brightness;
}
////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
int32_t enc_start[2];
int16_t mot_reg_integral[2]; //I-Part of the speed regulator

MOTOR_ROB_t mot;

#define SPEED_MAX 255

void controlSpeed(void) //eigentliche Geschwindigkeitsregelung, muss mit 25Hz aufgerufen werden!
{
    for(uint8_t i = 0; i < 2; i++)
        {
            mot.d[i].speed.is = (mot.d[i].enc - enc_start[i]);
            enc_start[i] = mot.d[i].enc;

            if(mot.d[i].speed.to != 0)
            {
                pwr[i] = ((mot.d[i].speed.to - mot.d[i].speed.is)*2.4);
                if(pwr[i] > SPEED_MAX)
                    pwr[i] = SPEED_MAX;
                if(pwr[i] < -SPEED_MAX)
                    pwr[i] = -SPEED_MAX;
            }
            else
            {
                pwr[i] = 0;
            }
        }

        set_speed();

    /* for(uint8_t i = 0; i < 2; i++)
    {
        mot.d[i].speed.is = mot.d[i].enc - enc_start[i]; //Ticks per 40ms (full speed (pwm ~200): 31
        enc_start[i] = mot.d[i].enc;

        if(mot.d[i].speed.to != 0)
        {
            if(mot.d[i].speed.to > 35)
                mot.d[i].speed.to = 35;
            if(mot.d[i].speed.to < -35)
                mot.d[i].speed.to = -35;

            int16_t e = mot.d[i].speed.to - mot.d[i].speed.is;

            if(abs(mot_reg_integral[i] + e) < 200)
                mot_reg_integral[i] += e;

            pwr[i] = (e * (abs(mot.d[i].speed.to) / 3)) + (mot_reg_integral[i]*((abs(mot.d[i].speed.to) / 10) + 1));

            if(pwr[i] > SPEED_MAX)
                pwr[i] = SPEED_MAX;
            if(pwr[i] < -SPEED_MAX)
                pwr[i] = -SPEED_MAX;
        }
        else
        {
            mot_reg_integral[i] = 0;
            pwr[i] = 0;
        }
    }

    set_speed();*/

    //bt_putStr("\e[2J"); //clear//
    //bt_putStr("\e[H");
    /*bt_putLong(mot.d[LEFT].speed.to); bt_putStr("\t"); bt_putLong(mot.d[RIGHT].speed.to); bt_putStr("\n");
    bt_putLong(mot.d[LEFT].speed.is); bt_putStr("\t"); bt_putLong(mot.d[RIGHT].speed.is); bt_putStr("\n");
    bt_putLong(mot_reg_integral[LEFT]);bt_putStr("\t"); bt_putLong(mot_reg_integral[RIGHT]); bt_putStr("\n");
    bt_putLong(pwr[LEFT]);bt_putStr("\t"); bt_putLong(pwr[RIGHT]); bt_putStr("\n");
    bt_putStr("\n");*/
}
