////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
//////////////////////////////////maze.c////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//	Alles, was mit Kartierung und Navigation zu tun hat:
//	- Entscheidung (Statemachine), wie der Roboter jetzt fährt
//		- ggf. Tarry Algorithmus oder rechte Hand Regel
//	- Kartierung
//	- Anzeige der Karte und bzgl. Informationen
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "main.h"
#include "maze.h"
#include "mazefunctions.h"
#include "bluetooth.h"
#include "drive.h"
#include "system.h"
#include "funktionen.h"
#include "display.h"
#include "tsl1401.h"
#include "um6.h"
#include "victim.h"
#include "i2cdev.h"

TILE maze[MAZE_SIZE_X][MAZE_SIZE_Y][MAZE_SIZE_Z];
OFF offset[MAZE_SIZE_Z]; //Für jede Ebene eigenen Offset
	int8_t offset_z; //Eigenen Offset für z, da sonst in beiden Offset [0,1] vorhanden wäre
COORD rr_result; //RouteRequest result positions
COORD off_start; //offset startposition (change startposition via rotary-encoder)
POS robot; //Positionsdaten über Roboter
POS robot_save;
	int8_t rob_z_lost; //Stage in which the robot lost his position
POS checkpoint;
POS ramp[MAZE_SIZE_Z];		//Rampenanschlüsse

/////////////////////////////////////////////////////////////////


uint8_t maze_solve_state_path = DRIVE_READY;
uint8_t maze_solve_state_ready = 0;
uint8_t maze_solve_depl_kit = FALSE; //Deploy Kit after turn?
uint8_t maze_solve_check_blacktile = 0;
uint8_t rt_noposs_radius = 0;

int16_t tsl_th; //Schwellwert Sackgasse; Wird aus EEPROM gelesen
int16_t tsl_th_ground;
int16_t ground_th;

////////////////////////////////////////////////////////////////////////////////
// Main solving function, responsible for the decision of what will happen next,
// where the robot is going to drive, if he calculates a path etc...
//
////////////////////////////////////////////////////////////////////////////////

uint8_t incr_ok_mode = 4; //Positions- oder Richtungswahl? Siehe auch unten

int16_t ramp_start; //value of the rampsensor in the 2nd half of drive one tile
int16_t ramp_stop;
int16_t groundsens_cnt;

int8_t dot_abort = 0; //Abbrechen der Geradeausfahrt?
	//Bit 0: abgebrochen wegen zu später Sackgassenerkennung
	//Bit 1: abgebrochen wegen Hindernis (noch nicht implementiert)

int8_t dot_driven_oneTile = 0;

MATCHINGWALLS matchingWalls;
TILE cleartiles;

