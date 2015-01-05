////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////drive.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Alle Fahrfunktionen
//	->	Grundgerüst
//			- 90° links
//			- 90° rechts
//			- 30cm geradeaus
//			- Kleinkram (rückwärts, nur eine Seite, Rampe etc.
//			also alles, was nichts mit Kartierung und Navigation zu tun hat!
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "drive.h"
#include "maze.h"
#include "mazefunctions.h"
#include "um6.h"
#include "funktionen.h"
#include "system.h"
#include "bluetooth.h"
#include "i2cdev.h"

void drive_limitSpeed(int16_t *speed_l, int16_t *speed_r, int8_t limit)
{
	if(*speed_l > limit)
	 	*speed_l = limit;
	if(*speed_l < -limit)
		*speed_l  = -limit;
	
	if(*speed_r > limit)
		*speed_r = limit;
	if(*speed_r < -limit)
		*speed_r  = -limit;
}

////////

int16_t steer_dot = 0;

int16_t dist_r_old = 0;
int16_t dist_l_old = 0;

int8_t dot_tile_middle = 0;

uint8_t sm_dot = DOT_INIT; //driveonetile statemachine
uint8_t dot_aligned_turn = NONE; //The robot had to align via the front Sensors (collision avoidance)?

int32_t enc_dot_comp[2];
int16_t dot_corr_angle = 0;

int32_t enc_lr_start_dot = 0;
int16_t enc_lr_add_dot = 0; //Add this length to the distance-to-drive after an (avoided) collision

int16_t um6_phi_t_start_dot = 0; //Slow down when getting odd (Ramp etc.)

//////////////////////////////////

