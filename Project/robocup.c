////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////robocup.c/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///	Kopf des Programmes mit Endlosschleife und Timer ISR
///	-> Zusammenführung sämtlicher anderer Funktionen
///		- Auswerten der Encoder und des Inkrementalgebers
///		- Timer
///		- Aufrufen der Initialisierungsfunktion
///		- Aufrufen der Navigationsfunktionen
///		- Batterieauswertung
///		- Fehleranzeige via RGB LED
///		- Opferanzeige
///		- Displayausgaben und Displayloop
////////////////////////////////////////////////////////////////////////////////
///	To do:
/// - checkpoints
/// - bei geradeausfahrt abbruch und keine wand einzeichenn? Gefangen!
/// - überpsringen von Fliesen testen (wegen encodervergleich nach drehen und so) testen!
/// - in der oberen Etage starten funktioniert nicht...
/// - Rampe generell?
///
/// - Kollisionserkennung: Korrektur auch noch über UM6
///		- Außerdem Optimierungsidee: Erkennung über SRF, dann in Richtung Hindernis
///		drehen um zu gucken, wo es steht (über Sharp IR) und dann weit genug in andere Richtugn drehen.
///		Danch wieder gerade drehen!
///	- Roboter hochheben bei RESTART (Timer dann zurücksetzen)
///	- Rampe flexibel einzeichnen
///		- maze_getRamp gibt Position falsch zurück
///			- je nach Rampenrichtung eine Fliese weiter!
///		- dynamisch?
///		- muss Mindestdistanz gefahren sein bevor er in die nächste Etage spring
///		- Fährt IMMER NACH Geradeausfahrt Rampe und nicht vorher
///	- Checkpoint auf Rampe!!!
///	- Opfererkennung automatisch (ohne Schwellwert)
///	- Opfer in die Karte einzeichnen
///		- Bug: Display stürzt ab!
///		- Handhabung: Zwei Opfer auf einer Fliese?
///	- Menü: m2tklib
///		- Weitere Einstellungen für Schwellwerte (zweiter Schwellwert,
///			Schwellwert Untergrundsensor)
///	- Beim SLAM (Rotieren/Übertragen der Wände) werden NUR Wände berücksichtigt
///		(-> Opfer)
///	- Scheduler: Erkennen, ob Task zu viel Zeit braucht und ggf. killen
///	- SLAM
///		- Erkennung einer übersprungenen Fliese?
///	- CMUCAM
///		- Entfernungssensor?
///		- Unterscheidung Wand/Hindernis
///		- Wände: Bit „ausrichten erlaubt" Wenn Sharp IR pro Richtung alle Wand sehen
///		- Sonst in Karte Hindernis einzeichnen
///		- Hindernisumfahrung
////////////////////////////////////////////////////////////////////////////////
///	Testen:
///		- Bug: dot_abort nachdem 16cm gefahren wurden funktioniert nicht???
///			- wahrscheinlich nicht... Beobachten!
///		- Hindernisse: Erkennung, wenn der Roboter nach Ende der Geradeausfahrt aufgrund
///			von Korrekturen schräg steht (vergleich der Encoderwerte links/rechts) und
///			ggf. gerade korrigieren
///      - Kollisionserkennung: Wenn der Roboter an etwas an der Wand hängen bleibt
///			(Winkel zur Wand groß, vordere Sensoren kleine Distanz) korrigieren
////////////////////////////////////////////////////////////////////////////////
/// Zeitplan:
///		- 17. Januar: Untergrundsensor funktioniert wieder
///     - danach: sonstige Tests
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "bluetooth.h"
#include "display.h"
#include "um6.h"
#include "u8g.h"
#include "i2cmaster.h"
#include "uart.h"
#include "system.h"
#include "i2cdev.h"
#include "funktionen.h"
#include "maze.h"
#include "mazefunctions.h"
#include "drive.h"
#include "tsl1401.h"
#include "memcheck.h"
#include "victim.h"
#include "pixy.h"
#include "menu.h"

////////////////////////////////////////////////////////////////////
//UART:
#define UART_UM6_BAUD_RATE    115200
#define UART_MCU_BAUD_RATE    115200

//////////////////////////////////////////////////////////////////////
#define WDT_TRIGGERED() if(MCUSR & (1<<WDRF)) check_res = 1;

////////////////////////////////////////////////////////////////////////////////
//Scheduler: http://www.riosscheduler.org/

task tasks[TASKS_NUM];

