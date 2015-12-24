//////////////////////////////////////////////////////////////////////////////
//////////////////////////Labyrinth///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include "drive.h"

#define MAXSPEED 50

////////
#define ENC_FAC_CM_L 38 //Fakor: Encoderwert in Strecke (cm) umrechnen
#define ENC_FAC_CM_R 37
#define ENC_FAC_CM_LR 41

//#define TILE1_FRONT 				150 //Entfernung zur Wand bei einer Fliese (Entfernung für Sensor nach vorne, wallsensor[][])
#define TILE1_FRONT_TOP 		520 //case 1; Entfernung zur Wand bei einer Fliese (Entfernung für Sensor nach vorne, wallsensor_top[])
#define TILE1_FRONT_TH_TOP 	360 //Schwellwert nach vorne, wallsensor_top[]
#define TILE1_FRONT_SRF 		900 //Entfernung zur Wand bei einer Fliese (Entfernung für Sensor nach vorne)
#define TILE1_FRONT_TH 			70 //Schwellwert nach vorne, wallsensor[][]
#define TILE1_FRONT_TH_SRF 	1800 //Schwellwert nach vorne
#define TILE2_FRONT_TOP 		255 //case 2
#define TILE2_FRONT_TH_TOP 	210
#define TILE3_FRONT_TOP 		180
#define TILE3_FRONT_TH_TOP 	130

#define TILE1_BACK_TH_TOP 	500
#define TILE2_BACK_TOP 			380 //case 4
#define TILE2_BACK_TH_TOP 	250
#define TILE3_BACK_TOP 			185 //case 5

#define TILE1_SIDE_TH 			180

#define DRIVE_FRONTORBACK		-60 //Unter dieser Differenz zwischen Sensor vorne-hinten wird auf den vorderen Sensor geachtet

#define WALLSENSOR_SOLL 300

#define TILE_LENGTH 30
#define TILE_LENGTH_ISWALL 0 //restliche Strecke ,,Feineinstellung" wird über Entfernungssensoren gemacht
#define TILE_LENGTH_MAX 33 //Zu weit gefahren => abbrechen

#define KP_DOT_DIR 0.37//15
#define KP_DOT_DIST 0.15//08

int16_t steer_dot = 0;

uint8_t sm_dot = 0; //driveonetile statemachine

uint8_t dot_tile_middle = 0; //Bei Fliesenwechsel wird einmal anderer Wert zurückgegeben

uint8_t whichTile = 0; //Beim Geradeausfahren über Entfernungssensor: Zu welcher Fliese fahren?

int32_t enc_lr_start_dot = 0;
int32_t enc_lr = 0; //Durchschnitt l/r
//////////////////////////////////

//Return 0: Fertig
//Return 1: > 15cm fahren
//Return 2: 15cm fahren
//Return 3: < 15cm fahren