uint8_t drive_oneTile(uint8_t abort)
{
	uint8_t returnvar = 0;
	int16_t maxspeed = MAXSPEED;

	int16_t rel_angle = 0;

	int16_t robot_angleToRightWall = robot_getAngleToWall(EAST);
	int16_t robot_angleToLeftWall = robot_getAngleToWall(WEST);

	switch(sm_dot)
	{
		case DOT_INIT:
		
							enc_dot_comp[LEFT] = mot.d[LEFT].enc;
							enc_dot_comp[RIGHT] = mot.d[RIGHT].enc;
							enc_lr_add_dot = 0;

							um6_phi_t_start_dot = um6.phi_t;

							dot_tile_middle = 0;
							dot_aligned_turn = NONE;
							timer_drive = -1;
							steer_dot = 0;

							if(dist[LIN][FRONT][FRONT] < TILE1_FRONT_FRONT) //weiter als hier geht nicht!
							{
								returnvar = DOT_RET_READY; //SOFORT abbrechen! Nicht erst in andere Teile der SM springen lassen (Zeitverschwendung)
								if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_oneTile()::dontstart:dist"));}
							}
							else
							{
								if((dist[LIN][BACK][BACK] < TILE1_BACK_TH_BACK) &&
									(maze_getWall(&robot.pos, robot.dir+2) > 0))
								{
									sm_dot = DOT_ALIGN_BACK;
									timer_drive = TIMER_ALIGN;
								}
								else
								{
									sm_dot = DOT_DRIVE;
									enc_lr_start_dot = mot.enc;
								}
									
								returnvar = DOT_RET_INIT;
								if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_oneTile()::enc_lr_start:"));bt_putLong(enc_lr_start_dot);}
							}
							
						break;
						
		case DOT_ALIGN_BACK:
		
							steer_dot = ((TILE1_BACK_BACK - (dist[LIN][BACK][BACK])) * KP_ALIGN_BACK);
							
							mot.d[LEFT].speed.to = steer_dot;
							mot.d[RIGHT].speed.to = steer_dot;
						
							drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);
						
							if((abs(steer_dot) <= STEER_ALIGN_BACK_END) || (timer_drive == 0) ||
								((dist[LIN][BACK][BACK] > TILE1_BACK_TH_BACK) && (dist[LIN][BACK][LEFT] > TILE1_BACK_TH_BACK) && (dist[LIN][BACK][RIGHT] > TILE1_BACK_TH_BACK)))
							{
								sm_dot = DOT_ALIGN;
								enc_lr_start_dot = mot.enc;
								timer_drive = -1;
							}
							
							returnvar = DOT_RET_ALIGN; // < 15cm gefahren
							
						break;
		case DOT_ALIGN:
		
							if(!drive_align())
								sm_dot = DOT_DRIVE;
							
							returnvar = DOT_RET_ALIGN; // < 15cm gefahren
							
						break;
						
		case DOT_DRIVE:

							////////////Limit maximum speed (Ramp, speed bumps...)/////////////

							rel_angle = abs((um6_phi_t_start_dot - um6.phi_t) * 2);
							if(rel_angle > 70)
								rel_angle = 70;
							maxspeed = MAXSPEED - rel_angle;

							/////////Regelung (Abstand links/rechts)////////

							if((abs(dist[LIN][LEFT][FRONT] - dist_l_old) < DELTADIST_MAX) &&
							   (abs(robot_angleToLeftWall) < abs(robot_angleToRightWall)))
							{
								if(sensinfo.newDat.left && sensinfo.newDat.right)
								{
									if(dist[LIN][LEFT][FRONT] < dist[LIN][LEFT][BACK])								
										steer_dot = (((int16_t)(dist[LIN][LEFT][BACK] - dist[LIN][LEFT][FRONT])) * -KP_DOT_DIR);
									else
										steer_dot = (((int16_t)(dist[LIN][LEFT][FRONT] - dist[LIN][LEFT][BACK])) * KP_DOT_DIR);
							
									steer_dot += ((int16_t)(DIST_SOLL - dist[LIN][LEFT][FRONT]) * -KP_DOT_DIST);
								
									sensinfo.newDat.left = 0;
									sensinfo.newDat.right = 0;
								}
							}
							else if((abs(dist[LIN][RIGHT][FRONT] - dist_r_old) < DELTADIST_MAX) &&
									(abs(robot_angleToLeftWall) > abs(robot_angleToRightWall)))
							{
								if(sensinfo.newDat.left && sensinfo.newDat.right)
								{
									if(dist[LIN][RIGHT][FRONT] < dist[LIN][RIGHT][BACK])
										steer_dot = (((int16_t)(dist[LIN][RIGHT][BACK] - dist[LIN][RIGHT][FRONT])) * KP_DOT_DIR);
									else
										steer_dot = (((int16_t)(dist[LIN][RIGHT][FRONT] - dist[LIN][RIGHT][BACK])) * -KP_DOT_DIR);
							
									steer_dot += ((int16_t)(DIST_SOLL - dist[LIN][RIGHT][FRONT]) * KP_DOT_DIST);
									
									sensinfo.newDat.left = 0;
									sensinfo.newDat.right = 0;
								}
							}
							else	steer_dot = 0;
							
							dist_l_old = dist[LIN][LEFT][FRONT];
							dist_r_old = dist[LIN][RIGHT][FRONT];
							
							////////////////////////////////////////////////////////////////////////
							////////Ziel erreicht? Kollision? Sollgeschwindigkeiten berechnen///////

							if(((robot_angleToRightWall > 20) && (robot_angleToRightWall != GETANGLE_NOANGLE) &&
								(dist[LIN][RIGHT][FRONT] < 15)))
							{
								dot_aligned_turn = WEST;

								mot.d[LEFT].speed.to = -SPEED_COLLISION_AVOIDANCE;
								mot.d[RIGHT].speed.to = SPEED_COLLISION_AVOIDANCE;
							}
							else if(((robot_angleToLeftWall > 20) && (robot_angleToLeftWall != GETANGLE_NOANGLE) &&
									 (dist[LIN][LEFT][FRONT] < 15)))
							{
								dot_aligned_turn = EAST;

								mot.d[LEFT].speed.to = SPEED_COLLISION_AVOIDANCE;
								mot.d[RIGHT].speed.to = -SPEED_COLLISION_AVOIDANCE;
							}
							else if(dot_aligned_turn == WEST)
							{
								sm_dot = DOT_ROT_WEST;
							}
							else if(dot_aligned_turn == EAST)
							{
								sm_dot = DOT_ROT_EAST;
							}
							else if((dist[LIN][FRONT][LEFT] < TILE1_FRONT_TH_FRONT) &&
									(dist[LIN][FRONT][FRONT] < TILE1_FRONT_TH_FRONT) &&
									(dist[LIN][FRONT][RIGHT] < TILE1_FRONT_TH_FRONT))
							{
								if(timer_drive == -1)
								{
									timer_drive = TIMER_ALIGN;
								}
								else if(timer_drive == 0)
								{
									sm_dot = DOT_COMP_ENC; //End, compare angle of the robot
								}
								else
								{
									steer_dot = ((TILE1_FRONT_FRONT - (dist[LIN][FRONT][FRONT])) * (-KP_ALIGN_FRONT));
					
									mot.d[LEFT].speed.to = steer_dot;
									mot.d[RIGHT].speed.to = steer_dot;
				
									drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);
				
									if(abs(steer_dot) <= STEER_ALIGN_BACK_END)
									{
										sm_dot = DOT_COMP_ENC; //End, compare angle of the robot
										mot.d[LEFT].speed.to = 0;
										mot.d[RIGHT].speed.to = 0;
									}
								}
							}
							else if((mot.enc > (enc_lr_start_dot + (TILE_LENGTH * ENC_FAC_CM_LR) + enc_lr_add_dot)) &&
								 			(abort == 0))  //30cm gefahren?
							{
								sm_dot = DOT_COMP_ENC; //End, compare angle of the robot
								mot.d[LEFT].speed.to = 0;
								mot.d[RIGHT].speed.to = 0;
							}
							else if((mot.enc < enc_lr_start_dot) &&
											(abort != 0))
							{
								sm_dot = DOT_COMP_ENC; //End, compare angle of the robot
								mot.d[LEFT].speed.to = 0;
								mot.d[RIGHT].speed.to = 0;
							}
							else if(abort != 0)
							{
								maxspeed /= 2;
								mot.d[LEFT].speed.to = -(maxspeed + steer_dot);
								mot.d[RIGHT].speed.to = -(maxspeed - steer_dot);
							}
							else
							{
								mot.d[LEFT].speed.to = (maxspeed - steer_dot);
								mot.d[RIGHT].speed.to = (maxspeed + steer_dot);
							}

							drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);
							////////////////////////////////
							if((abort != 0) && (dot_tile_middle & (1<<0)) && !((dot_tile_middle & (1<<1))>>1))
							{
								returnvar = AT_15; //Ist schon 15cm gefahren (-> Position geändert), muss deshalb wieder zurückgezählt werden
								dot_tile_middle |= (1<<1);
							}
							else if(mot.enc > (enc_lr_start_dot + enc_lr_add_dot + (TILE_LENGTH_MIN_DRIVE * ENC_FAC_CM_LR))) // >= 16cm gefahren *GEÄNDERT VON 15, KONSTANTE EINFÜHREN!!!*
							{
								if((dot_tile_middle & (1<<0)) == 0)
								{
									returnvar = NOW_15; //15cm gefahren
									dot_tile_middle |= (1<<0);
								}
								else
								{
									returnvar = AB_15; // > 15cm gefahren
								}
							}
							else
							{
								returnvar = UD_15; // < 15cm gefahren
							}

						break;

		case DOT_ROT_WEST:

							if(!drive_rotate(-TURN_ANGLE_COLLISION_AVOIDED, MAXSPEED))
							{
								sm_dot = DOT_DRIVE;
								dot_aligned_turn = NONE;

								if(enc_lr_add_dot < (DIST_ADD_COLLISION_MAX * ENC_FAC_CM_LR))
								{
									enc_lr_add_dot += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
								}
							}
							returnvar = DOT_RET_ALIGN;

						break;

		case DOT_ROT_EAST:
							if(!drive_rotate(TURN_ANGLE_COLLISION_AVOIDED, MAXSPEED))
							{
								sm_dot = DOT_DRIVE;
								dot_aligned_turn = NONE;

								enc_lr_add_dot += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
							}
							returnvar = DOT_RET_ALIGN;

						break;

		case DOT_COMP_ENC:

							if(abs((mot.d[RIGHT].enc - enc_dot_comp[RIGHT]) - (mot.d[LEFT].enc - enc_dot_comp[LEFT])) > 100)
							{
								if(robot_getAngleToWall(NONE) == GETANGLE_NOANGLE)
								{
									dot_corr_angle = ((mot.d[RIGHT].enc - enc_dot_comp[RIGHT]) - (mot.d[LEFT].enc - enc_dot_comp[LEFT]))/13;

									if(dot_corr_angle > 30)
										dot_corr_angle = 30;
									else if(dot_corr_angle < -30)
										dot_corr_angle = -30;

									sm_dot = DOT_CORR;
								}
								else
								{
									sm_dot = DOT_ALIGN_WALL;
								}
							}
							else
							{
								sm_dot = DOT_END;
							}

							returnvar = DOT_RET_ALIGN;

						break;

		case DOT_CORR:
							if(!drive_rotate(dot_corr_angle, MAXSPEED))
								sm_dot = DOT_END;

							returnvar = DOT_RET_ALIGN;
						break;

		case DOT_ALIGN_WALL:

							if(!drive_align())
							{
								sm_dot = DOT_END;
							}

						break;

		case DOT_END:
							
							sm_dot = DOT_INIT;
							returnvar = DOT_RET_READY;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_oneTile():done"));}
							
						break;
						
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL: went into drive_oneTile():sm_dot:DEFAULT_CASE"));}
							returnvar = 0;
						break;
							
	}
	return returnvar;
}