uint8_t maze_solve(void) //called from RIOS periodical task
{		
	uint8_t returnvar = 1;
	
	if((timer_rdy_restart == 0) && !mot.off) //Neustart
	{
		timer_rdy_restart = -1;
		maze_init();
	}

	if((!mot.off) && (incr_ok_mode == 4)) //If the robot does not moves, there is no input and no victim identification
	{
		uint8_t depthsearchNum = 0;
		uint8_t driveVar = 0;

		//////////////////////////////LOP//////////////////////////////////////

		if((maze_solve_state_path != LOP_INIT) && (maze_solve_state_path != LOP_WAIT))
		{
			if(dist_down < GROUNDDIST_TH_LOP) //Robot lifted up -> Lack of Progress
			{
				if(timer_lop == -1)
					timer_lop = TIMER_LOP_SENSOR;

				timer_rdy_restart = -1;
			}
			else
			{
				timer_lop = -1;
			}

			if(timer_lop == 0)
			{
				maze_solve_state_path = LOP_INIT;
			}
		}

		//////////////////////Statemachine///////////////////////////////////////

		switch(maze_solve_state_path)
		{
			case DRIVE_READY:
									switch(maze_solve_state_ready)
									{
										case DR_INIT:

													mot.d[LEFT].speed.to = 0;
													mot.d[RIGHT].speed.to = 0;

													robot_save.dir = robot.dir; //Has to be robot.dir to compare it with robot.pos!!!
													robot_save.pos.x = MAZE_SIZE_X_USABLE-1; //Upper right tile, the size  has to be dimensioned so that the robot will never reach this tile!
													robot_save.pos.y = MAZE_SIZE_Y_USABLE-1;
													robot_save.pos.z = MAZE_SAVESTAGE;

													maze_solve_state_ready = DR_UPDATEWALLS;

										case DR_UPDATEWALLS:

                                                    if(maze_updateWalls()) //Save the walls around the robot in the MAZE_SAVESTAGE to compare it later with the environment
                                                    {
														maze_solve_state_ready = DR_UPDATEVICTIMS;
                                                    }

												break;

										case DR_UPDATEVICTIMS:

                                                    //if(victim_check())
                                                    //{
														maze_solve_state_ready = DR_CHECK;
                                                    //}

												//break;

										case DR_CHECK:

													matchingWalls.buffer[matchingWalls.next] = 0;
													for(uint8_t dir = NORTH; dir <= WEST; dir++)
													{
														if(((maze_getWall(&robot.pos, dir) >= 0) && (maze_getWall(&robot_save.pos, dir) >= 0)) ||
															 ((maze_getWall(&robot.pos, dir) <= 0) && (maze_getWall(&robot_save.pos, dir) <= 0)))
														{
															matchingWalls.buffer[matchingWalls.next] ++;
														}
													}

													//displayvar[0] = matchingWalls.buffer[matchingWalls.next];

													matchingWalls.lowmatches = 0;
													for(uint8_t i = 0; i < MATCHINGWALLS_BUFF; i++)
													{
														if((matchingWalls.buffer[i] > 0) &&
																(matchingWalls.buffer[i] < 4))
															matchingWalls.lowmatches ++;
													}

													//displayvar[1] = matchingWalls.lowmatches;

													if(1)//(matchingWalls.buffer[matchingWalls.next] > 2) &&  //The last buffer entry shows us enough matches
													   //(matchingWalls.lowmatches < 4)) //In the last few tiles were not too many unsure mathces
													{
														maze_corrTile(&robot_save.pos, &robot.pos); //Copy the information of the savetile into the robots tile

														for(uint8_t dir = NORTH; dir <= WEST; dir++)
															maze_corrWall(&robot_save.pos, dir, -maze_getWall(&robot_save.pos, dir)); //and delete the information on the savetile

														maze_setBeenthere(&robot.pos, NONE, TRUE);

														if(((!maze_getBeenthere(&robot.pos, NORTH)) && maze_tileIsVisitable(&robot.pos, NORTH)) ||
															 ((!maze_getBeenthere(&robot.pos, EAST)) && maze_tileIsVisitable(&robot.pos, EAST)) ||
															 ((!maze_getBeenthere(&robot.pos, SOUTH)) && maze_tileIsVisitable(&robot.pos, SOUTH)) ||
															 ((!maze_getBeenthere(&robot.pos, WEST)) && maze_tileIsVisitable(&robot.pos, WEST)))
														{
															maze_clearDepthsearch();
															maze_solve_state_path = FOLLOW_RIGHTWALL; //When the program came into RESTART by RR_RTNOPOSS and there is suddenly an option (Wall wrong detected?) proceed!
														}
														else
														{
															maze_solve_state_path = FOLLOW_DFS; //Statemachine automatically jumps into FOLLOW_RIGHTWALL when there is no path possibility
														}
													}
													else
													{
														for(uint8_t i = 0; i < MATCHINGWALLS_BUFF; i++)
														{
															matchingWalls.buffer[i] = 0;
															matchingWalls.next = 0;
															matchingWalls.lowmatches = 0;
														}

														//maze_clearStage(MAZE_SAVESTAGE);

													/*	COORD c = robot.pos;
														uint8_t maze_err_del_radius = 0;
														uint8_t stage_visitedTiles = maze_stageGetVisitedTiles(robot.pos.z);
														if(stage_visitedTiles > 15)
															maze_err_del_radius = MAZE_ERR_DEL_RADIUS_S;
														else if(stage_visitedTiles > 25)
															maze_err_del_radius = MAZE_ERR_DEL_RADIUS_L;
														for(c.y = robot.pos.y-maze_err_del_radius; c.y <= robot.pos.y+maze_err_del_radius; c.y++)
														{
															for(c.x = robot.pos.x-maze_err_del_radius; c.x <= robot.pos.x+maze_err_del_radius; c.x++)
															{
																if((c.x >= ROB_POS_X_MIN) && (c.x <= MAZE_SIZE_X_USABLE) &&
																   (c.y >= ROB_POS_Y_MIN) && (c.y <= MAZE_SIZE_Y_USABLE))
																		maze_clearTile(&c);
															}
														}*/

														rob_z_lost = robot.pos.z;

														robot = robot_save;
														robot.pos.x = ROB_POS_X_MIN;
														robot.pos.y = ROB_POS_Y_MIN;

														maze_solve_state_path = DRIVE_NEUTRPOS;
													}

													matchingWalls.next ++; //FIFO
													if(matchingWalls.next >= MATCHINGWALLS_BUFF)
														matchingWalls.next = 0;

													if(robot.pos.z == MAZE_SAVESTAGE)
													{
														locRequest = LR_MATCH;
														maze_solve_state_ready = DR_MATCH;
													}
													else
													{
														maze_solve_state_ready = DR_INIT;
													}
												break;
										case DR_MATCH: //maze_solve_state_ready cleared in LocateRequest
												break;
										default:
												break;
									}

								break;
						
			case FOLLOW_RIGHTWALL:
							
									if(maze_tileIsVisitable(&robot.pos, robot.dir+1) &&
					 					(!maze_getBeenthere(&robot.pos, robot.dir+1)))
									{
										maze_solve_state_path = TURN_RIGHT;
									}
									else if(maze_tileIsVisitable(&robot.pos, robot.dir) &&
					 							 (!maze_getBeenthere(&robot.pos, robot.dir)))
									{
										maze_solve_state_path = DRIVE_DOT;
									}
									else if(maze_tileIsVisitable(&robot.pos, robot.dir+3) &&
					 							 (!maze_getBeenthere(&robot.pos, robot.dir+3)))
									{
										maze_solve_state_path = TURN_LEFT;
									}
									else if(maze_tileIsVisitable(&robot.pos, robot.dir+2) &&
					 							 (!maze_getBeenthere(&robot.pos, robot.dir+2)))
									{
										maze_solve_state_path = TURN_LEFT;
									}
									else //No more possibilitys! Calculate Route to nearest, unvisited tile.
									{
										switch(routeRequest)
										{
											case RR_WAIT:					routeRequest = RR_CALCNEARESTTILE;	break;
											case RR_CALCNEARESTTILE:											break; //Has to be checked in maze_solveRoute()
											case RR_CALCROUTE:													break; //"
						
											case RR_RTTIMEOUT:													break; //To do... Restart?
											case RR_NEARTIMEOUT:												break; //"

											case RR_NEARDONE:
											case RR_RTDONE:					rt_noposs_radius = 0;
																			routeRequest = RR_WAIT;
																			maze_solve_state_path = FOLLOW_DFS;
																											break; //Drive via path
											case RR_NEARNOPOSS: //No unvisited tiles in the maze
						
																			rr_result = *maze_getStartPos(); //Request start position, save it in the vars for the route calculation

																			if(!maze_cmpCoords(&robot.pos, &rr_result))
																			{
																				routeRequest = RR_CALCROUTE;
																			}
																			else
																			{
																				maze_solve_state_path = RESTART; //Robot is at start Position -> Restart
																			}
									
																		break;
						
											case RR_RTNOPOSS:	//That’s bad. The robot detected a wall wrong and can’t find the way or someone put the walls from their places. ;)

																			cleartiles.beenthere = 0;
																			cleartiles.depthsearch = 1;
																			cleartiles.ground = 1;
																			cleartiles.wall_s = 1;
																			cleartiles.wall_w = 1;

																			COORD c = robot.pos;

																			rt_noposs_radius = 1;

																			if((rt_noposs_radius < (MAZE_SIZE_X_USABLE)/2) &&
																				(rt_noposs_radius < (MAZE_SIZE_Y_USABLE)/2))
																			{
																				for(c.y = robot.pos.y-rt_noposs_radius; c.y <= robot.pos.y+rt_noposs_radius; c.y++)
																				{
																					for(c.x = robot.pos.x-rt_noposs_radius; c.x <= robot.pos.x+rt_noposs_radius; c.x++)
																					{
																						if((c.x >= ROB_POS_X_MIN) && (c.x <= MAZE_SIZE_X_USABLE) &&
																						   (c.y >= ROB_POS_Y_MIN) && (c.y <= MAZE_SIZE_Y_USABLE))
																								maze_clearTile(&c, &cleartiles);
																					}
																				}

																				rt_noposs_radius ++;
																			}
																			else
																			{
																				maze_clear(&cleartiles);
																			}

																			maze_solve_state_path = FOLLOW_RIGHTWALL;
																			routeRequest = RR_NEARNOPOSS;

																		break;
											default:
																if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.03]:DEFAULT_CASE"));}
																fatal_err = 1;
										}
									}
				
						break;
				
			case FOLLOW_DFS:
	
									depthsearchNum = maze_getDepthsearch(&robot.pos, NONE);

									if((maze_getDepthsearch(&robot.pos, robot.dir) < depthsearchNum) &&
													 maze_tileIsVisitable(&robot.pos, robot.dir))
									{
										maze_solve_state_path = DRIVE_DOT; //Geradeaus
									}
									else if((maze_getDepthsearch(&robot.pos, robot.dir+1) < depthsearchNum) &&
											maze_tileIsVisitable(&robot.pos, robot.dir+1))
									{
										bt_putLong(maze_getDepthsearch(&robot.pos, robot.dir+1));
										maze_solve_state_path = TURN_RIGHT;
									}
									else if((maze_getDepthsearch(&robot.pos, robot.dir+3) < depthsearchNum) &&
													 maze_tileIsVisitable(&robot.pos, robot.dir+3))
									{
										maze_solve_state_path = TURN_LEFT;
									}
									else if((maze_getDepthsearch(&robot.pos, robot.dir+2) < depthsearchNum) &&
													 maze_tileIsVisitable(&robot.pos, robot.dir+2))
									{
										maze_solve_state_path = TURN_LEFT;
									}
									else
									{
										maze_clearDepthsearch();
										maze_solve_state_path = FOLLOW_RIGHTWALL;
									}
							
								break;
								
			case MAZE_ERR:
									
									mot.d[LEFT].speed.to = 0;
									mot.d[RIGHT].speed.to = 0;
									
								break;
								
			case RESTART:
							
									mot.d[LEFT].speed.to = 0;
									mot.d[RIGHT].speed.to = 0;

									returnvar = 0; //0 zurückgeben, wenn fertig, sonst 1

									if(timer_rdy_restart == -1)
										timer_rdy_restart = TIMER_RDY_RESTART; //Setze Timer

									depthsearchNum = maze_getDepthsearch(&robot.pos, NONE);
									for(uint8_t dir = NORTH; dir <= WEST; dir++) //check for each direction
									{
										if(((maze_getDepthsearch(&robot.pos, dir) < depthsearchNum) ||
												(!maze_getBeenthere(&robot.pos, dir))) &&
												maze_tileIsVisitable(&robot.pos, dir))
										{
											maze_clearDepthsearch();
											timer_rdy_restart = -1;
											maze_solve_state_path = FOLLOW_RIGHTWALL; //When the program came into RESTART by RR_RTNOPOSS and there is suddenly an option (Wall wrong detected?) proceed!
										}
									}

									maze_updateWalls();

									if(debug > 0){bt_putStr_P(PSTR("\r")); bt_putLong(timer); bt_putStr_P(PSTR(": DONE::restart:")); bt_putLong(timer_rdy_restart*25); bt_putStr_P(PSTR("ms"));}
						
								break;

        case LOP_INIT:

                                drive_reset(); //Reset Drive Functions...
                                maze_clearDepthsearch();

                                robot.pos = *maze_getCheckpoint(&robot.pos);

                                mot.off_invisible = 1;

                                wall_size_part = WALL_SIZE_MEDIUM; //Detailed view of the tile

                                maze_solve_state_path = LOP_WAIT;

        case LOP_WAIT:
                                if(dist_down > GROUNDDIST_TH_NORMAL)
                                {
                                    if(timer_lop == -1)
                                    {
                                        timer_lop = TIMER_LOP_RESET;
                                    }
                                    else if(timer_lop == 0)
                                    {
                                        mot.off_invisible = 0;
                                        wall_size_part = WALL_SIZE_STD;
                                        timer_lop = -1;
                                        maze_solve_state_path = DRIVE_READY;
                                    }
                                }

                            break;
			///////////////////////////////////////////////////////////////////////////////
			///////////////////////Drivefunctions//////////////////////////////////////////
			///////////////////////////////////////////////////////////////////////////////
			
			case DRIVE_DOT:
									
									////////////////
									if((maze_getRampPosDirAtDir(&robot.pos, NONE) == robot.dir) &&
										 (robot.pos.z == 0))
									{
										maze_solve_state_path = RAMP_UP; //Rampe hoch
									}
									else if((maze_getRampPosDirAtDir(&robot.pos, NONE) == robot.dir) &&
													(robot.pos.z == 1))
									{
										maze_solve_state_path = RAMP_DOWN; //Rampe runter
									}
									else
									{
										driveVar = drive_oneTile(dot_abort);
									
										if((driveVar == DOT_RET_INIT) && !dot_abort)
										{
											dot_driven_oneTile = 0;

											groundsens_cnt = 0;
											ramp_start = um6.theta_t;
										}
										else if(driveVar == DOT_RET_ALIGN)
										{}
										else if((driveVar == AB_15) || (driveVar == UD_15)) //Driving... Dont change! See rotations in drive_oneTile...
										{
                                            if((groundsens_r > GROUNDSENS_R_TH_BLACKTILE) && (groundsens_l > GROUNDSENS_L_TH_BLACKTILE) && (dot_abort == 0))
											{
												maze_solve_state_path = CHECK_BLACKTILE;
											}
											//else if(dot_abort != 1)
											//	maze_corrGround(&robot.pos, robot.dir, -1);

											if((groundsens_l < GROUNDSENS_L_TH_CHECKPOINT) && (driveVar == AB_15))
                                                groundsens_cnt ++;
										}
										else if((driveVar == NOW_15) || (driveVar == AT_15)) //Mitte beim Geradeausfahren (Übergang 2 Fliesen)
										{
											if(driveVar == NOW_15)
											{
												dot_driven_oneTile ++;

												switch(robot.dir)
												{
													case NORTH:	robot.pos.y++;	break;
													case EAST:	robot.pos.x++;	break;
													case SOUTH:	robot.pos.y--;	break;
													case WEST:	robot.pos.x--;	break;
													default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.02]:DEFAULT_CASE"));}
																		fatal_err = 1;
												}
											}
											else //AT_15 (Rückgängig machen)
											{
												dot_driven_oneTile --;

												switch(robot.dir)
												{
													case NORTH:	robot.pos.y--;	break;
													case EAST:	robot.pos.x--;	break;
													case SOUTH:	robot.pos.y++;	break;
													case WEST:	robot.pos.x++;	break;
													default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.02]:DEFAULT_CASE"));}
																		fatal_err = 1;
												}
											}

											if(robot.pos.x < ROB_POS_X_MIN)
											{
												maze_chgOffset(X, robot.pos.z, -1);
												robot.pos.x ++;
											}
											else if(robot.pos.x >= (MAZE_SIZE_X-1))
											{
												robot.pos.x = (MAZE_SIZE_X-2);
												maze_solve_state_path = RESTART;

												if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::robot.pos.x:MEMORY_TOO_SMALL:RESTART"));}
											}

											if(robot.pos.y < ROB_POS_Y_MIN)
											{
												maze_chgOffset(Y, robot.pos.z, -1);
												robot.pos.y ++;
											}
											else if(robot.pos.y >= (MAZE_SIZE_Y-1))
											{
												robot.pos.y = (MAZE_SIZE_Y-2);
												maze_solve_state_path = RESTART;

												if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::robot.pos.y:MEMORY_TOO_SMALL:RESTART"));}
											}
										}
										else if(driveVar == DOT_RET_READY) // Ende geradeaus fahren (eine Fliese gefahren)
										{
											if(!dot_abort)
											{
												//////////////////////////Checkpoint///////////////////

                                                if(groundsens_cnt > GROUNDSENS_CNT_TH_CHECKPOINT) //Checkpoint detected
												{
													if(dot_driven_oneTile == 1)
														maze_setCheckpoint(&robot.pos, NONE);
                                                }

												/////////////////Rampe/////////////////////////
                                                ramp_stop = ramp_start - um6.theta_t;

												if((maze_getRampPosDirAtDir(&robot.pos, NONE) == robot.dir) &&
													 (robot.pos.z == 0))
												{
													if(abs(ramp_stop) < RAMP_DIFF_TH)
													{
														maze_setRamp(&robot.pos, NONE, NONE, FALSE); //Delete Ramp
													}
													else
													{
														maze_solve_state_path = RAMP_UP;
													}
												}
												else if((maze_getRampPosDirAtDir(&robot.pos, NONE) == robot.dir) &&
														(robot.pos.z == 1))
												{
													if(abs(ramp_stop) < RAMP_DIFF_TH)
													{
														maze_setRamp(&robot.pos, NONE, NONE, FALSE); //Delete Ramp
													}
													else
													{
														maze_solve_state_path = RAMP_DOWN;
													}
												}
												else if((maze_getRampDir(0) == NONE) &&
																(ramp_stop > RAMP_DIFF_TH)) //Rampe hoch
												{
													if((groundsens_r < GROUNDSENS_R_TH_BLACKTILE)) //No black tile
														maze_setRamp(&robot.pos, robot.dir, NONE, TRUE);

													maze_solve_state_path = RAMP_UP;
												}
												else if((maze_getRampDir(1) == NONE) &&
																(ramp_stop < -RAMP_DIFF_TH)) //Rampe runter
												{
													maze_setRamp(&robot.pos, robot.dir, NONE, TRUE);

													maze_solve_state_path = RAMP_DOWN;
                                                }
											}

											dot_driven_oneTile = 0;
											dot_abort = 0;

											if(maze_solve_state_path == DRIVE_DOT) //Only, when there is no RESTART and no RAMP_xx or something other than DRIVE_DOT
												maze_solve_state_path = DRIVE_READY;
										}
									}
									
								break;
								
            case TURN_RIGHT:
                                            //Rechts drehen
                                            if(drive_turn(90, 1) == 0)
                                            {
                                                robot.dir = maze_alignDir(robot.dir + 1);

                                                if(maze_solve_depl_kit)
                                                    maze_solve_state_path = VIC_DEPL;
                                                else
                                                    maze_solve_state_path = DRIVE_READY;
                                            }

                                        break;

            case TURN_LEFT:

                                            if(drive_turn(-90, 1) == 0)
                                            {
                                                robot.dir = maze_alignDir(robot.dir + 3);

                                                if(maze_solve_depl_kit)
                                                    maze_solve_state_path = VIC_DEPL;
                                                else
                                                    maze_solve_state_path = DRIVE_READY;
                                            }

                                        break;
								
            case RAMP_UP:
                                if(drive_ramp(RAMP_UP_SPEED) == 0)
                                {
                                    robot.pos.z ++; //normalerweise muss z jetzt 1 sein, da er die Rampe hochgefahren ist und somit unten gestartet sein muss.

                                    if(maze_getRampDir(robot.pos.z) == NONE) //Rampe oben noch nicht gesetzt
                                    {
                                        robot.pos.x = ROB_POS_X_MIN;
                                        robot.pos.y = ROB_POS_Y_MIN;

                                        maze_setBeenthere(&robot.pos,maze_alignDir(robot.dir + 2),TRUE);
                                        maze_setRamp(&robot.pos, maze_alignDir(robot.dir + 2), maze_alignDir(robot.dir + 2), TRUE);
                                    }
                                    else
                                    {
                                        robot.pos = *maze_getRamp(robot.pos.z);

                                        switch(maze_getRampDir(robot.pos.z))
                                        {
                                            case NORTH:	robot.pos.y--;	break;
                                            case EAST:	robot.pos.x--;	break;
                                            case SOUTH:	robot.pos.y++;	break;
                                            case WEST:	robot.pos.x++;	break;
                                            default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.02]:DEFAULT_CASE"));}
                                                                fatal_err = 1;
                                        }
                                    }

                                    maze_solve_state_path = DRIVE_READY;
                                }

                            break;

        case RAMP_DOWN:

                                if(drive_ramp(-RAMP_DOWN_SPEED) == 0) //Herunterfahren
                                {
                                    robot.pos.z --; //Muss jetzt 0 sein, wurde schon oben angepasst

                                    if(robot.pos.z < 0)
                                    {
                                        maze_chgOffset(Z, NONE, -1);
                                        robot.pos.z = 0;
                                    }

                                    if(maze_getRampDir(robot.pos.z) == NONE) //Rampe unten noch nicht gesetzt
                                    {
                                        robot.pos.x = ROB_POS_X_MIN;
                                        robot.pos.y = ROB_POS_Y_MIN;

                                        maze_setBeenthere(&robot.pos,maze_alignDir(robot.dir + 2),TRUE);
                                        maze_setRamp(&robot.pos, maze_alignDir(robot.dir + 2), maze_alignDir(robot.dir + 2), TRUE);
                                    }
                                    else
                                    {
                                        robot.pos = *maze_getRamp(robot.pos.z);

                                        switch(maze_getRampDir(robot.pos.z))
                                        {
                                            case NORTH:	robot.pos.y--;	break;
                                            case EAST:	robot.pos.x--;	break;
                                            case SOUTH:	robot.pos.y++;	break;
                                            case WEST:	robot.pos.x++;	break;
                                            default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.02]:DEFAULT_CASE"));}
                                                                fatal_err = 1;
                                        }
                                    }

                                    maze_solve_state_path = DRIVE_READY;
                                }

                            break;

			case DRIVE_NEUTRPOS:
									
									if(!drive_neutralPos())
									{
										maze_solve_state_path = DRIVE_READY;
									}
									
								break;

			case VIC_DEPL_L:

									if(!drive_deployResKit(LEFT, 1))
									{
										timer_victim_led = -1;

										if(maze_solve_depl_kit == 0)
											maze_solve_state_path = DRIVE_DOT;
										else
										{
											maze_solve_state_path = DRIVE_READY;
											maze_solve_depl_kit = 0;
										}
									}
								break;
			case VIC_DEPL_R:

									//maze_solve_state_path - VIC_DEPL_L == LEFT for VIC_DEPL_L, otherwise RIGHT

									if(!drive_deployResKit(RIGHT, 1))
									{
										timer_victim_led = -1;
										if(maze_solve_depl_kit == 0)
											maze_solve_state_path = DRIVE_DOT;
										else
										{
											maze_solve_state_path = DRIVE_READY;
											maze_solve_depl_kit = 0;
										}
									}

								break;

			case VIC_DEPL:

									if(!rescueKit_drop(2))
									{
										maze_solve_depl_kit = FALSE;
										timer_victim_led = -1;

										maze_solve_state_path = DRIVE_READY;
									}

								break;

			case CHECK_BLACKTILE:

									switch(maze_solve_check_blacktile)
									{
										case 0:

												if(!drive_dist(0,20,3))
												{
													if((groundsens_r > GROUNDSENS_R_TH_BLACKTILE) && (groundsens_l > GROUNDSENS_L_TH_BLACKTILE))
													{
														maze_solve_check_blacktile = 1;
													}
													else
													{
														maze_solve_check_blacktile = 3;
													}
												}

											break;

										case 1:

												if(!drive_dist(0,20,3))
												{
													if((groundsens_r > GROUNDSENS_R_TH_BLACKTILE) && (groundsens_l > GROUNDSENS_L_TH_BLACKTILE))
													{
														maze_solve_check_blacktile = 2;
													}
													else
													{
														maze_solve_check_blacktile = 4;
													}
												}

											break;

										case 2:
												if(!drive_dist(0,20,3))
												{
													if((groundsens_r > GROUNDSENS_R_TH_BLACKTILE) && (groundsens_l > GROUNDSENS_L_TH_BLACKTILE) &&
															(abs(ramp_start - um6.theta_t) < 4))
													{
														dot_abort = 1;
														if(dot_driven_oneTile == 0)
															maze_corrGround(&robot.pos, robot.dir, 2);
														else
															maze_corrGround(&robot.pos, NONE, 2);
													}
													else
													{
														maze_solve_check_blacktile = 5;
													}

													maze_solve_check_blacktile = 0;
													maze_solve_state_path = DRIVE_DOT;
												}
											break;

										case 3 ... 5:

											if(!drive_dist(0,20,-((maze_solve_check_blacktile-2)*3)))
											{
												maze_solve_state_path = DRIVE_DOT;
												maze_solve_check_blacktile = 0;
											}

										break;
									}

								break;
								
			default:
									if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.03]:DEFAULT_CASE"));}
									fatal_err = 1;
		}

		////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////Victim//////////////////////////////////

        if((maze_solve_state_path >= DRIVE_DOT) &&
			(maze_solve_state_path <= RAMP_DOWN))
		{
			if(timer_victim_led < 0) //Timer not running
			{
				ui_setLED(-1, 0);
				ui_setLED(1, 0);

				if(victim_BufIsVic(LEFT))
				{
					if(dist[LIN][LEFT][BACK] < DIST_VICTIM_MIN)
					{
                        if((maze_getVictim(&robot.pos, robot.dir) == 0) &&
							//	(maze_getVictim(&robot.pos, robot.dir+1) == 0) &&
							//	(maze_getVictim(&robot.pos, robot.dir+2) == 0) &&
								(maze_getVictim(&robot.pos, robot.dir+3) == 0))
						{
							timer_victim_led = TIMER_VICTIM_LED;

							if(maze_solve_state_path == DRIVE_DOT)
							{
								maze_corrVictim(&robot.pos, robot.dir+3, 1);
								maze_solve_state_path = VIC_DEPL_L;
								maze_solve_depl_kit = 0; //Proceed with DRIVE_DOT after deploying the kit
							}
							else if((maze_solve_state_path == RAMP_UP) || (maze_solve_state_path == RAMP_DOWN))
							{
								if(timer_vic_ramp > 0)
									timer_victim_led = -1;
							}
							else if((maze_solve_state_path == TURN_LEFT) ||
									(maze_solve_state_path == TURN_RIGHT))
							{
								if(rotate_progress < 33) //Robot rotates now...
								{
									maze_corrVictim(&robot.pos, robot.dir+3, 1);
									maze_solve_depl_kit = 1;
								}
								else
								{
									maze_corrVictim(&robot.pos, robot.dir, 1);
									maze_solve_depl_kit = 2;
								}
							}
							else
							{
								timer_victim_led = -1;
							}


							if(debug > 0)	{	bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": victim::found:atTemp:l:"));bt_putLong(mlx90614[LEFT].is);bt_putStr_P(PSTR("°C"));	}
						}
					}
				}
				else if(victim_BufIsVic(RIGHT))
				{
					if(dist[LIN][RIGHT][BACK] < DIST_VICTIM_MIN)
					{
						if((maze_getVictim(&robot.pos, robot.dir) == 0) &&
								(maze_getVictim(&robot.pos, robot.dir+1) == 0) )// &&
							//	(maze_getVictim(&robot.pos, robot.dir+2) == 0) &&
							//	(maze_getVictim(&robot.pos, robot.dir+3) == 0))
						{
							timer_victim_led = TIMER_VICTIM_LED;

							if(maze_solve_state_path == DRIVE_DOT)
							{
								maze_corrVictim(&robot.pos, robot.dir+1, 1);
								maze_solve_state_path = VIC_DEPL_R;
								maze_solve_depl_kit = 0;
							}
							else if((maze_solve_state_path == RAMP_UP) || (maze_solve_state_path == RAMP_DOWN))
							{
								if(timer_vic_ramp > 0)
									timer_victim_led = -1;
							}
							else if((maze_solve_state_path == TURN_LEFT) ||
									(maze_solve_state_path == TURN_RIGHT))
							{
								if(rotate_progress < 33) //Robot rotates now...
								{
									maze_corrVictim(&robot.pos, robot.dir+1, 1);
									maze_solve_depl_kit = 1;
								}
								else
								{
									maze_corrVictim(&robot.pos, robot.dir, 1);
									maze_solve_depl_kit = 3;
								}
							}
							else
							{
								timer_victim_led = -1;
							}

							if(debug > 0)	{	bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(":atTemp:r:"));bt_putLong(mlx90614[RIGHT].is); bt_putStr_P(PSTR("°C"));	}
						}
					}
				}
			}

			if(((maze_solve_state_path == RAMP_UP) || (maze_solve_state_path == RAMP_DOWN)) && (timer_victim_led == 0))
			{
				timer_victim_led = -1;
				timer_vic_ramp = TIMER_VIC_RAMP;
			}
		}

		if(timer_victim_led > 0) //Timer running
		{
			//wall_size_part = WALL_SIZE_LARGE; //Detailed view of the tile

			mot.d[LEFT].speed.to = 0;
			mot.d[RIGHT].speed.to = 0;

			ui_setLED(-1, 255);
			ui_setLED(1, 255);
        }
	}

	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Organizes calculations of paths. Executed in the main loop to prevent