uint8_t drive_oneTile()
{
	uint8_t returnvar = 0;
	uint8_t maxspeed = MAXSPEED;

	switch(sm_dot)
	{
		case 0: 	enc_lr_start_dot = (((int32_t)enc_l/2) + ((int32_t)enc_r/2));

							sm_dot = 1;

							if((int16_t)(wallsensor_top[FRONT] - wallsensor_top[BACK]) > DRIVE_FRONTORBACK) //An vorderem Sensor orientieren
							{
								if(wallsensor_top[FRONT] > TILE1_FRONT_TOP) //weiter als hier geht nicht!
								{
									sm_dot = 2;
									whichTile = 0;
								}
								else if(wallsensor_top[FRONT] > TILE2_FRONT_TH_TOP)
								{
									whichTile = 1; //Fahre zur Mitte der Fliese an der Wand
								}
								else if(wallsensor_top[FRONT] > TILE3_FRONT_TH_TOP)
								{
									whichTile = 2; //Fahre zur Mitte der Fliese vor der Fliese an der Wand
								}
								else
								{ 
									whichTile = 3; //Fahre über Encoder
								}
							}
							else
							{
								if(wallsensor_top[BACK] > TILE1_BACK_TH_TOP)
								{
									whichTile = 4;
								}
								else if(wallsensor_top[BACK] > TILE2_BACK_TH_TOP)
								{
									whichTile = 5;
								}
								else
								{
									whichTile = 6;
								}
							}
displayvar[0] = whichTile;
							returnvar = 1;
						break;
		case 1: 	enc_lr = (((int32_t)enc_l/2) + ((int32_t)enc_r/2));

							/////////Regelung (Abstand links/rechts)////////

							if((wallsensor[RIGHT][FRONT] > TILE1_SIDE_TH) && (wallsensor[RIGHT][BACK] > TILE1_SIDE_TH))
							{
								if(wallsensor[RIGHT][FRONT] > wallsensor[RIGHT][BACK])
								{
									steer_dot = ((0 - (wallsensor[RIGHT][BACK] - wallsensor[RIGHT][FRONT])) * KP_DOT_DIR);
								}
								else
								{
									steer_dot = ((0 - (wallsensor[RIGHT][FRONT] - wallsensor[RIGHT][BACK])) * (-(KP_DOT_DIR)));
								}
								steer_dot += ((int16_t)(WALLSENSOR_SOLL - wallsensor[RIGHT][FRONT]) * (-(KP_DOT_DIST)));
							}
							else if((wallsensor[LEFT][FRONT] > TILE1_SIDE_TH) && (wallsensor[LEFT][BACK] > TILE1_SIDE_TH))
							{
								if(wallsensor[LEFT][FRONT] > wallsensor[LEFT][BACK])
								{
									steer_dot = ((0 - (wallsensor[LEFT][BACK] - wallsensor[LEFT][FRONT])) * (-(KP_DOT_DIR)));
								}
								else
								{
									steer_dot = ((0 - (wallsensor[LEFT][FRONT] - wallsensor[LEFT][BACK])) * KP_DOT_DIR);
								}
								steer_dot += ((int16_t)(WALLSENSOR_SOLL - wallsensor[LEFT][FRONT]) * KP_DOT_DIST);
							}
							else
							{
								steer_dot = 0;
							}

							//////////////////////////////////////////////////////////////////

							////////Ziel erreicht?///////

							if(wallsensor_top[FRONT] > TILE1_FRONT_TOP) //weiter als hier geht nicht!
							{
								sm_dot = 2;
							}

							switch(whichTile)
							{ //Bei Encoder: Abschalten bzw. beenden wenn ÜBER Schwellwert oder vorderer Sensor Ende erkennt
								case 1:	if((wallsensor_top[FRONT] > TILE1_FRONT_TOP) &&
													 (enc_lr > (enc_lr_start_dot + (TILE_LENGTH_ISWALL * ENC_FAC_CM_LR))))
												{
													sm_dot = 2; //Fertig
												}
												break;
								case 2:	if((wallsensor_top[FRONT] > TILE2_FRONT_TOP) &&
													 (enc_lr > (enc_lr_start_dot + (TILE_LENGTH_ISWALL * ENC_FAC_CM_LR))))
												{
													sm_dot = 2; //Fertig
												}
												break;
								case 3: if(enc_lr > (enc_lr_start_dot + (TILE_LENGTH * ENC_FAC_CM_LR)))  //30cm gefahren?
												{
													sm_dot = 2; //Fertig
												}
												break;
								//Hintere Sensoreinheit:
								case 4: if((wallsensor_top[BACK] < TILE2_BACK_TOP) &&
													 (enc_lr > (enc_lr_start_dot + (TILE_LENGTH_ISWALL * ENC_FAC_CM_LR))))
												{
													sm_dot = 2; //Fertig
												}
												break;
								case 5: if((wallsensor_top[BACK] < TILE3_BACK_TOP) &&
													 (enc_lr > (enc_lr_start_dot + (TILE_LENGTH_ISWALL * ENC_FAC_CM_LR))))
												{
													sm_dot = 2; //Fertig
												}
												break;
								case 6: if(enc_lr > (enc_lr_start_dot + (TILE_LENGTH * ENC_FAC_CM_LR)))  //30cm gefahren?
												{
													sm_dot = 2; //Fertig
												}
												break;
							}

							if(enc_lr > (enc_lr_start_dot + (TILE_LENGTH_MAX * ENC_FAC_CM_LR)))  //zu weit gefahren?
							{
								sm_dot = 2; //abbrechen
							}

							///////Geschw. setzen//////////

							speed_l = (maxspeed - steer_dot);
							if(speed_l > maxspeed)  {
							 	speed_l = maxspeed;		}
							if(speed_l < -maxspeed)	{
								speed_l  = -maxspeed;	}
							 
							speed_r = (maxspeed + steer_dot); 
							if(speed_r > maxspeed)	{
								speed_r = maxspeed;		}
							if(speed_r < -maxspeed)	{
								speed_r  = -maxspeed; }
							////////////////////////////////
							if(enc_lr > (enc_lr_start_dot + ((TILE_LENGTH/2)*ENC_FAC_CM_LR))) // >= 15cm gefahren
							{
								if(dot_tile_middle == 0)
								{
									returnvar = 2; //15cm gefahren
									dot_tile_middle = 1;
								}
								else
								{
									returnvar = 1; // > 15cm gefahren
								}
							}
							else
							{
								returnvar = 3; // < 15cm gefahren
							}
displayvar[1] = enc_lr;
						break;
		case 2: 	sm_dot = 0;
							dot_tile_middle = 0;
							returnvar = 0;
						break;
	}
	return returnvar;
}

