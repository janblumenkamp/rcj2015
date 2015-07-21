////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////funktionen.c//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///	Alles Mögliche, was nicht in die anderen Teile passt:
///	- RGB LED: Farbkonvertierung und Heartbeat
///	- Zuweisung der Raw Werte des Sharp Sensoren in Array (*dist[][])
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "funktionen.h"
#include "system.h"
#include "main.h"
#include "maze.h"
#include "bluetooth.h"
#include "drive.h"
#include "mazefunctions.h"
#include "um6.h"

////////////////////////////////////////////////////////////////////////////////
void limitVar(int16_t *var, int16_t limit)
{
	if(*var > limit)
	 	*var = limit;
	if(*var < -limit)
		*var  = -limit;
}
////////////////////////////////////////////////////////////////////////////////
//Amount via pointer
void abs_ptr(int16_t *var)
{
	if(*var < 0)
		*var = -(*var);
}

//RGB LED:
//Source: http://www.mikrocontroller.net/topic/158185

void led_hsvToRgb (uint8_t h, uint8_t  s, uint8_t v) //Farbe, Sättigung, Helligkeit
{
	uint16_t vs = v * s, h6 = 6 * h, f;
	uint8_t i, p, u, r = v, g = v, b = v;

	p = ((v<<8) - vs) >> 8;
	i = h6 >> 8;
	f = ((i|1) << 8) - h6;
	if(i & 1)
		f = -f;
	u = (((uint32_t) v << 16) - (uint32_t) vs * f) >> 16;

	switch(i)
	{
		case 0: g = u;	b = p;	break;
		case 1: r = u;	b = p;	break;
		case 2: r = p;	b = u;	break;
		case 3: r = p;	g = u;	break;
		case 4: r = u;	g = p;	break;
		case 5: g = p;	b = u;	break;
	}

	OCR4A = r; OCR4C = g; OCR4B = b;
}

///////////////////////////////////////Rescue Kit/////////////////////////////

#define DEPLKIT_TIME_DEPLOYING	30
#define DEPLKIT_TIME_WAIT		70
uint8_t sm_dropKit = 0;
uint16_t servo_dropKitAngle = 0;

uint8_t rescueKit_drop(uint8_t speed)
{
	uint8_t returnvar = 1;

	switch(sm_dropKit)
	{
		case 0:
				servo_dropKitAngle ++;
				if(servo_dropKitAngle == 90*speed)
					sm_dropKit = 1;
			break;
		case 1 ... DEPLKIT_TIME_DEPLOYING:
				sm_dropKit ++;
				break;
		case (DEPLKIT_TIME_DEPLOYING+1):
				servo_dropKitAngle = 0;
				sm_dropKit = (DEPLKIT_TIME_DEPLOYING+2);
		case (DEPLKIT_TIME_DEPLOYING+2) ... (DEPLKIT_TIME_DEPLOYING + DEPLKIT_TIME_WAIT):
				sm_dropKit ++;
				break;
		case (DEPLKIT_TIME_DEPLOYING + DEPLKIT_TIME_WAIT + 1):
				sm_dropKit = 0;
				returnvar = 0;
			break;
	}
	servo_setPos(servo_dropKitAngle/speed);

	return returnvar;
}

////////////////////////////LED///////////////////////////////////////////////
uint8_t led_heartbeat = 0; //Helligkeit (schwellt auf und ab als ,,Lebenszeichen")
uint8_t led_heartbeatDir = 0; //Aufschwellen oder abschwellen?