// blocking the whole scheduler and allowing the watchdog to break the program
// if something goes wrong!
////////////////////////////////////////////////////////////////////////////////

uint8_t routeRequest = RR_WAIT;

void maze_solveRoutes(void) //called from main-loop (time-intensive route calculations)
{
	int16_t tileRes = 0;
	
	switch(routeRequest)
	{
		case RR_WAIT:					break; //Has to be checked in maze_solve().
		
		case RR_CALCNEARESTTILE:
		
						bt_putStr_P(PSTR("clcnear, tileres:")); 
						
						tileRes = maze_findNearestTile(&robot.pos, &rr_result);
																	 
						if(tileRes == -1)			routeRequest = RR_NEARNOPOSS; //Driven over every visitable tile -> Drive back to start
						else if(tileRes < 4000)		routeRequest = RR_CALCROUTE;
						else						routeRequest = RR_NEARTIMEOUT;
						
						//bt_putLong(tileRes); bt_putStr_P(PSTR("\n\r"));
						
					break;
						
		case RR_CALCROUTE:

						maze_clearDepthsearch();
						
						tileRes = maze_findPath(&robot.pos, &rr_result);
						
						if(tileRes == -1)			routeRequest = RR_RTNOPOSS; //No possible route
						else if(tileRes < 4000)		routeRequest = RR_RTDONE;
						else						routeRequest = RR_RTTIMEOUT;
						
						//bt_putStr_P(PSTR("clcroute, tileres:")); bt_putLong(tileRes); bt_putStr_P(PSTR("\n\r"));
		
					break;
					
		case RR_NEARDONE:		break; //Has to be checked in maze_solve().
		case RR_NEARNOPOSS:		break; //"
		case RR_NEARTIMEOUT:	break; //"
		
		case RR_RTDONE:			break; //"
		case RR_RTNOPOSS:		break; //"
		case RR_RTTIMEOUT:		break; //"
		
		default:
						if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.06]:DEFAULT_CASE"));}
						fatal_err = 1;
						routeRequest = RR_WAIT;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Updates the walls at the robot’s save position in MAZE_SAVESTAGE.
// Returns 1 if the walls were updated.
////////////////////////////////////////////////////////////////////////////////

//Helperfunction for correcting the walls with sensors in the desired direction
void maze_checkCorrWall(COORD *_coord, uint8_t sensinfoA, uint8_t sensinfoB, uint8_t dir, int16_t threshold, uint8_t updateFac_wall)
{
	if(dist[LIN][sensinfoA][sensinfoB] < threshold) //Is wall
		maze_corrWall(_coord, dir, updateFac_wall);
	else
		maze_corrWall(_coord, dir, -updateFac_wall);
}

#define DIST_FR_FAC		4 //([LEFT]/[RIGHT])[FRONT]
#define DIST_BA_FAC		2 //([LEFT]/[RIGHT])[BACK]

#define DIST_FRBA_FAC 1 //Sensore Front (nicht Mitte)
#define DIST_FRFR_FAC 7 //Mitlerer Sensor vorne
#define DIST_BABA_FAC 7 //Mitlerer Sensor hinten

uint8_t sm_updateWalls = 0;

uint8_t maze_updateWalls(void)
{
	uint8_t returnvar = 0;

	switch(sm_updateWalls)
	{
		case 0:
				sensinfo.newDat.left = 0;
				sensinfo.newDat.right = 0;
				sensinfo.newDat.mid = 0;

				sm_updateWalls = 1;

			break;

		case 1:

				if(sensinfo.newDat.left &&
				   sensinfo.newDat.right &&
				   sensinfo.newDat.mid)
				{
					if((abs(maze_getWall(&robot_save.pos, NORTH)) <= MAZE_ISWALL) ||
						 (abs(maze_getWall(&robot_save.pos, EAST)) <= MAZE_ISWALL) ||
						 (abs(maze_getWall(&robot_save.pos, SOUTH)) <= MAZE_ISWALL) ||
						 (abs(maze_getWall(&robot_save.pos, WEST)) <= MAZE_ISWALL))
					{
						//////////Wand rechts/////////////
						maze_checkCorrWall(&robot_save.pos, RIGHT, FRONT, robot_save.dir+1, SIDE_TH, DIST_FR_FAC);
						maze_checkCorrWall(&robot_save.pos, RIGHT, BACK, robot_save.dir+1, SIDE_TH, DIST_BA_FAC);

						//////////Wand links/////////////

						maze_checkCorrWall(&robot_save.pos, LEFT, FRONT, robot_save.dir+3, SIDE_TH, DIST_FR_FAC);
						maze_checkCorrWall(&robot_save.pos, LEFT, BACK, robot_save.dir+3, SIDE_TH, DIST_BA_FAC);

						//////////Wand vorne/////////////
						maze_checkCorrWall(&robot_save.pos, FRONT, LEFT, robot_save.dir, FRONT_TH, DIST_FRBA_FAC);
						maze_checkCorrWall(&robot_save.pos, FRONT, RIGHT, robot_save.dir, FRONT_TH, DIST_FRBA_FAC);
						maze_checkCorrWall(&robot_save.pos, FRONT, FRONT, robot_save.dir, FRONT_FRONT_TH, DIST_FRFR_FAC);

						//////////Wand hinten/////////////
						maze_checkCorrWall(&robot_save.pos, BACK, LEFT, robot_save.dir+2, BACK_TH, DIST_FRBA_FAC);
						maze_checkCorrWall(&robot_save.pos, BACK, RIGHT, robot_save.dir+2, BACK_TH, DIST_FRBA_FAC);
						maze_checkCorrWall(&robot_save.pos, BACK, BACK, robot_save.dir+2, BACK_BACK_TH, DIST_BABA_FAC);

						/*sensinfo.newDat.left = 0;
						sensinfo.newDat.right = 0;
						sensinfo.newDat.mid = 0;*/
					}
					else //enough information
					{
						returnvar = 1;
						sm_updateWalls = 0;
					}
				}
			break;
	}

	return returnvar;
}

////////////////////////////////////////////////////////////////////////////////
// Localize the robot, being in the MAZE_SAVESTAGE, in the maze
////////////////////////////////////////////////////////////////////////////////

uint8_t locRequest = LR_WAIT;

uint8_t loc_ambiguity = 0;
uint8_t loc_accordance = 0;

void maze_localize(void)
{
	MATCHSTAGES matchStages;
	COORD sizediff[MAZE_SIZE_Z];

//	COORD savestage_size;
//	COORD overwrite_orig, overwrite_pos_save;

	switch(locRequest)
	{
		case LR_WAIT: break; //Has to be checked in maze_solve
		
		case LR_MATCH:
		
					for(uint8_t i = 0; i < MAZE_SIZE_Z; i++)
					{
						sizediff[i].z = i;
						maze_stageGetSize(&sizediff[i]);
					}
					for(uint8_t i = 0; i < MAZE_SAVESTAGE; i++)
					{
						sizediff[i].x -= sizediff[MAZE_SAVESTAGE].x;
						sizediff[i].y -= sizediff[MAZE_SAVESTAGE].y;
					}

					if((sizediff[rob_z_lost].x < 0) || (sizediff[rob_z_lost].y < 0))
					{
						matchStages.stage_a = MAZE_SAVESTAGE;
						matchStages.stage_b = rob_z_lost;
					}
					else
					{
						matchStages.stage_a = rob_z_lost;
						matchStages.stage_b = MAZE_SAVESTAGE;
					}
					maze_matchStages(&matchStages);

					loc_ambiguity = matchStages.ambiguity;
					loc_accordance = matchStages.accordance;

					/*displayvar[0] = matchStages.match.dir;
					displayvar[1] = matchStages.match.pos.x;
					displayvar[2] = matchStages.match.pos.y;
					displayvar[3] = matchStages.accordance;
					displayvar[4] = matchStages.ambiguity;

					displayvar[5] = sizediff[0].x;
					displayvar[6] = sizediff[0].y;*/

					if((matchStages.ambiguity == 0) && (matchStages.accordance > 80) && (maze_stageGetVisitedTiles(MAZE_SAVESTAGE) > 5))
					{
						if((sizediff[rob_z_lost].x < 0) || (sizediff[rob_z_lost].y < 0))
						{
							for(uint8_t i = 0; i < matchStages.match.dir; i++)
								maze_rotateStage(rob_z_lost);

							maze_chgOffset(X,rob_z_lost,-(matchStages.match.pos.x-ROB_POS_X_MIN-1));
							maze_chgOffset(Y,rob_z_lost,-(matchStages.match.pos.y-ROB_POS_Y_MIN-1));

							robot.pos.z = rob_z_lost;
						}
						else
						{
							for(uint8_t i = 0; i < matchStages.match.dir; i++)
								maze_rotateStage(MAZE_SAVESTAGE);

							/*for(uint8_t i = 0; i < MAZE_SIZE_Z; i++)
							{
								sizediff[i].z = i;
								maze_stageGetSize(&sizediff[i]);
								sizediff[i].x ++;
								sizediff[i].y ++;
							}
							sizediff[rob_z_lost].x -= sizediff[MAZE_SAVESTAGE].x;
							sizediff[rob_z_lost].y -= sizediff[MAZE_SAVESTAGE].y;

							maze_chgOffset(X,MAZE_SAVESTAGE,-sizediff[rob_z_lost].x);
							maze_chgOffset(Y,MAZE_SAVESTAGE,-sizediff[rob_z_lost].y);*/

							robot.pos.x += matchStages.match.pos.x - ROB_POS_X_MIN-1;
							robot.pos.y += matchStages.match.pos.y - ROB_POS_Y_MIN-1;
							robot.pos.z = matchStages.match.pos.z;
						}

						/*
						//Overwrite stage with new data - does not work yet
						savestage_size.z = MAZE_SAVESTAGE;
						maze_stageGetOuterSize(&savestage_size);

						overwrite_orig.z = rob_z_lost;

						for(overwrite_orig.y = ROB_POS_Y_MIN; overwrite_orig.y <= savestage_size.y+1; overwrite_orig.y++)
						{
							for(overwrite_orig.x = ROB_POS_X_MIN; overwrite_orig.x <= savestage_size.x+1; overwrite_orig.x++)
							{
								overwrite_pos_save = overwrite_orig;
								overwrite_pos_save.z = MAZE_SAVESTAGE;

								if(maze_getBeenthere(&overwrite_pos_save, NONE) ||
								   maze_getBeenthere(&overwrite_pos_save, SOUTH) ||
								   maze_getBeenthere(&overwrite_pos_save, WEST))
								{
									maze_setTile(&overwrite_orig, maze_getTile(&overwrite_pos_save));
								}
								maze_clearTile(&overwrite_pos_save);
							}
						}*/

						//Alignment to the border (let ROB_POS_n_MIN be what it says)
					/*	sizediff[rob_z_lost].z = rob_z_lost;
						maze_stageGetInnerSize(&sizediff[rob_z_lost]);

						sizediff[rob_z_lost].x -= ROB_POS_X_MIN;
						sizediff[rob_z_lost].y -= ROB_POS_Y_MIN;

						robot.pos.x -= sizediff[rob_z_lost].x;
						robot.pos.y -= sizediff[rob_z_lost].y;
						maze_chgOffset(X,rob_z_lost,sizediff[rob_z_lost].x);
						maze_chgOffset(Y,rob_z_lost,sizediff[rob_z_lost].y);*/

						//End
						locRequest = LR_SUCCESS;
						loc_ambiguity = 0;
						loc_accordance = 0;
					}
					else
					{
						locRequest = LR_FAILURE;
					}
				break;
		
		case LR_SUCCESS:	break; //Has to be checked in maze_solve
		case LR_FAILURE:	break; //"

	}
}

////Schwarze Fliesen

int8_t updateGround_lastPosX = -1;
int8_t updateGround_lastPosY = -1;

void maze_updateGround(int8_t updateFac_ground, int8_t isGround)
{
	//if(!isGround)
	//{
		//if(tsl_res > tsl_th) //Keine schwarze Fliese
		//{
			//updateFac_ground *= -1;
		//}
	//}
	//else if(solveMaze_drive == 1) //Wenn schwarze Fliese ggf. Geradeausfahrt abbrechen
	//	dot_abort |= (1<<0);

	switch(robot.dir)
	{
		case NORTH:
		case EAST:
							if(maze_getWall(&robot.pos, robot.dir) < MAZE_ISWALL) //Nur, wenn da keine Wand ist
								maze_corrGround(&robot.pos, robot.dir, updateFac_ground);
								
						break;
						
		case SOUTH:
							if(((robot.pos.y-1) < 0) && (robot.pos.y != updateGround_lastPosY) && (updateFac_ground > 0))
							{
								maze_chgOffset(Y, robot.pos.z, -1);
								robot.pos.y ++;
								updateGround_lastPosY = robot.pos.y;
							}
							if(maze_getWall(&robot.pos, robot.dir) < MAZE_ISWALL) //Nur, wenn da keine Wand ist
								maze_corrGround(&robot.pos, robot.dir, updateFac_ground);
								
				break;
				
		case WEST:
							if(((robot.pos.x-1) < 0) && (robot.pos.x != updateGround_lastPosX) && (updateFac_ground > 0))
							{
								maze_chgOffset(X, robot.pos.z, -1);
								robot.pos.x ++;
								updateGround_lastPosX = robot.pos.x;
							}
							if(maze_getWall(&robot.pos, robot.dir) < MAZE_ISWALL) //Nur, wenn da keine Wand ist
								maze_corrGround(&robot.pos, robot.dir, updateFac_ground);
								
				break;
				
		default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.07]:DEFAULT_CASE"));}
							fatal_err = 1;
	}
		
	if(debug > 0){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": update::ground"));}
}