///////////////////////////////////////
uint8_t sm_rotate = 0;

int32_t um6_psi_t_start_rotate = 0;
int32_t enc_l_start_rotate = 0;
int32_t enc_r_start_rotate = 0;

#define ENC_DEGROTFAC 7.5

int16_t steer_rotate = 0;
uint8_t rotate_enc = 0; //Über ENcoder oder UM6 drehen?

#define TIMER_ROTATE_DEAD	127 //SIEHE ROBOCUP.C!!!
uint8_t timer_rotate_set = 0; //Timer gesetzt?

#define KP_ROTATE 0.9

#define STEER_ROTATE_ENC_TH 1 //Wenn UM6 eigtl. fertig ist, ENC aber noch nciht weitgenug gezählt haben (TH für Steer (=> Ende naht))
#define STEER_ROTATE_ENC 40//Mit dem Steer drehen (bei UM6 err)

#define UM6_ROTATE_OFFSET -3 //So weit dreht der Robo falsch

uint8_t drive_rotate(int16_t angle)
{
	uint8_t returnvar = 1;
	
	switch(sm_rotate)
	{
		case 0:		um6_psi_t_start_rotate = um6_psi_t;
							enc_l_start_rotate = enc_l;
							enc_r_start_rotate = enc_r;

							sm_rotate = 1;
						break;
		case 1: 	if(angle > 0)
								steer_rotate = ((um6_psi_t_start_rotate + angle + UM6_ROTATE_OFFSET) - um6_psi_t) * KP_ROTATE;
							else
								steer_rotate = ((um6_psi_t_start_rotate + angle - UM6_ROTATE_OFFSET) - um6_psi_t) * KP_ROTATE;
							
							if(check_um6 != 0)
							{
								rotate_enc = 1;
							}

							if(rotate_enc)
							{
								if(angle > 0)
								{
									if((enc_l < (enc_l_start_rotate + (ENC_DEGROTFAC * angle))) ||
										 (enc_r > (enc_r_start_rotate + (ENC_DEGROTFAC * angle))))
									{
										steer_rotate = STEER_ROTATE_ENC;
									}
									else
									{
										sm_rotate = 2;
										steer_rotate = 0;
									}
								}
								else
								{
									if((enc_l > (enc_l_start_rotate + (ENC_DEGROTFAC * angle))) ||
										 (enc_r < (enc_r_start_rotate + (ENC_DEGROTFAC * angle))))
									{
										steer_rotate = -STEER_ROTATE_ENC;
									}
									else
									{
										sm_rotate = 2;
										steer_rotate = 0;
									}
								}
							}
							else if(steer_rotate == 0)
							{
								sm_rotate = 2;	
							}

							if((steer_rotate < 10) && (steer_rotate > -10) && (!timer_rotate_set))
							{
								timer_rotate_dead = TIMER_ROTATE_DEAD;
								timer_rotate_set = 1;
							}
							if((timer_rotate_dead == 0) && timer_rotate_set)
							{
								sm_rotate = 2;
							}

							speed_l = steer_rotate;
							if(speed_l > MAXSPEED)  {
							 	speed_l = MAXSPEED;		}
							if(speed_l < -MAXSPEED)	{
								speed_l  = -MAXSPEED;	}

							speed_r = -(steer_rotate); 
							if(speed_r > MAXSPEED)	{
								speed_r = MAXSPEED;		}
							if(speed_r < -MAXSPEED)	{
								speed_r  = -MAXSPEED; }

						break;
		case 2: 	sm_rotate = 0;
							rotate_enc = 0;
							timer_rotate_set = 0;
							returnvar = 0;
						break;
	}
	return returnvar;		
}

