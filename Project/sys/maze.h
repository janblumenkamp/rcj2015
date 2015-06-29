////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
///////////////////////////////////maze.h///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe maze.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef MAZE_H
#define MAZE_H

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "drive.h"

enum QUARTERS {NONE, NORTH, EAST, SOUTH, WEST};

#define MAZE_SIZE_X 13
#define MAZE_SIZE_Y MAZE_SIZE_X //Has to be a square, important for rotations
#define MAZE_SIZE_Z 2

#define MAZE_SIZE_X_USABLE (MAZE_SIZE_X-1) //Usable tiles (outer tiles cause some errors when accessed)
#define MAZE_SIZE_Y_USABLE (MAZE_SIZE_Y-1)
#define MAZE_SIZE_Z_USABLE MAZE_SIZE_Z

#define ROB_POS_X_MIN			1 //DARF NICHT UNTER 1 LIEGEN!!!
#define ROB_POS_Y_MIN			1

#define ROB_START_MAZE_X	ROB_POS_X_MIN //Startkoordinaten
#define ROB_START_MAZE_Y	ROB_POS_Y_MIN //Kein Startfeld => starte bei y=1 
#define ROB_START_MAZE_Z	0
#define ROB_START_DIR			NORTH

enum XYZ {X = 1, Y, Z};

typedef struct _off OFF;
typedef struct _tile TILE;
typedef struct _coord COORD;
typedef struct _pos POS;
typedef struct _matchingWalls MATCHINGWALLS;

struct _tile {
	unsigned beenthere:1;
	signed ground:7;
	int8_t wall_s;
	int8_t wall_w;
	int8_t obstacle;
	signed victim_n:4;
	signed victim_e:4;
	signed victim_s:4;
	signed victim_w:4;
	uint8_t depthsearch;
};

struct _off {
	int8_t x;
	int8_t y;
};

struct _coord { //Coordinates (x, y and z) of an object
	int8_t x;
	int8_t y;
	int8_t z;
};

struct _pos { //Position (coordinates + orientation (direction) of an object)
	COORD pos;
	int8_t dir;
};

#define MATCHINGWALLS_BUFF 8 //The amount of matchingwalls of the last @param driven tiles is saved and based of this it will be judged if there is a error or not

struct _matchingWalls {
	uint8_t buffer[MATCHINGWALLS_BUFF];
	uint8_t next;
	uint8_t lowmatches;
};

#define MAZE_ISWALL 4 //Above this threshold there is a wall
#define MAZE_ISBLTILE 1 //Above this threshold there is a black tile

#define MAZE_UPDATEWALLFAC_DRIVE_MID 1 //Gewichtung bei Wandkorrektur bei sicheren Mittelpositionen
#define MAZE_UPDATEWALLFAC_TURN 1 //Gewichtung bei Wandkorrektur nach Drehen
#define MAZE_UPDATEGROUNDFAC_DRIVE_MID 40 //Gewichtung bei Untergrundkorrektur bei sicheren Mittelpositionen
#define MAZE_UPDATEGROUNDFAC_DRIVE_SENS 20 //Gewichtung bei Untergrundsensor nach Erkennung
#define MAZE_UPDATEGROUNDFAC_TURN 2 //Gewichtung bei Untergrundkorrektur während Drehen

#define FRONT_FRONT_TH 	170 //Schwellwert nach vorne
#define FRONT_TH 		170 //Schwellwert nach vorne (Sensoren L/R)
#define BACK_TH 		170 //Schwellwert nach hinten "
#define SIDE_TH 		170 //Schwellwert zur Seite
#define BACK_BACK_TH	170

#define DIST_VICTIM_MIN 150

#define DRIVE_BUMPED_WIDTH 2

#define RAMP_UP_SPEED 80
#define RAMP_DOWN_SPEED 60

#define MAZE_WALLVALUE_MAX 126 //++ ~ Overflow
#define MAZE_WALLVALUE_MIN -127 //-- ~Overflow
#define MAZE_ERROR 127 //Fehler

#define MAZE_GROUNDVALUE_MAX 63 ////++ ~ Overflow
#define MAZE_GROUNDVALUE_MIN -63 ////-- ~ Overflow

#define MAZE_OBSTACLEVALUE_MAX 126 ////++ ~ Overflow
#define MAZE_OBSTACLEVALUE_MIN -127 ////-- ~ Overflow