//////////////////////////////////////////////////

int8_t cursor_robot_pos_x = 0;
int8_t cursor_robot_pos_y = 0;
int8_t cursor_robot_dir = 0;
//incr_ok_mode wird oben deklariert!
int32_t incremental_old_mz = 0;

#define INCR_OK_MODE_CNT 4 //0..4 Modi (x ändern, y ändern, dir ändern, Katengröße ändern, ok)

#define CURSOR_INCR_STEP 1 //Für Positionswahl (CONST = 1 Step -> Wenn 1ncr CONST weiterzählt wird var (pos) geändert))

#define MAPEND_PART_X 70
#define MAPEND_PART_Y 63
#define MAPEND_PART_Y_TOP 7

#define MAP_ROB_POS_X MAPEND_PART_X/2
#define MAP_ROB_POS_Y (MAPEND_PART_Y-MAPEND_PART_Y_TOP)/2

int8_t wall_size_part = WALL_SIZE_STD;

void u8g_DrawMaze(void)
{
	///////Infos//////////////////
	//  Karte		//In// Frei //
	//				//fo// hier://
	//				//Ka//      //
	//    X			//rt//      //
	//				//e	//      //
	//////////////////////////////
	
	if(wall_size_part < WALL_SIZE_MIN)
		wall_size_part = WALL_SIZE_MAX;
	else if(wall_size_part > WALL_SIZE_MAX)
		wall_size_part = WALL_SIZE_MIN;

	int8_t disp_x = 0;
	int8_t disp_y = 0;
	COORD _maze;
	_maze.z = robot.pos.z;

	u8g_drawArrow(wall_size_part/2, MAP_ROB_POS_X, MAP_ROB_POS_Y, maze_alignDir(robot.dir + 3), 1);

	uint8_t map_tiles_rob_x = 0; //How many displayable tiles are at the x-achse on the left side of the robot?
	uint8_t map_tiles_rob_y = 0; //... y ...

	int8_t mapstart_x = MAP_ROB_POS_X - wall_size_part/2;
	while(mapstart_x > 0)
	{
		map_tiles_rob_x ++;
		mapstart_x -= wall_size_part;

		if(map_tiles_rob_x > MAZE_SIZE_X)
		{
			break;
			fatal_err = 1;
		}
	}
	disp_x = mapstart_x;

	int8_t mapstart_y = MAP_ROB_POS_Y + wall_size_part/2;
	while(mapstart_y < MAPEND_PART_Y)
	{
		map_tiles_rob_y ++;
		mapstart_y += wall_size_part;

		if(map_tiles_rob_y > MAZE_SIZE_Y)
		{
			break;
			fatal_err = 1;
		}
	}
	disp_y = mapstart_y;

	uint8_t map_part_size_x = MAPEND_PART_X / wall_size_part; //Size of the extract of the map
	if(MAPEND_PART_X % wall_size_part != 0)
		map_part_size_x ++;

	uint8_t map_part_size_y = MAPEND_PART_Y / wall_size_part; //Size of the extract of the map
	if(MAPEND_PART_Y % wall_size_part != 0)
		map_part_size_y ++;

	for(uint8_t maze_part_y = 0; maze_part_y <= map_part_size_y; maze_part_y++)
	{
		for(uint8_t maze_part_x = 0; maze_part_x <= map_part_size_x; maze_part_x++)
		{
			_maze.x = robot.pos.x + maze_part_x - map_tiles_rob_y;
			_maze.y = robot.pos.y + maze_part_y - map_tiles_rob_y;

			if((_maze.x >= 0) && (_maze.y >= 0) && (_maze.x < MAZE_SIZE_X) && (_maze.y < MAZE_SIZE_Y)) //Existiert die Position in der Karte?
			{
				if((disp_x < MAPEND_PART_X) && (disp_y >= MAPEND_PART_Y_TOP))
						u8g_DrawPixel(&u8g, disp_x, disp_y);

				int8_t wall_size_x_temp;
				int8_t wall_size_y_temp;

				if(maze_getGround(&_maze, NONE) > MAZE_ISBLTILE)
				{
					wall_size_x_temp = wall_size_part;
					if(((int16_t) disp_x + wall_size_part) > MAPEND_PART_X)
						wall_size_x_temp = MAPEND_PART_X-disp_x;

					wall_size_y_temp = wall_size_part;
					if(((int16_t)disp_y - wall_size_y_temp) < MAPEND_PART_Y_TOP)
						wall_size_y_temp = disp_y - MAPEND_PART_Y_TOP + 2;

					u8g_DrawBox(&u8g, disp_x+1, (uint8_t)disp_y+1-wall_size_y_temp, wall_size_x_temp-1, wall_size_y_temp-1);
				}
				else
				{
					if(!((_maze.x == robot.pos.x) && (_maze.y == robot.pos.y))) //Where is the robot?
					{
						if((disp_x + wall_size_part-2 < MAPEND_PART_X) &&
						   (disp_x >= 0) &&
						   (disp_y - wall_size_part+2 > MAPEND_PART_Y_TOP))
						{
							if(maze_getBeenthere(&_maze, NONE))
							{
								u8g_DrawPixel(&u8g,disp_x + wall_size_part - 2, disp_y - wall_size_part + 2);
							}

                         //   uint8_t ramp_dir = maze_getRampPosDir(&_maze);

                            COORD checkpoint_disp = *maze_getCheckpoint(&robot.pos);

                            if((_maze.x == checkpoint_disp.x) && (_maze.y == checkpoint_disp.y)) //Where is the checkpoint?
                            {
                                u8g_DrawLine(&u8g, disp_x, disp_y, disp_x + wall_size_part, disp_y - wall_size_part);
                                u8g_DrawLine(&u8g, disp_x, disp_y - wall_size_part, disp_x + wall_size_part, disp_y);
                            }
                            else if((maze_getDepthsearch(&_maze, NONE) < 0xff) && (wall_size_part > 6))
                            {
                                if((disp_x + wall_size_part < MAPEND_PART_X) &&
                                   (disp_y - wall_size_part > MAPEND_PART_Y_TOP))
                                {
                                    u8g_DrawLong(disp_x+2, 	disp_y-2, maze_getDepthsearch(&_maze, NONE));
                                }
                            }
						}
					}

					//Victims
					if(wall_size_part <= WALL_SIZE_STD)
					{
						if((disp_x + wall_size_part-2 < MAPEND_PART_X) &&
						   (disp_x >= 0) &&
						   (disp_y - wall_size_part+2 > MAPEND_PART_Y_TOP))
						{
							if(maze_getVictim(&_maze, NORTH) > 0)
							{
								u8g_DrawEllipse(&u8g, disp_x + (wall_size_part/2), disp_y - wall_size_part+1, wall_size_part/6, wall_size_part/8, U8G_DRAW_LOWER_RIGHT);
								u8g_DrawEllipse(&u8g, disp_x + (wall_size_part/2), disp_y - wall_size_part+1, wall_size_part/6, wall_size_part/8, U8G_DRAW_LOWER_LEFT);
							}
							if(maze_getVictim(&_maze, EAST) > 0)
							{
								u8g_DrawEllipse(&u8g, disp_x + wall_size_part-1, disp_y - (wall_size_part/2), wall_size_part/8, wall_size_part/6, U8G_DRAW_UPPER_LEFT);
								u8g_DrawEllipse(&u8g, disp_x + wall_size_part-1, disp_y - (wall_size_part/2), wall_size_part/8, wall_size_part/6, U8G_DRAW_LOWER_LEFT);
							}
							if(maze_getVictim(&_maze, SOUTH) > 0)
							{
								u8g_DrawEllipse(&u8g, disp_x + (wall_size_part/2), disp_y-1, wall_size_part/6, wall_size_part/8, U8G_DRAW_UPPER_LEFT);
								u8g_DrawEllipse(&u8g, disp_x + (wall_size_part/2), disp_y-1, wall_size_part/6, wall_size_part/8, U8G_DRAW_UPPER_RIGHT);
							}
							if(maze_getVictim(&_maze, WEST) > 0)
							{
								u8g_DrawEllipse(&u8g, disp_x+1, disp_y - (wall_size_part/2), wall_size_part/8, wall_size_part/6, U8G_DRAW_UPPER_RIGHT);
								u8g_DrawEllipse(&u8g, disp_x+1, disp_y - (wall_size_part/2), wall_size_part/8, wall_size_part/6, U8G_DRAW_LOWER_RIGHT);
							}
						}
					}
				}

				//Wall south
				if(_maze.x < MAZE_SIZE_X-1)
				{
					int8_t wall_tmp = maze_getWall(&_maze, SOUTH);

					if(wall_tmp == 0)
					{
						for(int8_t i = disp_x + 2; i < disp_x + wall_size_part - 1; i += 2)
						{
							if((i >= 0) && (i < MAPEND_PART_X) && (disp_y >= MAPEND_PART_Y_TOP))
								u8g_DrawPixel(&u8g, i, disp_y);
						}
					}
					else if(wall_tmp > MAZE_ISWALL)
					{
						wall_size_x_temp = wall_size_part;

						if((uint8_t)(disp_x + wall_size_part) > MAPEND_PART_X)
						{
							wall_size_x_temp = MAPEND_PART_X-disp_x;
						}
						if((disp_x < MAPEND_PART_X) && (disp_y > MAPEND_PART_Y_TOP))
							u8g_DrawHLine(&u8g, disp_x, disp_y, wall_size_x_temp);
					}
				}

				//Wall west
				if(_maze.y < MAZE_SIZE_Y-1)
				{
					int8_t wall_tmp = maze_getWall(&_maze, WEST);

					if(wall_tmp == 0)
					{
						for(int8_t i = disp_y - wall_size_part + 2; i < disp_y - 1; i += 2)
						{
							if((i >= MAPEND_PART_Y_TOP) && (i <= MAPEND_PART_Y) && (disp_x < MAPEND_PART_X))
								u8g_DrawPixel(&u8g, disp_x, i);
						}
					}
					else if(wall_tmp > MAZE_ISWALL)
					{
						wall_size_y_temp = wall_size_part;
						if(disp_y - wall_size_y_temp <= MAPEND_PART_Y_TOP)
						{
							wall_size_y_temp = disp_y - MAPEND_PART_Y_TOP + 1;
						}
						if((disp_x < MAPEND_PART_X) && (disp_y > MAPEND_PART_Y_TOP))
							u8g_DrawVLine(&u8g, disp_x, disp_y-wall_size_y_temp, wall_size_y_temp);
					}
				}
			}
			disp_x += wall_size_part;
			if(maze_part_x == map_part_size_x)
			{
				disp_y -= wall_size_part;
				disp_x = mapstart_x;
			}
		}
	}

	///////Infos//////////////////
	//  Karte		//In// Frei //
	//				//fo// hier://
	//				//Ka//      //
	//				//rt//      //
	//				//eX//      //
	//////////////////////////////

	u8g_DrawVLine(&u8g, MAPEND_PART_X-1, 7, 57); //Vertikale Abgenzung

	//Positionswahl per Inkrementalgeber

	if(get_incrOk() && (timer_incr_entpr == 0) && mot.off)
	{
		incr_ok_mode ++;
		if(incr_ok_mode > INCR_OK_MODE_CNT)
			incr_ok_mode = 0;
		timer_incr_entpr = TIMER_ENTPR_INCR;

		if(incr_ok_mode == 1)
		{
			drive_reset(); //Fahrfunktionen zurücksetzen
			maze_clearDepthsearch();
				maze_solve_state_path = DRIVE_READY;
				routeRequest = RR_WAIT;
				locRequest = LR_WAIT;
		}
	}

	if(incr_ok_mode != 4)
	{
		switch(incr_ok_mode)
		{
			case 0:		cursor_robot_pos_x += (incremental-incremental_old_mz)*2;		break;
			case 1:		cursor_robot_pos_y += (incremental-incremental_old_mz)*2;		break;
			case 2:		cursor_robot_dir -= (incremental-incremental_old_mz)*2;			break;
			case 3:		wall_size_part += (incremental-incremental_old_mz)*2;

						if(wall_size_part < WALL_SIZE_MIN)
							wall_size_part = WALL_SIZE_MAX;
						else if(wall_size_part > WALL_SIZE_MAX)
							wall_size_part = WALL_SIZE_MIN;
						break;
			default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.17]:DEFAULT_CASE"));}
								fatal_err = 1;
		}

		incremental_old_mz = incremental;

		if(cursor_robot_pos_x > CURSOR_INCR_STEP)
		{
			robot.pos.x ++;
			cursor_robot_pos_x = 0;
		}
		else if(cursor_robot_pos_x < -CURSOR_INCR_STEP)
		{
			robot.pos.x --;
			cursor_robot_pos_x = 0;
		}
		if(cursor_robot_pos_y > CURSOR_INCR_STEP)
		{
			robot.pos.y --;
			cursor_robot_pos_y = 0;
		}
		else if(cursor_robot_pos_y < -CURSOR_INCR_STEP)
		{
			robot.pos.y ++;
			cursor_robot_pos_y = 0;
		}
		if(cursor_robot_dir > CURSOR_INCR_STEP)
		{
			robot.dir --;
			if(robot.dir < NORTH)
				robot.dir = WEST;
			cursor_robot_dir = 0;
		}
		else if(cursor_robot_dir < -CURSOR_INCR_STEP)
		{
			robot.dir ++;
			if(robot.dir > WEST)
				robot.dir = NORTH;
			cursor_robot_dir = 0;
		}							

		if(robot.pos.x > (MAZE_SIZE_X-2))
		{
			robot.pos.x = ROB_POS_X_MIN;
		}
		else if(robot.pos.x < ROB_POS_X_MIN)
		{
			robot.pos.x = (MAZE_SIZE_X-2);
		}

		if(robot.pos.y > (MAZE_SIZE_Y-2))
		{
			robot.pos.y = ROB_POS_Y_MIN;
			robot.pos.z ++;
			if(robot.pos.z > (MAZE_SIZE_Z-1))
			{
				robot.pos.z = 0;
			}
		}
		else if(robot.pos.y < ROB_POS_Y_MIN)
		{
			robot.pos.y = (MAZE_SIZE_Y-2);
			robot.pos.z --;
			if(robot.pos.z < 0)
			{
				robot.pos.z = (MAZE_SIZE_Z-1);
			}
		}
		
		if(maze_GetVisitedTiles() == 0)
		{
			off_start.x = robot.pos.x - ROB_START_MAZE_X; //ROB_START_MAZE_X is added again in maze_getStart
			off_start.y = robot.pos.y - ROB_START_MAZE_Y;
			off_start.z = robot.pos.z - ROB_START_MAZE_Z;
		}
		
		timer_rdy_restart = -1;
	}
	
	switch(incr_ok_mode)
	{
		case 0:		u8g_DrawStr(&u8g, MAPEND_PART_X+1, 14, "chgx");	break;
		case 1:		u8g_DrawStr(&u8g, MAPEND_PART_X+1, 14, "chgy");	break;
		case 2:		u8g_DrawStr(&u8g, MAPEND_PART_X+1, 14, "chgd");	break;
		case 3:		u8g_DrawStr(&u8g, MAPEND_PART_X+1, 14, "sze:");
					u8g_DrawLong(MAPEND_PART_X+5, 21, wall_size_part);	break;
		case 4:		u8g_DrawStr(&u8g, MAPEND_PART_X+1, 14, "ok");		break;
		default: 	if(debug > 1){bt_putStr_P(PSTR("\n\r")); bt_putLong(timer); bt_putStr_P(PSTR(": ERROR::FATAL:WENT_INTO:switch[maze.18]:DEFAULT_CASE"));}
							fatal_err = 1;
	}
	u8g_DrawStr(&u8g, MAPEND_PART_X+1, 28, "x"); u8g_DrawStr(&u8g, MAPEND_PART_X+4, 28, ":"); u8g_DrawLong(MAPEND_PART_X+7, 28, robot.pos.x);
	u8g_DrawStr(&u8g, MAPEND_PART_X+1, 35, "y"); u8g_DrawStr(&u8g, MAPEND_PART_X+4, 35, ":"); u8g_DrawLong(MAPEND_PART_X+7, 35, robot.pos.y);
	u8g_DrawStr(&u8g, MAPEND_PART_X+1, 42, "z"); u8g_DrawStr(&u8g, MAPEND_PART_X+4, 42, ":"); u8g_DrawLong(MAPEND_PART_X+7, 42, robot.pos.z);

	if(robot.pos.z == MAZE_SAVESTAGE)
	{
		u8g_DrawLong(MAPEND_PART_X+1, 56, loc_ambiguity);
			if(loc_ambiguity < 10)
				u8g_DrawStr(&u8g, MAPEND_PART_X+5, 56, "x");
			else
				u8g_DrawStr(&u8g, MAPEND_PART_X+9, 56, "x");
		u8g_DrawLong(MAPEND_PART_X+1, 63, loc_accordance);
			if(loc_accordance < 10)
				u8g_DrawStr(&u8g, MAPEND_PART_X+5, 63, "%");
			else if(loc_accordance < 100)
				u8g_DrawStr(&u8g, MAPEND_PART_X+9, 63, "%");
			else
				u8g_DrawStr(&u8g, MAPEND_PART_X+13, 63, "%");
	}
}

