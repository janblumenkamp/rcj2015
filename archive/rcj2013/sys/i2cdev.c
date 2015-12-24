#include "i2cdev.h"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// I²C ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define I2CBUF_SIZE 5
uint8_t i2cbuf[I2CBUF_SIZE] = {0}; 

/////////////////////////////
int16_t mlx90614_l = 0;
int16_t mlx90614_r = 0;

///////Hilfsfunktionen////

uint8_t get_MLXData(uint8_t adr)
{
	uint8_t returnvar = 0;

	if(i2c_start(adr) == 0)
	{
		if(i2c_write(I2C_REG_MLX90614) == 0)
		{
			if(i2c_rep_start((adr)+1) == 0)
			{
				for(uint8_t i = 0; i<2; i++)
				{
					i2cbuf[i] = i2c_readAck();
				}
				i2cbuf[2] = i2c_readNak();

				if(i2c_stop() == 0)
				{
					returnvar = 0;
				}
				else
				{
					returnvar = 1;
				}
			}
			else
			{
				returnvar = 2;
			}
		}
		else
		{
			returnvar = 3;
		}
	}
	else
	{
		returnvar = 4;
	}

	return returnvar;
}
////////////void/////////////////////////////

uint8_t getIR(void)
{
	uint16_t tempdata = 0;
	uint8_t check_MLXData = 0;
	uint8_t returnvar = 0;
	
	check_MLXData = get_MLXData(I2C_MLX90614_L);

	if(check_MLXData == 0)
	{
		// This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
		tempdata = (((i2cbuf[1] & 0x007F) << 8) + i2cbuf[0]);
		tempdata = ((tempdata * 2)-1);
		mlx90614_l = (tempdata - 27315);
	}
	else
	{
		for(uint8_t i = 0; i < I2CBUF_SIZE; i++)
		{
			i2cbuf[i] = 0;
		}
		returnvar |= (1<<0);
	}

	check_MLXData = get_MLXData(I2C_MLX90614_R);

	if(check_MLXData == 0)
	{
		// This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
		tempdata = (((i2cbuf[1] & 0x007F) << 8) + i2cbuf[0]);
		tempdata = ((tempdata * 2)-1);
		mlx90614_r = (tempdata - 27315);
	}
	else
	{
		for(uint8_t i = 0; i < I2CBUF_SIZE; i++)
		{
			i2cbuf[i] = 0;
		}
		returnvar |= (1<<1);
	}

	return returnvar;
}
/////////////////////////SRF//////////////////////////////////////////////
#define SRF10_US_TO_MM 0.2 //Fakor zur Umrechnung: Laufzeit => mm

uint8_t srf_sm_manage = 2; //Damit die Sensoren nacheinander abgefragt werden

uint8_t srf10_state[4] = {0};
uint16_t srf10[4] = {0};

///////////Hilfsfunktionen///////////

uint8_t srf_startMeasurement(uint8_t adr, uint8_t mode)
{
	uint8_t returnvar = 0;

	if(i2c_start(adr) == 0)
	{
		if(i2c_write(0x00) == 0)
		{
			if(i2c_write(mode) == 0)
			{
				if(i2c_stop() == 0)
				{
				}
				else
				{
					returnvar = 1;
				}
			}
			else
			{
				returnvar = 2;
			}
		}
		else
		{
			returnvar = 3;
		}
	}
	else
	{
		returnvar = 4;
	}

	return returnvar;
}

uint8_t srf_queryMeasurement(uint8_t adr)
{
	uint8_t returnvar = 0;

	if(i2c_start(adr) == 0) //Abfragen: Ist die Messung beendet?
	{
		if(i2c_write(0x00) == 0)
		{
			if(i2c_rep_start(adr+1) == 0) //Read
			{
				if(i2c_readNak() != 255) //Messung beendet
				{
					returnvar = 1;
				}
				else
				{
					returnvar = 0;
				}
				if(i2c_stop() == 0)
				{
				}
				else
				{
					returnvar = 2;
				}
			}
			else
			{
				returnvar = 3;
			}
		}
		else
		{
			returnvar = 4;
		}
	}
	else
	{
		returnvar = 5;
	}

	return returnvar;
}