////////////////////////////////////////
uint8_t sm_turn = 0;

int16_t steer_turn = 0;
#define KP_TURN 0.3

#define ALIGN_OFFSET_FRONT 0 //Roboter ist nicht zwangsläufig bei 0 Differenz gerade ==> Ausgleich (anderer Sollwert für Differenz)
#define ALIGN_OFFSET_BACK 0
#define ALIGN_OFFSET_LEFT 0
#define ALIGN_OFFSET_RIGHT 0

#define TILE1_FRONT_ALIGN_TH 100 //Schwellwert nach vorne
#define TILE1_SIDE_ALIGN_TH 150 //Schwellwert zur Seite

#define TURN_SENSDIFF_MAX 150 //-CONST - CONST ist Schwellwert für maximal erlaubte Sensordifferenz (ansonsten Fehler)
#define TIMER_ALIGN_DEAD  1000/25 //SIEHE ROBOCUP.C!!! Maximal so lange ausrichten, dann abbrechen

uint8_t drive_turn(int16_t angle, uint8_t align) //angle > 0 == Rechtsdrehung
{
	uint8_t returnvar = 1;
	
	switch(sm_turn)
	{
		case 0: if(!(drive_rotate(angle)))
						{
							sm_turn = 1;
							timer_align_dead = TIMER_ALIGN_DEAD;
						}

						returnvar = 1;
						break;
		case 1: //GGf. Ausrichtung an Wand
						if(align && (timer_align_dead > 0))
						{
							if(((wallsensor[RIGHT][FRONT] > TILE1_SIDE_ALIGN_TH) && (wallsensor[RIGHT][BACK] > TILE1_SIDE_ALIGN_TH)) && //GGf. Ausrichtung an Wand
								 (((wallsensor[RIGHT][FRONT] - wallsensor[RIGHT][BACK]) > (-(TURN_SENSDIFF_MAX))) ||
									((wallsensor[RIGHT][FRONT] - wallsensor[RIGHT][BACK]) < TURN_SENSDIFF_MAX)))
							{
								if(wallsensor[RIGHT][FRONT] > wallsensor[RIGHT][BACK])
								{
									steer_turn = (ALIGN_OFFSET_RIGHT - (wallsensor[RIGHT][BACK] - wallsensor[RIGHT][FRONT])) * -KP_TURN;
								}
								else
								{
									steer_turn = (ALIGN_OFFSET_RIGHT - (wallsensor[RIGHT][FRONT] - wallsensor[RIGHT][BACK])) * KP_TURN;
								}
							}
							else if(((wallsensor[LEFT][FRONT] > TILE1_SIDE_ALIGN_TH) && (wallsensor[LEFT][BACK] > TILE1_SIDE_ALIGN_TH)) && //linke Wand
								 			(((wallsensor[LEFT][FRONT] - wallsensor[LEFT][BACK]) > (-(TURN_SENSDIFF_MAX))) ||
											 ((wallsensor[LEFT][FRONT] - wallsensor[LEFT][BACK]) < TURN_SENSDIFF_MAX)))
							{
								if(wallsensor[LEFT][FRONT] > wallsensor[LEFT][BACK])
								{
									steer_turn = (ALIGN_OFFSET_LEFT - (wallsensor[LEFT][BACK] - wallsensor[LEFT][FRONT])) * KP_TURN;
								}
								else
								{
									steer_turn = (ALIGN_OFFSET_LEFT - (wallsensor[LEFT][FRONT] - wallsensor[LEFT][BACK])) * -KP_TURN;
								}
							}
							else if(((wallsensor[FRONT][LEFT] > TILE1_FRONT_ALIGN_TH) && (wallsensor[FRONT][RIGHT] > TILE1_FRONT_ALIGN_TH)) && //rechte Wand
								 			(((wallsensor[FRONT][LEFT] - wallsensor[FRONT][RIGHT]) > (-(TURN_SENSDIFF_MAX))) ||
											 ((wallsensor[FRONT][LEFT] - wallsensor[FRONT][RIGHT]) < TURN_SENSDIFF_MAX)))
							{
								if(wallsensor[FRONT][LEFT] > wallsensor[FRONT][RIGHT])
								{
									steer_turn = (ALIGN_OFFSET_FRONT - (wallsensor[FRONT][RIGHT] - (wallsensor[FRONT][LEFT]))) * -(KP_TURN);
								}
								else
								{
									steer_turn = (ALIGN_OFFSET_FRONT - (wallsensor[FRONT][LEFT] - wallsensor[FRONT][RIGHT])) * KP_TURN;
								}
							}
							else if(((wallsensor[BACK][LEFT] > TILE1_FRONT_ALIGN_TH) && (wallsensor[BACK][RIGHT] > TILE1_FRONT_ALIGN_TH)) && //hintere Wand
								 			(((wallsensor[BACK][LEFT] - wallsensor[BACK][RIGHT]) > (-(TURN_SENSDIFF_MAX))) ||
											 ((wallsensor[BACK][LEFT] - wallsensor[BACK][RIGHT]) < TURN_SENSDIFF_MAX)))
							{
								if(wallsensor[BACK][LEFT] > wallsensor[BACK][RIGHT])
								{
									steer_turn = (ALIGN_OFFSET_BACK - (int16_t)(wallsensor[BACK][LEFT] - wallsensor[BACK][RIGHT])) * -(KP_TURN);
								}
								else
								{
									steer_turn = (ALIGN_OFFSET_BACK - (int16_t)(wallsensor[BACK][RIGHT] - wallsensor[BACK][LEFT])) * KP_TURN;
								}
							}
							else
							{
								sm_turn = 2;
							}

							speed_l = steer_turn;
							if(speed_l > MAXSPEED)  {
							 	speed_l = MAXSPEED;		}
							if(speed_l < -MAXSPEED)	{
								speed_l  = -MAXSPEED;	}

							speed_r = -(steer_turn); 
							if(speed_r > MAXSPEED)	{
								speed_r = MAXSPEED;		}
							if(speed_r < -MAXSPEED)	{
								speed_r  = -MAXSPEED; }
						}
						else
						{
							sm_turn = 2;
						}

						returnvar = 1;
						break;
		case 2: sm_turn = 0;
						returnvar = 0;
						break;	
	}
	return returnvar;
}

