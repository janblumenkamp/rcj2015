#include "system.h"
#include "u8g.h"
	extern u8g_t u8g; //Pointer für u8g muss hier eingebunden werden
#include "funktionen.h"
#include "bluetooth.h"

////////////////////////////////////////////////////////////////////////////////
//System
void init_sys(void)
{
	//Pins bzw. Ports als Ein-/Ausgänge konfigurieren
	DDRA |= 0x00; //Bumper, Encoder (nur 2 Pins!) (Rest herausgeführt)
		PORTA |= 0xff; //Pullups an
	DDRB |= 0xff;	//SPI, PWM
	DDRC |= 0xf0;	//Taster (LCD), LCD Hintergund LED, SS1/SS2
		PORTC |= 0x0f; //Pullups für Eingänge
	DDRD |= 0xff;	//UART1, I²C, LED RNmega2560
	DDRE |= 0xc0;	//11000000 => PWM, INT4, ENC M1/M2
	DDRF |= 0x00; //herausgeführt auf Micromatch
	DDRG |= 0x20; //00100000 => NC, Opfer LED
	DDRH |= 0xff; //UART2 (Bluetooth), RGB LED, PWM (RGB LED)
	//PORT K ADC
	DDRL |= 0xff; //Motortreiber
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

uint16_t adc_read(uint8_t channel)
{
	ADMUX &= 0xE0;           //Clear bits MUX0-4
	ADMUX |= channel&0x07;   //Defines the new ADC channel to be read by setting bits MUX0-2
	ADCSRB = channel&(1<<3); //Set MUX5
	ADCSRA |= (1<<ADSC);      //Starts a new conversion
	while(ADCSRA & (1<<ADSC));  //Wait until the conversion is done
	return ADCW;
}
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
  TCCR0B = (1<<CS00); //Clk/1
	
	//OC1A/B:
	TCCR1A = (1<<COM1A1)
					|(1<<COM1B1)
					|(1<<WGM10);//Fast PWM 8bit
		
	TCCR1B = (1<<CS10); //Clk/1

	//OC2A/B:
	TCCR2A = (1<<COM2A1)
					|(1<<COM2B1)
					|(1<<WGM00);
					
	TCCR2B = (1<<CS20);

	//OC4A/B/C:
	TCCR4A = (1<<COM4A1)
					|(1<<COM4B1)
					|(1<<COM4C1)
					|(1<<WGM40);

	TCCR4B = (1<<CS40); //Clk/1

	//OC5A/B:
	TCCR5A = (1<<COM5B1)
					|(1<<COM5C1)
					|(1<<WGM50);

	TCCR5B = (1<<CS50); //Clk/1

	///////////////////////////////////
	OCR0A = 0; //herausgeführt
	OCR0B = 0; //Opfer LED
	OCR1A = 0; //herausgeführt
	OCR1B = 0; //herausgeführt
	OCR2A = 0; //herausgeführt
	OCR2B = 0; //herausgeführt
	OCR5B = 0; //PWM B
	OCR5C = 0; //PWM A
	//Zuweisung von OCR4x in hsv zu rgb (RGB LED)
}

void init_timer(void)
{
	//OC3:
	TCCR3B = (1<<WGM32)
					|(1<<CS30)
					|(1<<CS31); //Prescaler 64

	TIMSK3 = (1<<OCIE3A);

	OCR3A = 25; //10000Hz
}

///////////////////////////////////Display//////////////////////////////////////
void init_display(uint8_t ledstate)
{
	u8g_InitHWSPI(&u8g, &u8g_dev_st7920_128x64_hw_spi, PN(1, 0), U8G_PIN_NONE, U8G_PIN_NONE);
	display_setBGLED(1);	  //Hintergrundbeleuchtung
	u8g_DrawLogo();
}
////////////////////////////Taster, Bumper//////////////////////////////////////