int8_t task_maze(int8_t state);
int8_t task_speedreg(int8_t state);
int8_t task_sensors(int8_t state);
int8_t task_anasens(int8_t state);
int8_t task_timer(int8_t state);
int8_t task_cam(int8_t state);	
	
uint8_t runningTasks[TASKS_NUM+1] = {255};	//Track running tasks, [0] always idleTask
uint8_t idleTask = 255;						// 0 highest priority, 255 lowest
uint8_t currentTask = 0;					// Index of highest priority task in runningTasks

uint16_t schedule_time = 0;

////////////////////////////////////////////////////////////////////////
uint8_t check_mlx = 0;
uint8_t check_srf = 0;
uint8_t check_um6 = 0;
uint8_t check_res = 0; //resetsource?
uint8_t check_tsl = 0;
uint8_t fatal_err = 0; //Generel error, log has to be checked when it appears!
/////////////////////////////////////////////////
int32_t displayvar[DISPLAYVARS]; //Temporarly display something somewhere in the program

//////////////////////////////////////
uint8_t led_heartbeatColor = 85;	//Heartbeatcolor (0 == no heartbeat)
uint8_t led_fault = 0;				//Set to != 0, when an error has to be signalised (LED will brightly blink in the color of led_fault)
uint8_t led_top = 0;				//Point when the led swells down -> indirectly speed of led pulse/blink

#define LED_TOP_FAT_ERR 10
#define LED_TOP_NORMAL	30

#define TOGGLE_MAIN_LED() PORTD ^= (1<<PD5);
/////////////////////////////////

int8_t batt_percent = 0;
uint16_t batt_mV_old = 0xffff;
uint16_t batt_mV = 0;

//////////////////////////////////
//GLCD
uint8_t u8g_stateMachine = 0;	//Display
u8g_t u8g;						//u8g contructor

///////////////////////////////Drehgeber/Encoder/Taster/////////////////////////
#define INCR_PHASE_A     (PINC & (1<<PC1))
#define INCR_PHASE_B     (PINC & (1<<PC2))

const int8_t incr_table[16] PROGMEM = {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, 0, 0, 0, 0};    //Steptabelle
static int8_t incr_last = 0;
int32_t incremental = 0;


#define ENC_L_PHASE_A     (PINA & (1<<PA0))
#define ENC_L_PHASE_B     (PINE & (1<<PE7))

static int8_t enc_l_last = 0;
//Encoderwert in Datenstruktur für Motoren

#define ENC_R_PHASE_A     (PINE & (1<<PE6))
#define ENC_R_PHASE_B     (PINA & (1<<PA1))

static int8_t enc_r_last = 0;
//Encoderwert in Datenstruktur für Motoren

//////////
uint8_t hold_t1 = 0;
////////////////////////////////////Sonstiges///////////////////////////////////

uint8_t setup = 0; //Start the setup?

uint8_t debug = 0; //Debugmodus, Read from EEPROM in init_sys()
uint8_t debug_err_sendOneTime = 0;
///////////////////////////////Timer////////////////////////////////////////////

int16_t timer_victim_led = -1;
int8_t timer_entpr_tast = 0;
int8_t timer_incr_entpr = 0;
int8_t timer_bt_is_busy = 0;
int8_t timer_disp_msg = 0;
int8_t timer_get_tast = 0;
int16_t timer_rdy_restart = -1;
int8_t timer_map_wall_r = 0;
int16_t timer_lop = -1;
int8_t timer_vic_ramp = -1;

uint8_t timer_25ms = 0; //make the upper timers decrement only every 25ms in the timer task

uint32_t timer = 0; //Timer, resolution 1ms, continuisly incrementing in the scheduler ISR

////////////////////////////////////////////////////////////////////////////////

ISR(TIMER1_COMPA_vect) //1kHz
{
	for(uint8_t i = 0; i < TASKS_NUM; i++) 					// Heart of scheduler code
	{
		if((tasks[i].elapsedTime >= tasks[i].period)	// Task ready
			&& (runningTasks[currentTask] > i)					// Task priority > current task priority
			&& (!tasks[i].running))											// Task not already running (no self-preemption)
		{
			cli();
			tasks[i].elapsedTime = 0;										// Reset time since last tick
			tasks[i].running = 1;												// Mark as running
			currentTask += 1;
			runningTasks[currentTask] = i;							// Add to runningTasks
			sei();

			tasks[i].state = tasks[i].task_fct(tasks[i].state);	// Execute tick

			cli();
			tasks[i].running = 0;												// Mark as not running
			runningTasks[currentTask] = idleTask;				// Remove from runningTasks
			currentTask -= 1;
			sei();
		}
		/*else if((tasks[i].elapsedTime >= tasks[i].period)	// Task ready
				&& (tasks[i].running))											// Task still running
		{
			displayvar[4] = i;
			tasks[i].period = 1000;
			tasks[i].running = 0;
		}*/
		tasks[i].elapsedTime += TASKPERIOD_GCD;
	}
}