uint32_t srf_getMeasurement(uint8_t adr) 
{
	uint8_t lsb_srf = 0;
	uint8_t msb_srf = 0;

	uint8_t returnvar = 0;

	if(i2c_start(adr) == 0) //Auslesen der ENtfernung
	{
		if(i2c_write(0x02) == 0)
		{
			if(i2c_rep_start(adr+1) == 0) //Read
			{
				lsb_srf = i2c_readAck();
				msb_srf = i2c_readNak();
				if(i2c_stop() == 0)
				{
				}
				else
				{
					returnvar = 2;
				}
			}
			else
			{
				returnvar = 3;
			}
		}
		else
		{
			returnvar = 4;
		}
	}
	else
	{
		returnvar = 5;
	}

	return (((uint32_t)returnvar << 16) | ((uint16_t)lsb_srf << 8) | msb_srf);
}



//////////SM///////////////////

uint16_t getSRF_returnvar = 0; //Muss global sein, da sich Sensoren sonst gegenseitig überschreiben

uint16_t getSRF(void)
{
	uint32_t gM_srf = 0; //getMeasurement
	uint8_t gM_check = 0;

	switch(srf_sm_manage)
	{
		case 0:	switch(srf10_state[FRONT])
						{
							case 0:	if(srf_startMeasurement(I2C_SRF10_F, I2C_REG_SRF10_MEASERUEMENT_S) == 0)
											{
												srf10_state[FRONT] = 1; //Messung ok, nächster Schritt
												getSRF_returnvar &= ~(1<<0);
											}
											else
											{
												srf10_state[FRONT] = 0; //Beim nächsten Durchlauf nochmal
												srf_sm_manage = 1; //nächster Sensor
												getSRF_returnvar |= (1<<0); //etwas hat nicht funktioniert
											}
										break;
							case 1:	if(srf_queryMeasurement(I2C_SRF10_F) == 1)
											{
												srf10_state[FRONT] = 2;
												getSRF_returnvar &= ~(1<<1);
											}
											else
											{
												srf10_state[FRONT] = 0;
												srf_sm_manage = 1;
												getSRF_returnvar |= (1<<1);
											}
										break;
							case 2:	gM_srf = srf_getMeasurement(I2C_SRF10_F);
								
											gM_check = ((gM_srf & 0xff0000) >> 16);
								
											if(gM_check == 0)
											{
												srf10[FRONT] = (gM_srf & 0xffff);
												srf10_state[FRONT] = 3;
												getSRF_returnvar &= ~(1<<2);
											}
											else
											{
												srf10_state[FRONT] = 0;
												srf_sm_manage = 1;
												getSRF_returnvar |= (1<<2);
											}
										break;
							case 3:	srf_sm_manage = 1;
											srf10_state[FRONT] = 0;
										break;
						}
						break;
		case 1:	switch(srf10_state[RIGHT])
						{
							case 0:	if(srf_startMeasurement(I2C_SRF10_R, I2C_REG_SRF10_MEASERUEMENT_S) == 0)
											{
												srf10_state[RIGHT] = 1; //Messung ok, nächster Schritt
												getSRF_returnvar &= ~(1<<4);
											}
											else
											{
												srf10_state[RIGHT] = 0; //Beim nächsten Durchlauf nochmal
												srf_sm_manage = 2; //nächster Sensor
												getSRF_returnvar |= (1<<4); //etwas hat nicht funktioniert
											}
										break;
							case 1:	if(srf_queryMeasurement(I2C_SRF10_R) == 1)
											{
												srf10_state[RIGHT] = 2;
												getSRF_returnvar &= ~(1<<5);
											}
											else
											{
												srf10_state[RIGHT] = 0;
												srf_sm_manage = 2;
												getSRF_returnvar |= (1<<5);
											}
										break;
							case 2:	gM_srf = srf_getMeasurement(I2C_SRF10_R);
								
											gM_check = ((gM_srf & 0xff0000) >> 16);

											if(gM_check == 0)
											{
												srf10[RIGHT] = (gM_srf & 0xffff);
												srf10_state[RIGHT] = 3;
												getSRF_returnvar &= ~(1<<6);
											}
											else
											{
												srf10_state[RIGHT] = 0;
												srf_sm_manage = 2;
												getSRF_returnvar |= (1<<6);
											}
										break;
							case 3:	srf_sm_manage = 2;
											srf10_state[RIGHT] = 0;
										break;
						}
						break;
		case 2:	switch(srf10_state[BACK])
						{
							case 0:	if(srf_startMeasurement(I2C_SRF10_B, I2C_REG_SRF10_MEASERUEMENT_S) == 0)
											{
												srf10_state[BACK] = 1; //Messung ok, nächster Schritt
												getSRF_returnvar &= ~(1<<8);
											}
											else
											{
												srf10_state[BACK] = 0; //Beim nächsten Durchlauf nochmal
												srf_sm_manage = 3; //nächster Sensor
												getSRF_returnvar |= (1<<8); //etwas hat nicht funktioniert
											}
										break;
							case 1:	if(srf_queryMeasurement(I2C_SRF10_B) == 1)
											{
												srf10_state[BACK] = 2;
												getSRF_returnvar &= ~(1<<9);
											}
											else
											{
												srf10_state[BACK] = 0;
												srf_sm_manage = 3;
												getSRF_returnvar |= (1<<9);
											}
										break;
							case 2:	gM_srf = srf_getMeasurement(I2C_SRF10_B);
								
											gM_check = ((gM_srf & 0xff0000) >> 16);

											if(gM_check == 0)
											{
												srf10[BACK] = (gM_srf & 0xffff);
												srf10_state[BACK] = 3;
												getSRF_returnvar &= ~(1<<10);
											}
											else
											{
												srf10_state[BACK] = 0;
												srf_sm_manage = 3;
												getSRF_returnvar |= (1<<10);
											}
										break;
							case 3:	srf_sm_manage = 3;
											srf10_state[BACK] = 0;
										break;
						}
						break;
		case 3:	switch(srf10_state[LEFT])
						{
							case 0:	if(srf_startMeasurement(I2C_SRF10_L, I2C_REG_SRF10_MEASERUEMENT_S) == 0)
											{
												srf10_state[LEFT] = 1; //Messung ok, nächster Schritt
												getSRF_returnvar &= ~(1<<12);
											}
											else
											{
												srf10_state[LEFT] = 0; //Beim nächsten Durchlauf nochmal
												srf_sm_manage = 0; //nächster Sensor
												getSRF_returnvar |= (1<<12); //etwas hat nicht funktioniert
											}
										break;
							case 1:	if(srf_queryMeasurement(I2C_SRF10_L) == 1)
											{
												srf10_state[LEFT] = 2;
												getSRF_returnvar &= ~(1<<13);
											}
											else
											{
												srf10_state[LEFT] = 0;
												srf_sm_manage = 0;
												getSRF_returnvar |= (1<<13);
											}
										break;
							case 2:	gM_srf = srf_getMeasurement(I2C_SRF10_L);
								
											gM_check = ((gM_srf & 0xff0000) >> 16);

											if(gM_check == 0)
											{
												srf10[LEFT] = (gM_srf & 0xffff);
												srf10_state[LEFT] = 3;
												getSRF_returnvar &= ~(1<<14);
											}
											else
											{
												srf10_state[LEFT] = 0;
												srf_sm_manage = 0;
												getSRF_returnvar |= (1<<14);
											}
										break;
							case 3:	srf_sm_manage = 0;
											srf10_state[LEFT] = 0;
										break;
						}
						break;
	}

	return getSRF_returnvar;
}

//SRF Adresse ändern:
/*
uint8_t adr_old = 0xE0;
uint8_t adr_new = 0xE2;

i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xA0); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xAA); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xA5); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(adr_new); //Adresse ändern
i2c_stop();
*/
