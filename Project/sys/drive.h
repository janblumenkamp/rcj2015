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


typedef struct _d_rotate D_ROTATE;
typedef struct _dot DOT;
typedef struct _d_turn D_TURN;
typedef struct _d_deplKit D_DEPLOYKIT;

/////////////////////////////////////////////////////////////////////////////////////
///
/// drive align
#define KP_ALIGN 1.8

#define ALIGN_OFFSET_FRONT 0 //Roboter ist nicht zwangsläufig bei 0 Differenz gerade ==> Ausgleich (anderer Sollwert für Differenz)
#define ALIGN_OFFSET_BACK 0
#define ALIGN_OFFSET_LEFT 0
#define ALIGN_OFFSET_RIGHT 0

#define TILE1_FRONT_ALIGN_TH 150 //mm, Schwellwert nach vorne
#define TILE1_SIDE_ALIGN_TH 160 //Schwellwert zur Seite

#define TURN_SENSDIFF_MAX 150 //-CONST - CONST ist Schwellwert für maximal erlaubte Sensordifferenz (ansonsten Fehler)

#define STEER_ALIGN_DONE 1

#define TIMER_ALIGN	500 //Maximal so lange ausrichten, dann abbrechen

/////////////////////////////////////////////////////////////////////////////////////

///drive_rotate
enum DRIVE_ROTATE {ROTATE_INIT, ROTATE, ROTATE_END, ROTATE_FINISHED};

#define TIMER_ROTATE	1000 //Wenn steer zu klein und Opfererkennung => zurücksetzen => dreht nicht => abbrechen

#define ENC_DEGROTFAC 4.8

#define KP_ROTATE 3

#define STEER_ROTATE_ENC_TH 1 //Wenn UM6 eigtl. fertig ist, ENC aber noch nciht weitgenug gezählt haben (TH für Steer (=> Ende naht))
#define STEER_ROTATE_ENC 100//Mit dem Steer drehen (bei UM6 err)

#define UM6_ROTATE_OFFSET -3 //The smaller this offset, the less the robot rotates (usually as high as drift of the UM6)

#define STEER_ROTATE_TH_TIMER 20 //Unter diesem Wert (Betrag) wird ein Timer aktivierter, in dem Zeitraum 0 erreicht werden muss, ansonsten abbruch.

struct _d_rotate {
	unsigned state:7;
	unsigned use_enc:1; //Use the um6 or the encoders for rotating?

	int16_t angle; //Angle to rotate
	int8_t speed_limit; //Maximum rotating speed

	int32_t um6_psi_t_start;
	int32_t enc_l_start;
	int32_t enc_r_start;

	uint32_t timer;

	int16_t steer;

	uint8_t progress; //How much did the robot already move (%)
};

/////////////////////////////////////////////////////////////////////////////


enum DOT_STATES         {DOT_INIT, DOT_ALIGN_BACK, DOT_ALIGN, DOT_DRIVE, DOT_ROT_WEST, DOT_ROT_EAST, DRIVE_ROT_STRAIGHT, DOT_COMP_ENC, DOT_CORR, DOT_ALIGN_WALL, DOT_END, DOT_FINISHED};

#define MAXSPEED 100

///////////////////////drive_oneTile//////////
#define ENC_FAC_CM_L 23.5 //Fakor: Encoderwert in Strecke (cm) umrechnen
#define ENC_FAC_CM_R 23.5
#define ENC_FAC_CM_LR 23.5

#define TILE1_FRONT_FRONT 		55	//mm
#define TILE1_FRONT_TH_FRONT	190
#define TILE1_BACK_BACK			55	//Wenn eine Wand direkt hinter dem Roboter ist, ist bei diesem Rücksensorwert der Roboter in der Mitte
#define TILE1_BACK_TH_BACK		230

#define TILE1_SIDE_TH			120

#define DIST_SOLL	55	//IR; Seitenabstand

#define TILE_LENGTH				30	//cm
#define TILE_DIST_COLLISION_AV	25	//Only if the robot has driven less than this distance, the collision avoidance is active.
#define DIST_ADD_COLLISION		1	//This distance is added to the distance-to-drive after the robot avoided a collision
#define DIST_ADD_COLLISION_MAX	5

#define SPEED_COLLISION_AVOIDANCE		20	//After the robot detected a collision, he turns with this speed at the point until the object in the way is gone
#define TURN_ANGLE_COLLISION_AVOIDED	12	//After the object in the way is gone, the robot turns on this angle to be sure

