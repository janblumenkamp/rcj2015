/* 
 * ****************************************************************************
 * RP6 ROBOT SYSTEM - RP6 CONTROL M32 Examples
 * ****************************************************************************
 */

/*****************************************************************************/
// Includes:

#include "RP6Control_I2CMasterLib.h" 		

/*****************************************************************************/
// Sensors/ADCs: 

// ADCs:
uint16_t adcBat;
uint16_t adcMotorCurrentLeft;
uint16_t adcMotorCurrentRight;
uint16_t adcLSL;
uint16_t adcLSR;
uint16_t adc0;
uint16_t adc1;

//Accelerometer:
int16_t threedbs_x;
int16_t threedbs_y;
int16_t threedbs_z;

//IR (MLX90614)
int16_t mlx90614_l;
int16_t mlx90614_r;

// Measured Speed:
uint8_t mleft_speed;
uint8_t mright_speed;

// Distance
uint16_t mleft_dist;
uint16_t mright_dist;

// Desired Speed:
uint8_t mleft_des_speed;
uint8_t mright_des_speed;

// Power
uint8_t mleft_power;
uint8_t mright_power;

uint8_t sensorBuf[24]; 

/**
 * In order to use the same register names as in the RP6Lib, this
 * function reads all ADC channels and all motor parameters into
 * the same values as in the RP6Lib. 
 * Of course this function needs some time to read all these
 * 24 registers via the I2C Bus. 
 */
void getAllSensors(void)
{
	i2c_start(I2C_RP6_BASE_ADR+0);
	i2c_write(I2C_REG_POWER_LEFT);
	i2c_rep_start(I2C_RP6_BASE_ADR+1);

	for(uint8_t i = 0; i<23; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[23] = i2c_readNak();

	i2c_stop();

	mleft_power = sensorBuf[0];
	mright_power = sensorBuf[1];
	mleft_speed = sensorBuf[2];
	mright_speed = sensorBuf[3];
	mleft_des_speed = sensorBuf[4];
	mright_des_speed = sensorBuf[5];
	mleft_dist = sensorBuf[6] + (sensorBuf[7]<<8);
	mright_dist = sensorBuf[8] + (sensorBuf[9]<<8);
	adcLSL = sensorBuf[10] + (sensorBuf[11]<<8);
	adcLSR = sensorBuf[12] + (sensorBuf[13]<<8);
	adcMotorCurrentLeft = sensorBuf[14] + (sensorBuf[15]<<8);
	adcMotorCurrentRight = sensorBuf[16] + (sensorBuf[17]<<8);
	adcBat = sensorBuf[18] + (sensorBuf[19]<<8);
	adc0 = sensorBuf[20] + (sensorBuf[21]<<8);
	adc1 = sensorBuf[22] + (sensorBuf[23]<<8);
}

/**
 *
 */
void getLightSensors(void)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(I2C_REG_ADC_LSL_L);
	i2c_rep_start(I2C_RP6_BASE_ADR+1);

	for(uint8_t i = 0; i<3; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[3] = i2c_readNak();

	i2c_stop();

	adcLSL = sensorBuf[0] + (sensorBuf[1]<<8);
	adcLSR = sensorBuf[2] + (sensorBuf[3]<<8);
}

/**
 *
 */
void getADCs(void)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(I2C_REG_ADC_ADC0_L);
	i2c_rep_start(I2C_RP6_BASE_ADR+1);

	for(uint8_t i = 0; i<3; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[3] = i2c_readNak();

	i2c_stop();

	adc0 = sensorBuf[0] + (sensorBuf[1]<<8);
	adc1 = sensorBuf[2] + (sensorBuf[3]<<8);
}

/**
 *
 */
void get3DBS(void)
{
	i2c_start(I2C_3DBS_ADR);
	i2c_write(I2C_REG_3DBS);
	i2c_rep_start(I2C_3DBS_ADR+1);

	for(uint8_t i = 0; i<5; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[5] = i2c_readNak();

	i2c_stop();

	threedbs_x = sensorBuf[0] + (sensorBuf[1]<<8);
	threedbs_y = sensorBuf[3];
	threedbs_z = sensorBuf[5];
}

/*
 *
 */
void getIR(void)
{
	i2c_start_wait(I2C_MLX90614_L);
	i2c_write(I2C_REG_MLX90614);
	i2c_rep_start((I2C_MLX90614_L)+1);

	for(uint8_t i = 0; i<2; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[2] = i2c_readNak();

	i2c_stop();

	// This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
  int16_t tempdata = (((sensorBuf[1] & 0x007F) << 8) + sensorBuf[0]);
	tempdata = (tempdata * 2)-1;
	mlx90614_l = tempdata - 27315;


	i2c_start_wait(I2C_MLX90614_R);
	i2c_write(I2C_REG_MLX90614);
	i2c_rep_start((I2C_MLX90614_R)+1);

	for(uint8_t i = 0; i<2; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[2] = i2c_readNak();

	i2c_stop();

	// This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
  tempdata = (((sensorBuf[1] & 0x007F) << 8) + sensorBuf[0]);
	tempdata = (tempdata * 2)-1;
	mlx90614_r = tempdata - 27315;
}

/*
 *
 */
void getSRF(void)
{
	switch(srf_02_state)
	{
		case 0:
			i2c_start(I2C_SRF02);
			i2c_write(0x00);
			i2c_write(I2C_REG_SRF02_MEASERUEMENT);
			i2c_stop();
			srf_02_state = 1;
			timer_sm_srf = TIMER_SM_SRF;
			break;
		case 1:
			if(timer_sm_srf == 0)
			{
				i2c_start(I2C_SRF02);
				i2c_write(0x02);
				i2c_rep_start(I2C_SRF02+1); //Read
				uint8_t lsb_srf = i2c_readAck();
				uint8_t msb_srf = i2c_readNak();
				i2c_stop();
				srf_02 = (lsb_srf <<8) + msb_srf;

				srf_02_state = 0;
			}
			break;
		}
}
/*
 *
 */
void getDists(void)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(I2C_REG_DIST_LEFT_L);
	i2c_rep_start(I2C_RP6_BASE_ADR+1);

	for(uint8_t i = 0; i<3; i++)
	{
		sensorBuf[i] = i2c_readAck();
	}
	sensorBuf[3] = i2c_readNak();

	i2c_stop();

	mleft_dist = sensorBuf[0] + (sensorBuf[1]<<8);
	mright_dist = sensorBuf[2] + (sensorBuf[3]<<8);
}

/*****************************************************************************/
/*****************************************************************************/
void baseWatchdog_reset(void)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(0);
	i2c_write(CMD_SET_WDT_RESET);
	i2c_write(true);
	i2c_stop();
}

// Movement functions:

/**
 * Move at speed function - just the same as with RP6Lib!
 */
void moveAtSpeed(uint8_t desired_speed_left, uint8_t desired_speed_right)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(0);
	i2c_write(CMD_MOVE_AT_SPEED);
	i2c_write(desired_speed_left);
	i2c_write(desired_speed_right);
	i2c_stop();
}

/**
 * Change direction function - just the same as with RP6Lib!
 */
void changeDirection(uint8_t dir)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(0);
	i2c_write(CMD_CHANGE_DIR);
	i2c_write(dir);
	i2c_stop();
}

/**
 * Stop function - just the same as with RP6Lib!
 */
void stop(void)
{
	i2c_start_wait(I2C_RP6_BASE_ADR);
	i2c_write(0);
	i2c_write(CMD_STOP);
	i2c_stop();
}
