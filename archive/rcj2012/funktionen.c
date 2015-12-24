extern void getPulsedLightSensors();


extern int16_t temp_l_victim_back_near; 		//Schwellwerte aus EEPROM
extern int16_t temp_l_victim_front_far; 		
extern int16_t temp_l_wall_normal;  		
extern int16_t temp_r_victim_back_near;   		
extern int16_t temp_r_victim_front_far;  
extern int16_t temp_r_wall_normal; 

//Für Statemachine der Lichtsensorpulsung
uint8_t count_key5_wallswitch = 0;

extern uint16_t lichtsensor_pulsed_up;
extern uint16_t lichtsensor_pulsed_down;
uint16_t lichtsensor_up_pulsed_1 = 0;
uint16_t lichtsensor_up_pulsed_2 = 0;
uint16_t lichtsensor_down_pulsed_1 = 0;
uint16_t lichtsensor_down_pulsed_2 = 0;
uint8_t getPulsedLight_check = 0;

extern const int8_t TIMER_PULS_BOTTOMLED; //*25ms. In dieser Frequenz werden die Lichtsensoren auf dem Boden gepulst.
extern volatile int8_t timer_puls_bottomled;

//////////////////////////////////////////////////////////////////////////////
/* Menü:
- Auswahl:  Linker Wärmesensor
						Rechter Wärmesensor
						Lichtsensor
	- Wärmesensor: ,,Bitte an Rückseite stellen!"
									 [Auf Tastendruck warten]
										[X1]
									 [Messung speichern]
									 [Wert anzeigen]
									,,Weitere Messung?"
									 [Auf Tastendruck warten]	
										 [Ja: Goto X1]
										 [Nein: Durchschnitt gemachter Messungen errechnen
														Goto Auswahl]
	- Lichtsensor: 		[X2]
									,,Bitte auf weiße Fläche stellen!"
									 [Auf Tastendruck warten]
									 [Wert speichern]
									,,Bitte auf schwarze Fläche stellen!"
									 [Auf Tastendruck warten]
									 [Wert speichern]
									 [Schwellwert mitteln]
									 [Schwellwert anzeigen]
									,,Ok?"
									 [Auf Tastendruck warten]
										 [Ja: Goto Auswahl]
										 [Nein: Messung erneut durchführen
														Goto X2]
*/

void setUp(void)
{
	/*clearLCD();
	
	showScreenLCD("Rescue B", "SETUP");
	for(uint8_t i = 0; i<6; i++)//Blinken
	{
		setLEDs(0b1010);
	  mSleep(100);
		setLEDs(0b0101);
		mSleep(100);
	}

	mSleep(300);

	clearLCD();
	showScreenLCD("1:View  2:Setup", "");
	while(getPressedKeyNumber() == 0); //Auf Tastendruck warten
	
	if(getPressedKeyNumber() == 1)
	{
		clearLCD();
		showScreenLCD("1:View  2:Setup", "");
		while(getPressedKeyNumber() == 0); //Auf Tastendruck warten
	}
	else if(getPressedKeyNumber() == 2)
	{
		clearLCD();
		showScreenLCD("Setup, immer", "5 druecken!");
		mSleep(300);
		while(getPressedKeyNumber() != 5); //Auf Tastendruck warten

		showScreenLCD("Rechte Seite an", "Opfer Rueckseite");
		mSleep(300);
		while(getPressedKeyNumber() != 5); //Auf Tastendruck warten

		getIR();
		SPI_EEPROM_writeByte(0, (mlx90614_r >> 8) & 0xFF); mSleep(5); //Schreiben der Daten ins EEPROM (16bit zerlegen in 2 8bit)
		SPI_EEPROM_writeByte(1, mlx90614_r & 0xFF); mSleep(5);

		showScreenLCD("Rechte Seite an", "Opfer Vorderseite");
		mSleep(2000);
		showScreenLCD("dann gerade", "zurueckbewegen");
		while(getPressedKeyNumber() != 5); //Auf Tastendruck warten

		getADCs();
		while((241920 / (adc0 - 0.6))<1700) //wallsensor_h_r zurückbewegen bis Schwellwert
		{
			getADCs();
			showScreenLCD("Bewegen...", "");
			mSleep(50);
		}
		showScreenLCD("Stop!", "");
		getIR();
		SPI_EEPROM_writeByte(2, (mlx90614_r >> 8) & 0xFF); mSleep(5); //Schreiben der Daten ins EEPROM (16bit zerlegen in 2 8bit)
		SPI_EEPROM_writeByte(3, mlx90614_r & 0xFF); mSleep(5);
		mSleep(1000);

		showScreenLCD("Rechte Seite an", "Wand ohne Opfer");
		while(getPressedKeyNumber() != 5); //Auf Tastendruck warten
		SPI_EEPROM_writeByte(4, (mlx90614_r >> 8) & 0xFF); mSleep(5); //Schreiben der Daten ins EEPROM (16bit zerlegen in 2 8bit)
		SPI_EEPROM_writeByte(5, mlx90614_r & 0xFF); mSleep(5);
	}*/
}

void getPulsedLightSensors()
{
	switch(getPulsedLight_check)
	{
		case 0:
			setBottomLEDleft(1);
			setBottomLEDright(1);

			timer_puls_bottomled = TIMER_PULS_BOTTOMLED;
			getPulsedLight_check = 1;
		break;
		case 1:
			if(timer_puls_bottomled == 0)
			{
				lichtsensor_up_pulsed_1 = readADC(7);
				lichtsensor_down_pulsed_1 = readADC(6);
				
				setBottomLEDleft(0);
				setBottomLEDright(0);

				getPulsedLight_check = 2;
				timer_puls_bottomled = TIMER_PULS_BOTTOMLED;
			}
		break;
		case 2:
			if(timer_puls_bottomled == 0)
			{
				lichtsensor_up_pulsed_2 = readADC(7);
				lichtsensor_pulsed_up = (lichtsensor_up_pulsed_2 - lichtsensor_up_pulsed_1);
				
				lichtsensor_down_pulsed_2 = readADC(6);
				lichtsensor_pulsed_down = (lichtsensor_down_pulsed_2 - lichtsensor_down_pulsed_1);
				
				getPulsedLight_check = 0;
			}
		break;
	}
}