/*### Mainloop ###*/
int main(void)
{
	WDT_TRIGGERED(); //Watchdogreset?

	init_sys();
	init_pwm();
	init_timer();
	dist_init();
	uart1_init(UART_BAUD_SELECT(UART_UM6_BAUD_RATE,F_CPU)); //IMU
	uart_init(UART_BAUD_SELECT(UART_MCU_BAUD_RATE,F_CPU)); //Bluetooth
	init_display(1);
	//init_m2(); //Menu lib
	init_adc();
	init_i2c();
    //init_srf10();
	maze_init();
	victim_init();

	for(uint8_t i = 0; i < DISPLAYVARS; i++)
		displayvar[i] = DISPLAYVAR_UNUSED;

	mot.off = 1;
	//The higher the task_i of the task is, the higher is the priority
	
	tasks[TASK_MAZE_ID].state = -1;
	tasks[TASK_MAZE_ID].period = TASK_PERIOD_MAZE;
	tasks[TASK_MAZE_ID].elapsedTime = 0;
	tasks[TASK_MAZE_ID].running = 0;
	tasks[TASK_MAZE_ID].task_fct = &task_maze;
	
	tasks[TASK_TIMER_ID].state = -1;
	tasks[TASK_TIMER_ID].period = TASK_PERIOD_TIMER;
	tasks[TASK_TIMER_ID].elapsedTime = 0;
	tasks[TASK_TIMER_ID].running = 0;
	tasks[TASK_TIMER_ID].task_fct = &task_timer;

	tasks[TASK_SPEEDREG_ID].state = -1;
	tasks[TASK_SPEEDREG_ID].period = TASK_PERIOD_SPEEDREG;
	tasks[TASK_SPEEDREG_ID].elapsedTime = 0;
	tasks[TASK_SPEEDREG_ID].running = 0;
	tasks[TASK_SPEEDREG_ID].task_fct = &task_speedreg;
	
	tasks[TASK_SENSORS_ID].state = -1;
	tasks[TASK_SENSORS_ID].period = TASK_PERIOD_SENSORS;
	tasks[TASK_SENSORS_ID].elapsedTime = 0;
	tasks[TASK_SENSORS_ID].running = 0;
	tasks[TASK_SENSORS_ID].task_fct = &task_sensors;

	tasks[TASK_ANASENS_ID].state = -1;
	tasks[TASK_ANASENS_ID].period = TASK_PERIOD_ANASENS;
	tasks[TASK_ANASENS_ID].elapsedTime = 0;
	tasks[TASK_ANASENS_ID].running = 0;
	tasks[TASK_ANASENS_ID].task_fct = &task_anasens;

	tasks[TASK_CAM_ID].state = -1;
	tasks[TASK_CAM_ID].period = 20;//TASK_PERIOD_CAM;
	tasks[TASK_CAM_ID].elapsedTime = 0;
	tasks[TASK_CAM_ID].running = 0;
	tasks[TASK_CAM_ID].task_fct = &task_cam;
	
	if(get_incrOk())
	{
		motor_activate(0); //Shut down motor driver
		setup = 1;
	}
	else
	{
		motor_activate(1); //Activate motor driver
		setup = 0;
	}
	
	sei(); //Enable global interrupts. The Operating System and every task in it is running now and the cam already can regulate its initial aparture

	//u8g_DrawStartUp();
	
	if(debug > 0)
	{
		bt_putStr_P(PSTR("\n\rRCJ 2014, V3.0\n\rteamohnename.de"));
		bt_putStr_P(PSTR("\n\rDebugging grade: ")); bt_putLong(debug);
		bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": System initialized, ")); bt_putLong(TASKS_NUM); bt_putStr_P(PSTR(" running tasks."));
	}
	
	if(check_res)
	{
		if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr(PSTR(": WARNING: RECOVERED AFTER AN UNEXPECTED SHUTDOWN!!!"));}
		_delay_ms(5000);
	}

	//wdt_enable(WDTO_8S); //activate watchdog

	while(1)
    {
		wdt_reset();

		TOGGLE_MAIN_LED(); //Toggle LED on the RNmega Board

		////////////////////////////////////////////////////////////////////////////

		maze_solveRoutes(); //Has to be called to calculate routes in main-loop, when nessesary (because it needs up to 2s)
		//maze_localize(); //"

		////////////////////////////////////////////////////////////////////////////
		if(get_t1())
		{
			if((timer_get_tast == 0) && (timer_entpr_tast == 0) && (!hold_t1))
			{
				timer_get_tast = TIMER_GET_TAST;
				hold_t1 = 1;
			}
			else if((timer_get_tast == 0) && hold_t1)
			{
				drive_reset(&dot); //Fahrfunktionen zurücksetzen
				maze_clearDepthsearch();
					maze_solve_state_path = DRIVE_READY;
					routeRequest = RR_WAIT;

				hold_t1 = 0;
				mot.off ^= 1;
				timer_entpr_tast = TIMER_ENTPR_TAST;
			}
		}
		else
		{
			timer_get_tast = 0;
			hold_t1 = 0;
		}

		////////////////////Sensorcoordination//////////////////////////////////////

		if(check_res || fatal_err)	//Flashing of the Info LED when there is an error
			led_fault = 1;
		else if(check_mlx != 0)
			led_fault = 55;
		else if(check_srf != 0)
			led_fault = 130;
		else if(check_um6 != 0)
			led_fault = 180;
		else
			led_fault = 0;
		
		if(fatal_err)
			led_top = LED_TOP_FAT_ERR;
		else
			led_top = LED_TOP_NORMAL;
			
		if(check_res)
		{
			if(!(debug_err_sendOneTime & (1<<0)))
			{
				if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR: RESET"));}
				debug_err_sendOneTime |= (1<<0);
			}
		}
		else	debug_err_sendOneTime &= ~(1<<0);
		
		if(check_mlx != 0)
		{
			if(!(debug_err_sendOneTime & (1<<1)))
			{
				if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR: Melexis MLX90614 Temperature sensors"));}
				debug_err_sendOneTime |= (1<<1);
			}
		}
		else	debug_err_sendOneTime &= ~(1<<1);
		
		if(check_um6 != 0)
		{
			if(!(debug_err_sendOneTime & (1<<2)))
			{
				if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR: CHRobotics UM6 IMU"));}
				debug_err_sendOneTime |= (1<<2);
			}
		}
		else	debug_err_sendOneTime &= ~(1<<2);
		
		//Batterie/Akku
		if(batt_raw > 0)
		{
			batt_mV = (batt_raw*15)-300;

			if(batt_mV < batt_mV_old)
			{
				batt_mV_old = batt_mV;
				batt_percent = (0.037*batt_mV) - 363;
			}
			if(batt_percent < 20) //Batterie
				led_heartbeatColor = batt_percent;
		}

		////////////////////////////////////////////////////////////////////////////
		//LED heartbeat

		led_rgb(led_heartbeatColor, led_fault, led_top);

		////////////////////////Display/Bluetooth///////////////////////////////////
		//displayvar[4] = victimBuf[LEFT].value[0];
		//displayvar[5] = (victimBuf[LEFT].value[0]-victimBuf[LEFT].lowest);


		//setup = 1;

		if(!u8g_stateMachine)
		{
			u8g_FirstPage(&u8g);
			u8g_stateMachine = 1;
		}
		//////////////Displayausgaben////////////////

		u8g_DrawInfo(); //Message Line (upper 8 lines of lcd) in display.c!

		if(setup == 1)
			u8g_DrawSetUp();
			//m2_drawSetup();
		else if(setup == 2)
			u8g_DrawView();
		else if(setup == 3)
			u8g_DrawCamRaw();
		else if(setup == 4)
			u8g_DrawVictim();
		else
		{
			u8g_DrawMaze(); //Everything about the maze (in maze.c!)
			//u8g_DrawPixy();
			//u8g_DrawTempScan(LEFT);
			//u8g_DrawFrontScan();

			u8g_DrawVLine(&u8g, 87, 7, 57); //vertical line

			///////Infos//////////////////
			//  Karte   //Pfeil// Frei  //
			//          //     // hier: //
			//          /////////       //
			//          //Info //   X   //
			//          //Karte//       //
			//////////////////////////////

			u8g_SetFont(&u8g, u8g_font_5x7);

			displayvar[7] = get_mem_unused();

			for(uint8_t i = 0; i <= DISPLAYVARS; i++)
				if(displayvar[i] != 0xffff)
					u8g_DrawLong(90, i*7+15, displayvar[i]);
		}

		if(!u8g_NextPage(&u8g))
		{
			u8g_stateMachine = 0;
		}
  }
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////TASK MAZE///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int8_t testvar = 0;
uint8_t groundvar = 0;