////////////////////////////////////////////////////
#define WALLSENSOR_SOLL_RAMP 300

#define KP_RAMP_DIR 0.5
#define KP_RAMP_DIST 0.2

#define UM6_PHI_TH_RAMP 9
#define UM6_PHI_TH_NRAMP 4

#define TILE1_WALLSENSOR_BACK_RAMP 700
#define TILE1_WALLSENSOR_FRONT_RAMP 550

uint8_t ramp_ready = 0;

int16_t steer_ramp = 0;

uint8_t drive_ramp(int8_t speed)
{
	uint8_t wallsensor_1 = 0;
	uint8_t wallsensor_2 = 0;

	if(speed < 0)
	{
		wallsensor_1 = FRONT;
		wallsensor_2 = BACK;
	}
	else
	{
		wallsensor_2 = FRONT;
		wallsensor_1 = BACK;
	}
		

	if((wallsensor[RIGHT][wallsensor_2] > TILE1_SIDE_TH) && (wallsensor[RIGHT][wallsensor_1] > TILE1_SIDE_TH))
	{
		if(wallsensor[RIGHT][wallsensor_2] > wallsensor[RIGHT][wallsensor_1])
		{
			steer_ramp = ((0 - (wallsensor[RIGHT][wallsensor_1] - wallsensor[RIGHT][wallsensor_2])) * KP_RAMP_DIR);
		}
		else
		{
			steer_ramp = ((0 - (wallsensor[RIGHT][wallsensor_2] - wallsensor[RIGHT][wallsensor_1])) * -KP_RAMP_DIR);
		}
		steer_ramp += ((int16_t)(WALLSENSOR_SOLL - wallsensor[RIGHT][wallsensor_2]) * (-(KP_RAMP_DIST)));
	}
	else if((wallsensor[LEFT][wallsensor_2] > TILE1_SIDE_TH) && (wallsensor[LEFT][wallsensor_1] > TILE1_SIDE_TH))
	{
		if(wallsensor[LEFT][wallsensor_2] > wallsensor[LEFT][wallsensor_1])
		{
			steer_ramp = ((0 - (wallsensor[LEFT][wallsensor_1] - wallsensor[LEFT][wallsensor_2])) * -KP_RAMP_DIR);
		}
		else
		{
			steer_ramp = ((0 - (wallsensor[LEFT][wallsensor_2] - wallsensor[LEFT][wallsensor_1])) * KP_RAMP_DIR);
		}
		steer_ramp += ((int16_t)(WALLSENSOR_SOLL - wallsensor[LEFT][wallsensor_2]) * KP_RAMP_DIST);
	}
	else
	{
		steer_ramp = 0;
	}

	if(speed < 0)
		steer_ramp *= (-1);

	///////Geschw. setzen//////////

	speed_l = (speed - steer_ramp);
	if(speed > 0)
	{
		if(speed_l > speed)
		 	speed_l = speed;
		if(speed_l < -speed)
			speed_l  = -speed;
	}
	else
	{
		if(speed_l > -speed)
		 	speed_l = -speed;
		if(speed_l < speed)
			speed_l  = speed;
	}
	 
	speed_r = (speed + steer_ramp); 
	if(speed > 0)
	{
		if(speed_r > speed)
		 	speed_r = speed;
		if(speed_r < -speed)
			speed_r  = -speed;
	}
	else
	{
		if(speed_r > -speed)
		 	speed_r = -speed;
		if(speed_r < speed)
			speed_r  = speed;
	}

/////////////fertig?/////////////

	uint8_t returnvar = 1;

	switch(ramp_ready)
	{
		case 0:		if((um6_phi > UM6_PHI_TH_RAMP) && (um6_phi < 100))
							{
								ramp_ready = 1;
							}
						break;
		case 1:		if(um6_phi < UM6_PHI_TH_NRAMP)
							{
								ramp_ready = 2;
							}
						break;
		case 2:		if(speed < 0)
							{
								if(wallsensor_top[BACK] > TILE1_WALLSENSOR_BACK_RAMP)
								{
									returnvar = 0;
									ramp_ready = 0;
								}
							}
							else
							{
								if(wallsensor_top[FRONT] > TILE1_WALLSENSOR_FRONT_RAMP)
								{
									returnvar = 0;
									ramp_ready = 0;
								}
							}
						break;
	}
	return returnvar;
}

