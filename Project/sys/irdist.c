////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////i2cdev.c///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Abfrage/Management der I²C Geräte (Devices)
//	- Temperatursensoren
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "i2cmaster.h"
#include "irdist.h"
#include "main.h"
#include "funktionen.h"

const uint8_t i2c_addresses[IRDIST_I2CDEV_NUM] PROGMEM = {
	IRDIST_I2CADR_B_B,
	IRDIST_I2CADR_L_B,
	IRDIST_I2CADR_B_L,
	IRDIST_I2CADR_F_L,
	IRDIST_I2CADR_L_F,
	IRDIST_I2CADR_F_F,
	IRDIST_I2CADR_R_F,
	IRDIST_I2CADR_F_R,
	IRDIST_I2CADR_B_R,
	IRDIST_I2CADR_R_B
};

//////////SM///////////////////
int16_t dists_raw[IRDIST_I2CDEV_NUM];

uint8_t irDist_get(void)
{
	for(uint8_t i = 0; i < IRDIST_I2CDEV_NUM; i++)
	{
		if((i2c_start(pgm_read_byte(&i2c_addresses[i])) == 0) && //Auslesen der Entfernung
		   (i2c_write(0x5E) == 0) &&
		   (i2c_rep_start(pgm_read_byte(&i2c_addresses[i])+I2C_READ) == 0))
		{
			int16_t lsb = i2c_readAck();
			int16_t msb = i2c_readNak();

			dists_raw[i] = (((lsb << 4) + msb)*10)/64;
		}
		else
		{
			dists_raw[i] = -1;
		}
		i2c_stop();

		if(dists_raw[i] == -385)
			dists_raw[i] = 600;
	}

	dist[LIN][BACK][BACK] = dists_raw[0];
	dist[LIN][LEFT][BACK] = dists_raw[1];
	dist[LIN][BACK][LEFT] = dists_raw[2];
	dist[LIN][FRONT][LEFT] = dists_raw[3];
	dist[LIN][LEFT][FRONT] = dists_raw[4];
	dist[LIN][FRONT][FRONT] = dists_raw[5];
	dist[LIN][RIGHT][FRONT] = dists_raw[6];
	dist[LIN][FRONT][RIGHT] = dists_raw[7];
	dist[LIN][BACK][RIGHT] = dists_raw[8];
	dist[LIN][RIGHT][BACK] = dists_raw[9];

	return 0;
}
