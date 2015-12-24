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
#include "drive.h"

////////////////////////////////////////////////////////////////////
//UART:
#define UART_UM6_BAUD_RATE    115200
#define UART_MCU_BAUD_RATE    115200

////////////////////////////////////////////////////////////////////////
uint16_t check_srf = 0;
uint8_t check_mlx = 0;
uint8_t check_um6 = 0;
uint16_t check_sir = 0; //SharpIR
/////////////////////////////////////////////////
int32_t displayvar[3]; //Zur AUsgabe von Zahlen (damit auch aus voids etwas ausgegeben werden kann)

uint8_t motor_off = 1; //Mit dem kleinen Taster kann man Motoren an/aus schalten
//////////////////////////////////////
uint8_t led_heartbeatColor = 85; //Farbe, in der der Heartbeat läuft (0 == kein Heartbeat)
uint8_t led_fault = 0; //Auf !=0 setzen, wenn ein Fehler im Programm signalisiert werden soll (LED blinkt dann in entsprechender Farbe)

//////////////////////////////////
const uint8_t FRONT = 0;
const uint8_t BACK = 1;
const uint8_t LEFT = 2;
const uint8_t RIGHT = 3;
/////////////////////////////////

int8_t batt_percent = 0;
uint16_t batt_mV_old = 0xffff; //Muss für Vergleich gefüllt werden
uint16_t batt_mV = 0;

//////////////////////////////////
//GLCD
uint8_t u8g_stateMachine = 0; //Displayaufbau
u8g_t u8g; //Pointer für u8g

///////////////////////////////Drehgeber/Encoder////////////////////////////////
#define INCR_PHASE_A     (PINC & (1<<PC1))
#define INCR_PHASE_B     (PINC & (1<<PC2))

static int8_t incr_last = 0;
int8_t incr_delta = 0; //Zum Aufaddieren auf bestimmten Wert (um bei 0 anfangen zu können)
int16_t incremental = 0;


#define ENC_L_PHASE_A     (PINA & (1<<PA0))
#define ENC_L_PHASE_B     (PINE & (1<<PE7))

static int8_t enc_l_last = 0;
int32_t enc_l = 0;


#define ENC_R_PHASE_A     (PINA & (1<<PA1))
#define ENC_R_PHASE_B     (PINE & (1<<PE6))

static int8_t enc_r_last = 0;
int32_t enc_r = 0;
////////////////////////////////////Sonstiges///////////////////////////////////
uint8_t testvar = 0;

uint8_t setup = 0; //Einstellen von Sensorwerten?
	int16_t mlx90614_l_th = 2900; //Schwellwerte
	int16_t mlx90614_r_th = 2900;
	uint8_t use_tarry = 1; //Algo oder rechte Hand?
///////////////////////////////Timer////////////////////////////////////////////
#define TIMER_MOTOR_SPEED 2 //SIEHE AUCH SYSTEM.C!!!!!!!!!!!!!!!! Zeitraum in dem die zurückgelegte Strecke (Encoder) gemessen wird
#define TIMER_BT_SEND_DAT 2000/25 //In dieser Frequenz werden Daten per Bluetooth gesendet
#define TIMER_ADD_VAR_DRV 4 ////SIEHE AUCH MAZE.C!!!!!!!!! In dieser Frequenz wird der Wert für Entf. etc integriert (Geradeausfahrt).
#define TIMER_VICTIM_FND  3000/25 //Nach dieser Zeit kann ein Opfer wieder gefunden werden (Vermeidet schnelles Finden hintereinander)
#define TIMER_VICTIM_LED  2000/25 //So lange leuchtet LED und steht Robo bei Fund
#define TIMER_ENTPR_TAST	1000/25  //Timer zum Entprellen von Taster
#define TIMER_ENTPR_INCR  500/25  //SIEHE MAZE.C/DISPLAY.C!!! Timer zum Entprellen von incr_ok
#define TIMER_BT_IS_BUSY  127 //SIEHE BLUETOOTH.C!!! Timer für Bluetooth DIsplayanzeige (Nach Funkstille Anzeige deaktivieren)
#define TIMER_ALIGN_DEAD  1000/25 //SIEHE DRIVE.C!!! Maximal so lange ausrichten, dann abbrechen
#define TIMER_DISP_MSG    127 //SIEHE FUNKTIONEN.C!!! Bei mehreren Statusmeldungen werden die der Reihe nach oben angezeigt.
#define TIMER_ROTATE_DEAD	127 //SIEHE DRIVE.C!!! Wenn steer zu klein und Opfererkennung => zurücksetzen => dreht nicht => abbrechen