uint8_t get_t1(void) //Taster neben Power
{
	if((PINC & 1<<PC3) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t get_incrOk(void) //incrementalOk
{
	if((PINC & 1<<PC0) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t get_bumpL(void) //incrementalOk
{
	if((PINA & 1<<PA3) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t get_bumpR(void) //incrementalOk
{
	if((PINA & 1<<PA2) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}

	//return !(PINA & 1<<PA2);
}
////////////////////////////////////////////////////////////////////////////////

int16_t pwr_left = 0;
int16_t pwr_right = 0;

void set_speed(void)
{
	PORTL |= (1<<6); //kein Standby

	int16_t pwrvar_left = pwr_left;
	int16_t pwrvar_right = pwr_right;

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
		PORTL &= ~(1<<1); //DIRB1
		PORTL |= (1<<0); //DIRB2; //Rechter Motor vorwärts
	}
	else if(pwrvar_right == 0)
	{
		PORTL |= (1<<1); //DIRB1
		PORTL |= (1<<0); //DIRB2 Motor aus
	}
	else if(pwrvar_right < 0)
	{
		PORTL |= (1<<1); //DIRB1 //rechter Motor rückwärts
		PORTL &= ~(1<<0); //DIRB2
  }

	if(pwrvar_left < 0)
	{
		pwrvar_left *= (-1);
	}
	if(pwrvar_right < 0)
	{
		pwrvar_right *= (-1);
	}

	if(pwr_left > 210)
		pwr_left = 210;
	if(pwr_right > 210)
		pwr_right = 210;

	if(pwrvar_left == 0)
		OCR5C = 0;
	else
		 OCR5C = ((0.9*pwrvar_left)+25); //PWMB (Geschwindigkeit) linker Motor

	if(pwrvar_right == 0)
		OCR5B = 0;
	else
  	OCR5B = ((0.9*pwrvar_right)+25); //PWMA (Geschwindigkeit) rechter Motor
}

////////////////////////////////////////////////////////
//Einzelne IOs

//Hintergrundbeleuchtung Display
void display_setBGLED(uint8_t state)
{
	if(state == 1)
	{
		PORTC |= (1<<4);
	}
	else
	{
		PORTC &= ~(1<<4);
	}
}

//Opfer LED
void victim_setLED(uint8_t brightness)
{
	OCR0B = brightness;
}

////////////////////////////////////

uint8_t getLS(void)
{
	if((PINE & 1<<PC4) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
#define TIMER_MOTOR_SPEED 2 //SIEHE AUCH HAUPTPROGRAMM!!!

int32_t enc_l_start = 0;
int32_t enc_r_start = 0;

int16_t speed_l_raw = 0;
int16_t speed_r_raw = 0;

int16_t speed_l = 0;
int16_t speed_r = 0;

int16_t i_speed_l = 0;
int16_t i_speed_r = 0;

#define KI_SPEED 1

#define I_MAX 500
#define I_MIN -500

#define SPEED_MAX 210

uint8_t sm_ctrl_speed = 0; //Statemachine

void controlSpeed(void)
{
	switch(sm_ctrl_speed)
	{
		case 0: enc_l_start = enc_l;
						enc_r_start = enc_r;
						timer_motor_speed = TIMER_MOTOR_SPEED;
						sm_ctrl_speed = 1;
						break;
		case 1: if(timer_motor_speed == 0)
						{
							int8_t speedvar_l = speed_l;
							int8_t speedvar_r = speed_r;

							speed_l_raw = (enc_l - enc_l_start)*1;
							speed_r_raw = (enc_r - enc_r_start)*1;

							i_speed_l += (speedvar_l - speed_l_raw)*KI_SPEED;
							i_speed_r += (speedvar_r - speed_r_raw)*KI_SPEED;

							if(i_speed_l > I_MAX)
							{	i_speed_l = I_MAX;}
							if(i_speed_l < I_MIN)
							{	i_speed_l = I_MIN;}
							if(speedvar_l == 0)
							{	i_speed_l = 0;}
							pwr_left = i_speed_l;
							if(pwr_left > SPEED_MAX)
							{	pwr_left = SPEED_MAX;}
							if(pwr_left < -SPEED_MAX)
							{	pwr_left = -SPEED_MAX;}


							if(i_speed_r > I_MAX)
							{	i_speed_r = I_MAX;}
							if(i_speed_r < I_MIN)
							{	i_speed_r = I_MIN;}
							if(speedvar_r == 0)
							{	i_speed_r = 0;}
							pwr_right = i_speed_r;
							if(pwr_right > SPEED_MAX)
							{	pwr_right = SPEED_MAX;}
							if(pwr_right < -SPEED_MAX)
							{	pwr_right = -SPEED_MAX;}

							set_speed();
							

//displayvar[0] = (speed_r*SPEED_TO_RAW_FAC); displayvar[1] = (speed_l*SPEED_TO_RAW_FAC);
//displayvar[0] = i_speed_l; displayvar[1] = i_speed_r;
//displayvar[0] = e_speed_l; displayvar[1] = e_speed_r;
//displayvar[0] = steer_l; displayvar[1] = steer_r; 
//displayvar[0] = pwr_left; displayvar[1] = pwr_right; 
//displayvar[0] = speed_l_raw; displayvar[1] = speed_r_raw; 

							//bt_putStr("\e[2J"); //clear//
							//bt_putStr("\e[H");
							//bt_putLong(speed_l_raw);
							//bt_putStr("\n\r");
							//bt_putLong(speed_r);
							//bt_putStr("\n\r");
							/*bt_putStr("\n\r");
							bt_putLong(e_speed_l);
							bt_putStr("\n\r");
							bt_putStr("\n\r");
							bt_putLong(i_speed_l);
							bt_putStr("\n\r");
							bt_putLong(i_speed_r);*/
							/*bt_putStr("\n\r");
							bt_putLong(timer);
							bt_putStr(" ");
							bt_putLong(pwr_left);*/
							
							sm_ctrl_speed = 0;
						}
						break;
	}
}