D_DEPLOYKIT dep;

int8_t task_maze(int8_t state)
{	
	if(setup == 0)
	{
		maze_solve();
		//victim_check();
	}

	/*if(testvar == 1)
	{
		drive_deployResKit(&dep);
		if(dep.state == DK_FINISHED)
		{
			testvar = 2;
			dep.state = DK_INIT;
		}
	}
	else
	{
		if(get_incrOk())
		{
			dep.amount_to = 1;
			dep.config_dir = RIGHT;
			dep.config_no_turnBack = 1; //Don`t turn back after deployment!

			testvar = 1;
		}
	}*/

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////TASK SPEEDREG/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int8_t task_speedreg(int8_t state)
{
	//Turn motor off?
	if(	mot.off || mot.off_invisible)
	{
		mot.d[LEFT].speed.to = 0;
		mot.d[RIGHT].speed.to = 0;
	}
	controlSpeed(); //Speed Regulation
	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////TASK SENSDIGI/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int8_t task_sensors(int8_t state)
{
	//I²C sensors
	///Temperature
	check_mlx = getIR();
	victim_scan();

	////Ultrasonic distance
	//check_srf = getSRF();

	//UM6
	check_um6 = um6_getUM6();

	//pixy_get();
	//displayvar[2] = pixy_number_of_blocks;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////TASK SENSANA/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int8_t task_anasens(int8_t state)
{
	//analog
	get_analogSensors(); //Sharp infrared distance sensors, groundsensor
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////TASK TIMER//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int8_t task_timer(int8_t state)
{
	//////(rotary) encoder///////////
	//Source: http://www.mikrocontroller.net/articles/Drehgeber

	incr_last = (incr_last << 2)  & 0x0F;
	if(INCR_PHASE_A)
		incr_last |=2;
	if(INCR_PHASE_B)
		incr_last |=1;
	incremental += (int8_t)pgm_read_byte(&incr_table[incr_last]);
	
	
	int8_t new, diff;
			 
	new = 0;
	if(ENC_L_PHASE_A)		new = 3;
	if(ENC_L_PHASE_B)		new ^= 1;		// convert gray to binary
	diff = enc_l_last - new;				// difference last - new
	if( diff & 1 ){									// bit 0 = value (1)
		enc_l_last = new;							// store new as next last

		mot.d[LEFT].enc += (diff & 2) - 1;	// bit 1 = direction (+/-)
		mot.d[LEFT].enc_abs += abs((diff & 2) -1);
	}

	new = 0;
	if(ENC_R_PHASE_A)		new = 3;
	if(ENC_R_PHASE_B)		new ^= 1;		// convert gray to binary
	diff = enc_r_last - new;				// difference last - new
	if( diff & 1 ){									// bit 0 = value (1)
		enc_r_last = new;							// store new as next last
		mot.d[RIGHT].enc += (diff & 2) - 1;	// bit 1 = direction (+/-)
		mot.d[RIGHT].enc_abs += abs((diff & 2) -1);
	}

	mot.enc = ((mot.d[LEFT].enc/2) + (mot.d[RIGHT].enc/2));

	//////Timer/////////////

	timer++;
	timer_25ms ++;
	if(timer_25ms == 25) //40Hz
	{
		if(timer_victim_led > 0)
			timer_victim_led --;
		if(timer_entpr_tast > 0)
			timer_entpr_tast --;
		if(timer_incr_entpr > 0)
			timer_incr_entpr --;
		if(timer_bt_is_busy > 0)
			timer_bt_is_busy --;
		if(timer_disp_msg > 0)
			timer_disp_msg --;
		if(timer_get_tast > 0)
			timer_get_tast --;
		if(timer_rdy_restart > 0)
			timer_rdy_restart --;
		if(timer_map_wall_r > 0)
			timer_map_wall_r --;
		if(timer_lop > 0)
			timer_lop --;
		if(timer_vic_ramp > 0)
			timer_vic_ramp --;

		timer_25ms = 0;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////TASK CAM///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int8_t task_cam(int8_t state)
{
	//pixy_get();

	return 0;//tsl1401(state);
}