#define COLLISIONAVOIDANCE_SENS_TH_1	60	//If one front sensor is UNDER this threshold
#define COLLISIONAVOIDANCE_SENS_TH_2	150	//and the other two front sensors are ABOVE this threshold, there will happen a collision!

#define KP_DOT_DIR	1.3
#define KP_DOT_DIST	KP_DOT_DIR * 0.6

#define KP_ALIGN_FRONT	0.6
#define KP_ALIGN_BACK	0.6

#define DELTADIST_MAX	15	//Highest allowed distance difference from one point to the next (the sensorvalues go up in two steps if there is a big difference)

#define STEER_ALIGN_BACK_END	2	//Below this steering the alignment on the back is good and stops.
#define STEER_DOT_ALIGN_TOP_END		2	//----------------"----------------------- top --------"---------


#define TIMER_DOT_ALIGN			3000 //So lange wird maximal vorne/hinten angepasst


struct _dot {
	unsigned state:7; //statemachine
	unsigned abort:1; //abort driving one tile, drive back!

	uint32_t timer; //Timer for all drive functions (to abort after time...)

	uint8_t aligned_turn; //The robot had to align via the front Sensors (collision avoidance)?

	int32_t enc_comp[2]; //Compare angle of robot (driven not straight? Has to correct angle (important if there is no wall)?)
	int16_t corr_angle;

	int32_t enc_lr_start;
	int16_t enc_lr_add; //Add this length to the distance-to-drive after an (avoided) collision

	int16_t um6_phi_t_start; //Slow down when getting odd (Ramp etc.)

	int16_t steer; //Steering value for all the steering stuff. Has to be global, dont know why.

	int8_t maxspeed; //Maximum speed to drive

	D_ROTATE r; //For intern rotate function
};


////////////////////////////////////////////////////////////////////////////////////

enum DRIVE_TURN {TURN_INIT, TURN, TURN_ALIGN, TURN_ALIGN_BACK, TURN_END, TURN_FINISHED};

struct _d_turn {
	unsigned state:7;
	unsigned no_align:1; //dont alignment functions?
	uint8_t newRobDir; //Planned robot direction after turn

	D_ROTATE r; //For intern rotate function
};

////////////////////////////////////////////////////////////////////////////////////

enum DRIVE_DEPLKIT_CONF {DEPLOY_RIGHT, DEPLOY_LEFT, DEPLOY_RIGHT_TURN_BACK, DEPLOY_LEFT_TURN_BACK};
enum DRIVE_DEPLKIT {DK_INIT, DK_TURN_A, DK_ALIGN_A, DK_DEPL, DK_TURN_B, DK_ALIGN_B, DK_CHECK_TURN, DK_END, DK_FINISHED};

struct _d_deplKit {
	unsigned state:5;
	unsigned config_dir:1; //Deploy in which direction (LEFT/RIGHT?)
	unsigned config_turnBack:1; //Tturn back after deployment?
	unsigned alignedToBackwall:1;

	unsigned amount_to:4; //Amount of kits to deploy
	unsigned amount_is:4; //Amount of kits already deployed

	D_TURN turn; //Turning instruction
};

////////////////////////////////////////////////////////////////////////////////////
#define TIMER_ALIGN_BACK 2000

extern int32_t ramp_enc_start;

////////////////////////////////////////////////////////////////////////////////////
extern int32_t enc_lr_start_dot;

extern int8_t ramp_checkpoint;

extern void drive_oneTile(DOT *d);

extern uint8_t drive_ramp(int8_t speed_ramp_to, int8_t *checkpoint_ramp);

extern uint8_t drive_align(void);

extern uint8_t drive_align_back(uint8_t dist_to);

extern void drive_rotate(D_ROTATE *r);

extern void drive_turn(D_TURN *t);

extern uint8_t drive_instructions(char *instructions, uint8_t amount);

extern uint8_t drive_neutralPos(void);

extern void drive_deployResKit(D_DEPLOYKIT *dk);

extern uint8_t drive_lr(int8_t left, int8_t speed, uint8_t width);

extern uint8_t drive_dist(int8_t motor, int8_t speed_dd_to, int8_t dist_cm); //motor: <0 links, 0 beide, >0 rechts

extern void drive_reset(void);

#endif