///////////////////////////////////////
//MAPSAMPLE:
/*		uint8_t OFF_X =  0;
uint8_t OFF_Y =0;
uint8_t  VAL	=	30;


		maze_corrWall(0+OFF_X, 0+OFF_Y, 0, WEST, VAL);
		maze_corrWall(0+OFF_X, 1+OFF_Y, 0, WEST, VAL);
		maze_corrWall(0+OFF_X, 2+OFF_Y, 0, WEST, VAL);
		maze_corrWall(0+OFF_X, 3+OFF_Y, 0, WEST, VAL);

		maze_corrWall(0+OFF_X, 3+OFF_Y, 0, NORTH, VAL);
		maze_corrWall(1+OFF_X, 3+OFF_Y, 0, NORTH, VAL);
		maze_corrWall(2+OFF_X, 3+OFF_Y, 0, NORTH, VAL);
		maze_corrWall(3+OFF_X, 3+OFF_Y, 0, NORTH, VAL);

		maze_corrWall(0+OFF_X, 0+OFF_Y, 0, SOUTH, VAL);
		maze_corrWall(1+OFF_X, 0+OFF_Y, 0, SOUTH, VAL);
		maze_corrWall(2+OFF_X, 0+OFF_Y, 0, SOUTH, VAL);
		maze_corrWall(3+OFF_X, 0+OFF_Y, 0, SOUTH, VAL);
		
		maze_corrWall(3+OFF_X, 1+OFF_Y, 0, EAST, VAL);
		maze_corrWall(3+OFF_X, 2+OFF_Y, 0, EAST, VAL);
		maze_corrWall(3+OFF_X, 3+OFF_Y, 0, EAST, VAL);

		maze_corrWall(1+OFF_X, 0+OFF_Y, 0, NORTH, VAL);
		maze_corrWall(2+OFF_X, 0+OFF_Y, 0, NORTH, VAL);
		maze_corrWall(2+OFF_X, 0+OFF_Y, 0, WEST, VAL);

		maze_corrWall(1+OFF_X, 3+OFF_Y, 0, WEST, VAL);
		maze_corrWall(1+OFF_X, 3+OFF_Y, 0, SOUTH, VAL);
		maze_corrWall(3+OFF_X, 3+OFF_Y, 0, SOUTH, VAL);

	robot_pos.x = 3;*/