///////////////////////////////////////
enum DRIVE_ROTATE {ROTATE_INIT, ROTATE, ROTATE_END};

uint8_t sm_rotate = ROTATE_INIT;

int32_t um6_psi_t_start_rotate = 0;
int32_t enc_l_start_rotate = 0;
int32_t enc_r_start_rotate = 0;

int16_t steer_rotate = 0;
uint8_t rotate_enc = 0; //Über ENcoder oder UM6 drehen?

uint8_t rotate_progress = 0; //How much did the robot already move (%)

#define KP_ROTATE 3

#define STEER_ROTATE_ENC_TH 1 //Wenn UM6 eigtl. fertig ist, ENC aber noch nciht weitgenug gezählt haben (TH für Steer (=> Ende naht))
#define STEER_ROTATE_ENC 80//Mit dem Steer drehen (bei UM6 err)

#define UM6_ROTATE_OFFSET -3 //The smaller this offset, the less the robot rotates (usually as high as drift of the UM6)

#define STEER_ROTATE_TH_TIMER 20 //Unter diesem Wert (Betrag) wird ein Timer aktivierter, in dem Zeitraum 0 erreicht werden muss, ansonsten abbruch.

uint8_t drive_rotate(int16_t angle, uint8_t maxspeed)
{
	uint8_t returnvar = 1;
	
	switch(sm_rotate)
	{
		case ROTATE_INIT:
		
							um6_psi_t_start_rotate = um6.psi_t;
							enc_l_start_rotate = mot.d[LEFT].enc;
							enc_r_start_rotate = mot.d[RIGHT].enc;
							
							rotate_enc = 0;
							timer_drive = -1;
							
							rotate_progress = 0;

							sm_rotate = ROTATE;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_rotate()::psi_start: "));bt_putLong(um6_psi_t_start_rotate);}
							
						break;
						
		case ROTATE: 
		
							if(angle > 0)
								steer_rotate = ((um6_psi_t_start_rotate + angle + UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;
							else
								steer_rotate = ((um6_psi_t_start_rotate + angle - UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;
							
							rotate_progress = abs((um6.psi_t - um6_psi_t_start_rotate)*100)/angle;

							if(check_um6 != 0)
							{
								rotate_enc = 1;
							}

							if(rotate_enc)
							{
								if(angle > 0)
								{
									rotate_progress = abs(((mot.d[LEFT].enc - enc_l_start_rotate)*100)/(angle * ENC_DEGROTFAC));

									if((mot.d[LEFT].enc < (enc_l_start_rotate + (ENC_DEGROTFAC * angle))) ||
										 (mot.d[RIGHT].enc > (enc_r_start_rotate + (ENC_DEGROTFAC * angle))))
									{
										steer_rotate = STEER_ROTATE_ENC;
									}
									else
									{
										sm_rotate = ROTATE_END;
										steer_rotate = 0;
									}
								}
								else
								{
									rotate_progress = abs(((mot.d[LEFT].enc - enc_l_start_rotate)*100)/(angle * ENC_DEGROTFAC));

									if((mot.d[LEFT].enc > (enc_l_start_rotate + (ENC_DEGROTFAC * angle))) ||
										 (mot.d[RIGHT].enc < (enc_r_start_rotate + (ENC_DEGROTFAC * angle))))
									{
										steer_rotate = -STEER_ROTATE_ENC;
									}
									else
									{
										sm_rotate = ROTATE_END;
										steer_rotate = 0;
									}
								}
							}
							else if(steer_rotate == 0)
							{
								sm_rotate = ROTATE_END;	
							}

							if((abs(steer_rotate) < STEER_ROTATE_TH_TIMER) && (timer_drive == -1))
							{
								timer_drive = TIMER_ROTATE_DEAD;
							}
							if((timer_drive == 0) || (steer_rotate == 0))
							{
								sm_rotate = ROTATE_END;
							}

							mot.d[LEFT].speed.to = steer_rotate;
							mot.d[RIGHT].speed.to = -steer_rotate;
							
							drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);

						break;
						
		case ROTATE_END:
		
							mot.d[LEFT].speed.to = 0;
							mot.d[RIGHT].speed.to = 0;
							timer_drive = -1;
							sm_rotate = ROTATE_INIT;
							returnvar = 0;
						
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_oneTile(): done. "));}
							
						break;
						
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": FATAL ERROR: WENT INTO drive_rotate():sm_rotate DEFAULT CASE!"));}
						
						returnvar = 0;
						break;
	}
	
	return returnvar;
}

