#include "funktionen.h"

////////////////////////////////////////////////////////////////////////////////
//RGB LED:
void led_hsvToRgb (uint8_t hue, uint8_t  saturation, uint8_t value) //Farbe, Sättigung, Helligkeit
{
	uint8_t red, green, blue;
	uint8_t i, f;
	uint8_t  p, q, t;

  if( saturation == 0 )
 	{
    red = (255 * value )/255;
    green = (255 * value )/255;
    blue = (255 * value )/255;
	}
	else
	{	
    i=hue/43;
		f=hue%43;
		p = (value * (255 - saturation))/255;
		q = (value * ((10710 - (saturation * f))/42))/255;
		t = (value * ((10710 - (saturation * (42 - f)))/42))/255;

		switch( i )
		{
      case 0:
				red = value; green = t; blue = p; break;
			case 1:
				red = q; green = value; blue = p; break;
			case 2:
				red = p; green = value; blue = t; break;
			case 3:
				red = p; green = q; blue = value; break;			
			case 4:
				red = t; green = p; blue = value; break;				
			case 5:
	 			red = value; green = p; blue = q; break;
		}
	}  
	OCR4B = blue; OCR4C = green; OCR4A = red;  
}
////////////////////////////LED///////////////////////////////////////////////
uint8_t led_heartbeat = 0; //Helligkeit (schwellt auf und ab als ,,Lebenszeichen")
uint8_t led_heartbeatDir = 0; //Aufschwellen oder abschwellen?

#define LED_HEARTBEAT_TOP 30  //maximale Helligkeit, die beim Hardbeat erreicht wird, bis Helligkeit wieder gesenkt wird

void led_rgb(uint16_t heartbeat_color, uint16_t led_error)
{
	if(heartbeat_color > 0)
	{
		if(led_heartbeat == LED_HEARTBEAT_TOP)
		{
			led_heartbeatDir = 1;
		}
		if(led_heartbeat == 0)
		{
			led_heartbeatDir = 0;
		}
		if(led_heartbeatDir == 1)
		{
			led_heartbeat --;
		}
		else
		{
			led_heartbeat ++;
		}
		if(led_error == 0)
		{
			if(led_heartbeat > 1) //Nicht ganz aus
			{
				led_hsvToRgb(heartbeat_color,255,led_heartbeat);
			}
		}
		else
		{
			if((led_heartbeat == LED_HEARTBEAT_TOP) ||
				 (led_heartbeat == 0))
			{
				led_hsvToRgb(led_error,255,led_heartbeat);
			}
		}
	}
}
////////////////////////////////////////////////////////////////
uint16_t wallsensor[4][4] = {{0}};
uint16_t wallsensor_top[4] = {0};

#define WALLSENSOR_ERR_MIN 0
#define WALLSENSOR_ERR_MAX 1023

uint16_t get_sharpIR(void)
{
	/*
	ADC  0: Sharp vorne rechts
	ADC  1: Sharp rechts vorne
	ADC  2: Sharp hinten rechts
	ADC  3: Sharp rechts hinten
	ADC  4: Sharp vorne links
	ADC  5: Sharp links vorne
	ADC  6: Sharp hinten links
	ADC  7: Sharp links hinten
	ADC  8: –
	ADC  9: Batt
	ADC 10: Sharp top: vorne 1
	ADC 11: Sharp top: vorne 2
	ADC 12: Sharp top: rechts
	ADC 13: Sharp top: links
	ADC 14: Sharp top: hinten 1
	ADC 15: Sharp top: hinten 2
	*/

	uint16_t returnvar = 0;
	uint16_t wallsensor_var = 0;
	uint16_t wallsensor_var_2 = 0; //Für Seiten mit zwei Sensoren

	wallsensor_var = adc_read(4);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[FRONT][LEFT] = wallsensor_var;
		returnvar &= ~(1<<0);
	}
	else
	{
		returnvar |= (1<<0);
	}

	wallsensor_var = adc_read(0);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[FRONT][RIGHT] = wallsensor_var;
		returnvar &= ~(1<<1);
	}
	else
	{
		returnvar |= (1<<1);
	}

	wallsensor_var = adc_read(1);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[RIGHT][FRONT] = wallsensor_var;
		returnvar &= ~(1<<2);
	}
	else
	{
		returnvar |= (1<<2);
	}

	wallsensor_var = adc_read(3);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[RIGHT][BACK] = wallsensor_var;
		returnvar &= ~(1<<3);
	}
	else
	{
		returnvar |= (1<<3);
	}

	wallsensor_var = adc_read(6);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[BACK][LEFT] = wallsensor_var;
		returnvar &= ~(1<<4);
	}
	else
	{
		returnvar |= (1<<4);
	}

	wallsensor_var = adc_read(2);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[BACK][RIGHT] = wallsensor_var;
		returnvar &= ~(1<<5);
	}
	else
	{
		returnvar |= (1<<5);
	}

	wallsensor_var = adc_read(5);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[LEFT][FRONT] = wallsensor_var;
		returnvar &= ~(1<<6);
	}
	else
	{
		returnvar |= (1<<6);
	}

	wallsensor_var = adc_read(7);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor[LEFT][BACK] = wallsensor_var;
		returnvar &= ~(1<<7);
	}
	else
	{
		returnvar |= (1<<7);
	}
////////////////top://///////////

	/////FRONT//////
	wallsensor_var_2 = adc_read(11);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		returnvar &= ~(1<<9);
	}
	else
	{
		returnvar |= (1<<9);
	}

	wallsensor_var = adc_read(10);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		if(wallsensor_var > 300)
		{
			wallsensor_top[FRONT] = wallsensor_var+250;
		}
		else
		{
			wallsensor_top[FRONT] = wallsensor_var_2;
		}
		returnvar &= ~(1<<8);
	}
	else
	{
		returnvar |= (1<<8);
	}

	/////RIGHT/////
	wallsensor_var = adc_read(12);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor_top[RIGHT] = wallsensor_var;
		returnvar &= ~(1<<10);
	}
	else
	{
		returnvar |= (1<<10);
	}

	/////LEFT/////
	wallsensor_var = adc_read(13);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		wallsensor_top[LEFT] = wallsensor_var;
		returnvar &= ~(1<<11);
	}
	else
	{
		returnvar |= (1<<12);
	}

	/////BACK/////
	wallsensor_var_2 = adc_read(14);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		returnvar &= ~(1<<14);
	}
	else
	{
		returnvar |= (1<<14);
	}

	wallsensor_var = adc_read(15);
	if((wallsensor_var < WALLSENSOR_ERR_MAX) && (wallsensor_var > WALLSENSOR_ERR_MIN))
	{
		if(wallsensor_var > 300)
		{
			wallsensor_top[BACK] = wallsensor_var+250;
		}
		else
		{
			wallsensor_top[BACK] = wallsensor_var_2;
		}
		returnvar &= ~(1<<13);
	}
	else
	{
		returnvar |= (1<<13);
	}

	return returnvar;
}
