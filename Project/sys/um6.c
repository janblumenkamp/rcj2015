////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
///////////////////////////////////um6.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Abfrage und Management der IMU via UART
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "um6.h"
#include "uart.h"
#include "system.h"
#include "main.h"
#include "bluetooth.h"

#define PT_HAS_DATA        0x80  //10000000
#define PT_IS_BATCH        0x40  //01000000

#define UM6_ACCEL_PROC_XY 			0x5E
#define UM6_EULER_PHI_THETA			0x62
#define UM6_EULER_PSI 					0x63
#define UM6_EULER_SCALE_FACTOR 	0.0109863
#define UM6_GYRO_SCALE_FACTOR 0.0610352
#define UM6_ACCL_SCALE_FACTOR 0.000183105
#define UM6_ZERO_GYROS 0xAC

#define UM6_GYRO_PROC_XY 0x5C
#define UM6_GYRO_PROC_Z 0x5D
#define UM6_ACCEL_PROC_XY 0x5E

///////////////////////////

#define UM6_DATA_READ 0
#define UM6_DATA_WRITE 1
#define UM6_DATA_CMD 2

//////////////////////////

void um6_init(UM6_t *_um6, void (*put_c)(unsigned char c), unsigned int (*get_c)(void))
{
	um6.put_c = put_c;
	um6.get_c = get_c;
}

//Hilfsfunktion:
void flushdata(UM6_t *_um6)
{
	for(uint8_t i = 0; i < 20; i++)
		_um6->get_c();
}
//////////////////

uint8_t um6_rwc(UM6_t *_um6, uint8_t um6_register, uint8_t batch, uint8_t r_w_c, uint16_t um6_result[], uint8_t data_size)
{
	uint8_t checksum0 = 0;
	uint8_t checksum1 = 0;
	uint16_t checksum = 0;
	uint8_t pt_is_batch = 0;
	if(batch > 0)
		pt_is_batch = PT_IS_BATCH;
	
	if(r_w_c == UM6_DATA_READ)
		checksum = ('s'+'n'+'p' + (pt_is_batch | batch) + um6_register);
	else if(r_w_c == UM6_DATA_WRITE)
		checksum = ('s'+'n'+'p' + (PT_HAS_DATA | pt_is_batch | batch) + um6_register);
	else if(r_w_c == UM6_DATA_CMD)
		checksum = ('s'+'n'+'p' + um6_register);
	checksum1 = checksum >> 8;
	checksum0 = checksum & 0xff;

	_um6->put_c('s');
	_um6->put_c('n');
	_um6->put_c('p');
	if(r_w_c == UM6_DATA_READ)
		_um6->put_c(pt_is_batch | batch);
	else if(r_w_c == UM6_DATA_WRITE)
		_um6->put_c(PT_HAS_DATA | pt_is_batch | batch);
	else //if(r_w_c == UM6_DATA_CMD)
		_um6->put_c(0);
	_um6->put_c(um6_register);
	_um6->put_c(checksum1);
	_um6->put_c(checksum0);

	uint8_t data[8] = {0};
	uint16_t data_sum = 0;

	//if((r_w_c == UM6_DATA_CMD) || (r_w_c == UM6_DATA_WRITE))
	//	_delay_ms(25); //UM6 braucht dann etwas Zeit zum Antworten

	uint8_t new_dat = _um6->get_c();
	if(new_dat & UART_NO_DATA)
	{
		//no data
		return 7;
	}
	else if(new_dat == 's')
	{
		if(_um6->get_c() == 'n')
		{
			if(_um6->get_c() == 'p')
			{
				uint8_t r_w_c_batch = _um6->get_c();
				if( ((r_w_c_batch == (PT_HAS_DATA | pt_is_batch | batch)) && (r_w_c == UM6_DATA_READ)) ||
						((r_w_c_batch == 0) && (r_w_c == UM6_DATA_WRITE)) || //== 0 wegen COMMAND_COMPLETE
						((r_w_c_batch == 0) && (r_w_c == UM6_DATA_CMD)))
				{
					if(_um6->get_c() == um6_register)
					{	
						uint8_t i_stop = 0;
						if(batch > 0)
							i_stop = batch;
						else
							i_stop = 4; //1 Register
						
						for(uint8_t i = 0; i<i_stop; i++)
						{
							data[i] = _um6->get_c();
							data_sum += data[i];
						}
						checksum1 = _um6->get_c();
						checksum0 = _um6->get_c();
						checksum = ((checksum1 << 8) | checksum0);
						if( ((checksum == ('s' + 'n' + 'p' + (PT_HAS_DATA | pt_is_batch | batch) + um6_register + data_sum)) && (r_w_c == UM6_DATA_READ)) ||
								((checksum == 0) && (r_w_c == UM6_DATA_WRITE)) ||
								((checksum == 0) && (r_w_c == UM6_DATA_CMD)))
						{
							for(uint8_t i = 0; i < data_size; i++)
								um6_result[i] = (data[(i*2)+1] | (data[(i*2)]<<8));
							
							return 0;
						}
						else
						{
							flushdata(_um6);
							return 1;
						}
					}
					else
					{
						flushdata(_um6);
						return 2;
					}
				}
				else
				{
					flushdata(_um6);
					return 3;
				}
			}
			else
			{
				flushdata(_um6);
				return 4;
			}
		}
		else
		{
			flushdata(_um6);
			return 5;
		}
	}
	else
	{
		flushdata(_um6);
		return 6;
	}
}