//////////////////////////////////////////////////////////
int16_t steer_turn = 0;


uint8_t sm_driveAlign = 0;

uint8_t drive_align(void)
{
	uint8_t returnvar = 1;
	
	switch(sm_driveAlign)
	{
		case 0:
						steer_turn = 0;
						timer_drive = TIMER_ALIGN_DEAD;
						
						sm_driveAlign = 1;
						
					break;
					
		case 1:
						if((dist[LIN][RIGHT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
							 (dist[LIN][RIGHT][BACK] < TILE1_SIDE_ALIGN_TH) &&
							 (abs(dist[LIN][RIGHT][FRONT] - dist[LIN][RIGHT][BACK]) < TURN_SENSDIFF_MAX))
						{
							if(dist[LIN][RIGHT][FRONT] > dist[LIN][RIGHT][BACK])
							{
								steer_turn = ((dist[LIN][RIGHT][BACK] + ALIGN_OFFSET_RIGHT) - dist[LIN][RIGHT][FRONT]) * -KP_ALIGN;
							}
							else
							{
								steer_turn = (dist[LIN][RIGHT][FRONT] - (dist[LIN][RIGHT][BACK] + ALIGN_OFFSET_RIGHT)) * KP_ALIGN;
							}
						}
						else if((dist[LIN][LEFT][FRONT] < TILE1_SIDE_ALIGN_TH) &&
										(dist[LIN][LEFT][BACK] < TILE1_SIDE_ALIGN_TH) &&
										(abs(dist[LIN][LEFT][FRONT] - dist[LIN][LEFT][BACK]) < TURN_SENSDIFF_MAX))
						{
							if(dist[LIN][LEFT][FRONT] > dist[LIN][LEFT][BACK])
							{
								steer_turn = ((dist[LIN][LEFT][BACK] + ALIGN_OFFSET_LEFT) - dist[LIN][LEFT][FRONT]) * KP_ALIGN;
							}
							else
							{
								steer_turn = (dist[LIN][LEFT][FRONT] - (dist[LIN][LEFT][BACK] + ALIGN_OFFSET_LEFT)) * -KP_ALIGN;
							}
						}
						else if((dist[LIN][FRONT][LEFT] < TILE1_SIDE_ALIGN_TH) &&
										(dist[LIN][FRONT][RIGHT] < TILE1_SIDE_ALIGN_TH) &&
										(abs(dist[LIN][FRONT][LEFT] - dist[LIN][FRONT][RIGHT]) < TURN_SENSDIFF_MAX))
						{
							if(dist[LIN][FRONT][LEFT] < dist[LIN][FRONT][RIGHT])
							{
								steer_turn = ((dist[LIN][FRONT][RIGHT] + ALIGN_OFFSET_FRONT) - dist[LIN][FRONT][LEFT]) * -KP_ALIGN;
							}
							else
							{
								steer_turn = (dist[LIN][FRONT][LEFT] - (dist[LIN][FRONT][RIGHT] + ALIGN_OFFSET_FRONT)) * KP_ALIGN;
							}
						}
						else if((dist[LIN][BACK][LEFT] < TILE1_SIDE_ALIGN_TH) &&
										(dist[LIN][BACK][RIGHT] < TILE1_SIDE_ALIGN_TH) &&
										(abs(dist[LIN][BACK][LEFT] - dist[LIN][BACK][RIGHT]) < TURN_SENSDIFF_MAX))
						{
							if(dist[LIN][BACK][LEFT] < dist[LIN][BACK][RIGHT])
							{
								steer_turn = ((dist[LIN][BACK][RIGHT] + ALIGN_OFFSET_BACK) - dist[LIN][BACK][LEFT]) * KP_ALIGN;
							}
							else
							{
								steer_turn = (dist[LIN][BACK][LEFT] - (dist[LIN][BACK][RIGHT] + ALIGN_OFFSET_BACK)) * -KP_ALIGN;
							}
						}
						else
						{
							sm_driveAlign = 2;
						}
						
						if((abs(steer_turn) <= STEER_ALIGN_DONE) || (timer_drive == 0))
							sm_driveAlign = 2;
							
					break;
					
		case 2:			steer_turn = 0;
						timer_drive = -1;
						sm_driveAlign = 0;
						returnvar = 0;
					break;
	}
	
	mot.d[LEFT].speed.to = steer_turn;
	mot.d[RIGHT].speed.to = -steer_turn;

	drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, MAXSPEED);
	
	return returnvar;
}
/////////////////////////////////////////////

uint8_t sm_dab = 0;
int16_t steer_dab = 0;

uint8_t drive_align_back(uint8_t dist_to) //Distance to the back
{
	uint8_t returnvar = 1;

	switch(sm_dab)
	{
		case 0:

				if(dist[LIN][BACK][BACK] < TILE1_BACK_TH_BACK)
				{
					timer_drive = TIMER_ALIGN;
					sm_dab = 1;
				}
				else
					sm_dab = 2;

			break;

		case 1:

				steer_dab = ((dist_to - (dist[LIN][BACK][BACK])) * KP_ALIGN_BACK);

				mot.d[LEFT].speed.to = steer_dab;
				mot.d[RIGHT].speed.to = steer_dab;

				drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, MAXSPEED);

				if((abs(steer_dab) <= STEER_ALIGN_BACK_END) || (timer_drive == 0) ||
					((dist[LIN][BACK][BACK] > TILE1_BACK_TH_BACK) && (dist[LIN][BACK][LEFT] > TILE1_BACK_TH_BACK) && (dist[LIN][BACK][RIGHT] > TILE1_BACK_TH_BACK)))
				{
					sm_dab = 2;
					timer_drive = -1;
				}

			break;

		case 2:
				mot.d[LEFT].speed.to = 0;
				mot.d[RIGHT].speed.to = 0;

				sm_dab = 0;
				returnvar = 0;

	}

	return returnvar;
}

