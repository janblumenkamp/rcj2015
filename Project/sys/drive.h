////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////drive.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	siehe drive.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifndef DRIVE_H
#define DRIVE_H

enum DOT_STATES         {DOT_INIT, DOT_ALIGN_BACK, DOT_ALIGN, DOT_DRIVE, DOT_ROT_WEST, DOT_ROT_EAST, DOT_COMP_ENC, DOT_CORR, DOT_ALIGN_WALL, DOT_END};

#define MAXSPEED 100

///////////////////////drive_oneTile//////////
#define ENC_FAC_CM_L 22 //Fakor: Encoderwert in Strecke (cm) umrechnen
#define ENC_FAC_CM_R 22
#define ENC_FAC_CM_LR 22.5

#define TILE1_FRONT_FRONT 		90	//mm
#define TILE1_FRONT_TH_FRONT	240
#define TILE1_BACK_BACK			70	//Wenn eine Wand direkt hinter dem Roboter ist, ist bei diesem Rücksensorwert der Roboter in der Mitte
#define TILE1_BACK_TH_BACK		250	//Gibt es eine Wand hinter dem Roboter auf dieser Fliese?

#define TILE1_SIDE_TH			150

#define DIST_SOLL	56	//IR; Seitenabstand

#define TILE_LENGTH				30	//cm
#define TILE_LENGTH_MIN_DRIVE	18.5	//After the robot drive this distance, he will start a new tile in the map, otherwise not!!!
#define TILE_DIST_COLLISION_AV	25	//Only if the robot has driven less than this distance, the collision avoidance is active.
#define DIST_ADD_COLLISION		1	//This distance is added to the distance-to-drive after the robot avoided a collision
#define DIST_ADD_COLLISION_MAX	5

#define SPEED_COLLISION_AVOIDANCE		20	//After the robot detected a collision, he turns with this speed at the point until the object in the way is gone
#define TURN_ANGLE_COLLISION_AVOIDED	12	//After the object in the way is gone, the robot turns on this angle to be sure

#define COLLISIONAVOIDANCE_SENS_TH_1	80	//If one front sensor is UNDER this threshold
#define COLLISIONAVOIDANCE_SENS_TH_2	150	//and the other two front sensors are ABOVE this threshold, there will happen a collision!

#define KP_DOT_DIR	1.3
#define KP_DOT_DIST	KP_DOT_DIR * 0.6

#define KP_ALIGN_FRONT	0.6
#define KP_ALIGN_BACK	0.6

#define DELTADIST_MAX	15	//Highest allowed distance difference from one point to the next (the sensorvalues go up in two steps if there is a big difference)

#define STEER_ALIGN_BACK_END	2	//Below this steering the alignment on the back is good and stops.
#define STEER_DOT_ALIGN_TOP_END		2	//----------------"----------------------- top --------"---------


#define TIMER_DOT_ALIGN			3000 //So lange wird maximal vorne/hinten angepasst

typedef struct _dot DOT;

struct _dot {
	unsigned state:7; //statemachine
	unsigned abort:1; //abort driving one tile, drive back!

	uint32_t timer; //Timer for all drive functions (to abort after time...)

	int16_t dist_r_old; //Last sensor distance (avoid huge jumps)
	int16_t dist_l_old;

	uint8_t aligned_turn; //The robot had to align via the front Sensors (collision avoidance)?

	int32_t enc_comp[2]; //Compare angle of robot (driven not straight? Has to correct angle (important if there is no wall)?)
	int16_t corr_angle;

	int32_t enc_lr_start;
	int16_t enc_lr_add; //Add this length to the distance-to-drive after an (avoided) collision

	int16_t um6_phi_t_start; //Slow down when getting odd (Ramp etc.)
};

/////////////////////////////////////////////////////////////////////////////////////
///
/// drive align
#define KP_ALIGN 1.8

#define ALIGN_OFFSET_FRONT 0 //Roboter ist nicht zwangsläufig bei 0 Differenz gerade ==> Ausgleich (anderer Sollwert für Differenz)
#define ALIGN_OFFSET_BACK 0
#define ALIGN_OFFSET_LEFT 0
#define ALIGN_OFFSET_RIGHT 0

#define TILE1_FRONT_ALIGN_TH 100 //mm, Schwellwert nach vorne
#define TILE1_SIDE_ALIGN_TH 100 //Schwellwert zur Seite

#define TURN_SENSDIFF_MAX 150 //-CONST - CONST ist Schwellwert für maximal erlaubte Sensordifferenz (ansonsten Fehler)

#define STEER_ALIGN_DONE 1

#define TIMER_ALIGN	500 //Maximal so lange ausrichten, dann abbrechen

/////////////////////////////////////////////////////////////////////////////////////

///drive_rotate

#define TIMER_ROTATE	1000 //Wenn steer zu klein und Opfererkennung => zurücksetzen => dreht nicht => abbrechen

#define ENC_DEGROTFAC 4.8
extern uint8_t rotate_progress;

extern int8_t ramp_checkpoint;

////////////////////////////////////////////////////////////////////////////////////

#define TIMER_ALIGN_BACK 2000

////////////////////////////////////////////////////////////////////////////////////
extern int32_t enc_lr_start_dot;

extern uint8_t drive_oneTile(DOT *d);

extern uint8_t drive_ramp(int8_t speed_ramp_to);

extern uint8_t drive_align(void);

extern uint8_t drive_align_back(uint8_t dist_to);

extern uint8_t drive_rotate(int16_t angle, uint8_t maxspeed);

extern uint8_t drive_turn(int16_t angle, uint8_t align);

extern uint8_t drive_getBall(void);

extern uint8_t drive_releaseBall(void);

extern uint8_t drive_instructions(char *instructions, uint8_t amount);

extern uint8_t drive_neutralPos(void);

extern uint8_t drive_deployResKit(int8_t dir, uint8_t amount);

extern uint8_t drive_lr(int8_t left, int8_t speed, uint8_t width);

extern uint8_t drive_dist(int8_t motor, int8_t speed_dd_to, int8_t dist_cm); //motor: <0 links, 0 beide, >0 rechts

extern void drive_reset(DOT *d);

#endif
