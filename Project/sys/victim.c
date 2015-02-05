////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////victim.c//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Opfer
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "victim.h"
#include "main.h"
#include "system.h"
#include "drive.h"
#include "i2cdev.h"
#include "funktionen.h"
#include "bluetooth.h"
#include "maze.h"
#include "mazefunctions.h"

VICTIMBUFFER victimBuf[NUMBER_OF_MLX];

uint32_t victim_enc_zero[NUMBER_OF_MLX];

void victim_init(void)
{
	for(uint8_t dir = 0; dir < 2; dir++)
	{
		for(uint8_t j = 0; j < VICTIMBUFFER_SIZE; j++)
		{
			victimBuf[dir].value[j] = VICTIMBUF_UNUSED;
		}
		victimBuf[dir].next = 0;
		victimBuf[dir].lowest = VICTIMBUF_UNUSED;
		victimBuf[dir].highest = 0;
		victim_enc_zero[dir] = 0;
	}
}

//Has to be called when there are new temperature infos (after the temperature sensors were read out)

void victim_scan(void)
{
	if(maze_solve_state_path != VIC_DEPL && mot.d[LEFT].speed.to != 0 && mot.d[RIGHT].speed.to != 0) //No kit deployment, robot driving
	{
		for(uint8_t dir = 0; dir < NUMBER_OF_MLX; dir++)
		{
			if((mlx90614[dir].is > victimBuf[dir].value[victimBuf[dir].next]) || (victimBuf[dir].value[victimBuf[dir].next] == VICTIMBUF_UNUSED))
			{
				victimBuf[dir].value[victimBuf[dir].next] = mlx90614[dir].is;
			}

			if(mot.d[dir].enc_abs > (victim_enc_zero[dir] + victimBuf[dir].next * ENC_FAC_CM_LR)) //1cm driven
			{
				victimBuf[dir].next ++;
				if(victimBuf[dir].next >= VICTIMBUFFER_SIZE)
				{
					victimBuf[dir].next = 0;
					victim_enc_zero[dir] = mot.d[dir].enc_abs;
				}

				victimBuf[dir].value[victimBuf[dir].next] = mlx90614[dir].is;
			}

			victimBuf[dir].lowest = VICTIMBUF_UNUSED;
			for(uint8_t j = 0; j < VICTIMBUFFER_SIZE; j++)
			{
				if((victimBuf[dir].value[j] < victimBuf[dir].lowest) && (victimBuf[dir].value[j] != VICTIMBUF_UNUSED))
					victimBuf[dir].lowest = victimBuf[dir].value[j];
			}

			victimBuf[dir].highest = -VICTIMBUF_UNUSED;
			for(uint8_t j = 0; j < VICTIMBUFFER_SIZE; j++)
			{
				if((victimBuf[dir].value[j] > victimBuf[dir].highest) && (victimBuf[dir].value[j] != VICTIMBUF_UNUSED))
					victimBuf[dir].highest = victimBuf[dir].value[j];
			}
		}
	}
}

int16_t victim_BufGetRaw(uint8_t dir)
{
	return victimBuf[dir].value[victimBuf[dir].next] - victimBuf[dir].lowest;
}

int16_t victim_BufGetMaxDiff(uint8_t dir)
{
	return victimBuf[dir].highest - victimBuf[dir].lowest;
}

uint8_t victim_BufIsVic(uint8_t dir)
{
	if(victim_BufGetRaw(dir) > mlx90614[RIGHT].th)
	//if(mlx90614[dir].is > mlx90614[RIGHT].th)
		return TRUE;
	else
		return FALSE;
}