/////////////////////////////////////////
enum DRIVE_TURN {TURN_INIT, TURN, TURN_ALIGN, TURN_ALIGN_BACK, TURN_END};

uint8_t sm_turn = TURN_INIT;

uint8_t drive_turn(int16_t angle, uint8_t align) //angle > 0 == turn right
{
	uint8_t returnvar = 1;
	
	switch(sm_turn)
	{
		case TURN_INIT:
		
						sm_turn = TURN;
						
		case TURN:
						if(!drive_rotate(angle, MAXSPEED))
						{
							if(align)
								sm_turn = TURN_ALIGN;
							else
								sm_turn = TURN_END;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_turn()"));}
						}

					break;
					
		case TURN_ALIGN:
		
						//GGf. Ausrichtung an Wand
						if(!drive_align())
						{
							if((dist[LIN][BACK][BACK] < TILE1_BACK_TH_BACK) &&
								(maze_getWall(&robot.pos, robot.dir+2) > 0))
							{
								sm_turn = TURN_ALIGN_BACK;
							}
							else
							{
								sm_turn = TURN_END;
							}

							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_turn(): aligned"));}
						}

					break;

		case TURN_ALIGN_BACK:

							if(!drive_align_back(TILE1_BACK_BACK))
							{
								sm_turn = TURN_END;
							}

					break;
					
		case TURN_END:
		
						sm_turn = TURN_INIT;
						returnvar = 0;
						
						if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_turn(): done. "));}
						
					break;
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": FATAL ERROR: WENT INTO drive_turn():sm_turn DEFAULT CASE!"));}
							returnvar = 0;
						break;
	}
	return returnvar;
}

