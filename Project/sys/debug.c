////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////debug.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///	Debug (Bluetooth)
/////////////////////////////////////////////////////////////////////////////
/// Protocol for PC User interface
/// [Startseq][Length][Checksum][ID][Data]
///
/// [Startseq]: ["PCUI_MSG"] (8 chars)
/// [Lenght]: [{b2},{b1_lsb}] (16bit; 2 chars)
/// [checksum]: (Sum of all Data chars) [{b4},{b3},{b2},{b1_lsb}] (32bit; 4 chars)
/// [ID]: (3 chars)
///		["MPD"]: Map Data (7 chars). Rob->PC
///					- size x (1byte)
///					- size y "
///					- size z "
///					- rob x  (1byte)
///					- rob y  "
///					- rob z  "
///					- dir    "
///		["MAP"]: Map transmission (size x * 2 + 2 bytes). Rob->PC
///					- pos z (1 byte)
///					- pos y "
///					- wall south (1 byte)
///					- wall west "
///	[Data]: [Lenght] chars
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "debug.h"
#include "bluetooth.h"
#include "maze.h"
#include "mazefunctions.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////
/// \brief pcui_sendMsg
///			Sends a message (definition: see protocol) via bluetooth to the
///			PC (including calculating and sending checksum)
/// \param id
///			ID of the message (see protocol)
/// \param length
///			Length of message (in bytes)
/// \param msg
///			Message
///
void pcui_sendMsg(char *id, uint16_t length, char *msg)
{
	int32_t checksum = 0;
	char len[2];
	char chk[4];

	len[0] = (char) (length & 0x00ff);
	len[1] = (char) ((length & 0xff00) >> 8);

	for(uint16_t i = 0; i < length; i++)
		checksum += msg[i];

	chk[0] = (char) (checksum & 0x000000ff);
	chk[1] = (char) ((checksum & 0x0000ff00) >> 8);
	chk[2] = (char) ((checksum & 0x00ff0000) >> 16);
	chk[3] = (char) ((checksum & 0xff000000) >> 24);


	out_puts_l(&str_pcui, "PCUI_MSG", 8); //Startseq
	out_puts_l(&str_pcui, len, 2);
	out_puts_l(&str_pcui, chk, 4);
	out_puts_l(&str_pcui, id, 3);
	out_puts_l(&str_pcui, msg, length);
}

void pcui_sendMPD(POS *rob)
{
	char mpd[7]; //MaPData message container array

	mpd[0] = MAZE_SIZE_X;
	mpd[1] = MAZE_SIZE_Y;
	mpd[2] = MAZE_SIZE_Z;
	mpd[3] = rob->pos.x;
	mpd[4] = rob->pos.y;
	mpd[5] = rob->pos.z;
	mpd[6] = rob->dir;

	//out_puts_l(&slamUI, "\e[0m", 5); //VT100: clear all colorsettings
	pcui_sendMsg((char *)"MPD", 7, mpd); //Send mapdata
}

COORD sendMap_coord;

void pcui_sendMAP(void)
{
	TILE thisTile;
	char mapData[MAZE_SIZE_X * 2 + 2];

	mapData[0] = sendMap_coord.z;
	mapData[1] = sendMap_coord.y;
	for(sendMap_coord.x = 0; sendMap_coord.x < MAZE_SIZE_X; sendMap_coord.x ++)
	{
		thisTile = *maze_getTile(&sendMap_coord);

		mapData[2 + (sendMap_coord.x * 2)] = (char)(thisTile.wall_s + 127);
		mapData[2 + (sendMap_coord.x * 2) + 1] = (char)(thisTile.wall_w + 127);
	}
	pcui_sendMsg((char *)"MAP", MAZE_SIZE_X * 2 + 2, mapData); //Send mapdata

	sendMap_coord.y ++;
	if(sendMap_coord.y == MAZE_SIZE_Y)
	{
		sendMap_coord.y = 0;
		sendMap_coord.z ++;
		if(sendMap_coord.z == MAZE_SIZE_Z)
		{
			pcui_sendMPD(&robot);
			sendMap_coord.z = 0;
		}
	}
}
