/* 
 * ****************************************************************************
 * RP6 ROBOT SYSTEM - RP6 CONTROL M32
 * ****************************************************************************
 * RoboCup Rescue B
 * ****************************************************************************
 * Beschreibung:
 * 
 *
 * 
 * ****************************************************************************
 */

/*****************************************************************************/

#include <avr/wdt.h> //watchdog

#include "i2cmaster.h"
#include "RP6ControlLib.c" 							
#include "RP6Control_I2CMasterLib.c"
#include "RP6uart.c"

/*****************************************************************************/

#include "system.c"
#include "funktionen.c"

/*****************************************************************************/
//Für Wallfollower PID:
#define KiWF  0.001   //Aufsummierung der Regelabweichung (je länger die Regelabweichung, desto größer...)
#define KpWF  0.28   //Verstärkungsfaktor
#define KdWF  0.001 //Differentieller Faktor
#define TaWF  0.0005 //Abtastzeit, so oft wird aufaddiert und aktualisiert (in S)

const uint16_t IMPASSE_THRESHOLD = 100; //Unter diesem Wert (vorderer Lichtsensor!) wird Sackgasse erkannt

const uint16_t WALLSENSOR_TARGET = 1340;    //Soll-Abstand zur Wand
const uint16_t WALLSENSOR_MINWALL = 2200;  //Mindestwert zur Garantie, dass Wand vorhanden ist (sonst Außenkurve, ggf. kann man auch Sackgassenumfahren abbrechen)
const uint16_t WALLSENSOR_MINWALL_IMPASSE = 1500; //Ab diesem Wert muss eine Wand direkt neben dem Robo sein, ggf. kann er also Sackgassenumfahrung beenden
const uint16_t WALLSENSOR_L_VICTIM = 1100;  //Nur unter diesem Abstand werden Opfer erkannt
const uint16_t WALLSENSOR_R_VICTIM = 2100;

const uint16_t WALLSENSOR_MAXVALUE = 8000; //Aufgrund der Linearisierung ein max Wert, sonst overflow
const uint16_t FRONTSENSOR_MAXVALUE = 8000;
const uint16_t FRONTSENSOR_MINVALUE = 870; //Bei großer Entfernung und bei sehr kleiner dieser Wert!
const uint16_t FRONTSENSOR_MINWALL_IMPASSE = 1300; //Bei dieser Entfernung ist Wand sicherlich direkt vor Robo (zum Beenden der Sackgassenumfahrung)

const uint16_t FRONTSENSOR_THRESHOLD = 1800;     // Frontsensor Schwellwert für Außenkurven

const uint8_t MAXSPEED = 200; //Maximale Geschwindigkeit. Am besten nicht ändern!

const int16_t STEER_MAX_LEFT = -270;   // Wenn Roboter relativ weit von Wand entfernt ist, ist das der maximalwert, damit Roboter nicht übersteuert
const int16_t STEER_HARD_LEFT = 310;   // Erzwungene Linksabbiegung, wenn Frontsensor Schwellwert erreicht ist [=> pwrLeft/pwrRight sollte ca. +-1,61 sein]
const int16_t STEER_HARD_RIGHT = -270;     // Steuervariable für Außenecken
const int16_t STEER_OUTER_IMPASSE = -190; //Nach finden von Sackgasse so um Sackgasse fahren
const int16_t STEER_OUTER_IMPASSE_2 = -400; //Nach finden von Sackgasse STOP_BLACKTILE_COUNT Durchläufe zurückdrehen, Motor wird effektiv abgeremst, saubere Umfah.
const int16_t STEER_INNER_IMPASSE = 300; //Beim Finden von Sackgasse so in Sackgasse drehen, danach entsprechend kurz in andere Richtung drehen
const int16_t STEER_RAMP_MAX = 120; //Maximaler Steuerwert auf Rampe
/////////Ab 1000 werden bestimmte Steuerbefehle erkannt:////////////////////////
const int16_t STEER_STOP = 1000; //Stop
const int16_t STEER_REVERSE = 1001; //Rückwärts (bis 1255)

const int8_t STOP_BLACKTILE_COUNT = 170; //So viele S.Durchläufe wird nach Detektieren von Sackgasse und verlassen in a. Richtung gedreht, Motor wird nur gebremst