/////////////////////////////////////////////////////
///
uint8_t sm_drive_getBall = 0;

uint8_t drive_getBall(void)
{
	uint8_t returnvar = 1;

	switch (sm_drive_getBall) {
	case 0: if(!drive_dist(0,30,25))
			{
				servo_setPos(100);
				sm_drive_getBall ++;
			}
		break;
	case 1: if(!drive_dist(0,30,5))
			{
				sm_drive_getBall ++;
			}
		break;
	case 2: if(!drive_dist(0,30,-30))
			{
				sm_drive_getBall = 0;
				returnvar = 0;
			}
		break;
	default:
		break;
	}

	return returnvar;
}

uint8_t sm_drive_thrBall = 0;

uint8_t drive_releaseBall(void)
{
	uint8_t returnvar = 1;

	switch (sm_drive_getBall) {
	case 0: if(!drive_dist(0,30,20))
			{
				servo_setPos(30);
				sm_drive_getBall ++;
			}
		break;
	case 1: if(!drive_dist(0,30,-20))
			{
				sm_drive_getBall = 0;
				returnvar = 0;
			}
		break;
	default:
		break;
	}


	return returnvar;
}

uint8_t drive_instr_sm = 0;

uint8_t drive_instructions(char *instructions, uint8_t amount)
{
	if(drive_instr_sm < amount)
	{
		switch (instructions[drive_instr_sm]) {
		case 'f':	if(!drive_oneTile(0))	drive_instr_sm ++; break;
		case 'l':	if(!drive_turn(-90,1))	drive_instr_sm ++; break;
		case 'r':	if(!drive_turn(90, 1))	drive_instr_sm ++; break;
		case 'd':	if(!drive_getBall())	drive_instr_sm ++;	break;
		case 'u':	if(!drive_releaseBall())	drive_instr_sm ++;	break;
		default:	drive_instr_sm ++; break;
		}
		return 1;
	}
	else
	{
		drive_instr_sm = 0;
		return 0;
	}
}

////////////////////////////////////////////////////////
#define DIST_SOLL_RAMP 50 //mm, Sollabstand Rampe

#define KP_RAMP_DIR 0.8
#define KP_RAMP_DIST KP_RAMP_DIR * 0.6

#define RAMP_UM6_GYR_DELTAX 15 //Winkelgeschwindigkeit (Rampenerkennung)

#define TILE1_DIST_FRONT_RAMP 70 //Vorne, IR

int16_t steer_ramp = 0;
int8_t sm_ramp = 0;
int8_t ramp_checkpoint = 0;
int32_t ramp_checkpoint_enc = 0;

uint16_t groundsens_ramp_start = 0;

