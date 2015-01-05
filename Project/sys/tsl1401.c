////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
////////////////////////////////tsl1401.c///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Library: Reading out the linecam
//	Functions to read out and analyze
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "tsl1401.h"
#include "system.h"
#include "funktionen.h"
#include "bluetooth.h"
#include "main.h"

//Defines in the Header...

int16_t tsl_exposure = TSL_EXPOSURE_MAX/2; //Clockcycles of the aparture regulator to-value
uint8_t tsl_exposure_now; //actual aparture
uint8_t tsl_led_brightness = 0; //Brightness of the LED

int16_t tslData[128/TSL_RESOLUTION];
int16_t tslData_med[(128/TSL_RESOLUTION)-TSL_CNT_OFFSET_ROB]; //Für Median
uint8_t tsl_pixCnt;
uint8_t tsl_resolutionCnt;
int8_t tsl_adc_sync = -1;

int16_t tsl_res = 0;

uint8_t tsl_state = 0;

int8_t tsl1401(int8_t state)
{
	switch(tsl_state)
	{
		case TSL_INIT:

				tsl_resolutionCnt = (128/TSL_RESOLUTION)-2; //Inverting the cam to make pixel 0 the pixel on the robot (Reference)
				tsl_pixCnt = 1; //First Clock at SI in TSL_EXPOSE already counts as clock!
				tsl_exposure_now = 0;
				
				TSL_CK_PORT &= ~(1<<TSL_CK_PIN);
				TSL_SI_PORT |= (1<<TSL_SI_PIN);
				TSL_CK_PORT |= (1<<TSL_CK_PIN);
				TSL_SI_PORT &= ~(1<<TSL_SI_PIN);
				
				for(uint8_t i = 0; i < 128; i++)
				{
					TSL_CK_PORT &= ~(1<<TSL_CK_PIN);
					TSL_CK_PORT |= (1<<TSL_CK_PIN);
				}

				tsl_state = TSL_EXPOSE;
				
			break;

		case TSL_EXPOSE:
		
				tsl_exposure_now ++;
				if(tsl_exposure_now == tsl_exposure)
				{
					TSL_CK_PORT &= ~(1<<TSL_CK_PIN);
					TSL_SI_PORT |= (1<<TSL_SI_PIN);
					TSL_CK_PORT |= (1<<TSL_CK_PIN);
					TSL_SI_PORT &= ~(1<<TSL_SI_PIN);
				
					for(uint8_t i = 0; i < TSL_RES_CNT_START; i++)
					{
						TSL_CK_PORT &= ~(1<<TSL_CK_PIN);
						TSL_CK_PORT |= (1<<TSL_CK_PIN);
						
						tsl_pixCnt ++;
					}
					
					tsl_state = TSL_WAIT_GET; //Now, the anSens Task has to set the tsl_adc_state to ADC_CAM_INIT, when the adc is not needed anymore
				}
				else break;
				
		case TSL_WAIT_GET: break;
		
		case TSL_GET:
				
				if(tsl_adc_state == ADC_CAM_DONE)
				{
					tsl_adc_state = ADC_CAM_IDLE;
					
					tsl_exposure += (int16_t)(TSL_PREG_EXP_SOLL-tslData[TSL_REG_PIX])/TSL_REG_EXP_FAC; //Aparture Regulation
					
					if(tsl_exposure < TSL_EXPOSURE_MIN)
						tsl_exposure = TSL_EXPOSURE_MIN;
					else if(tsl_exposure > TSL_EXPOSURE_MAX)
						tsl_exposure = TSL_EXPOSURE_MAX;

					if((tsl_exposure > TSL_EXPOSURE_MAX_LED) &&	 //LED power up
						 (tsl_led_brightness < 255))
						tsl_led_brightness ++;
					else if((tsl_exposure < TSL_EXPOSURE_MIN_LED) && //or power down
									(tsl_led_brightness > 0))			
						tsl_led_brightness --;
		
					tsl_setLED(tsl_led_brightness);
					
					tsl_state = TSL_PROCESS;
				}
				
			break; 

		case TSL_PROCESS:

				for(uint8_t i = TSL_CNT_OFFSET_ROB; i < (128/TSL_RESOLUTION)-1; i++)
					tslData_med[i-TSL_CNT_OFFSET_ROB] = tslData[i];
				
				tsl_res = *get_sorted(((128/TSL_RESOLUTION)-TSL_CNT_OFFSET_ROB)-1, tslData_med, 6);
				
				tsl_state = TSL_INIT;
			break;
		default:
				if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:task_cam:tsl1401::case:default"));}
				state = -1;
				fatal_err = 1;
	}
				
	return state;
}

////////////////////////////////////////////////////////////////////////////////
//ISR:

#define ADC_MED_SAMPLES 5 //How many measurements for the median filter? !ATTENTION!: If this is changed, the statemachine also has to be adapted (MEASURE_n)

int16_t tsl_adc_data[ADC_MED_SAMPLES];

int8_t tsl_adc_state = ADC_CAM_IDLE;

ISR(TIMER2_COMPA_vect) //8kHz -> (1/8000*(128/TSL_RES)
{	
	switch(tsl_adc_state)
	{
		case ADC_CAM_IDLE: break;
		case ADC_CAM_INIT:
				
				if(!(ADCSRA & (1<<ADSC))) //No running conversions
				{
					//Set MUX for next channel:
					ADMUX &= 0xE0;						//Clear bits MUX0-4
					ADMUX |= TSL_A0_CH & 0x07;		//Defines the new ADC channel to be read by setting bits MUX0-2
					ADCSRB = TSL_A0_CH & (1<<3);	//Set MUX5
					ADCSRA |= (1<<ADSC); 			//New Conversion for the next Channel
				
					tsl_adc_state = MEASURE_1;
				}
			
			break;
				
		case MEASURE_1:
		case MEASURE_2:
		case MEASURE_3:
		case MEASURE_4:
		
				tsl_adc_data[tsl_adc_state - MEASURE_1] = ADCW;
				ADCSRA |= (1<<ADSC);	//Start new Conversion

				tsl_adc_state ++;

			break;
		case MEASURE_5_RES:

				tsl_adc_data[4] = ADCW;
				
				tslData[tsl_resolutionCnt] = *get_sorted(ADC_MED_SAMPLES, tsl_adc_data, ADC_MED_SAMPLES/2);	//Median-filter
				
				tsl_resolutionCnt --;
	
				for(uint8_t i = 0; i < TSL_RESOLUTION; i ++)
				{
					TSL_CK_PORT &= ~(1<<TSL_CK_PIN);
					TSL_CK_PORT |= (1<<TSL_CK_PIN);
		
					tsl_pixCnt ++;
					
					if(tsl_pixCnt >= 129) //Ready
					{
						tsl_adc_state = ADC_CAM_DONE;
						break;
					}
					else
					{
						tsl_adc_state = MEASURE_1;
					}
				}
			break;
			
		case ADC_CAM_DONE: break;
		default:
				if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:ISR(TIMER2_COMPA_vect)[task_adc]::case:default"));}
				tsl_adc_state = -1;
				fatal_err = 1;
	}
}
