#include <avr/io.h>
#include <util/delay.h>

#include "main.h"
#include "adns3080.h"
#include "spimaster.h"

#define PORT_SS    PORTC
#define DDR_SS     DDRC
#define DD_SS      DDC6
#define PORT_RES    PORTC
#define DDR_RES     DDRC
#define DD_RES      DDC5


void adns_init(void)
{
	spi_init(); //Only works after u8g was initialized with SPI Backup!!!

	DDR_SS |= (1<<DD_SS); //Set slave select as output
	adns_setSlaveSelect(0); //Un-select device

	DDR_RES |= (1<<DD_RES); //Set reset as output

	adns_reset();
}

/////////////////////////////////////////////////////
/// \brief adns_setSlaveSelect
///		Select/de-select adns for SPI communication (Pin low: selected/active)
/// \param select
///
void adns_setSlaveSelect(uint8_t select)
{
	if(select)
		PORT_SS &= ~(1<<DD_SS); //Set ss to low (selected)
	else
		PORT_SS |= (1<<DD_SS); //Set ss to high (not selected)
}

/////////////////////////////////////////////////////
/// \brief adns_reset
///		Resets device
void adns_reset(void)
{
	PORT_SS |= (1<<DD_SS);
	_delay_us(10);
	PORT_SS &= ~(1<<DD_SS);
}

///////////////////////////////////////////////////////
/// \brief adns_readRegister
///		Reads data from adns
/// \param reg
///		Register to read data from
/// \return
///		value
uint8_t adns_readRegister(uint8_t reg)
{
	adns_setSlaveSelect(1);

	spi_fast_shift(reg); // send the device the register you want to read:

	_delay_us(50);

	uint8_t result = spi_fast_shift(0x00);

	_delay_us(50);

	adns_setSlaveSelect(0);

	return result;
}

///////////////////////////////////////////////////////
/// \brief adns_writeRegister
///		Writes a value into a specified register of adns
/// \param reg
///		Register to write value to
/// \param val
///		Value to write
void adns_writeRegister(uint8_t reg, uint8_t val)
{
	adns_setSlaveSelect(1);

	spi_fast_shift(reg | 0x80);  // send register address

	_delay_us(50);

	spi_fast_shift(val); //send data

	adns_setSlaveSelect(0);
}

void adns_getFlowData(struct adnsData *data)
{
	data->quality = adns_readRegister(ADNS3080_SQUAL);

	// check for movement, update x,y values
	uint8_t motion_reg = adns_readRegister(ADNS3080_MOTION);
	//_overflow = ((motion_reg & 0x10) != 0);  // check if we've had an overflow
	if((motion_reg & 0x80) != 0)
	{
		data->delta_x = adns_readRegister(ADNS3080_DELTA_X);
		data->delta_y = adns_readRegister(ADNS3080_DELTA_Y);
	}
	else
	{
		data->delta_x = 0;
		data->delta_y = 0;
	}
}