uint8_t drive_ramp(int8_t speed)
{
	if((dist[LIN][LEFT][FRONT] < TILE1_SIDE_TH) &&
		(dist[LIN][LEFT][BACK] < TILE1_SIDE_TH))
	{
		if(sensinfo.newDat.left && sensinfo.newDat.right)
		{
			if(dist[LIN][LEFT][FRONT] < dist[LIN][LEFT][BACK])
				steer_ramp = (((int16_t)(dist[LIN][LEFT][BACK] - dist[LIN][LEFT][FRONT])) * -KP_RAMP_DIR);
			else
				steer_ramp = (((int16_t)(dist[LIN][LEFT][FRONT] - dist[LIN][LEFT][BACK])) * KP_RAMP_DIR);

			steer_ramp += ((int16_t)(DIST_SOLL - dist[LIN][LEFT][FRONT]) * -KP_RAMP_DIST);

			sensinfo.newDat.left = 0;
			sensinfo.newDat.right = 0;
		}
	}
	else if((dist[LIN][RIGHT][FRONT] < TILE1_SIDE_TH) &&
					(dist[LIN][RIGHT][BACK] < TILE1_SIDE_TH))
	{
		if(sensinfo.newDat.left && sensinfo.newDat.right)
		{
			if(dist[LIN][RIGHT][FRONT] < dist[LIN][RIGHT][BACK])
				steer_ramp = (((int16_t)(dist[LIN][RIGHT][BACK] - dist[LIN][RIGHT][FRONT])) * KP_RAMP_DIR);
			else
				steer_ramp = (((int16_t)(dist[LIN][RIGHT][FRONT] - dist[LIN][RIGHT][BACK])) * -KP_RAMP_DIR);

			steer_ramp += ((int16_t)(DIST_SOLL - dist[LIN][RIGHT][FRONT]) * KP_RAMP_DIST);

			sensinfo.newDat.left = 0;
			sensinfo.newDat.right = 0;
		}
	}
	/////////////Checkpoints/////////////////

	if(groundsens_l < GROUNDSENS_L_TH_CHECKPOINT)
	{
		ramp_checkpoint_enc = mot.enc;
		ramp_checkpoint = 1;
	}

	if((((mot.enc - ramp_checkpoint_enc)/ENC_FAC_CM_LR) < 30) && (ramp_checkpoint == 1))
		ramp_checkpoint = 2;
	else if((((mot.enc - ramp_checkpoint_enc)/ENC_FAC_CM_LR) >= 30) && (ramp_checkpoint == 2))
		ramp_checkpoint = 1;

	displayvar[4] = ramp_checkpoint;
	displayvar[5] = ((mot.enc - ramp_checkpoint_enc)/ENC_FAC_CM_LR);

/////////////fertig?/////////////
	uint8_t returnvar = 1;
	
	///////Geschw. setzen//////////

	if(speed < 0)
		speed *= -1;
	
	mot.d[LEFT].speed.to = (speed - steer_ramp);
	mot.d[RIGHT].speed.to = (speed + steer_ramp);

	drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, 80);

	//Return
	switch(sm_ramp)
	{
		case 0: if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_ramp()"));}
						sm_ramp = 1;
						ramp_checkpoint = 0;
						ramp_checkpoint_enc = 0;

		case 1:			if((um6.gyr.y > RAMP_UM6_GYR_DELTAX) && //Arrived at upper end
		 					 (speed < 0))
						{
		 					sm_ramp = 2;
						}
						else if((um6.gyr.y < -RAMP_UM6_GYR_DELTAX) && //Arrived at lower end
	 					 	 (speed > 0))
						{
							sm_ramp = 2;
						}
		 			break;
		case 2:

						if((dist[LIN][FRONT][FRONT] < TILE1_DIST_FRONT_RAMP) &&
							(dist[LIN][FRONT][LEFT] < TILE1_DIST_FRONT_RAMP))
		 				{
							returnvar = 0;
	 						sm_ramp = 0;
							mot.d[LEFT].speed.to = 0;
							mot.d[RIGHT].speed.to = 0;
	 						if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_ramp()::done::speed:")); bt_putLong(speed);}
						}
					break;
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": FATAL ERROR: WENT INTO drive_ramp():sm_ramp DEFAULT CASE!"));}
							returnvar = 0;
							sm_ramp = 0;
						break;
	}
	return returnvar;
}

///////////////////////////////////////

uint8_t sm_nP = 0;

uint8_t drive_neutralPos(void)
{
	uint8_t returnvar = 1;

	switch(sm_nP)
	{
		case 0:
				if(!drive_turn(90, 1))
				{
					sm_nP = 1;
				}
				
			break;
		
		case 1:
				if(dist[LIN][FRONT][FRONT] > TILE1_FRONT_FRONT+10)
				{
					drive_oneTile(0);
				}
				else
					sm_nP = 2;
			
			break;
			
		case 2:
				if(!drive_turn(90, 1))
				{
					sm_nP = 3;
				}
			
			break;
		
		case 3:
				if(dist[LIN][FRONT][FRONT] > TILE1_FRONT_FRONT+10)
				{
					drive_oneTile(0);
				}
				else	sm_nP = 4;
				
			break;
			
		case 4:
				if(!drive_turn(90, 1))
				{
					sm_nP = 5;
				}
				
			break;
			
		case 5:
				sm_nP = 0;
				returnvar = 0;
			break;
	}
	
	return returnvar;
}

/////////////////////////////////////////////////////

uint8_t sm_d_deplKit = 0;
int8_t d_deplKit_amount = 0;

uint8_t drive_deployResKit(int8_t dir, uint8_t amount)
{
	uint8_t returnvar = 1;

	dir --;
	if(!dir)
		dir = 1;
	//Now: dir: LEFT = -1, RIGHT = 1;

	switch(sm_d_deplKit)
	{
		case 0:
				d_deplKit_amount = amount;

				sm_d_deplKit = 1;

				break;
		case 1:
				if(!drive_turn(90 * (-dir), 1))
				{
					sm_d_deplKit = 2;
				}
			break;

		case 2:

				if(dist[LIN][BACK][BACK] > 70)
				{
					if(!drive_dist(0,30,-4))
					{
						sm_d_deplKit = 3;
					}
				}
				else
					sm_d_deplKit = 3;

			break;

		case 3:

				mot.d[LEFT].speed.to = 0;
				mot.d[RIGHT].speed.to = 0;

				if(d_deplKit_amount > 0)
				{
					if(!rescueKit_drop(2))
					{
						d_deplKit_amount --;
					}
				}
				else
				{
					sm_d_deplKit = 4;
				}

			break;

		case 4:

				if(dist[LIN][BACK][BACK] > TILE1_BACK_TH_BACK)
				{
					if(!drive_dist(0,30,4))
					{
						sm_d_deplKit = 5;
					}
				}
				else if(!drive_align_back(50))
				{
					sm_d_deplKit = 5;
				}

			break;

		case 5:
				if(!drive_turn(90 * dir, 1))
				{
					sm_d_deplKit = 0;
					returnvar = 0;
				}
			break;
	}

	return returnvar;
}

