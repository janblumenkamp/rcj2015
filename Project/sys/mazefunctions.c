////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////mazefunctions.c///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	All relevant mapping/navigation functions: 
//	- Access to the map (change/read walls)
//
//	The actual maze solving algorithms is in the maze.c file!
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "maze.h"
#include "mazefunctions.h"
#include "main.h"
#include "bluetooth.h"

////////////////////////////////////////////////////////////////////////////////
// Makes sure there is no direction-overflow (adapts periodical direction)
//
// Param:
// @dir:	desired direction
//
// @return: adapted direction
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_alignDir(uint8_t dir)
{
	while(dir > WEST)
		dir -= WEST;
	
	return dir;
}

////////////////////////////////////////////////////////////////////////////////
// Compares two structs of the type COORD
//
// Param:
// @dir:	desired direction
//
// @return: TRUE when identical, otherwise FALSE
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_cmpCoords(COORD *_coordA, COORD *_coordB)
{
	if((_coordA->x - _coordB->x == 0) &&
		 (_coordA->y - _coordB->y == 0) &&
		 (_coordA->z - _coordB->z == 0))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Sets the given Tile at the given Position
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @*_tile:		Pointer to the Structure with the tile-information to set on the
//						given coords
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_setTile(COORD *_coord, TILE *_tile)
{
	COORD c = *_coord;
	
	if(maze_adaptOffset(&c))
		maze[c.x][c.y][c.z] = *_tile;
}

////////////////////////////////////////////////////////////////////////////////
// Returns a pointer to the static TILE at the given position.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
//
// @return: Pointer to the TILE information at the given position
////////////////////////////////////////////////////////////////////////////////

TILE *maze_getTile(COORD *_coord)
{
	TILE *_tile = NULL;
	COORD c = *_coord;
	
	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
		_tile = &maze[c.x][c.y][c.z];
	
	return _tile;
}

////////////////////////////////////////////////////////////////////////////////
// Clears the tile at the given position.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
//
// @return: void
////////////////////////////////////////////////////////////////////////////////
void maze_clearTile(COORD *_coord, TILE *_clear)
{
	COORD c = *_coord;

	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		if(_clear->beenthere)
				maze[c.x][c.y][c.z].beenthere = 0;
		if(_clear->ground)
			maze[c.x][c.y][c.z].ground = 0;
		if(_clear->wall_s)
			maze[c.x][c.y][c.z].wall_s = 0;
		if(_clear->wall_w)
			maze[c.x][c.y][c.z].wall_w = 0;
		if(_clear->obstacle)
			maze[c.x][c.y][c.z].obstacle = 0;
		if(_clear->depthsearch)
			maze[c.x][c.y][c.z].depthsearch = 0xff;
		if(_clear->victim_n)
			maze[c.x][c.y][c.z].victim_n = 0;
		if(_clear->victim_e)
			maze[c.x][c.y][c.z].victim_e = 0;
		if(_clear->victim_s)
			maze[c.x][c.y][c.z].victim_s = 0;
		if(_clear->victim_w)
			maze[c.x][c.y][c.z].victim_w = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Adds the wallvalues of @*coordA to the existing wallvalues of @_coordB.
//
// Param:
// @*_coordA:	Pointer to the Structure with the coordinate-information of the
//						tile to get the values from
// @*_coordB:	Pointer to the Structure with the coordinate-information of the
//						tile to change the values from
//
// @return: 	void
////////////////////////////////////////////////////////////////////////////////

void maze_corrTile(COORD *_coordA, COORD *_coordB)
{
	for(uint8_t dir = NORTH; dir <= WEST; dir++)
		maze_corrWall(_coordB, dir, maze_getWall(_coordA, dir));
}

////////////////////////////////////////////////////////////////////////////////
// Sets the value of the Victim at the given coordinate and direction to the
// value of @value.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir:		The value of which victim in which direction of this tile should be
//				changed?
// @value:		New value of the wall
//
// @return:		void
////////////////////////////////////////////////////////////////////////////////

void maze_setVictim(COORD *_coord, int8_t dir, int8_t value)
{
	COORD c = *_coord;
	dir = maze_alignDir(dir); //NOW Align

	if(maze_adaptOffset(&c))
	{
		switch(dir)
		{
			case NORTH:	maze[c.x][c.y][c.z].victim_n = value;	break;
			case EAST:	maze[c.x][c.y][c.z].victim_e = value;	break;
			case SOUTH:	maze[c.x][c.y][c.z].victim_s = value;	break;
			case WEST:	maze[c.x][c.y][c.z].victim_w = value;	break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.08]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Returns the value of the victim at the given coordinate and direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The value of which victim in which direction of this tile should be
//						returned?
//
// @return: value of the wall at the given coordinate and direction
////////////////////////////////////////////////////////////////////////////////

int8_t maze_getVictim(COORD *_coord, uint8_t dir)
{
	int8_t returnvar = MAZE_ERROR;
	COORD c = *_coord;

	dir = maze_alignDir(dir);

	if(maze_adaptOffset(&c)) //Positionen für Speicher an Offset anpassen
	{
		switch(dir)
		{
			case NORTH:	returnvar = maze[c.x][c.y][c.z].victim_n;	break;
			case EAST:	returnvar = maze[c.x][c.y][c.z].victim_e;	break;
			case SOUTH:	returnvar = maze[c.x][c.y][c.z].victim_s;	break;
			case WEST:	returnvar = maze[c.x][c.y][c.z].victim_w;	break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.08]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}

	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Changes the value of the victim at the given coordinate and direction with the
// value of @value.
//
// Param:
// @*_coord: Pointer to the Structure with the coordinate-information
// @dir: 	The value of which victim in which direction of this tile should be
//			changed?
// @value: 	Change of the Victim
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_corrVictim(COORD *_coord, int8_t dir, int8_t value)
{
	dir = maze_alignDir(dir);

	int16_t wallVal = maze_getVictim(_coord, dir) + value;
	if(wallVal > MAZE_VICTIMVALUE_MAX)
		wallVal = MAZE_VICTIMVALUE_MAX;
	else if(wallVal < MAZE_VICTIMVALUE_MIN)
		wallVal = MAZE_VICTIMVALUE_MIN;

	maze_setVictim(_coord, dir, wallVal);
}

////////////////////////////////////////////////////////////////////////////////
// Sets the value of the Wall at the given coordinate and direction to the
// value of @value.
//
// Param:
// @*_coord: Pointer to the Structure with the coordinate-information
// @dir: 		The value of which wall in which direction of this tile should be
//					changed?
// @value: 	New value of the wall
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_setWall(COORD *_coord, int8_t dir, int8_t value)
{
	COORD c = *_coord;
	dir = maze_alignDir(dir); //NOW Align

	if(maze_adaptOffset(&c))
	{
		switch(dir)
		{
			case NORTH:
								if((c.y + 1) <= (MAZE_SIZE_Y-1))	maze[c.x][c.y + 1][c.z].wall_s = value; //Aktuelle Platte Wand Norden = nächste Platte Norden Wand Süden
								else								maze[c.x][0		 ][c.z].wall_s = value;
							break;
			case EAST:
								if((c.x + 1) <= (MAZE_SIZE_X-1))	maze[c.x + 1][c.y][c.z].wall_w = value; //Aktuelle Platte Wand Osten = nächste Platte Wand Westen
								else								maze[0		][c.y][c.z].wall_w = value;
							break;
			case SOUTH:
								maze[c.x][c.y][c.z].wall_s = value; //Aktuelle Platte Wand Süden = Aktuelle Platte Wand Süden
							break;
			case WEST:
								maze[c.x][c.y][c.z].wall_w = value; //Aktuelle Platte Wand Westen = Aktuelle Platte Wand Westen
							break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.08]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Changes the value of the Wall at the given coordinate and direction with the
// value of @value.
//
// Param:
// @*_coord: Pointer to the Structure with the coordinate-information
// @dir: 		The value of which wall in which direction of this tile should be
//					changed?
// @value: 	Change of the wall
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_corrWall(COORD *_coord, int8_t dir, int8_t value)
{
	int16_t wallVal = maze_getWall(_coord, dir) + value;
	if(wallVal > MAZE_WALLVALUE_MAX)
		wallVal = MAZE_WALLVALUE_MAX;
	else if(wallVal < MAZE_WALLVALUE_MIN)
		wallVal = MAZE_WALLVALUE_MIN;

	maze_setWall(_coord, dir, wallVal);
}

////////////////////////////////////////////////////////////////////////////////
// Returns the value of the wall at the given coordinate and direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The value of which wall in which direction of this tile should be
//						returned?
//
// @return: value of the wall at the given coordinate and direction
////////////////////////////////////////////////////////////////////////////////

int8_t maze_getWall(COORD *_coord, uint8_t dir)
{
	int8_t returnvar = MAZE_ERROR;
	COORD c = *_coord;
	
	dir = maze_alignDir(dir);

	if(maze_adaptOffset(&c)) //Positionen für Speicher an Offset anpassen
	{
		switch(dir)
		{
			case NORTH:
								if((c.y + 1) <= (MAZE_SIZE_Y-1))	returnvar = maze[c.x][c.y + 1][c.z].wall_s; //Aktuelle Platte Wand Norden = nächste Platte Norden Wand Süden
								else								returnvar = maze[c.x][0		 ][c.z].wall_s;
							break;
			case EAST:
								if((c.x + 1) <= (MAZE_SIZE_X-1))	returnvar = maze[c.x + 1][c.y][c.z].wall_w; //Aktuelle Platte Wand Osten = nächste Platte Wand Westen
								else								returnvar = maze[0		][c.y][c.z].wall_w;
							break;
			case SOUTH:
								returnvar = maze[c.x][c.y][c.z].wall_s; //Aktuelle Platte Wand Süden = Aktuelle Platte Wand Süden
							break;
			case WEST:
								returnvar = maze[c.x][c.y][c.z].wall_w; //Aktuelle Platte Wand Westen = Aktuelle Platte Wand Westen
							break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.09]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
	else returnvar = -1;
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Sets the value of the ground (Black tile or no black tile) at the given
// coordinate and direction to the value of @value.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @value: 		New value of the ground
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_setGround(COORD *_coord, int8_t dir, int8_t value)
{
	COORD c = *_coord;

	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)switch(dir)
	{
		dir = maze_alignDir(dir); //NOW align the direction

		switch(dir)
		{
			case NONE:
							maze[c.x][c.y][c.z].ground += value;
						break;
			case NORTH:
							if((c.y + 1) <= (MAZE_SIZE_Y-1))	maze[c.x][c.y + 1][c.z].ground = value;
							else								maze[c.x][0		 ][c.z].ground = value;
						break;
			case EAST:
							if((c.x + 1) <= (MAZE_SIZE_X-1))	maze[c.x + 1][c.y][c.z].ground = value;
							else								maze[0		][c.y][c.z].ground = value;
						break;
			case SOUTH:
							if((c.y - 1) >= 0)					maze[c.x][c.y - 1	   ][c.z].ground = value;
							else								maze[c.x][MAZE_SIZE_Y-1][c.z].ground = value;
						break;
			case WEST:
							if((c.x - 1) >= 0)					maze[c.x - 1	  ][c.y][c.z].ground = value;
							else								maze[MAZE_SIZE_X-1][c.y][c.z].ground = value;
						break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Changes the value of the ground (Black tile or no black tile) at the given
// coordinate and direction with the value of @value.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @value: 		Change of the ground
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_corrGround(COORD *_coord, int8_t dir, int8_t value)
{
	int16_t groundVal = maze_getGround(_coord, dir) + value;
	if(groundVal > MAZE_GROUNDVALUE_MAX)
		groundVal = MAZE_GROUNDVALUE_MAX;
	else if(groundVal < MAZE_GROUNDVALUE_MIN)
		groundVal = MAZE_GROUNDVALUE_MIN;

	maze_setGround(_coord, dir, groundVal);
}

////////////////////////////////////////////////////////////////////////////////
// Returns the value of the ground at the given coordinate and direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The value of which ground in which direction of this tile should
//						be returned?
//
// @return: value of the ground at the given coordinate and direction
////////////////////////////////////////////////////////////////////////////////

int8_t maze_getGround(COORD *_coord, int8_t dir)
{
	int8_t returnvar = -1;	
	COORD c = *_coord;

	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		dir = maze_alignDir(dir);
		switch(dir)
		{
			case NONE:
							returnvar = maze[c.x][c.y][c.z].ground;
						break;
			case NORTH:
							if((c.y + 1) <= (MAZE_SIZE_Y-1))	returnvar = maze[c.x][c.y + 1][c.z].ground;
							else								returnvar = maze[c.x][0		 ][c.z].ground;
						break;
			case EAST:
							if((c.x + 1) <= (MAZE_SIZE_X-1))	returnvar = maze[c.x + 1][c.y][c.z].ground;
							else								returnvar = maze[0		][c.y][c.z].ground;
						break;
			case SOUTH:
							if((c.y - 1) >= 0)					returnvar = maze[c.x][c.y - 1	   ][c.z].ground;
							else								returnvar = maze[c.x][MAZE_SIZE_Y-1][c.z].ground;
						break;
			case WEST:
							if((c.x - 1) >= 0)					returnvar = maze[c.x - 1	  ][c.y][c.z].ground;
							else								returnvar = maze[MAZE_SIZE_X-1][c.y][c.z].ground;
						break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Sets the beenthere-bit of the tile at the given coordinate and direction to
// the value of @value.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The value of which ground in which direction of this tile should
//						be returned?
// @value: 		New value of the beenthere-bit (TRUE or FALSE)
//
// @return: void
////////////////////////////////////////////////////////////////////////////////

void maze_setBeenthere(COORD *_coord, int8_t dir, uint8_t value)
{
	COORD c = *_coord;
	
	if(maze_adaptOffset(&c))
	{
		dir = maze_alignDir(dir);
		
		switch(dir)
		{
			case NONE:
							maze[c.x][c.y][c.z].beenthere = value;
						break;
			case NORTH:
							if((c.y + 1) <= (MAZE_SIZE_Y-1))	maze[c.x][c.y + 1][c.z].beenthere = value;
							else								maze[c.x][0		 ][c.z].beenthere = value;
						break;
			case EAST:
							if((c.x + 1) <= (MAZE_SIZE_X-1))	maze[c.x + 1][c.y][c.z].beenthere = value;
							else								maze[0		][c.y][c.z].beenthere = value;
						break;
			case SOUTH:
							if((c.y - 1) >= 0)					maze[c.x][c.y - 1	   ][c.z].beenthere = value;
							else								maze[c.x][MAZE_SIZE_Y-1][c.z].beenthere = value;
						break;
			case WEST:
							if((c.x - 1) >= 0)					maze[c.x - 1	  ][c.y][c.z].beenthere = value;
							else								maze[MAZE_SIZE_X-1][c.y][c.z].beenthere = value;
						break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Returns the beenthere-bit of the tile at the given coordinate and direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The beenthere-bit of which tile in which direction of this tile
//						should be returned?
//
// @return: 	beenthere-bit of the given tile in the given direction (has the
//						robot visited this tile yet?), in case something went wrong
//						MAZE_ERROR
////////////////////////////////////////////////////////////////////////////////

int8_t maze_getBeenthere(COORD *_coord, int8_t dir) 
{
	int8_t returnvar = MAZE_ERROR;
	dir = maze_alignDir(dir);
			
	if((dir != NONE) &&
		 ((int8_t)maze_getRampPosDir(_coord) == dir))
	{
		COORD _ramp = *_coord;
		
		_ramp.z ++; //Works only in arenas with two stages...
		if(_ramp.z >= MAZE_SIZE_Z)
			_ramp.z = 0;
			
		_ramp = *maze_getRamp(_ramp.z);
		returnvar = maze_getBeenthere(&_ramp, NONE);
	}
	else
	{
		COORD c = *_coord;
		
		if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
		{
			switch(dir)
			{
				case NONE:
								returnvar = maze[c.x][c.y][c.z].beenthere;
							break;
				case NORTH:
								if((c.y + 1) <= (MAZE_SIZE_Y-1))	returnvar = maze[c.x][c.y + 1][c.z].beenthere;
								else								returnvar = maze[c.x][0		 ][c.z].beenthere;
							break;
				case EAST:
								if((c.x + 1) <= (MAZE_SIZE_X-1))	returnvar = maze[c.x + 1][c.y][c.z].beenthere;
								else								returnvar = maze[0		][c.y][c.z].beenthere;
							break;
				case SOUTH:
								if((c.y - 1) >= 0)					returnvar = maze[c.x][c.y - 1	   ][c.z].beenthere;
								else								returnvar = maze[c.x][MAZE_SIZE_Y-1][c.z].beenthere;
							break;
				case WEST:
								if((c.x - 1) >= 0)					returnvar = maze[c.x - 1	  ][c.y][c.z].beenthere;
								else								returnvar = maze[MAZE_SIZE_X-1][c.y][c.z].beenthere;
							break;
				default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
									fatal_err = 1;
			}
		}
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Sets the depthsearch var of the tile at the given coordinate to the value of
// @value.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @value: 		New value of depthsearch
//
// @return: 	void
////////////////////////////////////////////////////////////////////////////////

void maze_setDepthsearch(COORD *_coord, int8_t dir, uint8_t value)
{
	COORD c = *_coord;
	
	if(maze_adaptOffset(&c)) //Positionen für Speicher an Offset anpassen
	{
		switch(dir)
		{
			case NONE:
							maze[c.x][c.y][c.z].depthsearch = value;
						break;
			case NORTH:
							if((c.y + 1) <= (MAZE_SIZE_Y-1))	maze[c.x][c.y + 1][c.z].depthsearch = value;
							else								maze[c.x][0		 ][c.z].depthsearch = value;
						break;
			case EAST:
							if((c.x + 1) <= (MAZE_SIZE_X-1))	maze[c.x + 1][c.y][c.z].depthsearch = value;
							else								maze[0		][c.y][c.z].depthsearch = value;
						break;
			case SOUTH:
							if((c.y - 1) >= 0)					maze[c.x][c.y - 1			][c.z].depthsearch = value;
							else								maze[c.x][MAZE_SIZE_Y-1][c.z].depthsearch = value;
						break;
			case WEST:
							if((c.x - 1) >= 0)					maze[c.x - 1	  ][c.y][c.z].depthsearch = value;
							else								maze[MAZE_SIZE_X-1][c.y][c.z].depthsearch = value;
						break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Returns the beenthere-bit of the tile at the given coordinate and direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir: 			The depthsearch var of which tile in which direction of this tile
//						should be returned?
//
// @return: 	depthsearch-var of the given tile in the given direction (has the
//						robot visited this tile yet?), in case something went wrong
//						0xff
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_getDepthsearch(COORD *_coord, int8_t dir) 
{
	int8_t returnvar = MAZE_ERROR;
	dir = maze_alignDir(dir);
			
	if((dir != NONE) &&
		 ((int8_t)maze_getRampPosDir(_coord) == dir))
	{
		COORD _ramp = *_coord;
		
		_ramp.z ++; //Works only in arenas with two stages...
		if(_ramp.z >= MAZE_SIZE_Z)
			_ramp.z = 0;
			
		_ramp = *maze_getRamp(_ramp.z);
		returnvar = maze_getDepthsearch(&_ramp, NONE);
	}
	else
	{
		COORD c = *_coord;
		
		if(maze_adaptOffset(&c)) //Positionen für Speicher an Offset anpassen
		{
			switch(dir)
			{
				case NONE:
								returnvar = maze[c.x][c.y][c.z].depthsearch;
							break;
				case NORTH:
								if((c.y + 1) <= (MAZE_SIZE_Y-1))	returnvar = maze[c.x][c.y + 1][c.z].depthsearch;
								else								returnvar = maze[c.x][0		 ][c.z].depthsearch;
							break;
				case EAST:
								if((c.x + 1) <= (MAZE_SIZE_X-1))	returnvar = maze[c.x + 1][c.y][c.z].depthsearch;
								else								returnvar = maze[0		][c.y][c.z].depthsearch;
							break;
				case SOUTH:
								if((c.y - 1) >= 0)					returnvar = maze[c.x][c.y - 1	   ][c.z].depthsearch;
								else								returnvar = maze[c.x][MAZE_SIZE_Y-1][c.z].depthsearch;
							break;
				case WEST:
								if((c.x - 1) >= 0)					returnvar = maze[c.x - 1	  ][c.y][c.z].depthsearch;
								else								returnvar = maze[MAZE_SIZE_X-1][c.y][c.z].depthsearch;
							break;
				default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
									fatal_err = 1;
			}
		}
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Checks if the tile in the given direction is visitable. Definition of
// visitable (in this case):
// - no walls in that direction
// - no black tiles in that direction
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir:			Directions
//
// @return: 	TRUE when visitable, otherwise FALSE.
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_tileIsVisitable(COORD *_coord, int8_t dir)
{
	uint8_t returnvar = 0;
	dir = maze_alignDir(dir);
	
	if((maze_getGround(_coord, dir) < MAZE_ISBLTILE) &&
		 (maze_getWall(_coord, dir) < MAZE_ISWALL))
	{
		returnvar = 1;
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Fills the *.depthsearch variables in the maze with values determined by the
// Depth-First-Search (DFS) Algorithm with the goal to find the nearest,
// unvisited tile. Only tiles that can be visited by the robot will be used.
// Returns the time in ms needed to calculate the route when a route was found,
// otherwise -1.
//
// Param:
// @*_pos_start:	The nearest, unvisited, visitable tile outgoing from this point
//								has to be find.
// @*_pos_goal:		Pointer of X/Y/Z-Position of the nearest, unvisited, visitable
//								tile.
//
// @return: When a route was found the time in ms needed to find the route,
//					otherwise -1.
////////////////////////////////////////////////////////////////////////////////

int16_t maze_findNearestTile(COORD *_pos_start, COORD *_pos_goal)
{
	uint32_t timer_findNearestTile = timer;
	uint8_t solutionFound = FALSE;
	static COORD _maze;
	
	COORD maze_size[MAZE_SIZE_Z];
	for(uint8_t i = 0; i < MAZE_SIZE_Z; i++)
	{
		maze_size[i].z = i;
		maze_stageGetSize(&maze_size[i]);
	}

	maze_clearDepthsearch();
	maze_setDepthsearch(_pos_start, NONE, 0);
	
	for(int8_t i = 0; i < 70; i++) //Index
	{
		for(_maze.z = 0; _maze.z < MAZE_SIZE_Z; _maze.z++) //check for each stage...
		{
			for(_maze.y = ROB_POS_X_MIN - 1; _maze.y <= maze_size[_maze.z].y + ROB_POS_X_MIN + 1; _maze.y++) //each line...
			{
				for(_maze.x = ROB_POS_Y_MIN - 1; _maze.x <= maze_size[_maze.z].x + ROB_POS_Y_MIN + 1; _maze.x++) //each column...
				{
					for(uint8_t maze_dir = NORTH; maze_dir <= WEST; maze_dir ++) //each direction...
					{
						if((maze_getDepthsearch(&_maze, maze_dir) == i) &&
							maze_tileIsVisitable(&_maze, maze_dir) &&
						   (maze_getRampPosDirAtDir(&_maze, maze_dir) == NONE))//!= maze_alignDir(maze_dir + 2)))
						{
							if(maze_getGround(&_maze, NONE) < MAZE_ISBLTILE)
							{
								if((maze_getBeenthere(&_maze, NONE) == TRUE) &&
									 (maze_getDepthsearch(&_maze, NONE) == 0xff))
								{
									maze_setDepthsearch(&_maze, NONE, i+1);
								}
								else if(maze_getBeenthere(&_maze, NONE) == FALSE)
								{
									_pos_goal->x = _maze.x; //Save the nearest tile in the overgiven var
									_pos_goal->y = _maze.y;
									_pos_goal->z = _maze.z;
									
									solutionFound = TRUE; //Solution found - stop all loops!
								}
							}
						}
						if(solutionFound)
							break;
					}
					if(solutionFound)
						break;
				}
				if(solutionFound)
					break;
			}
			if(solutionFound)
				break;
		}
		if(solutionFound)
			break;
	}
	
	if(solutionFound)
		return((int16_t)(timer - timer_findNearestTile));
	else
		return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Fills the .depthsearch variables in the maze with values determined by the
// Depth-First-Search (DFS) Algorithm with the goal to find a route between two
// points. Only tiles that can be visited by the robot will be used.
// Returns TRUE when a route was found, otherwise FALSE.
//
// Param:
// @*_start:	Coordinate in the maze that represents the start-field
//						(the DFS 0-field)
// @*_goal:		X/Y/Z-Position in the maze that represent the goal the robot
//						wants to reach.
//
// @return: When a route was found the time in ms needed to find the route,
//					otherwise -1.
////////////////////////////////////////////////////////////////////////////////

int16_t maze_findPath(COORD *_start, COORD *_goal)
{
	uint32_t timer_findNearestTile = timer;
	uint8_t solutionFound = FALSE;
	COORD _maze;
	
	COORD maze_size[MAZE_SIZE_Z];
	for(uint8_t i = 0; i < MAZE_SIZE_Z; i++) //Calculate the size of all stages
	{
		maze_size[i].z = i;
		maze_stageGetSize(&maze_size[i]);
	}

	maze_clearDepthsearch();
	
	maze_setDepthsearch(_goal, NONE, 0); //Set goal as root
	
	for(int8_t i = 0; i < 70; i++) //Index
	{
		for(_maze.z = 0; _maze.z < MAZE_SIZE_Z; _maze.z++) //check for each stage...
		{
			for(_maze.y = ROB_POS_X_MIN - 1; _maze.y <= maze_size[_maze.z].y + ROB_POS_X_MIN + 1; _maze.y++) //each column...
			{
				for(_maze.x = ROB_POS_Y_MIN - 1; _maze.x <= maze_size[_maze.z].x + ROB_POS_Y_MIN + 1; _maze.x++) //each line...
				{
					for(uint8_t maze_dir = NORTH; maze_dir <= WEST; maze_dir ++) //each direction...
					{
						if((maze_getDepthsearch(&_maze, maze_dir) == i) &&
						   maze_tileIsVisitable(&_maze, maze_dir) &&
						   (maze_getRampPosDirAtDir(&_maze, maze_dir) != maze_alignDir(maze_dir + 2)))
						{
							if((maze_getBeenthere(&_maze, NONE) == TRUE) &&
							   (maze_getGround(&_maze, NONE) < MAZE_ISBLTILE))
							{
								if(maze_getDepthsearch(&_maze, NONE) == 0xff)
								{
									maze_setDepthsearch(&_maze, NONE, i+1);
								}
						
								if(maze_cmpCoords(&_maze, _start)) //Compare the two structs
									solutionFound = TRUE; //Solution found - stop all loops!
							}
						}
						if(solutionFound)
							break;
					}
					if(solutionFound)
						break;
				}
				if(solutionFound)
					break;
			}
			if(solutionFound)
				break;
		}
		if(solutionFound)
			break;
	}
	
	if(solutionFound)
		return((int16_t)(timer - timer_findNearestTile));
	else
		return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Fills the .depthsearch variables in the maze with 0xff (clears the path)
//
// Param:
// NONE
//
// @return: NONE
////////////////////////////////////////////////////////////////////////////////

void maze_clearDepthsearch(void)
{
	for(uint8_t maze_z = 0; maze_z < MAZE_SIZE_Z; maze_z++)
	{
		for(uint8_t maze_y = 0; maze_y < MAZE_SIZE_Y; maze_y++)
		{
			for(uint8_t maze_x = 0; maze_x < MAZE_SIZE_X; maze_x++)
			{
				maze[maze_x][maze_y][maze_z].depthsearch = 0xff;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Tries to find a match between the savecard and the original card
//
////////////////////////////////////////////////////////////////////////////////
	
void maze_rotateStage(uint8_t stage)
{
	uint8_t n = 1;
	COORD stage_size;
	stage_size.z = stage;
	maze_stageGetSize(&stage_size);
	if(stage_size.y > stage_size.x)
		n += stage_size.y + 1 + ROB_POS_Y_MIN;
	else
		n += stage_size.x + 1 + ROB_POS_X_MIN;
	
	TILE tile_tmp;
	COORD _coord_get, _coord_set;
	_coord_get.z = stage;
	_coord_set.z = stage;
	
	for (uint8_t i = 0; i < n / 2; i++)
	{
		for (uint8_t j = i; j < n - i - 1; j++)
		{
			_coord_get.x = i;
			_coord_get.y = j;
			tile_tmp = *maze_getTile(&_coord_get);
			COORD _coord_tmp = _coord_get;
			COORD rob_pos_save = robot.pos;
			if(maze_cmpCoords(&_coord_tmp, &robot.pos))
			{
				robot.pos.x = -1;
				robot.pos.y = -1;
			}
			
			_coord_set.x = i;
			_coord_set.y = j;
			_coord_get.x = n - j - 1;
			_coord_get.y = i;
			maze_setTile(&_coord_set, maze_getTile(&_coord_get));
			if(maze_cmpCoords(&_coord_get, &robot.pos)) //Fehlerhaft
			{
				robot.pos = _coord_set;
			}
			
			_coord_set.x = n - j - 1;
			_coord_set.y = i;
			_coord_get.x = n - 1 - i;
			_coord_get.y = n - 1 - j;
			maze_setTile(&_coord_set, maze_getTile(&_coord_get));
			if(maze_cmpCoords(&_coord_get, &robot.pos))
			{
				robot.pos = _coord_set;
			}
			
			_coord_set.x = n - 1 - i;
			_coord_set.y = n - 1 - j;
			_coord_get.x = j;
			_coord_get.y = n - 1 - i;
			maze_setTile(&_coord_set, maze_getTile(&_coord_get));
			if(maze_cmpCoords(&_coord_get, &robot.pos))
			{
				robot.pos = _coord_set;
			}
			
			_coord_set.x = j;
			_coord_set.y = n - 1 - i;
			maze_setTile(&_coord_set, &tile_tmp);
			if(maze_cmpCoords(&_coord_tmp, &rob_pos_save))
			{
				robot.pos = _coord_set;
			}
		}
	}
	
	for(_coord_set.x = 0; _coord_set.x < n; _coord_set.x ++)
	{
		for(_coord_set.y = n; _coord_set.y >= 0; _coord_set.y --)
		{
			maze_setWall(&_coord_set, NORTH, maze_getWall(&_coord_set, WEST));
			maze_setWall(&_coord_set, WEST, maze_getWall(&_coord_set, SOUTH));
		}
	}
	
	robot.dir = maze_alignDir(robot.dir+1);
}

void maze_matchStages(MATCHSTAGES *matchStages)
{
	POS stage_a;
	stage_a.pos.z = matchStages->stage_a;

	COORD stage_b;
	stage_b.z = matchStages->stage_b;
	
	POS match_coord;
	match_coord.pos.x = 0;
	match_coord.pos.y = 0;
	match_coord.pos.z = 0;
	
	int16_t matches = 0;
	int8_t accordance = 0; //Übereinstimmung in %
	int8_t accordance_max = 0;
	uint8_t ambiguity = 0; //Uneindeutigkeiten
	
	COORD stage_b_innerSize, stage_b_outerSize;
	stage_b_innerSize.z = matchStages->stage_b;
	maze_stageGetInnerSize(&stage_b_innerSize);
	stage_b_outerSize.z = matchStages->stage_b;
	maze_stageGetOuterSize(&stage_b_outerSize);

	uint8_t stage_b_visitedTiles;
	
	for(stage_a.dir = NONE; stage_a.dir < WEST; stage_a.dir ++)
	{
		COORD stage_a_innerSize, stage_a_outerSize;
		stage_a_innerSize.z = matchStages->stage_a;
		maze_stageGetInnerSize(&stage_a_innerSize);
		stage_a_outerSize.z = matchStages->stage_a;
		maze_stageGetOuterSize(&stage_a_outerSize);
		for(stage_a.pos.y = stage_a_innerSize.y; stage_a.pos.y <= stage_a_outerSize.y+1; stage_a.pos.y ++)
		{
			for(stage_a.pos.x = stage_a_innerSize.x; stage_a.pos.x <= stage_a_outerSize.x+1; stage_a.pos.x ++)
			{
				COORD stage_a_save = stage_a.pos;
				stage_b_visitedTiles = 0;

				for(stage_b.y = stage_b_innerSize.y; stage_b.y <= stage_b_outerSize.y+1; stage_b.y ++)
				{
					stage_a_save.y = stage_a.pos.y + stage_b.y - 1- ROB_POS_Y_MIN;
					if(stage_a_save.y > MAZE_SIZE_Y_USABLE)
					{
						matches = 0;
						break;
					}

					for(stage_b.x = stage_b_innerSize.x; stage_b.x <= stage_b_outerSize.x+1; stage_b.x ++)
					{
						stage_a_save.x = stage_a.pos.x + stage_b.x - 1-ROB_POS_X_MIN;
						if(stage_a_save.x > MAZE_SIZE_X_USABLE)
						{
							matches = 0;
							break;
						}

						if(maze_getBeenthere(&stage_b, NONE) &&
							 maze_getBeenthere(&stage_a_save, NONE))
						{
							stage_b_visitedTiles ++;

							for(int8_t dir = NORTH; dir <= WEST; dir ++)
							{
								int8_t wall_save = maze_getWall(&stage_b, dir);
								int8_t wall_orig = maze_getWall(&stage_a_save, dir);

								if(((wall_save > 0) && (wall_orig > 0)) ||
									 ((wall_save < 0) && (wall_orig < 0)))
								{
									matches ++;
								}
							}
						}
					}
				}

				accordance = (matches * 100) / (stage_b_visitedTiles * 4);

				if(accordance > accordance_max)
				{
					ambiguity = 0;
					accordance_max = accordance;
					match_coord = stage_a;
				}
				else if(accordance == accordance_max)
				{
					ambiguity ++;
				}
				matches = 0;
			}
		}
		maze_rotateStage(matchStages->stage_b);
	}
	
	matchStages->match = match_coord;
	matchStages->ambiguity = ambiguity;
	matchStages->accordance = accordance_max;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the outer boundaries (highest visited x/y-position) of the given
// stage
//
////////////////////////////////////////////////////////////////////////////////

void maze_stageGetOuterSize(COORD *_size)
{
	_size->x = 0;
	_size->y = 0;
	
	COORD maze_tmp;
	maze_tmp.z = _size->z;
	
	for(maze_tmp.x = 0; maze_tmp.x < MAZE_SIZE_X_USABLE; maze_tmp.x++)
	{
		for(maze_tmp.y = 0; maze_tmp.y < MAZE_SIZE_Y_USABLE; maze_tmp.y++)
		{
			uint8_t beenthere = maze_getBeenthere(&maze_tmp, NONE);
			if(beenthere && (maze_tmp.x > _size->x))
				_size->x = maze_tmp.x;
			if(beenthere && (maze_tmp.y > _size->y))
				_size->y = maze_tmp.y;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the inner boundaries (lowest visited x/y-position) of the given
// stage
//
////////////////////////////////////////////////////////////////////////////////

void maze_stageGetInnerSize(COORD *_size)
{
	_size->x = MAZE_SIZE_X;
	_size->y = MAZE_SIZE_Y;
	
	COORD maze_tmp;
	maze_tmp.z = _size->z;
	
	for(maze_tmp.x = MAZE_SIZE_X_USABLE; maze_tmp.x >= 0 ; maze_tmp.x--)
	{
		for(maze_tmp.y = MAZE_SIZE_Y_USABLE; maze_tmp.y >= 0; maze_tmp.y--)
		{
			uint8_t beenthere = maze_getBeenthere(&maze_tmp, NONE);
			if(beenthere && (maze_tmp.x < _size->x))
				_size->x = maze_tmp.x;
			if(beenthere && (maze_tmp.y < _size->y))
				_size->y = maze_tmp.y;
		}
	}
	
	if(_size->x == MAZE_SIZE_X)
		_size->x = 0;
	if(_size->y == MAZE_SIZE_Y)
		_size->y = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the size of the given stage
//
////////////////////////////////////////////////////////////////////////////////

void maze_stageGetSize(COORD *_size)
{
	COORD outerSize;
	outerSize.z = _size->z;
	COORD innerSize;
	innerSize.z = _size->z;
	
	maze_stageGetOuterSize(&outerSize);
	maze_stageGetInnerSize(&innerSize);
	
	_size->x = outerSize.x - innerSize.x;
	_size->y = outerSize.y - innerSize.y;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the number of visited tiles on the given stage.
//
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_stageGetVisitedTiles(uint8_t stage)
{
	COORD _maze;
	_maze.z = stage;
	
	uint8_t cnt = 0;
	
	for(_maze.y = 0; _maze.y < MAZE_SIZE_Y_USABLE; _maze.y ++)
	{
		for(_maze.x = 0; _maze.x < MAZE_SIZE_X_USABLE; _maze.x ++)
		{
			if(maze_getBeenthere(&_maze, NONE))
				cnt ++;
		}
	}
	
	return cnt;
}

////////////////////////////////////////////////////////////////////////////////
// Calculates the number of visited tiles in the maze.
//
////////////////////////////////////////////////////////////////////////////////

uint16_t maze_GetVisitedTiles(void)
{
	uint16_t visitedTiles = 0;
	for(uint8_t z = 0; z < MAZE_SIZE_Z; z++)
	{
		visitedTiles += maze_stageGetVisitedTiles(z);
	}
	
	return visitedTiles;
}

////////////////////////////////////////////////////////////////////////////////
// Clears the whole given stage.
//
// Param:
// @stage:	Stage to clear
//
// @return: 	void
////////////////////////////////////////////////////////////////////////////////

void maze_clearStage(uint8_t stage, TILE *clear)
{
	COORD c;
	c.x = 0;
	c.y = 0;
	c.z = stage;

	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		for(c.y = 0; c.y < MAZE_SIZE_Y; c.y++)
		{
			for(c.x = 0; c.x < MAZE_SIZE_X; c.x++)
			{
				maze_clearTile(&c, clear);
			}
		}
	}
}

void maze_clear(TILE *clear)
{
	for(uint8_t z = 0; z < MAZE_SIZE_Z_USABLE; z++)
	{
		maze_clearStage(z, clear);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Sets the checkpoint to the new position
//
// Param:
// @*coord:	Pointer to the new Position of the checkpoint
//
// @return: 	void
////////////////////////////////////////////////////////////////////////////////

void maze_setCheckpoint(COORD *_coord, int8_t dir)
{
	COORD c = *_coord;

	switch(dir)
	{
		case NONE:	break; //Positionen müssen nicht geändert werden
		case NORTH:
							c.y ++;	break;
		case EAST:
							c.x ++;	break;
		case SOUTH:
							if((c.y - 1) < 0)
							{
								maze_chgOffset(Y, c.z, -1);
								_coord->y ++;
								c.y ++;
							}

							c.y --;

						break;
		case WEST:
							if((c.x - 1) < 0)
							{
								maze_chgOffset(X, c.z, -1);
								_coord->x ++;
								c.x ++;
							}

							c.x --;

						break;
		default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.44]:DEFAULT_CASE"));}
							fatal_err = 1;
	}

	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		checkpoint.pos = c;
		checkpoint.dir = robot.dir; //Mark checkpoint as set, of this is NONE the checkpoint is represented by the start tile!
	}
}


////////////////////////////////////////////////////////////////////////////////
// Returns the checkpoint.
//
// Param:
// @*coord: Position of the robot in the maze
//
// @return: Pointer to the coordinates of the checkpoint
////////////////////////////////////////////////////////////////////////////////

COORD *maze_getCheckpoint(COORD *_coord)
{
	static COORD checkpointPos;

	if(checkpoint.dir == NONE)
		checkpointPos = *maze_getStartPos();
	else
	{
		checkpointPos = checkpoint.pos;

		if(offset_z != 0)
		{
			if(checkpointPos.z == 0)
				checkpointPos.z = 1;
			else
				checkpointPos.z = 0;
		}

		checkpointPos.x = checkpoint.pos.x;
		checkpointPos.x -= offset[checkpointPos.z].x;
		while(checkpointPos.x >= MAZE_SIZE_X)
			checkpointPos.x -= MAZE_SIZE_X;

		checkpointPos.y = checkpoint.pos.y;
		checkpointPos.y -= offset[checkpointPos.z].y;
		while(checkpointPos.y >= MAZE_SIZE_Y)
			checkpointPos.y -= MAZE_SIZE_Y;
	}

	return &checkpointPos;
}


////////////////////////////////////////////////////////////////////////////////
// Checks if there is a ramp at the given coordinate and returns the direction
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
//
// @return: 	NONE when there is no ramp, otherwise the direction of the ramp
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_getRampPosDir(COORD *_coord)
{
	uint8_t returnvar = NONE;
	
	COORD c = *_coord;
	
	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		if((ramp[c.z].dir != NONE) &&
			 (ramp[c.z].pos.x == c.x) &&
			 (ramp[c.z].pos.y == c.y))
		{
			returnvar = ramp[c.z].dir;
		}
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Checks if there is a ramp at the given coordinate and directions and returns
// the direction.
//
// Param:
// @*_coord:	Pointer to the Structure with the coordinate-information
// @dir:			Desired direction
//
// @return: 	NONE when there is no ramp, otherwise the direction of the ramp
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_getRampPosDirAtDir(COORD *_coord, int8_t dir)
{
	uint8_t returnvar = NONE;
	
	COORD c = *_coord;
	
	switch(dir)
	{
		case NONE:				returnvar = maze_getRampPosDir(&c);		break;
		case NORTH:	c.y ++;		returnvar = maze_getRampPosDir(&c);		break;
		case EAST:	c.x ++;		returnvar = maze_getRampPosDir(&c);		break;
		case SOUTH:	c.y --;		returnvar = maze_getRampPosDir(&c);		break;
		case WEST:	c.x --;		returnvar = maze_getRampPosDir(&c);		break;
		default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
							fatal_err = 1;
	}
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Checks if there is a ramp at the given stage and returns the direction.
//
// Param:
// @pos_z:	Stage of the maze
//
// @return: NONE when there is no ramp, otherwise the direction of the ramp
////////////////////////////////////////////////////////////////////////////////

uint8_t maze_getRampDir(int8_t pos_z)
{
	uint8_t returnvar = 0;
	
	COORD c;
	c.x = 0;
	c.y = 0;
	c.z = pos_z;
	
	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		returnvar = ramp[c.z].dir;
	}
	else returnvar = NONE;
	
	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Sets a ramp with the direcion @ramp dir to the tile in direction of @tile_dir
// seen from the given coordinates.
//
// Param:
// @*_coord:	Base coordinates
// @ramp_dir:	Direction of the ramp
// @tile_dir:	In this direction of the base coordinates the ramp will be set.
// @set:		set (TRUE) or clear (FALSE) the ramp
// @return: void
////////////////////////////////////////////////////////////////////////////////
void maze_setRamp(COORD *_coord, int8_t ramp_dir, int8_t tile_dir, int8_t set)
{
	COORD c = *_coord;

	if(set)
	{
		switch(tile_dir)
		{
			case NONE:	break; //Positionen müssen nicht geändert werden
			case NORTH:
								c.y ++;	break;
			case EAST:
								c.x ++;	break;
			case SOUTH:
								if((c.y - 1) < 0)
								{
									maze_chgOffset(Y, c.z, -1);
									_coord->y ++;
									c.y ++;
								}

								c.y --;

							break;
			case WEST:
								if((c.x - 1) < 0)
								{
									maze_chgOffset(X, c.z, -1);
									_coord->x ++;
									c.x ++;
								}

								c.x --;

							break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.04]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
		maze_setWall(&c, ramp_dir+1, MAZE_ISWALL+1);
		maze_setWall(&c, ramp_dir+3, MAZE_ISWALL+1);
		maze_setWall(&c, ramp_dir, -MAZE_ISWALL+1);
		maze_setWall(&c, ramp_dir+2, -MAZE_ISWALL+1);
		maze_setBeenthere(&c,NONE,TRUE);
	}
	else
	{
		maze_setWall(&robot.pos, ramp_dir+1, 0);
		maze_setWall(&robot.pos, ramp_dir+3, 0);
		maze_setWall(&robot.pos, ramp_dir, 0);
		maze_setWall(&robot.pos, ramp_dir+2, 0);
		maze_setBeenthere(&c,NONE,FALSE);
	}


	if(maze_adaptOffset(&c)) //Adapt position to offset of the map in the memory (RAM)
	{
		if(set)
		{
			ramp[c.z].pos.x = c.x;
			ramp[c.z].pos.y = c.y;
			ramp[c.z].dir = ramp_dir;
		}
		else
			ramp[c.z].dir = NONE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Sets a ramp with the direcion @ramp dir to the tile in direction of @tile_dir
// seen from the given coordinates.
//
// Param:
// @*_coord:	Base coordinates
// @ramp_dir:	Direction of the ramp
// @tile_dir: In this direction of the base coordinates the ramp will be set.
//
// @return: void
////////////////////////////////////////////////////////////////////////////////
COORD *maze_getRamp(int8_t pos_z)
{
	static COORD rampPos;
	rampPos.z = pos_z;
	
	if(offset_z != 0)
	{
		if(pos_z == 0)
			pos_z = 1;
		else
			pos_z = 0;
	}
	
	rampPos.x = ramp[pos_z].pos.x;
	rampPos.x -= offset[pos_z].x;
	while(rampPos.x >= MAZE_SIZE_X)
		rampPos.x -= MAZE_SIZE_X;
	
	rampPos.y = ramp[pos_z].pos.y;
	rampPos.y -= offset[pos_z].y;
	while(rampPos.y >= MAZE_SIZE_Y)
		rampPos.y -= MAZE_SIZE_Y;
	
	return &rampPos;
}

////////////////////////////////////////////////////////////////////////////////
// Returns the X/Y/Z-Startpositions of the robot to the pointers.
//
// Param:
// @*pos_x: Start X-Position
// @*pos_y: Start Y-Position
// @*pos_z: Start Z-Position
//
// @return: NONE (indirectly the start-position by pointer)
////////////////////////////////////////////////////////////////////////////////

COORD *maze_getStartPos(void)
{
	//The start positions are represented by the START_MAZE constants, the offsets
	//and the values of off_start. In the offset[] index don’t has to be putted the
	//offset_z value (because the first used level will always be offset[0],
	//except for when the off_start.z is 1 (started by rotary encoder in upper
	//stage)
	static COORD start;
	
	start.x = abs(offset[off_start.z].x) + off_start.x + ROB_START_MAZE_X;
	start.y = abs(offset[off_start.z].y) + off_start.y + ROB_START_MAZE_Y;
	start.z = abs(offset_z) + off_start.z + ROB_START_MAZE_Z;
	
	return &start;
}

////////////////////////////////////////////////////////////////////////////////
// Resetting the whole map (nessesary at the begin of the program and after the
// robot finished the maze)
//
// Param:
// NONE
//
// @return: NONE
////////////////////////////////////////////////////////////////////////////////

void maze_init(void)
{
	TILE clear;
	clear.beenthere = 1;
	clear.depthsearch = 1;
	clear.ground = 1;
	clear.obstacle = 1;
	clear.victim_e = 1;
	clear.victim_s = 1;
	clear.victim_w = 1;
	clear.victim_n = 1;
	clear.wall_s = 1;
	clear.wall_w = 1;

	for(uint8_t maze_z = 0; maze_z < MAZE_SIZE_Z; maze_z++) //Labyrinth löschen
	{
		maze_clearStage(maze_z, &clear);
	}
	
	for(int8_t i = 0; i < MAZE_SIZE_Z; i++)
	{
		offset[i].x = 0;
		offset[i].y = 0;
	}
	offset_z = 0;
	
	robot.pos.x = ROB_START_MAZE_X;
	robot.pos.y = ROB_START_MAZE_Y;
	robot.pos.z = ROB_START_MAZE_Z;
	robot.dir = ROB_START_DIR;

    checkpoint.pos = robot.pos;
    checkpoint.dir = NONE;

	off_start.x = 0;
	off_start.y = 0;
	off_start.z = 0;
	
	for(uint8_t i = 0; i < MAZE_SIZE_Z; i++)
	{
		ramp[i].pos.x = -1;
		ramp[i].pos.y = -1;
		ramp[i].pos.z = -1;
		ramp[i].dir = NONE;
    }

	maze_solve_state_path = DRIVE_READY;
	
    routeRequest = 0;
}

//////////////////////////////////////////////////
void maze_chgOffset(int8_t whichOff, int8_t pos_z, int8_t cnt)
{
	COORD c;
	c.x = 0;
	c.y = 0;
	c.z = pos_z;
	
	if(maze_adaptOffset(&c))
	{
		switch(whichOff)
		{
			case X: offset[c.z].x += cnt; break;
			case Y: offset[c.z].y += cnt; break;
			case Z: offset_z += cnt; 			break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.14]:DEFAULT_CASE"));}
								fatal_err = 1;
		}
	
		while(offset[c.z].x >= MAZE_SIZE_X)
			offset[c.z].x -= MAZE_SIZE_X;
		while(offset[c.z].x <= -MAZE_SIZE_X)
			offset[c.z].x += MAZE_SIZE_X;
			
		while(offset[c.z].y >= MAZE_SIZE_Y)
			offset[c.z].y -= MAZE_SIZE_Y;
		while(offset[c.z].y <= -MAZE_SIZE_Y)
			offset[c.z].y += MAZE_SIZE_Y;
			
		while(offset_z >= MAZE_SIZE_Z_USABLE)
			offset_z -= MAZE_SIZE_Z_USABLE;
		while(offset_z <= -MAZE_SIZE_Z_USABLE)
			offset_z += MAZE_SIZE_Z_USABLE;
		
		if(debug > 0)
		{
			bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": new::offset[")); bt_putLong(robot.pos.z); 
			switch(whichOff)
			{
				case X: bt_putStr_P(PSTR("].x: "));bt_putLong(offset[c.z].x); break;
				case Y: bt_putStr_P(PSTR("].y: "));bt_putLong(offset[c.z].y); break;
				case Z: bt_putStr_P(PSTR("]_z: "));bt_putLong(offset_z); 				break;
				default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.15]:DEFAULT_CASE"));}
									fatal_err = 1;
			}
		}
	}
	else
	{
		if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:maze_chgOffset():CANT"));}
		fatal_err = 1;
	}
}

int8_t maze_adaptOffset(COORD *_coord)
{
	int8_t returnvar = 1;
	
	if((_coord->z >= 0) && (_coord->z <= (MAZE_SIZE_Z-1))) //Offset z has to be adapted as first, because otherwise the difference between offset[z] is not rigth
	{
		_coord->z += offset_z;
		while(_coord->z < 0)
			_coord->z += MAZE_SIZE_Z;
		while(_coord->z >= MAZE_SIZE_Z)
			_coord->z -= MAZE_SIZE_Z;
	}
	else returnvar = 0;
	
	if((_coord->x >= 0) && (_coord->x <= (MAZE_SIZE_X-1)))
	{
		_coord->x += offset[_coord->z].x;
		while(_coord->x < 0)
			_coord->x += MAZE_SIZE_X;
	}
	else returnvar = 0;
	
	if((_coord->y >= 0) && (_coord->y <= (MAZE_SIZE_Y-1)))
	{
		_coord->y += offset[_coord->z].y;
		while(_coord->y < 0)
			_coord->y += MAZE_SIZE_Y;
	}
	else returnvar = 0;
	
	return returnvar;
}