///////////////////////////////////////////////////

#define UM6_DATA_BUFFER_SIZE 4

#define UM6_T_ROTDIFF 300 //ab dieser DIfferenz zwischen alt/neu ist eine Umdrehung

#define UM6_GYR_MAXDRIFT 2 //Maximal zugelassener Drift bei Stillstand
#define UM6_GYR_DRIFT_CNT_TOP 80

uint8_t um6_getUM6(UM6_t *_um6)
{
	uint16_t um6_data[UM6_DATA_BUFFER_SIZE]; //Wenn batch abgefragt wird, werden hier hintereinander die Daten abgelegt
	
	uint8_t returnvar = 0;
	
	////////////////////////////////

	/*if(um6_rwc(UM6_ACCEL_PROC_XY, 0, UM6_DATA_READ, um6_data, UM6_DATA_BUFFER_SIZE) != 0)
	{
		returnvar |= (1<<2);
	}
	else
	{
		um6_accl_x = (um6_data[0]*UM6_ACCL_SCALE_FACTOR);
		um6_accl_y = (um6_data[1]*UM6_ACCL_SCALE_FACTOR);
	}*/
	
/////////////////////////////
	if(um6_rwc(_um6, UM6_EULER_PHI_THETA, 8, UM6_DATA_READ, um6_data, UM6_DATA_BUFFER_SIZE) != 0)
	{
		returnvar |= (1<<0);
	}
	else
	{
		_um6->phi = (um6_data[0]*UM6_EULER_SCALE_FACTOR);
		if(_um6->phi > 180)
			_um6->phi -= 360;

		_um6->theta = (um6_data[1]*UM6_EULER_SCALE_FACTOR);
		if(_um6->theta > 180)
			_um6->theta -= 360;

		_um6->psi = (um6_data[2]*UM6_EULER_SCALE_FACTOR);
		if(_um6->psi > 180)
			_um6->psi -= 360;
	////////////

		if((_um6->phi_old - _um6->phi) > UM6_T_ROTDIFF)
			_um6->phi_t += 360;
		else if((_um6->phi_old - _um6->phi) < -(UM6_T_ROTDIFF))
			_um6->phi_t -= 360;
		_um6->phi_t += (_um6->phi - _um6->phi_old);
		_um6->phi_old = _um6->phi;
		
		if((_um6->theta_old - _um6->theta) > UM6_T_ROTDIFF)
			_um6->theta_t += 360;
		else if((_um6->theta_old - _um6->theta) < -(UM6_T_ROTDIFF))
			_um6->theta_t -= 360;
		_um6->theta_t += (_um6->theta - _um6->theta_old);
		_um6->theta_old = _um6->theta;
		
		if((_um6->psi_old - _um6->psi) > UM6_T_ROTDIFF)
			_um6->psi_t += 360;
		else if((_um6->psi_old - _um6->psi) < -(UM6_T_ROTDIFF))
			_um6->psi_t -= 360;
		_um6->psi_t += (_um6->psi - _um6->psi_old);
		_um6->psi_old = _um6->psi;
	}
////////////////////////////////

	if(um6_rwc(_um6, UM6_GYRO_PROC_XY, 8, UM6_DATA_READ, um6_data, UM6_DATA_BUFFER_SIZE) == 0)
	{
		_um6->gyr_x = (um6_data[0]*UM6_GYRO_SCALE_FACTOR);
		if(_um6->gyr_x > 2000)
			_um6->gyr_x = -(4000 - _um6->gyr_x);
			
		_um6->gyr_y = (um6_data[1]*UM6_GYRO_SCALE_FACTOR);
		if(_um6->gyr_y > 2000)
			_um6->gyr_y = -(4000 - _um6->gyr_y);
			
		_um6->gyr_z = (um6_data[2]*UM6_GYRO_SCALE_FACTOR);
		if(_um6->gyr_z > 2000)
			_um6->gyr_z = -(4000 - _um6->gyr_z);
		
		if(((_um6->gyr_z < -UM6_GYR_MAXDRIFT) || (_um6->gyr_z > UM6_GYR_MAXDRIFT)) &&
				(mot.d[LEFT].speed.is == 0) && (mot.d[RIGHT].speed.is == 0)) //Gyrobewegung trotz Drift
		{
			if(_um6->gyr_drift_cnt > UM6_GYR_DRIFT_CNT_TOP)
				returnvar |= (1<<2);
			else
				_um6->gyr_drift_cnt++;
		}
		else	_um6->gyr_drift_cnt = 0;
	}
	else returnvar |= (1<<1);

	return returnvar;
}

#define UM6_CMD_DATA_BUFFER_SIZE 4
uint8_t um6_gyroZeroRate(UM6_t *_um6)
{
	uint16_t um6_cmd_data[UM6_CMD_DATA_BUFFER_SIZE];

	return um6_rwc(_um6, UM6_ZERO_GYROS, 0, UM6_DATA_CMD, &um6_cmd_data[0], UM6_CMD_DATA_BUFFER_SIZE);
}

void um6_checkRamp(UM6_t *um6)
{
	if((um6->theta_t) % 360 > 15)
	{
		um6->isRamp = 1;
	}
	else if((um6->theta_t) % 360 < -15)
	{
		um6->isRamp = 1;
	}
	else
	{
		um6->isRamp = 0;
	}
}