///////////////////////////////////////
uint8_t sm_d_lr = 0;//drive_leftright

#define DRIVE_LEFT_FAC 0.8
#define DRIVE_RIGHT_FAC 0.95

uint8_t drive_lr(int8_t left, int8_t speed, uint8_t width) //left == 1: links, sonst rechts
{
	uint8_t returnvar = 1;
	uint8_t width_var = width;
	
	switch(sm_d_lr)	
	{
		case 0:		if(left)
								sm_d_lr = 1;
							else
								sm_d_lr = 2;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_lr()"));}
						break;
		case 1: 	if(!(drive_dist(1, speed, -width)))
							{
								if(left)
									sm_d_lr = 2;
								else
									sm_d_lr = 4;
							}
						break;
		case 2: 	if(!(drive_dist(-1, speed, -width)))
							{
								if(left)
									sm_d_lr = 3;
								else
									sm_d_lr = 1;
							}
						break;
		case 3: 	if(!(left))
								width_var *= DRIVE_RIGHT_FAC;
								
							if(!(drive_dist(1, speed, width_var)))
							{
								if(left)
									sm_d_lr = 4;
								else
									sm_d_lr = 5;
							}
						break;
		case 4: 	if(left)
								width_var *= DRIVE_LEFT_FAC;
								
							if(!(drive_dist(-1, speed, width_var)))
							{
								if(left)
									sm_d_lr = 5;
								else
									sm_d_lr = 3;
							}
						break;
		case 5: 	sm_d_lr = 0;
							returnvar = 0;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_lr(): done. "));}
						break;
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": FATAL ERROR: WENT INTO drive_lr():sm_d_lr DEFAULT CASE!"));}
							returnvar = 0;
						break;
	}
	return returnvar;
}

/////////////////////////////////////////////////////////////

uint8_t sm_ddist = 0; //drivedist

int32_t enc_l_start_ddist = 0;
int32_t enc_r_start_ddist = 0;

uint8_t drive_dist(int8_t motor, int8_t speed, int8_t dist_cm) //which @motor to move: < 0 ~ left, 0 ~ both, > 0 ~ right
{
	uint8_t returnvar = 1;

	switch(sm_ddist)
	{
		case 0: 	enc_l_start_ddist = mot.d[LEFT].enc;
							enc_r_start_ddist = mot.d[RIGHT].enc;

							sm_ddist = 1;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_dist()"));}
						break;
		case 1: 	if(dist_cm < 0)
							{
								if(motor < 0)
								{
									if(mot.d[LEFT].enc > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
									{
										mot.d[LEFT].speed.to = -speed;
										mot.d[RIGHT].speed.to = 0;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor == 0)
								{
									if((mot.d[LEFT].enc > (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
										 (mot.d[RIGHT].enc > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
									{
										mot.d[LEFT].speed.to = -speed;
										mot.d[RIGHT].speed.to = -speed;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor > 0)
								{
									if(mot.d[RIGHT].enc > (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
									{
										mot.d[LEFT].speed.to = 0;
										mot.d[RIGHT].speed.to = -speed;
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
									if(mot.d[LEFT].enc < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) //Ziel erreicht?
									{
										mot.d[LEFT].speed.to = speed;
										mot.d[RIGHT].speed.to = 0;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor == 0)
								{
									if((mot.d[LEFT].enc < (enc_l_start_ddist + (dist_cm*ENC_FAC_CM_L))) || //Ziel erreicht?
										 (mot.d[RIGHT].enc < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))))
									{
										mot.d[LEFT].speed.to = speed;
										mot.d[RIGHT].speed.to = speed;
									}
									else
									{
										sm_ddist = 2;
									}
								}
								else if(motor > 0)
								{
									if(mot.d[RIGHT].enc < (enc_r_start_ddist + (dist_cm*ENC_FAC_CM_R))) //Ziel erreicht?
									{
										mot.d[LEFT].speed.to = 0;
										mot.d[RIGHT].speed.to = speed;
									}
									else
									{
										sm_ddist = 2;
									}
								}
							}
						break;
		case 2:
							sm_ddist = 0;

							mot.d[LEFT].speed.to = 0;
							mot.d[RIGHT].speed.to = 0;

							returnvar = 0;
							
							if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_dist(): done. "));}
						break;
		default:	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": FATAL ERROR: WENT INTO drive_dist():sm_ddist DEFAULT CASE!"));}
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
	sm_d_lr = 0;	
	sm_ddist = 0;
	
	if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": Reset::sm_dot:dot_tile_middle:sm_rotate:sm_turn:ramp_ready:sm_d_lr:sm_ddist"));}
}
