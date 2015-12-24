extern const uint8_t MAXSPEED;
extern int16_t pwrLeft;
extern int16_t pwrRight;

////////////////////////////////////////////////////////////////////////////////

void initSystem(void)
{
  DDRC = ((1 << PC2 ) | (1<<PC3) | (1<<PC5) | (1<<7)); //LEDs: Lichtsensor links, rechts, Signal LED links, rechts als Ausgang
	DDRD = (1 << PC5); //LCD Hintegrundbeleuchtung
	
	PORTD |= (1<<5); //Hintergrundbeleuchtung an
  PORTC &= ~(1<<2); //LS links aus
  PORTC &= ~(1<<3); //LS rechts aus
  PORTC &= ~(1<<5); //LED links aus
	PORTC &= ~(1<<7);  //LED rechts aus
}

void setVictimLEDleft(uint8_t victimLEDl)
{
  if(victimLEDl == 1)
	{
		PORTC |= (1<<5);
	}
	else
	{
		PORTC &= ~(1<<5);
	}
}

void setVictimLEDright(uint8_t victimLEDr)
{
  if(victimLEDr == 1)
	{
		PORTC |= (1<<7);
	}
	else
	{
		PORTC &= ~(1<<7);
	}
}

void setBottomLEDleft(uint8_t bottomLEDl)
{
  if(bottomLEDl == 1)
	{
		PORTC |= (1<<3);
	}
	else
	{
		PORTC &= ~(1<<3); //LS rechts aus
	}
}

void setBottomLEDright(uint8_t bottomLEDr)
{
  if(bottomLEDr == 1)
	{
		PORTC |= (1<<2); //LS links aus
	}
	else
	{
		PORTC &= ~(1<<2); //LS links aus
	}
}

void setLCDBacklight(uint8_t lcd_backlight)
{
	if(lcd_backlight == 1)
	{
		PORTD |= (1<<5); //Hintergrundbeleuchtung an
	}
	else
	{
		PORTD &= ~(1<<5); //Hintergrundbeleuchtung aus
	}
}

void setSpeed(void)
{
	uint8_t speed_l = 0;
	uint8_t speed_r = 0;

	if(pwrLeft >= 0 && pwrRight >= 0) //beide mehr als 0, daher muss der Roboter mit beiden Motoren vorwärts fahren
  {
    changeDirection(FWD);
    moveAtSpeed(pwrLeft, pwrRight);
  }
  else if(pwrLeft < 0 && pwrRight < 0)
  {
    changeDirection(BWD);
		speed_l = (pwrLeft * (-1));
		speed_r = (pwrRight * (-1));
		moveAtSpeed(speed_l, speed_r);
  }
  else if(pwrLeft <= 0 && pwrRight >= 0) //pwrLeft ist im Minusbereich, der Roboter muss also mit dem linken Motor rückwärts drehen
  {
    changeDirection(LEFT);
    speed_l = (pwrLeft * (-1));
		speed_r = pwrRight;
		moveAtSpeed(speed_l, speed_r);
  }
  else if(pwrLeft >= 0 && pwrRight <= 0) //pwrLeft ist im Minusbereich, der Roboter muss also mit dem linken Motor rückwärts drehen
  {
    changeDirection(RIGHT);
		speed_l = pwrLeft;
		speed_r = (pwrRight * (-1));
    moveAtSpeed(speed_l, speed_r);
  }
}
