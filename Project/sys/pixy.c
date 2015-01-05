////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////pixy.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	CMUcam (PixyCam)
//	- Abfragen über I²C
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "i2cmaster.h"
#include "pixy.h"
#include "main.h"
#include "bluetooth.h"
#include "system.h"

struct _PIXY_BLOCK pixy_blocks[NUMBER_OF_PIXY_BLOCKS];


////////////////////////////////////////////////////////////////////////////////
/// \brief pixy_getWord
///			Reads a single word by i2c from the pixy
/// \param adr
/// \return
///			Returns the word
////////////////////////////////////////////////////////////////////////////////

#define PIXY_TESTARR 1200

uint16_t index_pixy = 0;
uint16_t pixy_buf[PIXY_TESTARR];

uint16_t pixy_getWord(uint8_t adr)
{
	uint16_t word = 0;

	i2c_start(adr + I2C_READ);
	word += i2c_readAck(); //msb
	word += (uint16_t)(i2c_readNak()<<8); //lsb

	i2c_stop();

	/*if(index_pixy < PIXY_TESTARR)
	{
		pixy_buf[index_pixy] = word;
		index_pixy ++;
	}*/

	//if(get_incrOk())
	//{
		/*signed int i;
		char digit;

		for (i = 28; i >= 0; i -= 4)
		{
			digit = (word >> i) & 0x0000000f; // eine hex-Stelle extrahieren
			digit = (digit > 9) ? (digit + 'A' - 10) : (digit + '0'); // in ASCII-Zeichen konvertieren
			bt_putCh(digit); // oder was immer die UART-Funktion in Deinem compiler ist
		}

		bt_putStr(" ");*/
	//}

	//bt_putLong(word);
	//bt_putCh(' ');

	return word;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief pixy_getByte
///			Reads a single byte by i2c from the pixy
/// \param adr
///			Pixy address
/// \return
///			Returns the byte
////////////////////////////////////////////////////////////////////////////////

uint8_t pixy_getByte(uint8_t adr)
{
	uint8_t pixy_byte = 0;

	i2c_start(adr + I2C_READ);
	pixy_byte += i2c_readNak();

	i2c_stop();

	return pixy_byte;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief pixy_get
///			Manages the Pixy statemachine
/// \param void
/// \return
///			Returns the state
////////////////////////////////////////////////////////////////////////////////

uint8_t sm_pixy = 0;
uint8_t pixy_number_of_blocks = 0;

uint8_t pixy_get(void)
{
	uint16_t pixy_word = 0;

	/*if(index_pixy == PIXY_TESTARR)
	{
		index_pixy ++;
		for(uint16_t i = 0; i < PIXY_TESTARR; i++)
		{
			bt_putLong(pixy_buf[i]); bt_putCh(' ');
		}
	}
	if(get_incrOk())
		index_pixy = 0;*/

	switch(sm_pixy)
	{
		case GET_START_1:
		case GET_START_2:

				pixy_number_of_blocks = 0;

				pixy_word = pixy_getWord(PIXY_I2C_ADR);

				if(pixy_word == PIXY_START_WORD)
					sm_pixy ++;
				else
				{
					sm_pixy = GET_START_1;

					if(pixy_word == PIXY_START_WORDX)
						pixy_getByte(PIXY_I2C_ADR); //resync
				}


				if(sm_pixy == GET_BLOCKS && get_incrOk()) bt_putStr("Found Start!\n\n\n");
			break;

		case GET_BLOCKS:

				for(uint8_t blockCnt = 0; blockCnt < NUMBER_OF_PIXY_BLOCKS; blockCnt ++)
				{
					pixy_word = pixy_getWord(PIXY_I2C_ADR); //represents checksum or the beginning of the next frame

					if(pixy_word == PIXY_START_WORD) //Two times startword means the beginning of a new frame. Read out next time.
					{
						if(get_incrOk()) {bt_putStr("\nSucceed. "); bt_putLong(pixy_number_of_blocks); bt_putStr(" objects.\nNew Frame\n\n");}
						break;
					}
					else if(pixy_word == 0) //checksum is 0 -> there are no data to get.
					{
						sm_pixy = GET_START_1;
						if(get_incrOk()) bt_putStr("No Data.\n");
						break;
					}

					pixy_number_of_blocks = blockCnt+1;

					pixy_blocks[blockCnt].signature = pixy_getWord(PIXY_I2C_ADR);
					pixy_blocks[blockCnt].x = pixy_getWord(PIXY_I2C_ADR);
					pixy_blocks[blockCnt].y = pixy_getWord(PIXY_I2C_ADR);
					pixy_blocks[blockCnt].width = pixy_getWord(PIXY_I2C_ADR);
					pixy_blocks[blockCnt].height = pixy_getWord(PIXY_I2C_ADR);

					pixy_word -= pixy_blocks[blockCnt].signature; //Control checksum...
					pixy_word -= pixy_blocks[blockCnt].x;
					pixy_word -= pixy_blocks[blockCnt].y;
					pixy_word -= pixy_blocks[blockCnt].width;
					pixy_word -= pixy_blocks[blockCnt].height;

					if(pixy_word != 0) //Data were not ok...
					{
						if(get_incrOk()) {bt_putLong(pixy_word); bt_putStr(": Checksum error\n");}
						pixy_number_of_blocks = 0;
						sm_pixy = GET_START_1; //Return to the start, find next block.
						break;
					}


					if(pixy_getWord(PIXY_I2C_ADR) != PIXY_START_WORD) //Arrived at new block (otherwise something went wrong...)
					{
						//if(get_incrOk())
							bt_putStr("End.\n");
						sm_pixy = GET_START_1; //Return to the start, find next block.
						break;
					}
					else
						if(get_incrOk()) bt_putStr("Next Object.\n");
				}


			break;

		default:
			break;
	}

	return sm_pixy;
}
