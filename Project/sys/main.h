////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////main.h/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Siehe robocup.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <avr/wdt.h> //watchdog
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <math.h>
#include <avr/eeprom.h>
#include "u8g.h"
#include "um6.h"

extern u8g_t u8g; //Pointer für u8g muss hier eingebunden werden

extern UM6_t um6; //UM6 sructure

///////////////////////////////////////////
#define TASKS_NUM				4
#define TASKPERIOD_GCD			1

#define TASK_PERIOD_MAZE		5
	#define TASK_MAZE_ID			1
#define TASK_PERIOD_TIMER		1
	#define TASK_TIMER_ID			0
#define TASK_PERIOD_SPEEDREG	25
	#define TASK_SPEEDREG_ID		3
#define TASK_PERIOD_SENSORS		40
	#define TASK_SENSORS_ID			2

struct _task
{
	uint8_t running;						// 1 indicates task is running
	int8_t state;								// Current state of state machine
	uint16_t period;						// Rate at which the task should tick
	uint16_t elapsedTime;				// Time since task's previous tick
	int8_t (*task_fct)(int8_t);	// Function to call for task's tick
};

typedef struct _task task;
extern task tasks[TASKS_NUM];

/////////////////////////////////////////

#define DISPLAYVARS 8
#define DISPLAYVAR_UNUSED 0xffff

extern int32_t displayvar[DISPLAYVARS];

enum DIRECTIONS {LEFT, RIGHT, FRONT, BACK};

extern int8_t batt_percent;

extern uint16_t batt_mV;

extern uint8_t check_mlx;
extern uint8_t check_srf;
extern uint8_t check_um6;
extern uint8_t check_res;
extern uint8_t fatal_err;
extern uint8_t setup;
extern uint8_t debug;
extern int32_t incremental;

#define TIMER_VICTIM_LED	2000/25 //So lange leuchtet LED und steht Robo bei Fund
#define TIMER_ENTPR_TAST	500/25  //Timer zum Entprellen von Taster
#define TIMER_GET_TAST		1300/25	//So lange muss der Taster gedrückt werden, bis eine Eingabe registriert wird
#define TIMER_ENTPR_INCR	500/25  //Timer zum Entprellen von incr_ok
#define TIMER_BT_IS_BUSY	127 //Timer für Bluetooth DIsplayanzeige (Nach Funkstille Anzeige deaktivieren)
#define TIMER_DISP_MSG		127 //Bei mehreren Statusmeldungen werden die der Reihe nach oben angezeigt.
#define TIMER_RDY_RESTART	45000/25 //Nachdem Labyrinth (vermeintlich?) fertig, wrid nach TIMER neugestartet.
#define TIMER_MAP_WALL_R	50/25		//Wait this time until the wallvalues will be refreshed again (let the distance sensros time to refresh)
#define TIMER_LOP_SENSOR	1000/25 //The groundsens has to be above it’s threshold for this time
#define TIMER_LOP_RESET		5000/25 //After an LOP was detected (the robot was lifted) and set down again, the robot waits this time until it proceeds
#define TIMER_VIC_RAMP		1000/25 //After detecting a victim on the ramp wait this time

extern uint32_t timer;
extern int16_t timer_victim_led;
extern int8_t timer_entpr_tast;
extern int8_t timer_incr_entpr;
extern int8_t timer_bt_is_busy;
extern int8_t timer_disp_msg;
extern int16_t timer_motoff;
extern int16_t timer_rdy_restart;
extern int8_t timer_map_wall_r;
extern int16_t timer_lop;
extern int16_t timer_vic_ramp;

extern int32_t incremental;

#define TRUE 1
#define FALSE 0