const uint16_t MAX_ESUM = 800;
const int16_t MIN_ESUM = -800;

const uint16_t MRC_PAST_WALL_R = 256;    // So weit soll der Roboter weiterfahren, wenn eine Außenkurve dedektiert wird (weil Sensor schräg ist, erkennt er die Kurven eher und muss daher später abbigen)

const uint8_t ACCL_MEDIAN_ADD = 8; //So viele Messengen werden für den Beschelunigungssensor gemacht, bevor ein Durchschnitt berechnet wird.

const uint8_t LOP_ADD = 4; //So viele Messengen werden für wallsensor gemacht, bevor ein Durchschnitt berechnet wird. (==> LOP Erkennung)

const int8_t TIMER_VICTIM_FOUND = 80; //*25ms. Solange wird nach einem Fund gewartet, bis wieder signalisiert werden darf
const int8_t TIMER_VICTIM_FLASH_LED = 80; //*25ms. Solange ist die LED nach einem Opferfund an.
const int8_t TIMER_ACCL_MEDIAN = 12; //*25ms. So oft wird der Beschleunigungssensor abgefragt, um einen Durchschnitt zu berechnen.
const int8_t TIMER_PULS_BOTTOMLED = 1; //*25ms. In dieser Frequenz werden die Lichtsensoren auf dem Boden gepulst.
const int8_t TIMER_SM_SRF = 3; //*25ms. So lange wird nach Start einer Messung gewartet, bis Sensor abgefragt werden kann (==> automatisch ABfragfrequenz) max. 3
const int8_t TIMER_LCD_REFRESH = 4; //*25ms. In dieser Frequenz wird der Displayinhalt aktualisiert, falls vorhanden.
const int8_t TIMER_LOP = 20; //*25ms. So oft wird iSteer abgefragt, um einen Durchschnittswert zu berechnen, über dem dann ein LOP erkannt werden kann.
const int16_t TIMER_RAMP_REV = 1000; //So lange nach Rampenerkennung rückwärts

int16_t tempL_var; 		//Schwellwert mlx90614_l  	 		 		(aus EEPROM!)
int16_t tempR_var; 		//Schwellwert mlx90614_r	 	 				(     "     )
int16_t ramp_var;  		//Schwellwert rampsensor	 	 				(     "     )
int16_t lsL_var;   		//Schwellwert Lichtsensor L 				(     "     )
int16_t lsR_var;   		//Schwellwert Lichtsensor R 				(     "     )
int16_t threedbs_var; //Schwellwert Lichtsensor R 				(     "     )

////////////////////Globale Variablen///////////////////////////////////////////

int16_t pwrLeft = 0, pwrRight = 0; //Globale Variablen für Geschwindigkeit Motor links und Motor rechts

//Werte der beiden Lichtsensoren für Sackgassen
uint16_t lichtsensor_pulsed_up = 0;
uint16_t lichtsensor_pulsed_down = 0;
uint16_t lichtsensor_pulsed = 0; //Addierung up und down

//Wert des Ultraschallsensors
uint16_t srf_02 = 0;

////////////////////////////////////////////////////////////////////////////////

volatile int8_t timer_victim_found = 0; //siehe TIMER_VICTIM_FOUND. Timer aus
volatile int8_t timer_victim_flash_led = 0; //siehe TIMER_VICTIM_FLASH_LED. Timer aus.
volatile int8_t timer_accl_median = 0; //siehe TIMER_ACCL_MEDIAN. Timer aus
volatile int8_t timer_puls_bottomled = 0; //siehe TIMER_PULS_BOTTOMLED
volatile int8_t timer_sm_srf = 0;		//siehe TIMER_SM_SRF
volatile int8_t timer_lcd_refresh = 0; //siehe TIMER_LCD_REFRESH 
volatile int8_t timer_lop = 0; //siehe TIMER_LOP