#define MAZE_VICTIMVALUE_MAX 7
#define MAZE_VICTIMVALUE_MIN -7

#define TSL_GROUNDSENS_DIFF 50

#define GROUNDSENS_R_TH_BLACKTILE 850 //Above this Threshold there is a black tile
#define GROUNDSENS_L_TH_BLACKTILE 850 //Above this Threshold there is a black tile
#define GROUNDSENS_L_TH_CHECKPOINT 680 //Below this Threshold there is a checkpoint
#define GROUNDDIST_TH_LOP		50 //Below this Threshold of the sharp IR looking down there is a LOP
#define GROUNDDIST_TH_NORMAL		200 //Above this Threshold of the sharp IR looking down there is no more LOP
#define GROUNDSENS_CNT_TH_CHECKPOINT 5 //More than n times below the Threshold of a checkpoint (IS checkpoint!)
#define GROUNDSENS_CNT_TH_BLACKTILE 12 //More than n times below the Threshold of a black tile
#define RAMP_CNT_ISRAMP 7 //More than n times above the Threshold of ramp (up and down)

#define MAZE_ERR_DEL_RADIUS_L 2 //If the robot detects an error the tiles in the radius of this are cleared (mistaken tiles) (Large Radius)
#define MAZE_ERR_DEL_RADIUS_S 1 //If the robot detects an error the tiles in the radius of this are cleared (mistaken tiles) (Small Radius)

#define TILE_LENGTH_MIN_DRIVE	18.5	//After the robot drive this distance, he will start a new tile in the map, otherwise not!!!

extern TILE maze[MAZE_SIZE_X][MAZE_SIZE_Y][MAZE_SIZE_Z];
extern OFF offset[MAZE_SIZE_Z]; //Für jede Ebene eigenen Offset
	extern int8_t offset_z; //Eigenen Offset für z, da sonst in beiden Offset [0,1] vorhanden wäre
extern COORD rr_result; //RouteRequest startpositions
extern COORD off_start; //offset startposition (change startposition via rotary-encoder)
extern POS robot; //Positionsdaten über Roboter
extern POS checkpoint; //Last passed checkpoint
extern POS ramp[MAZE_SIZE_Z];	//Rampenanschlüsse

extern MATCHINGWALLS matchingWalls;

extern DOT dot; //drive one tail main struct!
//Display

#define WALL_SIZE_MAX 40
#define WALL_SIZE_STD 12
#define WALL_SIZE_MIN 2
#define WALL_SIZE_SMALL 4
#define WALL_SIZE_MEDIUM 16
#define WALL_SIZE_LARGE 24

///////
enum MAZE_STATE_SOLVE { DRIVE_READY, FOLLOW_RIGHTWALL, FOLLOW_DFS, MAZE_ERR, RESTART, LOP_INIT, LOP_WAIT,
												DRIVE_DOT, DRIVE_DOT_DRIVE, TURN_RIGHT, TURN_RIGHT_DRIVE, TURN_LEFT, TURN_LEFT_DRIVE, RAMP_UP, RAMP_DOWN,
												VIC_DEPL, CHECK_BLACKTILE};

enum MAZE_STATE_DRIVE_READY { DR_INIT, DR_UPDATEWALLS, DR_UPDATEVICTIMS, DR_CHECK, DR_MATCH}; //DR = DriveReady

enum MAZE_RR {RR_WAIT, RR_CALCNEARESTTILE, RR_CALCROUTE,
							RR_NEARDONE, RR_NEARNOPOSS, RR_NEARTIMEOUT,
							RR_RTDONE, RR_RTNOPOSS, RR_RTTIMEOUT}; //RR = RouteRequest, RT = Route
					
enum MAZE_LR {LR_WAIT, LR_MATCH, LR_SUCCESS, LR_FAILURE}; //LR = LocalizationRequest;
		
extern uint8_t maze_solve_state_path;

extern int16_t tsl_th;
extern int16_t tsl_th_ground;
extern int16_t ground_th;

extern uint8_t routeRequest;
extern uint8_t locRequest;

extern int8_t wall_size_part;

////////////////////////////////////////

extern void maze_solve_drive_reset(void);

extern uint8_t maze_solve(void);

extern void maze_solveRoutes(void);

extern uint8_t maze_updateWalls(void);

extern void maze_localize(void);

extern void maze_updateGround(int8_t updateFac_ground, int8_t isGround);

extern void u8g_DrawMaze(void);

#endif
