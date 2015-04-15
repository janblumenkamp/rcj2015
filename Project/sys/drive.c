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


//////////////////////////////////

void drive_oneTile(DOT *d)
{
	int16_t maxspeed = MAXSPEED;

	int16_t rel_angle = 0;

	int16_t robot_angleToRightWall = robot_getAngleToWall(EAST);
	int16_t robot_angleToLeftWall = robot_getAngleToWall(WEST);

	switch(d->state)
	{
		case DOT_INIT:
							d->abort = 0; //Should never be 1 on this point -> Reset it in case it is still set

							d->enc_comp[LEFT] = mot.d[LEFT].enc;
							d->enc_comp[RIGHT] = mot.d[RIGHT].enc;
							d->enc_lr_add = 0;

							d->um6_phi_t_start = um6.phi_t;

							d->aligned_turn = NONE;

							if(dist[LIN][FRONT][FRONT] < TILE1_FRONT_FRONT) //There is a wall directly in front of the robot
							{
								d->state = DOT_END; //Break because we can`t drive straight
								foutf(&str_debugDrive, "%i: dot:noSt_dst\n\r", timer);
							}
							else
							{
								if((dist[LIN][BACK][BACK] < TILE1_BACK_TH_BACK) && //Enough place in back of the robot
									(maze_getWall(&robot.pos, robot.dir+2) > 0)) //AND there is a wall in the map (prevent false sensordata...)
								{
									d->state = DOT_ALIGN_BACK;
									d->timer = timer;
								}
								else //Directly jump to the drive part
								{
									d->state = DOT_DRIVE;
									d->enc_lr_start = mot.enc;
									d->timer = 0; //Unactivate timer
								}

								foutf(&str_debugDrive, "%i: dot:st.en: %i\n\r", timer, d->enc_lr_start);
							}
							
						break;
						
		case DOT_ALIGN_BACK:
		
							if(!drive_align_back(TILE1_BACK_BACK))
							{
								d->state = DOT_ALIGN;
								d->enc_lr_start = mot.enc;
								d->timer = 0; //Unactivate timer
							}
							
						break;
		case DOT_ALIGN:
		
							if(!drive_align())
								d->state= DOT_DRIVE;
							
						break;
						
		case DOT_DRIVE:

							////////////Limit maximum speed (Ramp, speed bumps...)/////////////

							rel_angle = abs((d->um6_phi_t_start - um6.phi_t) * 2);
							if(rel_angle > 70)
								rel_angle = 70;
							maxspeed = MAXSPEED - rel_angle;

							/////////Regelung (Abstand links/rechts)////////

							if((abs(dist[LIN][LEFT][FRONT] - d->dist_l_old) < DELTADIST_MAX) &&
							   (abs(robot_angleToLeftWall) < abs(robot_angleToRightWall)))
							{
								if(sensinfo.newDat.left && sensinfo.newDat.right)
								{
									if(dist[LIN][LEFT][FRONT] < dist[LIN][LEFT][BACK])
										d->steer = (((int16_t)(dist[LIN][LEFT][BACK] - dist[LIN][LEFT][FRONT])) * -KP_DOT_DIR);
									else
										d->steer = (((int16_t)(dist[LIN][LEFT][FRONT] - dist[LIN][LEFT][BACK])) * KP_DOT_DIR);
							
									d->steer += ((int16_t)(DIST_SOLL - dist[LIN][LEFT][FRONT]) * -KP_DOT_DIST);
								
									sensinfo.newDat.left = 0;
									sensinfo.newDat.right = 0;
								}
							}
							else if((abs(dist[LIN][RIGHT][FRONT] - d->dist_r_old) < DELTADIST_MAX) &&
									(abs(robot_angleToLeftWall) > abs(robot_angleToRightWall)))
							{
								if(sensinfo.newDat.left && sensinfo.newDat.right)
								{
									if(dist[LIN][RIGHT][FRONT] < dist[LIN][RIGHT][BACK])
										d->steer = (((int16_t)(dist[LIN][RIGHT][BACK] - dist[LIN][RIGHT][FRONT])) * KP_DOT_DIR);
									else
										d->steer = (((int16_t)(dist[LIN][RIGHT][FRONT] - dist[LIN][RIGHT][BACK])) * -KP_DOT_DIR);
							
									d->steer += ((int16_t)(DIST_SOLL - dist[LIN][RIGHT][FRONT]) * KP_DOT_DIST);
									
									sensinfo.newDat.left = 0;
									sensinfo.newDat.right = 0;
								}
							}
							else
							{
								d->steer = 0;
							}

							d->dist_l_old = dist[LIN][LEFT][FRONT];
							d->dist_r_old = dist[LIN][RIGHT][FRONT];
							
							////////////////////////////////////////////////////////////////////////
							////////Ziel erreicht? Kollision? Sollgeschwindigkeiten berechnen///////

							if(d->abort)
							{
								if(mot.enc < d->enc_lr_start) //There is a request to drive back - drive back until we reach the same encoder dist as start
								{
									d->state = DOT_COMP_ENC; //End, compare angle of the robot
									mot.d[LEFT].speed.to = 0;
									mot.d[RIGHT].speed.to = 0;
								}
								else
								{
									maxspeed /= 2;
									mot.d[LEFT].speed.to = -(maxspeed + d->steer);
									mot.d[RIGHT].speed.to = -(maxspeed - d->steer);
								}
							}
							else
							{
								if(((dist[LIN][FRONT][RIGHT] < COLLISIONAVOIDANCE_SENS_TH_1) &&
									(dist[LIN][FRONT][LEFT] >= COLLISIONAVOIDANCE_SENS_TH_2) && (dist[LIN][FRONT][FRONT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
									(mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
									(rel_angle < 20)) ||

									get_bumpR() ||

								   ((robot_angleToRightWall > 20) && (robot_angleToRightWall != GETANGLE_NOANGLE) &&
									(dist[LIN][RIGHT][FRONT] < 15)))
								{
									d->aligned_turn = WEST;

									mot.d[LEFT].speed.to = -SPEED_COLLISION_AVOIDANCE;
									mot.d[RIGHT].speed.to = SPEED_COLLISION_AVOIDANCE;
								}
								else if(((dist[LIN][FRONT][LEFT] < COLLISIONAVOIDANCE_SENS_TH_1) &&
										 (dist[LIN][FRONT][RIGHT] >= COLLISIONAVOIDANCE_SENS_TH_2) && (dist[LIN][FRONT][FRONT] >= COLLISIONAVOIDANCE_SENS_TH_2) &&
										 (mot.enc < (d->enc_lr_start + (TILE_DIST_COLLISION_AV * ENC_FAC_CM_LR) + d->enc_lr_add/2)) &&
										 (rel_angle < 20)) ||

										get_bumpL() ||

										((robot_angleToLeftWall > 20) && (robot_angleToLeftWall != GETANGLE_NOANGLE) &&
										 (dist[LIN][LEFT][FRONT] < 15)))
								{
									d->aligned_turn = EAST;

									mot.d[LEFT].speed.to = SPEED_COLLISION_AVOIDANCE;
									mot.d[RIGHT].speed.to = -SPEED_COLLISION_AVOIDANCE;
								}
								else if(d->aligned_turn == WEST) //Robot had to align via front sensors or bumpers. Now add a small alignment (turn a bit more in the corresponding direction)
								{
									d->state = DOT_ROT_WEST;
								}
								else if(d->aligned_turn == EAST)
								{
									d->state = DOT_ROT_EAST;
								}
								else if((dist[LIN][FRONT][LEFT] < TILE1_FRONT_TH_FRONT) &&
										(dist[LIN][FRONT][FRONT] < TILE1_FRONT_TH_FRONT) &&
										(dist[LIN][FRONT][RIGHT] < TILE1_FRONT_TH_FRONT))
								{
									if(d->timer == 0)
									{
										d->timer = timer;
									}
									else if((timer - d->timer) > TIMER_DOT_ALIGN)
									{
										d->state = DOT_COMP_ENC; //End, compare angle of the robot
									}
									else
									{
										d->steer = ((TILE1_FRONT_FRONT - (dist[LIN][FRONT][FRONT])) * (-KP_ALIGN_FRONT));

										mot.d[LEFT].speed.to = d->steer;
										mot.d[RIGHT].speed.to = d->steer;

										drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);

										if(abs(d->steer) <= STEER_ALIGN_BACK_END)
										{
											d->state = DOT_COMP_ENC; //End, compare angle of the robot
											mot.d[LEFT].speed.to = 0;
											mot.d[RIGHT].speed.to = 0;
										}
									}
								}
								else if(mot.enc > (d->enc_lr_start + (TILE_LENGTH * ENC_FAC_CM_LR) + d->enc_lr_add))  //driven 30cm
								{
									d->state = DOT_COMP_ENC; //End, compare angle of the robot
									mot.d[LEFT].speed.to = 0;
									mot.d[RIGHT].speed.to = 0;
								}
								else //Normal driving forward
								{
									mot.d[LEFT].speed.to = (maxspeed - d->steer);
									mot.d[RIGHT].speed.to = (maxspeed + d->steer);
								}
							}

							drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, maxspeed);

						break;

		case DOT_ROT_WEST:
							d->r.angle = -TURN_ANGLE_COLLISION_AVOIDED;
							drive_rotate(&d->r);

							if(d->r.state == ROTATE_FINISHED)
							{
								d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...

								d->state = DOT_DRIVE;
								d->aligned_turn = NONE;

								if(d->enc_lr_add < (DIST_ADD_COLLISION_MAX * ENC_FAC_CM_LR))
								{
									d->enc_lr_add += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
								}
							}

						break;

		case DOT_ROT_EAST:
							d->r.angle = TURN_ANGLE_COLLISION_AVOIDED;
							drive_rotate(&d->r);

							if(d->r.state == ROTATE_FINISHED)
							{
								d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...

								d->state = DOT_DRIVE;
								d->aligned_turn = NONE;

								if(d->enc_lr_add < (DIST_ADD_COLLISION_MAX * ENC_FAC_CM_LR))
								{
									d->enc_lr_add += DIST_ADD_COLLISION * ENC_FAC_CM_LR;
								}
							}

						break;

		case DOT_COMP_ENC:

							if(abs((mot.d[RIGHT].enc - d->enc_comp[RIGHT]) - (mot.d[LEFT].enc - d->enc_comp[LEFT])) > 100)
							{
								if(robot_getAngleToWall(NONE) == GETANGLE_NOANGLE)
								{
									d->corr_angle = ((mot.d[RIGHT].enc - d->enc_comp[RIGHT]) - (mot.d[LEFT].enc - d->enc_comp[LEFT]))/13;

									if(d->corr_angle > 30)
										d->corr_angle = 30;
									else if(d->corr_angle < -30)
										d->corr_angle = -30;

									d->state = DOT_CORR;
								}
								else
								{
									d->state = DOT_ALIGN_WALL;
								}
							}
							else
							{
								d->state = DOT_END;
							}

						break;

		case DOT_CORR:
							d->r.angle = d->corr_angle;
							drive_rotate(&d->r);

							if(d->r.state == ROTATE_FINISHED)
							{
								d->r.state = ROTATE_INIT; //Allow rotate function with this object to start again...
								d->state = DOT_END;
							}

						break;

		case DOT_ALIGN_WALL:

							if(!drive_align())
							{
								d->state = DOT_END;
							}

						break;

		case DOT_END:
							
							d->state = DOT_FINISHED;

							foutf(&str_debugDrive, "%i: dot:end\n\r", timer);

						break;

		case DOT_FINISHED:

						break;
						
		default:		foutf(&str_error, "%i: ERR:sw[drv.01]:DEF\n\r", timer);
						fatal_err = 1;
						d->state = DOT_INIT;
						break;
							
	}
}

///////////////////////////////////////

void drive_rotate(D_ROTATE *r)
{
	switch(r->state)
	{
		case ROTATE_INIT:
		
							r->um6_psi_t_start = um6.psi_t;
							r->enc_l_start = mot.d[LEFT].enc;
							r->enc_r_start = mot.d[RIGHT].enc;
							
							r->use_enc = 0;
							r->timer = 0; //Reset timer
							
							r->progress = 0;

							r->state = ROTATE;
							
							if(r->speed_limit == 0)
								r->speed_limit = MAXSPEED; //If speed limit not set, set it to MAXSPEED

							if(r->angle == 0)
								r->state = ROTATE_END; //If angle not set, break execution

							foutf(&str_debugDrive, "%i: drRot:st: %i\n\r", timer, r->um6_psi_t_start);
							
						break;
						
		case ROTATE: 
		
							if(r->angle > 0)
								r->steer = ((r->um6_psi_t_start + r->angle + UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;
							else
								r->steer = ((r->um6_psi_t_start + r->angle - UM6_ROTATE_OFFSET) - um6.psi_t) * KP_ROTATE;
							
							r->progress = abs((um6.psi_t - r->um6_psi_t_start)*100)/abs(r->angle);

							if(check_um6 != 0)
							{
								r->use_enc = 1;
							}

							if(r->use_enc)
							{
								if(r->angle > 0)
								{
									r->progress = abs(((mot.d[LEFT].enc - r->enc_l_start)*100)/abs(r->angle * ENC_DEGROTFAC));

									if((mot.d[LEFT].enc < (r->enc_l_start + (ENC_DEGROTFAC * r->angle))) ||
										 (mot.d[RIGHT].enc > (r->enc_r_start + (ENC_DEGROTFAC * r->angle))))
									{
										r->steer = STEER_ROTATE_ENC;
									}
									else
									{
										r->state = ROTATE_END;
										r->steer = 0;
									}
								}
								else
								{
									r->progress = abs(((mot.d[LEFT].enc - r->enc_l_start)*100)/abs(r->angle * ENC_DEGROTFAC));

									if((mot.d[LEFT].enc > (r->enc_l_start + (ENC_DEGROTFAC * r->angle))) ||
										 (mot.d[RIGHT].enc < (r->enc_r_start + (ENC_DEGROTFAC * r->angle))))
									{
										r->steer = -STEER_ROTATE_ENC;
									}
									else
									{
										r->state = ROTATE_END;
										r->steer = 0;
									}
								}
							}
							else if(r->steer == 0)
							{
								r->state = ROTATE_END;
							}

							if((abs(r->steer) > STEER_ROTATE_TH_TIMER) && (r->timer != 0)) //If we start a new turn with another turn object and the timer has already startet, we reset the timer if we stand on another point
							{
								r->timer = 0;
							}
							else if((abs(r->steer) < STEER_ROTATE_TH_TIMER) && (r->timer == 0))
							{
								r->timer = timer;
							}
							else if((((timer - r->timer) > TIMER_ROTATE) ||
									(r->steer == 0)) && (r->timer != 0))
							{
								r->state = ROTATE_END;
							}

							mot.d[LEFT].speed.to = r->steer;
							mot.d[RIGHT].speed.to = -r->steer;
							
							drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, r->speed_limit);

						break;
						
		case ROTATE_END:
		
							mot.d[LEFT].speed.to = 0;
							mot.d[RIGHT].speed.to = 0;

							r->state = ROTATE_FINISHED;

							foutf(&str_debugDrive, "%i: drRot:end\n\r", timer);

		case ROTATE_FINISHED:

						break;

						
		default:		foutf(&str_error, "%i: ERR:sw[drv.02]:DEF\n\r", timer);
						fatal_err = 1;
						r->state = ROTATE_INIT;
						break;
	}
}

//////////////////////////////////////////////////////////
int16_t steer_turn = 0;


uint8_t sm_driveAlign = 0;

uint32_t timer_align;

uint8_t drive_align(void)
{
	uint8_t returnvar = 1;
	
	switch(sm_driveAlign)
	{
		case 0:
						steer_turn = 0;
						timer_align = timer;
						
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
						
						if((abs(steer_turn) <= STEER_ALIGN_DONE) || (timer - timer_align > TIMER_ALIGN))
							sm_driveAlign = 2;
							
					break;
					
		case 2:			steer_turn = 0;
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

uint32_t timer_alignBack = 0;

uint8_t drive_align_back(uint8_t dist_to) //Distance to the back
{
	uint8_t returnvar = 1;

	int16_t dist_back; //any of the distance sensor in the back
	if(dist[LIN][BACK][BACK] != DIST_MAX_SRP_NEW)
		dist_back = dist[LIN][BACK][BACK];
	else if(dist[LIN][BACK][LEFT] != DIST_MAX_SRP_OLD)
		dist_back = dist[LIN][BACK][LEFT];
	else if(dist[LIN][BACK][RIGHT] != DIST_MAX_SRP_OLD)
		dist_back = dist[LIN][BACK][RIGHT];
	else
		dist_back = DIST_MAX_SRP_NEW;

	switch(sm_dab)
	{
		case 0:

				if(dist_back < TILE1_BACK_TH_BACK)
				{
					timer_alignBack = timer;
					sm_dab = 1;
				}
				else
					sm_dab = 2;

			break;

		case 1:


				steer_dab = ((dist_to - dist_back) * KP_ALIGN_BACK);

				if((abs(steer_dab) <= STEER_ALIGN_BACK_END) || ((timer - timer_alignBack) > TIMER_ALIGN_BACK) ||
					(dist_back > TILE1_BACK_TH_BACK))
				{
					mot.d[LEFT].speed.to = 0;
					mot.d[RIGHT].speed.to = 0;
					sm_dab = 2;
				}
				else
				{
					mot.d[LEFT].speed.to = steer_dab;
					mot.d[RIGHT].speed.to = steer_dab;

					drive_limitSpeed(&mot.d[LEFT].speed.to, &mot.d[RIGHT].speed.to, MAXSPEED);
				}

			break;

		case 2:

				sm_dab = 0;
				returnvar = 0;

	}

	return returnvar;
}

/////////////////////////////////////////

void drive_turn(D_TURN *t)
{
	switch(t->state)
	{
		case TURN_INIT:
		
						t->state = TURN;
						
		case TURN:
						drive_rotate(&t->r);

						if(t->r.state == ROTATE_FINISHED)
						{
							t->r.state = ROTATE_INIT; //Prepare for next rotate

							if(!t->no_align) //Align to walls?
								t->state = TURN_ALIGN;
							else
								t->state = TURN_END;
							
							foutf(&str_debugDrive, "%i: drTrn\n\r", timer);
						}

					break;
					
		case TURN_ALIGN:
		
						//GGf. Ausrichtung an Wand
						if(!drive_align())
						{
							t->state = TURN_ALIGN_BACK;

							foutf(&str_debugDrive, "%i: drTrn:algn\n\r", timer);
						}

					break;

		case TURN_ALIGN_BACK:

							if(!drive_align_back(TILE1_BACK_BACK))
							{
								t->state = TURN_END;
							}

					break;
					
		case TURN_END:
		
						t->state = TURN_FINISHED;

		case TURN_FINISHED:

						foutf(&str_debugDrive, "%i: drTrn:end\n\r", timer);

						break;

		default:		foutf(&str_error, "%i: ERR:sw[drv.03]:DEF\n\r", timer);
						fatal_err = 1;
						t->state = TURN_INIT;
						break;
	}
}

uint8_t drive_instr_sm = 0;

uint8_t drive_instructions(char *instructions, uint8_t amount)
{
	if(drive_instr_sm < amount)
	{
		switch (instructions[drive_instr_sm]) {
//		case 'f':	if(!drive_oneTile(0))	drive_instr_sm ++; break;
//		case 'l':	if(!drive_turn(-90,1))	drive_instr_sm ++; break;
//		case 'r':	if(!drive_turn(90, 1))	drive_instr_sm ++; break;
//		case 'd':	if(!drive_getBall())	drive_instr_sm ++;	break;
//		case 'u':	if(!drive_releaseBall())	drive_instr_sm ++;	break;
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
#define DIST_SOLL_RAMP 70 //mm, Sollabstand Rampe

#define KP_RAMP_DIR 0.9
#define KP_RAMP_DIST KP_RAMP_DIR * 0.5

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

	//displayvar[4] = ramp_checkpoint;
	//displayvar[5] = ((mot.enc - ramp_checkpoint_enc)/ENC_FAC_CM_LR);

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
		case 0: //if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_ramp()"));}
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
							//if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": drive_ramp()::done::speed:")); bt_putLong(speed);}
						}
					break;
		default:		foutf(&str_error, "%i: ERR:sw[drv.04]:DEF\n\r", timer);
						fatal_err = 1;
						returnvar = 0;
							sm_ramp = 0;
						break;
	}
	return returnvar;
}


/////////////////////////////////////////////////////

void drive_deployResKit(D_DEPLOYKIT *dk)
{
	switch(dk->state)
	{
		case DK_INIT:
				if(dk->amount_to == 0)
				{
					dk->state = DK_END;
					break;
				}

				dk->turn.no_align = 1; //Don`t align on this turn! We want to rotate exactly 90°!
				dk->amount_is = 0;

				dk->alignedToBackwall = 0;

				if(dk->config_dir == LEFT) //Turn 90° left...
				{
					dk->turn.r.angle = 90;
				}
				else //...or right
				{
					dk->turn.r.angle = -90;
				}

				dk->turn.state = TURN_INIT;
				dk->state = DK_TURN_A;

		case DK_TURN_A:

				drive_turn(&dk->turn);

				if(dk->turn.state == TURN_FINISHED)
				{
					dk->turn.state = TURN_INIT;

					dk->state = DK_ALIGN_A;
				}
			break;

		case DK_ALIGN_A:

				if(!drive_align_back(35))
				{
					dk->alignedToBackwall = 1;
					dk->state = DK_DEPL;
				}

			break;

		case DK_DEPL:

				if(dk->amount_is < dk->amount_to)
				{
					if(!rescueKit_drop(2))
					{
						dk->amount_is++;
					}
				}
				else
				{
					if(dk->alignedToBackwall)
					{
						dk->state = DK_ALIGN_B;
					}
					else
					{
						dk->state = DK_CHECK_TURN;
					}
				}

			break;

		case DK_ALIGN_B:

				if(!drive_align_back(TILE1_BACK_BACK))
				{
					dk->state = DK_CHECK_TURN;
				}

			break;

		case DK_CHECK_TURN:

				if(dk->config_turnBack) //Only if we want to turn back
				{
					dk->state = DK_TURN_B;

					if(dk->config_dir == LEFT) //Turn back
					{
						dk->turn.r.angle = -90;
					}
					else
					{
						dk->turn.r.angle = 90;
					}
				}
				else
				{
					dk->state = DK_END;
				}
			break;

		case DK_TURN_B:

				drive_turn(&dk->turn);

				if(dk->turn.state == TURN_FINISHED)
				{
					dk->turn.state = TURN_INIT;

					dk->state = DK_END;
				}
			break;

		case DK_END:
			dk->state = DK_FINISHED;

		case DK_FINISHED:
			break;

	}
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

							foutf(&str_debugDrive, "%i: drLR\n\r", timer);
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
		case 4:				if(left)
								width_var *= DRIVE_LEFT_FAC;
								
							if(!(drive_dist(-1, speed, width_var)))
							{
								if(left)
									sm_d_lr = 5;
								else
									sm_d_lr = 3;
							}
						break;
		case 5:			sm_d_lr = 0;
						returnvar = 0;

						foutf(&str_debugDrive, "%i: drLR:end\n\r", timer);
						break;
		default:		foutf(&str_error, "%i: ERR:sw[drv.05]:DEF\n\r", timer);
						fatal_err = 1;
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

					foutf(&str_debugDrive, "%i: drDst\n\r", timer);
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

					foutf(&str_debugDrive, "%i: drDst:end\n\r", timer);
					break;
		default:	foutf(&str_error, "%i: ERR:sw[drv.06]:DEF\n\r", timer);
					fatal_err = 1;
					returnvar = 0;
					break;
	}
	return returnvar;
}

////////////////////////////////////////////////////
//Reset all non-object-orientated driving functions
void drive_reset(void)
{
	sm_driveAlign = 0;
	sm_dab = 0;
	sm_ramp = 0;
	sm_d_lr = 0;
	sm_ddist = 0;

	foutf(&str_debugDrive, "%i: rstDrv\n\r", timer);
}