void timerControl(void)  //Regelung der Timer. Wird aus Timer ISR aufgerufen.
{
	if(timer_victim_found > 0) //siehe TIMER_VICTIM_RIGHT_FOUND
    timer_victim_found--;
	if(timer_victim_flash_led > 0) //siehe TIMER_VICTIM_FLASH_LED
		timer_victim_flash_led--;
	if(timer_accl_median > 0) //siehe TIMER_ACCL_MEDIAN
		timer_accl_median--;
	if(timer_puls_bottomled > 0) //siehe TIMER_PULS_BOTTOMLED
		timer_puls_bottomled--;
	if(timer_sm_srf > 0) //siehe TIMER_SM_SRF
		timer_sm_srf--;
	if(timer_lcd_refresh > 0) //siehe TIMER_LCD_REFRESH
		timer_lcd_refresh--;
	if(timer_lop > 0) //siehe TIMER_LOP
		timer_lop--;
}

int main(void)
{
	wdt_disable(); //Watchdog deaktivieren!!! Sonst wird nach einem Auslöser sofort erneut ausgelöst, da der Presclaer nicht mehr stimmt!	

	initRP6Control();  
	initLCD();
		setLCDBacklight(1); //Hintergrundbeleuchtung an
  initSystem();
	i2c_init(); //250kHz

	showScreenLCD("Rescue B", "teamohnename.de");
	//while(getPressedKeyNumber() == 0);
	if(getPressedKeyNumber() == 5)
	{
		setUp(); //Kalibrierung
	}
	
	tempL_var = SPI_EEPROM_readByte(1) + (SPI_EEPROM_readByte(0)<<8); //Sensorschwellwerte aus EEPROM lesen
	tempR_var = SPI_EEPROM_readByte(3) + (SPI_EEPROM_readByte(2)<<8);
	ramp_var = SPI_EEPROM_readByte(5) + (SPI_EEPROM_readByte(4)<<8);
	lsL_var = SPI_EEPROM_readByte(7) + (SPI_EEPROM_readByte(6)<<8);
	lsR_var = SPI_EEPROM_readByte(9) + (SPI_EEPROM_readByte(8)<<8);
	threedbs_var = SPI_EEPROM_readByte(11) + (SPI_EEPROM_readByte(10)<<8);
	
////////////////////////////////////////////////////////////////////////////////

	int16_t mrcLastSeeWall = 0;
  
	//Durchschnittswertberechnung des Beschleunigungssensors
	int32_t accl_median_buffer = 0;
	int16_t accl_median = 0;
	uint8_t accl_median_i = 0; //Durchscnitt des BS Sensors (Index)
	uint8_t accl_median_block = 0;

	//Durchschnittswertberechnung von iSteer, um LOP zu erkennen
	int32_t srf_median_buffer = 0;
	int16_t srf_median = 1000; //1000, weil sonst schon am ANfang LOP erkannt wird, weil Wert unter 150 ist
	uint8_t srf_median_i = 0; //Durchscnitt des BS Sensors (Index)
	int16_t srf_median_old = 0;

	//Werte der SHARP IR Sensoren
	int32_t wallsensor_v_r = 0; //wallsensor_vorne_nach rechts zeigend
	int32_t wallsensor_h_r = 0; //...
	int32_t wallsensor_v_l = 0; 
	int32_t wallsensor_h_l = 0; //wallsensor_hinten_nach rechts zeigend
	int32_t frontsensor_v = 0;
	int16_t frontsensor_v_raw = 0;
	int32_t frontsensor_h = 0;

	//Für PID Regler des Wallfollowers:
	int16_t iSteer = 0;
	int16_t e_wf = 0;
	int16_t esum_wf = 0;
	int16_t ealt_wf = 0;

	//Sackgasse
	lichtsensor_pulsed_up = IMPASSE_THRESHOLD; //Startwertzuweisung, damit nicht beim ersten DUrchlauf unter Schwellwert und deshalb Auslösung
	lichtsensor_pulsed_down = IMPASSE_THRESHOLD;

	uint8_t check_victim_found = 0; //Darin werden abgearbeitete Schritte gespeichert, nachdem ein Opfer gefunden wurde (für Timer)
	uint8_t blacktile = 0;
	uint8_t stop_blacktile = 0;

	uint8_t rampe = 0;
	
	wdt_enable(WDTO_2S); //Watchdog aktivieren (funktioniert ab 120MS, Spielraum lassen)
		
	while(1) 
	{
		wdt_reset(); //watchdog der m32
		baseWatchdog_reset(); //watchdog der base
////////////////////////////Acceleromter////////////////////////////////////

		get3DBS();
				
		if(timer_accl_median == 0)
		{
			if(accl_median_block == 1)
			{
				accl_median_buffer += 2200; //Normaler Wert (gerader Untergrund)
			}
			else
			{
				accl_median_buffer += threedbs_x;
			}

			accl_median_i ++;
			if(accl_median_i == ACCL_MEDIAN_ADD) //Nachdem ACCL_MEDIAN_ADD Messungen gemacht wurden
			{
				accl_median_i = 0; //Zurücksetzen div. Werte

				accl_median = (accl_median_buffer/ACCL_MEDIAN_ADD); //Durchschnitt berechnen
				if(accl_median < 0) //Sicherheitsmaßnahme
				{
					accl_median = 0;
				}
				accl_median_buffer = 0;
			}
			timer_accl_median = TIMER_ACCL_MEDIAN;
		}

////////////////////////////Wallfollower////////////////////////////////////////

		/*
		 * W V R 		adc0				GP2D120x
		 * W H R		adc1				GP2D120x
		 * W V L		readADC(4)	 GP2D12
		 * F V			readADC(3)	 GP2D12
		 * F H			readADC(2)	 GP2D12
		 */
    getADCs();

		wallsensor_v_r = 234720 / (adc1 - 0.4);
		wallsensor_h_r = 241920 / (adc0 - 0.6);
		wallsensor_v_l = 278600 / (readADC(3) + 24);
		wallsensor_h_l = 278600 / (readADC(2) + 24);
		frontsensor_h = 462900 / (readADC(4) - 29); //Linearisierung, Auflösung: mm (18,68cm ~~ 1868); Formel für GP2D12
		frontsensor_v = 462900 / (readADC(5) - 29);
		frontsensor_v_raw = readADC(5);

		if((frontsensor_h > FRONTSENSOR_MAXVALUE) || (frontsensor_h < FRONTSENSOR_MINVALUE)) {
			frontsensor_h = FRONTSENSOR_MAXVALUE;																						 	 }	
		if((frontsensor_v > FRONTSENSOR_MAXVALUE) || (frontsensor_v < FRONTSENSOR_MINVALUE)) {
			frontsensor_v = FRONTSENSOR_MAXVALUE;																							 }
		if(wallsensor_v_r > WALLSENSOR_MAXVALUE)  {
			wallsensor_v_r = WALLSENSOR_MAXVALUE;   }
		if(wallsensor_h_r > WALLSENSOR_MAXVALUE)  {
			wallsensor_h_r = WALLSENSOR_MAXVALUE;   }
		if(wallsensor_v_l > WALLSENSOR_MAXVALUE)  {
			wallsensor_v_l = WALLSENSOR_MAXVALUE;   }
		if(wallsensor_h_l > WALLSENSOR_MAXVALUE)  {
			wallsensor_h_l = WALLSENSOR_MAXVALUE;   }

	  e_wf = WALLSENSOR_TARGET - wallsensor_v_r; //calculationg deviation
		esum_wf = esum_wf + e_wf; //integral component
		if(esum_wf > MAX_ESUM)	{ //Limit of the integral component, otherwise overflow
			esum_wf = MAX_ESUM;	}
		else if(esum_wf < MIN_ESUM)	{
			esum_wf = MIN_ESUM;				}
		iSteer = KpWF * e_wf + KiWF * TaWF * esum_wf + KdWF * (e_wf - ealt_wf)/TaWF; //PID controller equation
		ealt_wf = e_wf;	//Differential component

		if(iSteer >= STEER_STOP)
		{
			iSteer = (STEER_STOP-1);
		}

		if(wallsensor_v_r > WALLSENSOR_MINWALL)
	  {
	   // The sensor value is smaller than the minimum possible distance to the wall, so it must be an outside corner. The robot have to be careful so that he does not touch the corner, when he turns. Therefore the Robot have to go straight a short way.
	   
			getDists(); //Query the encoder
	  
			if(mright_dist < (mrcLastSeeWall + MRC_PAST_WALL_R))
		  {
		    //The robot is not yet moved far enough in order not to touch the corner
				iSteer = 0;
		  }
		  else
		  {
		    // Der Roboter hat den Punkt erreicht, an dem er sicher abbiegen kann
	    	iSteer = STEER_HARD_RIGHT;
			}
	  } 
	  else
	  {
	    // Speichere Encoder Position, wenn die Wand sicher da ist
		  getDists(); //Abfragen der Encoder
	
			mrcLastSeeWall = mright_dist;

			if(iSteer < STEER_MAX_LEFT)
			{
				iSteer = STEER_MAX_LEFT;
			}
			else
			{
			  if((frontsensor_v < FRONTSENSOR_THRESHOLD) && (iSteer < STEER_HARD_LEFT)) //Wenn der Frontsensor eine Innenecke erkennt, soll der Roboter scharf links abbiegen. Das && stellt sicher, dass der Wandsensor notfalls eine schärfere Kurve erzwingen kann.
				{
			    iSteer = STEER_HARD_LEFT;
				}
			}
	  }
		
//////////////////////////////Opfersignalisierung///////////////////////////////

    if(check_victim_found == 0) //,,Missbrauch" von State Machine Variable. 1 für 2sek, wenn Opfer gefunden wurde.
		{
			getPulsedLightSensors(); //kein Opfer, deshalb normal pulsen
			lichtsensor_pulsed = (lichtsensor_pulsed_down+lichtsensor_pulsed_up); //Erreichnung des Wertes beider Sensoren, mindestens einer ist bei weiß immer über 800

			setLCDBacklight(1); //Hintergrundbeleuchtung an (==> wird niegrnds in Hauptschelife nach Opferfund reaktiviert)
		}
		else
		{
			setBottomLEDright(0); //check_victim_found != 0, deshalb so lange Licht aus. Beleuchtung wird als Signallampe verwendet.
			setBottomLEDleft(0); //Beleuchtung für Lichtsensor aus
			setLCDBacklight(0); //Hintergrundbeleuchtung aus
		}

// / / / / / / / / / / / / / / / / / /Sackgasse/ / / / / / / / / / / / / / / / / 

		if((lichtsensor_pulsed_up < IMPASSE_THRESHOLD) && (threedbs_x < threedbs_var))
		{
			blacktile = 1;
		}

		if(blacktile == 1)
		{
			if(lichtsensor_pulsed_up < IMPASSE_THRESHOLD)
			{
				iSteer = STEER_INNER_IMPASSE;
				stop_blacktile = STOP_BLACKTILE_COUNT;
			}
			else if(lichtsensor_pulsed_up >= IMPASSE_THRESHOLD)
			{
				if(stop_blacktile > 0)
				{
					iSteer = STEER_OUTER_IMPASSE_2;
					stop_blacktile --;
				}
				else
				{
					iSteer = STEER_OUTER_IMPASSE;
				}
			}

			if((frontsensor_v < FRONTSENSOR_MINWALL_IMPASSE) || (wallsensor_h_r < WALLSENSOR_MINWALL_IMPASSE))
			{
				blacktile = 0;
			}
		}
		


////////////////////////////LOP Erkennung///////////////////////////////////////
		getSRF();

		if((timer_lop == 0) && (iSteer != STEER_STOP)) //Wenn der Roboter absichtlich steht, soll kein LOP erkannt werden (==> STEER_STOP)
		{
			srf_median_buffer += frontsensor_v; 
			
			srf_median_i ++;
			if(srf_median_i == LOP_ADD) //Nachdem LOP_ADD Messungen gemacht wurden
			{
				srf_median_i = 0; //Zurücksetzen div. Werte
				srf_median_old = srf_median;
				srf_median = (srf_median_buffer/LOP_ADD); //Durchschnitt berechnen
				srf_median_buffer = 0;
			}
			timer_lop = TIMER_LOP;
		}

		if(((srf_median-srf_median_old)<50) && ((srf_median-srf_median_old)>-50)) //LOP!
		{
			//iSteer = STEER_HARD_LEFT;
		}

///////////////////////Rampe////////////////////////////////////////////////////

		
/*		if(accl_median > threedbs_var)
		{
			rampe = 1;
			setLEDs(0b1111);
		}
		
		if((rampe == 1) && (blacktile == 0))
		{
			if(frontsensor_h > 1200)
			{
				iSteer = 1200;
			}
			else
			{
				if(wallsensor_v_r < WALLSENSOR_MINWALL)
				{
					iSteer = STEER_HARD_RIGHT;
				}
				else
				{
					rampe = 0;
					setLEDs(0b0000);
				}
			}
		}*/

///////////////////////////////Wärmesensoren////////////////////////////////////

		getIR(); //Abfragen der Infrarotsensoren

		// Abfrage:
		if(((mlx90614_l > 3000) || (mlx90614_r > 3000)) && timer_victim_found == 0) //Einer der beiden IR Sensoren hat Opfer gefunden
		{
			if(check_victim_found == 0)
			{
				timer_victim_flash_led = TIMER_VICTIM_FLASH_LED;
				check_victim_found = 1;
			}
		}
		if(timer_victim_flash_led > 0)
		{
			setVictimLEDleft(1);
			setVictimLEDright(1);
			iSteer = STEER_STOP; //Motoren aus
			timer_victim_found = TIMER_VICTIM_FOUND; 					//Timer setzen, damit selber Opfer nicht erneut erkannt wird (in Konstante*25ms erneut)
		}
		else
		{
			setVictimLEDleft(0);
			setVictimLEDright(0);
			check_victim_found = 0;
		}
//////////////////Geschwindigkeitsberechnung und -setzen////////////////////////

		if(iSteer < STEER_STOP)
		{
			pwrLeft = (MAXSPEED-iSteer); //Geschwindigkeit auf MAXSPEED begrenzen
			if(pwrLeft > MAXSPEED)  {
			 	pwrLeft = MAXSPEED;		}
			if(pwrLeft < -MAXSPEED)	{
			  pwrLeft  = -MAXSPEED;	}

			pwrRight = (MAXSPEED+iSteer); 
			if(pwrRight > MAXSPEED)	{
			  pwrRight = MAXSPEED;	}
			if(pwrRight < -MAXSPEED){
			  pwrRight  = -MAXSPEED;}
		}
		else if(iSteer == STEER_STOP)
		{
			pwrLeft = 0;
			pwrRight = 0;
		}
		else if(iSteer >= STEER_REVERSE)
		{
			iSteer -= 1001; //Beispiel: Rückwärts mit 230: 1230 - 1001 = 229
			iSteer *= -1;   //Wäre vorwärts, wir brauchen negativen Wert: 229 * (-1) = -229
			iSteer += -1;   //Geschwindigkeit soll aber -230 sein: -229+(-1) = -230

			pwrLeft = iSteer;
			pwrRight = iSteer;
		}

		setSpeed();

////////////////////////////Debugging///////////////////////////////////////////

		if(timer_lcd_refresh == 0)
		{
			clearLCD();
 
			writeIntegerLCD(mlx90614_l,10);
			setCursorPosLCD(1, 0);
			writeIntegerLCD(mlx90614_r,10);
			setCursorPosLCD(0, 10);
			writeIntegerLCD(iSteer,10);
			setCursorPosLCD(1, 7);
			writeIntegerLCD(frontsensor_h, 10);
			/*setCursorPosLCD(0, 10);
			writeIntegerLCD(mlx90614_r, 10);
			setCursorPosLCD(1, 10);
			writeIntegerLCD(victim_l_var, 10);
			setCursorPosLCD(1, 15);
			writeIntegerLCD(vrlr, 10);
			writeIntegerLCD(wallsensor_v_r,10);
			setCursorPosLCD(0, 5);
			writeIntegerLCD(wallsensor_h_r, 10); 
			setCursorPosLCD(0, 10);
			writeIntegerLCD(wallsensor_v_l, 10);
			setCursorPosLCD(1, 0);
			writeIntegerLCD(frontsensor_v, 10);
			setCursorPosLCD(1, 5);
			writeIntegerLCD(frontsensor_h, 10);
			setCursorPosLCD(1, 12);
			writeIntegerLCD(iSteer, 10);*/

/*writeIntegerLCD((480000/mlx90614_l),10);
			setCursorPosLCD(0, 5);
			writeIntegerLCD((wallsensor_v_l+230)/7, 10); 
			setCursorPosLCD(1, 0);
			writeIntegerLCD((300000/mlx90614_l),10);
			setCursorPosLCD(1, 5);
			writeIntegerLCD((wallsensor_v_l+1800)/22, 10);*/

			timer_lcd_refresh = TIMER_LCD_REFRESH;
		}
	}

	return 0;
}
