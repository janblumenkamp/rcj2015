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

void setSensorStandby(uint8_t adr, uint8_t state)
{
	if(state)
		state = 1;

	i2c_start(adr);
	i2c_write(0xE8);
	i2c_write(state);
	i2c_stop();
}

int16_t getSensorDist(uint8_t adr)
{
	int16_t dist = -1;

	if((i2c_start(adr) == 0) && //Auslesen der Entfernung
	   (i2c_write(0x5E) == 0) &&
	   (i2c_rep_start(adr + I2C_READ) == 0))
	{
		int16_t lsb = i2c_readAck();
		int16_t msb = i2c_readNak();

		dist = (((lsb << 4) + msb)*10)/64;
	}
	i2c_stop();

	return dist;
}

//////////SM///////////////////
int16_t dists_raw[IRDIST_I2CDEV_NUM];
uint8_t toggle = 0;
uint8_t irDist_get(void)
{
	for(uint8_t i = 0; i < IRDIST_I2CDEV_NUM; i++)
	{
		setSensorStandby(pgm_read_byte(&i2c_addresses[i]), 0);
		//_delay_ms(50);
		dists_raw[i] = getSensorDist(pgm_read_byte(&i2c_addresses[i]));

		if(dists_raw[i] < -1 || dists_raw[i] > IRDIST_MAX)
			dists_raw[i] = IRDIST_MAX;

		setSensorStandby(pgm_read_byte(&i2c_addresses[i]), 1);
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

	if(um6.isRamp) //If on ramp set all front/ back sensors to max distance to not align
	{
		dist[LIN][BACK][LEFT] = IRDIST_MAX;
		dist[LIN][BACK][BACK] = IRDIST_MAX;
		dist[LIN][BACK][RIGHT] = IRDIST_MAX;
	}

	return 0;
}