volatile int8_t timer_motor_speed = TIMER_MOTOR_SPEED;
volatile int8_t timer_bt_send_dat = -1;//TIMER_BT_SEND_DAT;
volatile int8_t timer_add_var_drv = TIMER_ADD_VAR_DRV;
volatile int8_t timer_victim_fnd = TIMER_VICTIM_FND;
volatile int8_t timer_victim_led = 0;
volatile int8_t timer_entpr_tast = 0;
volatile int8_t timer_incr_entpr = 0;
volatile int8_t timer_bt_is_busy = 0;
volatile int8_t timer_align_dead = 0;
volatile int8_t timer_disp_msg = 0;
volatile int8_t timer_rotate_dead = 0;

volatile uint8_t timer_25ms = 0; //Damit bestimmte Dinge nur alle 25ms erledigt werden

volatile uint32_t timer = 0; //Timer, Auflösung 100us, inkrementiert die ganze Zeit
////////////////////////////////////////////////////////////////////////////////

ISR(TIMER3_COMPA_vect) //10kHz
{
	cli();
	timer++;

	//////Encoder////////////
	//Quelle: http://www.mikrocontroller.net/articles/Drehgeber

	int8_t new, diff;
 
  new = 0;
  if(INCR_PHASE_A)
    new = 3;
  if(INCR_PHASE_B)
    new ^= 1;                   // convert gray to binary
  diff = incr_last - new;                // difference last - new
  if( diff & 1 ){               // bit 0 = value (1)
    incr_last = new;                 // store new as next last
		incr_delta = (diff & 2) - 1; // bit 1 = direction (+/-)
    incremental += incr_delta;        
  }

	new = 0;
  if(ENC_L_PHASE_A)
    new = 3;
  if(ENC_L_PHASE_B)
    new ^= 1;                   // convert gray to binary
  diff = enc_l_last - new;                // difference last - new
  if( diff & 1 ){               // bit 0 = value (1)
    enc_l_last = new;                 // store new as next last
    enc_l += (diff & 2) - 1;        // bit 1 = direction (+/-)
  }

	new = 0;
  if(ENC_R_PHASE_A)
    new = 3;
  if(ENC_R_PHASE_B)
    new ^= 1;                   // convert gray to binary
  diff = enc_r_last - new;                // difference last - new
  if( diff & 1 ){               // bit 0 = value (1)
    enc_r_last = new;                 // store new as next last
    enc_r += (diff & 2) - 1;        // bit 1 = direction (+/-)
  }

	//////Timer/////////////

	timer_25ms++;
	if(timer_25ms == 250) //40Hz
	{
		if(timer_motor_speed > 0)
			timer_motor_speed--;
		if(timer_bt_send_dat > 0)
			timer_bt_send_dat--;
		if(timer_add_var_drv > 0)
			timer_add_var_drv--;
		if(timer_victim_fnd > 0)
			timer_victim_fnd--;
		if(timer_victim_led > 0)
			timer_victim_led--;
		if(timer_entpr_tast > 0)
			timer_entpr_tast--;
		if(timer_incr_entpr > 0)
			timer_incr_entpr--;
		if(timer_bt_is_busy > 0)
			timer_bt_is_busy--;
		if(timer_align_dead > 0)
			timer_align_dead--;
		if(timer_disp_msg > 0)
			timer_disp_msg--;
		if(timer_rotate_dead > 0)
			timer_rotate_dead--;
		
		timer_25ms = 0;
	}
	sei();
}