///////////////////////////////////////
uint8_t sm_d_lr = 0;//drive_leftright

uint8_t drive_lr(uint8_t left, uint8_t width) //left == 1: links, sonst rechts
{
	uint8_t returnvar = 1;
		
	switch(sm_d_lr)	
	{
		case 0:		if(left)
								sm_d_lr = 1;
							else
								sm_d_lr = 2;
						break;
		case 1: 	if(!(drive_dist(1, -width)))
							{
								if(left)
									sm_d_lr = 2;
								else
									sm_d_lr = 4;
							}
						break;
		case 2: 	if(!(drive_dist(-1, -width)))
							{
								if(left)
									sm_d_lr = 3;
								else
									sm_d_lr = 1;
							}
						break;
		case 3: 	if(!(drive_dist(1, width)))
							{
								if(left)
									sm_d_lr = 4;
								else
									sm_d_lr = 5;
							}
						break;
		case 4: 	if(!(drive_dist(-1, width)))
							{
								if(left)
									sm_d_lr = 5;
								else
									sm_d_lr = 3;
							}
						break;
		case 5: 	sm_d_lr = 0;
							returnvar = 0;
						break;
	}
	return returnvar;
}

/////////////////////////////////////////////////////////////

uint8_t sm_ddist = 0; //drivedist