void led_rgb(uint16_t heartbeat_color, uint16_t led_error, uint8_t led_top)
{
	if(heartbeat_color > 0)
	{
		if(led_heartbeat == led_top)
			led_heartbeatDir = 1;
		else if(led_heartbeat == 0)
			led_heartbeatDir = 0;
		
		if(led_heartbeatDir == 1)
			led_heartbeat --;
		else
			led_heartbeat ++;
		
		if(led_error == 0)
		{
			if(timer_rdy_restart == -1) //Only switch display LED if we are not restarting right now (turning display off)
				display_setBGLED(1);

			if(led_heartbeat > 1) //Nicht ganz aus
				led_hsvToRgb(heartbeat_color, 255, led_heartbeat);
		}
		else
		{
			if(led_heartbeat == led_top)
			{
				led_hsvToRgb(led_error, 255, led_top);
				display_setBGLED(0);
			}
			else if(led_heartbeat == 0)
			{
				led_hsvToRgb(led_error, 255, 0);
				if(timer_rdy_restart == -1) //Only switch display LED if we are not restarting right now (turning display off)
					display_setBGLED(1);
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the relative angle of the robot to the wall (not in degrees, but
/// as the differenceof the sensors on one side).
///
/// @param
///		dir: Wall in which direction?
///
///	@return
///		angle
//////////////////////////////////////////////////////////////////////////////////

int16_t robot_getAngleToWall(uint8_t dir)
{
	int16_t returnvar = GETANGLE_NOANGLE;
	dir = maze_alignDir(dir);

	switch(dir)
	{
		case NONE:

				if((dist[LIN][RIGHT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
				   (dist[LIN][RIGHT][BACK] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][RIGHT][BACK] - dist[LIN][RIGHT][FRONT]);
				}
				else if((dist[LIN][LEFT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
						(dist[LIN][LEFT][BACK] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][LEFT][BACK] - dist[LIN][LEFT][FRONT]);
				}
				else if((dist[LIN][FRONT][LEFT] < TILE1_SIDE_ALIGN_TH) &&
						(dist[LIN][FRONT][RIGHT] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][FRONT][RIGHT] - dist[LIN][FRONT][LEFT]);
				}
				else if((dist[LIN][BACK][LEFT] < TILE1_SIDE_ALIGN_TH) &&
						(dist[LIN][BACK][RIGHT] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][BACK][RIGHT] - dist[LIN][BACK][LEFT]);
				}

			break;

		case NORTH:

				if((dist[LIN][FRONT][LEFT] < TILE1_SIDE_ALIGN_TH) &&
				   (dist[LIN][FRONT][RIGHT] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][FRONT][RIGHT] - dist[LIN][FRONT][LEFT]);
				}

			break;

		case EAST:

				if((dist[LIN][RIGHT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
				   (dist[LIN][RIGHT][BACK] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][RIGHT][BACK] - dist[LIN][RIGHT][FRONT]);
				}

			break;

		case SOUTH:

				if((dist[LIN][BACK][LEFT] < TILE1_SIDE_ALIGN_TH) &&
				   (dist[LIN][BACK][RIGHT] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][BACK][RIGHT] - dist[LIN][BACK][LEFT]);
				}
			break;

		case WEST:

				if((dist[LIN][LEFT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
				   (dist[LIN][LEFT][BACK] < TILE1_SIDE_ALIGN_TH))
				{
					returnvar = (dist[LIN][LEFT][BACK] - dist[LIN][LEFT][FRONT]);
				}

			break;
	}

	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
/// Sorts the elements of the given array by their size from the smallest to the
/// biggest and returns a pointer to the element at @get (helpfull when used as
/// median-filter)
///
/// Param:
/// @cnt:		Number of elements in @data[] (size in words)
/// @*data: 	Pointer to the first element of the array to sort
/// @get:		Number of the element to return in the sorted array
///
/// @return: Adress of the element at @get int the given, sorted array
////////////////////////////////////////////////////////////////////////////////

int16_t *get_sorted(uint8_t cnt, int16_t *data, uint8_t get)
{
	for(uint8_t i = 0; i <= cnt-1; i ++)
		for(uint8_t j = i+1; j < cnt; j ++)
			if(data[i] > data[j]) 
   		{ 
	 			uint16_t data_temp = data[i]; 
   			data[i] = data[j]; 
   			data[j] = data_temp; 
   		}
   		
	return &data[get];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Returns the relative angle of the robot to the wall (not in degrees, but
/// as the differenceof the sensors on one side).
///
/// @param
///		dir: Wall in which direction?
///
///	@return
///		angle
//////////////////////////////////////////////////////////////////////////////////

uint8_t sm_scanFront = 0;
int16_t data_scanFront[DATA_SCANFRONT_SIZE + DATA_SCANFRONT_START];
int32_t rotinfo_old = 0;
uint8_t data_scanFrontIndex = 0;

#define DIST_FRONTFRONT_SCAN_OFFSET 50 //Distance in mm; how far is the sensor placed from the center of the robot (y-Component)

uint8_t robot_scanFront(uint8_t angle, uint8_t maxspeed)
{
	uint8_t returnvar = 1;

	/*switch(sm_scanFront)
	{
		case 0:
				if(!drive_rotate(-(angle/2), MAXSPEED))
				{
					rotinfo_old = um6.psi_t;
					data_scanFront[DATA_SCANFRONT_REG_ANGLE] = angle;
					data_scanFront[DATA_SCANFRONT_START] = (dist[LIN][FRONT][FRONT] + DIST_FRONTFRONT_SCAN_OFFSET); //First value...
					data_scanFrontIndex = DATA_SCANFRONT_START;
					sm_scanFront = 1;
				}
			break;

		case 1:

				if(!drive_rotate(angle, maxspeed))
				{
					sm_scanFront = 2;
				}

				//if(abs(um6.psi_t - rotinfo_old) > (angle / DATA_SCANFRONT_SIZE))
				if(abs(mot.d[LEFT].enc - rotinfo_old) > ((angle / DATA_SCANFRONT_SIZE)*ENC_DEGROTFAC))
				{
					rotinfo_old = um6.psi_t;
					rotinfo_old = mot.d[LEFT].enc;
					if(data_scanFrontIndex < (DATA_SCANFRONT_SIZE))
					{
						data_scanFront[data_scanFrontIndex] = (dist[LIN][FRONT][FRONT] + DIST_FRONTFRONT_SCAN_OFFSET);
						data_scanFrontIndex ++;
					}
				}

			break;

		case 2:
				if(!drive_turn(-(angle/2), 0))
				{
					sm_scanFront = 0;
					returnvar = 0;
				}
			break;
	}*/

	return returnvar;
}

////////////////////////////////////////////////////////////////
int16_t dist[2][4][4];

int16_t groundsens_l;
int16_t groundsens_r;
int16_t dist_down;
int16_t batt_raw;

#define ADC_BATTERY			1
#define SENS_IMPASSE_1		2
#define SENS_IMPASSE_2		3
#define SHARP_DOWN			5


#define SENS_ACTIVE_PORT PORTA
#define SENS_ACTIVE_PORTDIR DDRA
#define SENS_MID_PIN PA2
#define SENS_LEFT_PIN PA3
#define SENS_RIGHT_PIN PA4

#define MID 2 //LEFT = 0; RIGHT = 1 (main.h)


int8_t sm_anaSens = 0;

int16_t get_adc(uint8_t channel)
{
	ADMUX &= 0xE0;							//Clear bits MUX0-4
	ADMUX |= channel&0x07;			//Defines the new ADC channel to be read by setting bits MUX0-2
	ADCSRB = channel&(1<<3);		//Set MUX5
	ADCSRA |= (1<<ADSC);				//Starts a new conversion
	while(ADCSRA & (1<<ADSC));	//Wait until the conversion is done
	return ADCW;
}


uint8_t sm_groundSens = 0;

void get_groundSens(void)
{
    switch (sm_groundSens)
	{
	case 0:
		groundSens_setLED(255);
		sm_groundSens = 1;
		break;
	case 1:
		groundsens_r = get_adc(SENS_IMPASSE_1);
		groundsens_l = get_adc(SENS_IMPASSE_2);
		groundSens_setLED(0);
		sm_groundSens = 0;
	default:
		break;
    }
}

void get_analogSensors(void)
{
	batt_raw = 740;//get_adc(ADC_BATTERY);
	dist_down = get_adc(SHARP_DOWN);
	get_groundSens();
}