/*### Hauptschleife ###*/
int main(void)
{
	//Initialisierungen
	init_sys();
	init_display(1);
	init_pwm();	//Initialisierung Timer für PWM
	init_timer(); //Initialisierung Timer (1000Hz)
	init_adc();
	init_i2c();
	uart1_init(UART_BAUD_SELECT(UART_UM6_BAUD_RATE,F_CPU));//UM6
	uart_init(UART_BAUD_SELECT(UART_MCU_BAUD_RATE,F_CPU)); //Bluetooth

	//eeprom_write_byte((void*)100, 123);
	sei(); 							//Interrupts global aktivieren

	if(get_incrOk())
		setup = 1;
	else
		setup = 0;

	um6_gyroZeroRate();
	_delay_ms(1000);
	
	while(1)
  {
		//uint8_t eeprom_var = eeprom_read_byte((void*)100);
		//displayvar[1] = eeprom_var;

		uint32_t timer_start = timer;
		PORTD ^= (1<<PD5); //Toggle LED auf Board bei Schleifendurchlauf

		////////////////////////////////////////////////////////////////////////////

		if(get_t1() && (timer_entpr_tast == 0)) //Entprellen
		{
			motor_off ^= 1;
			timer_entpr_tast = TIMER_ENTPR_TAST;
		}

		////////////////////Sensorkoordination//////////////////////////////////////
		//I²C Sensoren
		check_srf = getSRF(); //Statemachine (modus funk. noch nicht)
		check_mlx = getIR(); //Statemachine

		//UM6
		check_um6 = um6_getUM6();

		//analog
		check_sir = get_sharpIR();

		if(check_um6 != 0) //Blinken der LED bei Fehler an einem Sensor
			led_fault = 1;
		else if(check_mlx != 0)
			led_fault = 40;
		else if(check_srf != 0)
			led_fault = 210;
		else
			led_fault = 0;
		
		//Batterie/Akku

		uint16_t batt_var = adc_read(9);
		if(batt_var < batt_mV_old)
		{
			batt_mV = batt_var*15;
			batt_mV_old = (batt_var-20);
			batt_percent = (0.286*(batt_mV/10)) - 278;
		}
		if(batt_percent < 20) //Batterie
		{
			led_heartbeatColor = batt_percent;
		}

		////////////////////////////////////////////////////////////////////////////
		//Pulsen der LED

		led_rgb(led_heartbeatColor, led_fault);

		////////////////////////////////////////////////////////////////////////////
		
		if(!setup)
			solve_maze(); //Labyrinth lösen

///////////////////////////////////Opfer////////////////////////////////////////

		if(((mlx90614_l > mlx90614_l_th) || (mlx90614_r > mlx90614_r_th)) && (timer_victim_fnd == 0))
		{
			timer_victim_led = TIMER_VICTIM_LED;
		}
		if(timer_victim_led > 0)
		{
			timer_victim_fnd = TIMER_VICTIM_FND;
			victim_setLED(255);
			speed_l = 0;
			speed_r = 0;
		}
		else
		{
			victim_setLED(0);
		}

/////////////////Geschwindigkeitsregelung///////////////////////////////////

		
		/*if(endmaze > 14)
		{
			led_heartbeatColor = 25;
			bt_putStr("END ");
			speed_l = 0;
			speed_r = 0;
		}
		else
		{*/
			//led_heartbeatColor = 85;
		//}
		uint8_t drivevar = 0;
		/*if(get_incrOk())
		{
			testvar = 1;
		}

		if(testvar == 1)
		{
			drivevar = drive_ramp(-25);//turn(90,0);

			//if(!(drive_oneTile()))//turn(90, 1)))
			if(drivevar == 0)
			{
				testvar = 0;
			}
		}
		else
		{ 
			speed_l = 0;
			speed_r = 0;
		}*/

		if(motor_off)
		{
			speed_l = 0;
			speed_r = 0;
		}
		else
		{
		//	speed_l = 10;
		//	speed_r = 10;
		}

		controlSpeed();

		//////////////////////////
		incr_delta = 0;
////////////////////////Display/Bluetooth///////////////////////////////////////
		if(!(u8g_stateMachine))
		{
			u8g_FirstPage(&u8g); //Nur ausführen, wenn Statemachine 0 ist
			u8g_stateMachine = 1; //Auf 1 setzen, diese ABfrage trifft ab jetzt nicht mehr zu
		}
		//////////////Displayausgaben////////////////
		 
		u8g_DrawInfo(); //Batterie und andere Infos (Statusmeldungen etc.)
		if(setup)
		{
			u8g_DrawSetUp();
		}
		else
		{
			u8g_DrawMaze(); //Labyrinth
		}
		u8g_DrawVLine(&u8g, 87, 7, 57); //Vertikale Abgenzung

		///////Infos//////////////////
		//  Karte   //Pfeil// Frei  //
		//          //     // hier: //
		//          /////////       //
		//          //Info //   X   //
		//          //Karte//       //
		//////////////////////////////

		u8g_SetFont(&u8g, u8g_font_5x7);

		u8g_DrawLong(90, 	15, displayvar[0]);
		u8g_DrawLong(90, 	22, displayvar[1]);
		u8g_DrawLong(90, 	29, displayvar[2]);
		u8g_DrawLong(90, 	36, enc_lr);//um6_psi);

		u8g_DrawLong(90, 	43, maze_getLast(NORTH));//um6_psi_t);//um6_phi);
		u8g_DrawLong(90, 	50, maze_getLast(EAST));//(timer-timer_start)/10);//wallsensor[RIGHT][BACK]);//um6_theta);
		u8g_DrawLong(90, 	57, maze_getLast(SOUTH));//wallsensor_top[BACK]);
		u8g_DrawLong(90,	64, maze_getLast(WEST));//wallsensor_top[FRONT]);

		u8g_DrawLong(100,	43, maze_getStop(NORTH));//um6_psi_t);//um6_phi);
		u8g_DrawLong(100, 50, maze_getStop(EAST));//(timer-timer_start)/10);//wallsensor[RIGHT][BACK]);//um6_theta);
		u8g_DrawLong(100, 57, maze_getStop(SOUTH));//wallsensor_top[BACK]);
		u8g_DrawLong(100,	64, maze_getStop(WEST));//wallsensor_top[FRONT]);


		u8g_DrawLong(105,	15, (timer-timer_start)/10);
		//u8g_DrawLong(105, 22, mlx90614_r);

		if(!(u8g_NextPage(&u8g)))
		{
			u8g_stateMachine = 0; //Statemachine auf 0 setzen, wenn NextPage != 1 (0) zurückgibt. Solange das nicht so ist, wird in den nächsten Hauptschleifendurchläufen die erste if-Abfrage nicht zutreffen (Firstpage somit nicht aufgerufen) und die DIsplayausgabefunktionen ausgerufen.
		}
//////////////////////////BT:

		if(timer_bt_send_dat == 0)
		{
			bt_putStr("\e[H"); //gehe nach links oben
			bt_putStr("\e[2J"); //clear

			bt_putStr("Batt.: ");        bt_putLong(batt_percent); bt_putStr("%");
			bt_putStr("\n\rSensor: ");
			if(check_um6 == 0 && check_mlx == 0 && check_srf == 0)
			{
				bt_putStr("ok");
			}
			else
			{
				bt_putStr("ERR:");
			}

			if(check_um6 != 0)
			{
				bt_putStr("\n\r\e[5C UM6_ERR");
			}
			else
			{
				bt_putStr("\n\r\e[5C phi: "); bt_putLong(um6_phi);
				bt_putStr("\n\r\e[5C theta: "); bt_putLong(um6_theta);
				bt_putStr("\n\r\e[5C psi: "); bt_putLong(um6_psi);
			}

			if(check_mlx != 0)
			{
				bt_putStr("\n\r\e[5C MLX_ERR");
			}
			else
			{
				bt_putStr("\n\r\e[5C Temp. l.: "); bt_putLong(mlx90614_l);
				bt_putStr("\n\r\e[5C Temp. r.: "); bt_putLong(mlx90614_r);
			}

			if(check_srf != 0)
			{
				bt_putStr("\n\r\e[5C SRF_ERR");
			}
			else
			{
				bt_putStr("\n\r\e[5C SRF F: "); bt_putLong(srf10[FRONT]);
				bt_putStr("\n\r\e[5C SRF R: "); bt_putLong(srf10[RIGHT]);
				bt_putStr("\n\r\e[5C SRF B: "); bt_putLong(srf10[BACK]);
				bt_putStr("\n\r\e[5C SRF L: "); bt_putLong(srf10[LEFT]);
			}

			timer_bt_send_dat = TIMER_BT_SEND_DAT;
		}
  }
	
	return 0;
}