int32_t enc_l_start_ddist = 0;
int32_t enc_r_start_ddist = 0;

uint8_t drive_dist(int8_t motor, int8_t dist_cm) //motor: <0 links, 0 beide, >0 rechts
{
	uint8_t returnvar = 0;

	switch(sm_ddist)
	{
		case 0: 	enc_l_start_ddist = enc_l;
							enc_r_start_ddist = enc_r;

							sm_ddist = 1;
							returnvar = 1; //noch nicht fertig
						break;
		case 1: 	if(dist_cm < 0)
							{
								if(motor < 0)
								{
									if(enc_l > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
									{
										speed_l = -MAXSPEED;
										speed_r = 0;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor == 0)
								{
									if((enc_l > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
										 (enc_r > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
									{
										speed_l = -MAXSPEED;
										speed_r = -MAXSPEED;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor > 0)
								{
									if(enc_r > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
									{
										speed_l = 0;
										speed_r = -MAXSPEED;
									}
									else
									{
										sm_ddist = 2;
									}
								}
							}
							else
							{
								if(motor < 0)
								{
									if(enc_l < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
									{
										speed_l = MAXSPEED;
										speed_r = 0;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor == 0)
								{
									if((enc_l < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
										 (enc_r < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
									{
										speed_l = MAXSPEED;
										speed_r = MAXSPEED;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor > 0)
								{
									if(enc_r < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
									{
										speed_l = 0;
										speed_r = MAXSPEED;
									}
									else
									{
										sm_ddist = 2;
									}
								}
							}
							returnvar = 1;
						break;
		case 2: 	sm_ddist = 0;

							returnvar = 0;
						break;
	}
	return returnvar;
}

////////////////////////////////////////////////////
//Alle Fahrfunktionen zurücksetzen (für Eingriff bzw. Positionsänderung)
void drive_reset(void)
{
	sm_dot = 0;
		dot_tile_middle = 0;
	sm_rotate = 0;
	sm_turn = 0;
	ramp_ready = 0;
	sm_d_lr = 0;	
	sm_ddist = 0;
}
