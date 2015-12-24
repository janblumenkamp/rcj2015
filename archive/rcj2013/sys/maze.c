#include "maze.h"
#include "bluetooth.h"
#include "drive.h"

const uint8_t NONE = 0; //Richtung (Orientierung), siehe auch DRIVE.C!!!!!
const uint8_t NORTH = 1;
const uint8_t EAST = 2;
const uint8_t SOUTH = 3;
const uint8_t WEST = 4;

uint8_t maze_size_x = 0;
uint8_t maze_size_y = 0;
uint8_t maze_size_z = 0;

#define MAZE_SIZE_X 11
#define MAZE_SIZE_Y 6 
#define MAZE_SIZE_Z 2

#define ROB_START_MAZE_X	0 //Startkoordinaten
#define ROB_START_MAZE_Y	0 //Kein Startfeld => starte bei y=1 
#define ROB_START_MAZE_Z	0
#define ROB_START_DIR			1

int32_t maze[MAZE_SIZE_X][MAZE_SIZE_Y][MAZE_SIZE_Z] = {{{0}}}; //Speicherung von Bodeneigenschaften und der Wand links/unten
		//byte 1: Algo Tarry [var]
		//byte 2: Wand Süden [var << 8]
		//byte 3: Wand Westen[var << 16]
		//byte 4: Untergrund [var << 24]

int8_t robot_pos_x = ROB_START_MAZE_X;
int8_t robot_pos_y = ROB_START_MAZE_Y;
int8_t robot_pos_z = ROB_START_MAZE_Z;
int8_t robot_dir = ROB_START_DIR;

int8_t maze_pos_x_ramp = 0; //X-Pos für Rampe

/////////////////////////////////////////////////////////////////
uint8_t solveMaze_ready = 1; //Darf die nächste Operation durchgeführt werden?
uint8_t solveMaze_drive = 0; //Soll geradeaus gefahren werden?
uint8_t solveMaze_whatToDo = 0; //Welche Operation soll durchgeführt werden?

int8_t cursor_robot_pos_x = 0;
int8_t cursor_robot_pos_y = 0;
int8_t cursor_robot_dir = 0;
uint8_t incr_ok_mode = 3; //Positions- oder Richtungswahl?

uint8_t use_map = 1; //Wird deaktiviert wenn Roboter z.B. von Karte fährt

#define INCR_OK_MODE_CNT 3 //0..3 Modi (x ändern, y ändern, dir ändern, ok)

#define MAZE_ISWALL 5 //Über diesem Schwellwert ist da eine Wand
#define MAZE_ISBLTILE 0 //Über diesem Schwellwert ist da eine Sackgasse

#define MAZE_UPDATEWALLFAC_DRIVE_MID 1 //Gewichtung bei Wandkorrektur bei sicheren Mittelpositionen
#define MAZE_UPDATEWALLFAC_DRIVE 0 //Gewichtung bei Wandkorrektur während der Geradeausfahrt
#define MAZE_UPDATEWALLFAC_TURN 1 //Gewichtung bei Wandkorrektur während der Geradeausfahrt
#define MAZE_UPDATEGROUNDFAC_DRIVE_MID 1 //Gewichtung bei Untergrundkorrektur bei sicheren Mittelpositionen
#define MAZE_UPDATEGROUNDFAC_DRIVE 0 //Gewichtung bei Untergrundkorrektur während der Geradeausfahrt
#define MAZE_UPDATEGROUNDFAC_TURN 0 //Gewichtung bei Untergrundkorrektur während Drehen

#define TILE1_FRONT 				130 //Entfernung zur Wand bei einer Fliese (Entfernung für Sensor nach vorne)
#define TILE1_FRONT_TOP 		530 //Entfernung zur Wand bei einer Fliese (Entfernung für Sensor nach vorne, wallsensor_top[])
#define TILE1_FRONT_TH_TOP 	360 //Schwellwert nach vorne, wallsensor_top[]
#define TILE1_FRONT_TH 			115 //Schwellwert nach vorne
#define TILE1_FRONT_TH_SRF 	1700 //Schwellwert nach vorne
#define TILE1_SIDE_TH 			150 //Schwellwert zur Seite
#define TILE1_SIDE_TH_TOP		240
#define TILE1_SIDE_TH_SRF		1200
#define TILE1_BACK_TH_SRF		1400 //Schwellwert nach hinten (SRF)
#define TILE1_BACK_TH_TOP		550

#define DRIVE_BUMPED_WIDTH 3

#define MAZE_WALLVALUE_MAX 127 //++ ~ Overflow
#define MAZE_WALLVALUE_MIN -127 //-- ~Overflow

#define MAZE_GROUNDVALUE_MAX 127 ////++ ~ Overflow
#define MAZE_GROUNDVALUE_MIN -127 ////++ ~ Overflow

#define CURSOR_INCR_STEP 2 //Für Positionswahl (CONST = 1 Step -> Wenn 1ncr CONST weiterzählt wird var (pos) geändert))

#define TIMER_INCR_ENTPR  500/25  //SIEHE ROBOCUP.C!!!

#define TIMER_ADD_VAR_DRV 4 //SIEHE ROBOCUP.C!!!!!!!!!!!!!!!!
uint8_t maze_cleared = 0;

uint8_t solve_maze(void)
{
	uint8_t returnvar = 1;

	/*if((um6_phi < 300) && (um6_phi > 250))
	{
		maze_clear(); //Labyrinth zurücksetzen (umgefallen)
		robot_pos_x -= 3;
		if(robot_pos_x > 0)
		{
			robot_pos_x = 0;
		}
		robot_pos_z = 0;
		robot_pos_y -= 1;
		if(robot_pos_y > 0)
		{
			robot_pos_y = 0;
		}
		maze_cleared = 1;
		_delay_ms(1000);
	}
	else
	{
		maze_cleared = 0;
	}*/

	if((!motor_off) && (incr_ok_mode == 3)) //Wenn der Roboter nicht steht und keine Eingaben
	{
		if(solveMaze_ready)
		{
			if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, NORTH) == 0) || //In einer Richtung noch keine Daten für Wand!
				 (maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, EAST) == 0) ||
				 (maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, SOUTH) == 0) ||
				 (maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, WEST) == 0))
			{
				maze_updateWall(MAZE_UPDATEWALLFAC_DRIVE_MID);
			}
			else //Überall Daten: Wo entlang?
			{
				if(use_tarry && use_map)
				{
					if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1) < MAZE_ISWALL) &&
						((maze_getLast(robot_dir+1) == 0) && (maze_getStop(robot_dir+1) == 0))) //Eingang rechts keine Markierung
					{
						solveMaze_whatToDo = 1; //Rechts drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir) < MAZE_ISWALL) &&
								 ((maze_getLast(robot_dir) == 0) && (maze_getStop(robot_dir) == 0))) //Eingang vorne keine Markierung
					{
						solveMaze_whatToDo = 0; //nicht drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3) < MAZE_ISWALL) &&
								 ((maze_getLast(robot_dir+3) == 0) && (maze_getStop(robot_dir+3) == 0))) //Eingang links keine Markierung
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2) < MAZE_ISWALL) &&
								 ((maze_getLast(robot_dir+2) == 0) && (maze_getStop(robot_dir+2) == 0))) //Eingang vorne keine Markierung
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 0; //nicht Geradeaus
					}
					//Überall Markierungen:
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1) < MAZE_ISWALL) &&
									(maze_getLast(robot_dir+1) == 1)) //Eingang rechts Markierung z
					{
						solveMaze_whatToDo = 1; //Rechts drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir) < MAZE_ISWALL) &&
									(maze_getLast(robot_dir) == 1)) //Eingang vorne Markierung z
					{
						solveMaze_whatToDo = 0; //nicht drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3) < MAZE_ISWALL) &&
									(maze_getLast(robot_dir+3) == 1)) //Eingang links Markierung z
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2) < MAZE_ISWALL) &&
									(maze_getLast(robot_dir+2) == 1)) //Eingang vorne keine Markierung
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 0; //nicht Geradeaus
					}
					//FERTIG
					else
					{
						speed_l = 0;
						speed_r = 0;

						solveMaze_whatToDo = 0; //nicht drehen
						solveMaze_drive = 0; //nicht Geradeaus

						maze_updateWall(MAZE_UPDATEWALLFAC_DRIVE_MID); //Falls Wand falsch eingezeichnet wurde weiter prüfen

						returnvar = 0; //0 zurückgeben, wenn fertig, sonst 1
					}
				}
				else //Kartenfehler oder Algorithmus nicht nutzen
				{
					if((wallsensor_top[RIGHT] < TILE1_SIDE_TH_TOP) ||
						 (wallsensor[RIGHT][FRONT] < TILE1_SIDE_TH) ||
						 (wallsensor[RIGHT][BACK] < TILE1_SIDE_TH))
					{
						solveMaze_whatToDo = 1; //Rechts drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((wallsensor_top[FRONT] < TILE1_FRONT_TH_TOP) ||
								  (wallsensor[FRONT][LEFT] < TILE1_FRONT_TH) ||
								  (wallsensor[FRONT][RIGHT] < TILE1_FRONT_TH))
					{
						solveMaze_whatToDo = 0; //nicht drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else if((wallsensor_top[LEFT] < TILE1_SIDE_TH_TOP) ||
									(wallsensor[LEFT][FRONT] < TILE1_SIDE_TH) ||
								  (wallsensor[LEFT][BACK] < TILE1_SIDE_TH))
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 1; //Geradeaus
					}
					else
					{
						solveMaze_whatToDo = 2; //links drehen
						solveMaze_drive = 0; //nicht Geradeaus
					}
				}
				solveMaze_ready = 0;
			}	
		}
	}

	if(solveMaze_ready == 0)
	{
		uint8_t driveOneTileVar = 0xff;

		switch(solveMaze_whatToDo)
		{
			case 0:	//Nicht drehen oder drehen fertig => Wenn geradeaus fahren soll dann fahren
								if(solveMaze_drive == 1)
								{
									if(use_map && (robot_pos_x > 0) && (robot_pos_y == 0) && (robot_pos_z == 0) && (robot_dir == WEST))
									{
										maze_pos_x_ramp = robot_pos_x; //X-Pos für Rampe speichern
										solveMaze_whatToDo = 6; //Rampe hoch
									}
									else if(use_map && (robot_pos_x == 0) && (robot_pos_y == 0) && (robot_pos_z == 1) && (robot_dir == EAST))
									{
										solveMaze_whatToDo = 3; //Rampe runter: 180° drehen
									}
									else
									{
										driveOneTileVar = drive_oneTile();
									}

									if(wallsensor[FRONT][LEFT] > 300)//get_bumpL()) //Einer der Taster gedrückt?
									{
										solveMaze_whatToDo = 4;
									}
									else if(wallsensor[FRONT][RIGHT] > 300)//get_bumpR())
									{
										solveMaze_whatToDo = 5;
									}
									else if(use_map && ((driveOneTileVar == 3) || (driveOneTileVar == 1))) //Nein? Dann geradeaus fahren.
									{
										if(timer_add_var_drv == 0)
										{
											maze_updateWall(MAZE_UPDATEWALLFAC_DRIVE);

											if(driveOneTileVar == 1)
											{
												maze_updateGround(MAZE_UPDATEGROUNDFAC_DRIVE);
											}

											timer_add_var_drv = TIMER_ADD_VAR_DRV;
										}
									}
									else if(driveOneTileVar == 2) //Mitte beim Geradeausfahren (Übergang 2 Fliesen)
									{
										if(use_map)
										{
											switch(robot_dir)
											{
												case 1: //NORTH
																	robot_pos_y++;
																break;
												case 2: //EAST
																	robot_pos_x++;
																break;
												case 3: //SOUTH
																	robot_pos_y--;
																break;
												case 4: //WEST
																	robot_pos_x--;
																break;
											}
										}

										if((robot_pos_x < 0)						|| //Von der Karte gefahren ==> unbrauchbar
											 (robot_pos_x == MAZE_SIZE_X) ||
											 (robot_pos_y < 0) 						||
											 (robot_pos_y == MAZE_SIZE_Y))
										{
											use_map = 0;
										}
									}
									else if(driveOneTileVar == 0) // Ende geradeaus fahren (eine Fliese gefahren)
									{
										//victim_setLED(100);_delay_ms(10);

										if(use_map)
										{
											maze_updateWall(MAZE_UPDATEWALLFAC_DRIVE_MID);
											maze_updateGround(MAZE_UPDATEGROUNDFAC_DRIVE_MID);

											maze_setStop(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir);

											if(maze_posCountWall() < 2) //Weniger als zwei Wände
											{
												if(!(maze_getLast(NORTH)) && //Keine L-Markierung an einem der Eingänge
													 !(maze_getLast(EAST)) &&
													 !(maze_getLast(SOUTH)) &&
													 !(maze_getLast(WEST)))
												{
													maze_setLast(1);
												}
												//victim_setLED(255);_delay_ms(20);
											}
											else
											{
												maze_setLast(0); //Löschen, falls vorher aufgrund Falscheinzeichnen einer Wand eingezeichnet
											}

											if(getLS())//maze_getGround(robot_pos_x, robot_pos_y, robot_pos_z) > MAZE_ISBLTILE)
											{
												maze_posSetBlacktile();
												maze_corrGround(robot_pos_x, robot_pos_y, robot_pos_z, 30);
											}
										}

										solveMaze_ready = 1;
									}
								}
								else
								{
									solveMaze_ready = 1;
								}
							break;
			case 1: //Rechts drehen
								if(drive_turn(90, 1) == 0)
								{
									if(use_map)
									{
										robot_dir ++;
										if(robot_dir > WEST)
											robot_dir = NORTH;

										maze_updateWall(MAZE_UPDATEWALLFAC_TURN);
										maze_updateGround(MAZE_UPDATEGROUNDFAC_TURN);
									}		

									solveMaze_whatToDo = 0; //Geradeausfahren? Da ggf. abbrechen.
								}
							break;
			case 2:	//Links drehen
								if(drive_turn(-90, 1) == 0)
								{
									if(use_map)
									{
										robot_dir --;
										if(robot_dir < NORTH)
											robot_dir = WEST;

										maze_updateWall(MAZE_UPDATEWALLFAC_TURN);
										maze_updateGround(MAZE_UPDATEGROUNDFAC_TURN);
									}								

									solveMaze_whatToDo = 0; //Geradeausfahren? Da ggf. abbrechen.
								}
							break;
			case 3:		if(drive_turn(180, 1) == 0) //Rampe oben
								{
									if(use_map)
									{
										robot_dir += 2; //180° drehen
										if(robot_dir > WEST)
											robot_dir -= WEST;

										maze_updateWall(MAZE_UPDATEWALLFAC_TURN);
										maze_updateGround(MAZE_UPDATEGROUNDFAC_TURN);
									}

									solveMaze_whatToDo = 7;
								}
							break;
			case 4: 	if(drive_dist(1, -DRIVE_BUMPED_WIDTH) == 0) //!(drive_lr(0,DRIVE_BUMPED_WIDTH))) //Rechts korrigieren
								{
									solveMaze_whatToDo = 0; //Danach bei Geradeausfahrt weitermachen
								}
							break;
			case 5:		if(drive_dist(-1, -DRIVE_BUMPED_WIDTH) == 0)//!(drive_lr(1,DRIVE_BUMPED_WIDTH))) //Links korrigieren
								{
									solveMaze_whatToDo = 0; //Danach bei Geradeausfahrt weitermachen
								}
							break;
			case 6:		if(!(drive_ramp(70)))
								{
									robot_pos_x = 0;
									robot_pos_y = 0;
									robot_pos_z = 1;
									solveMaze_ready = 1;
								}
							break;
			case 7:		if(!(drive_ramp(-60)))
								{
									robot_pos_x = maze_pos_x_ramp;
									robot_pos_y = 0;
									robot_pos_z = 0;
									solveMaze_ready = 1;
								}
							break;
		}
	}

/////////////Positionswahl per Inkrementalgeber/////////////////////////////////

	if(get_incrOk() && (timer_incr_entpr == 0) && motor_off)
	{
		drive_reset(); //Fahrfunktionen zurücksetzen
			solveMaze_ready = 1;
		//maze_clear(); //Labyrinth zurücksetzen
		use_map = 1; //Position wird jetzt manuell festgelegt ==> Karte wieder ok
		incr_ok_mode ++;
		if(incr_ok_mode > INCR_OK_MODE_CNT)
		{
			incr_ok_mode = 0;
		}
		timer_incr_entpr = TIMER_INCR_ENTPR;
	}

	if(incr_ok_mode != 3)
	{
		switch(incr_ok_mode)
		{
			case 0:		cursor_robot_pos_x += incr_delta;
							break;
			case 1:		cursor_robot_pos_y += incr_delta;
							break;
			case 2:		cursor_robot_dir += incr_delta;
							break;
		}
	
		int8_t robot_pos_x_var = robot_pos_x;
		int8_t robot_pos_y_var = robot_pos_y;
		int8_t robot_pos_z_var = robot_pos_z;

		if(cursor_robot_pos_x > CURSOR_INCR_STEP)
		{
			robot_pos_x_var ++;
			cursor_robot_pos_x = 0;
		}
		else if(cursor_robot_pos_x < -CURSOR_INCR_STEP)
		{
			robot_pos_x_var --;
			cursor_robot_pos_x = 0;
		}
		if(cursor_robot_pos_y > CURSOR_INCR_STEP)
		{
			robot_pos_y_var --;
			cursor_robot_pos_y = 0;
		}
		else if(cursor_robot_pos_y < -CURSOR_INCR_STEP)
		{
			robot_pos_y_var ++;
			cursor_robot_pos_y = 0;
		}
		if(cursor_robot_dir > CURSOR_INCR_STEP)
		{
			robot_dir --;
			if(robot_dir < NORTH)
				robot_dir = WEST;
			cursor_robot_dir = 0;
		}
		else if(cursor_robot_dir < -CURSOR_INCR_STEP)
		{
			robot_dir ++;
			if(robot_dir > WEST)
				robot_dir = NORTH;
			cursor_robot_dir = 0;
		}							

		if(robot_pos_x_var > (MAZE_SIZE_X-2))
		{
			robot_pos_x_var = 0;
		}
		else if(robot_pos_x_var < 0)
		{
			robot_pos_x_var = (MAZE_SIZE_X-2);
		}

		if(robot_pos_y_var > (MAZE_SIZE_Y-2))
		{
			robot_pos_y_var = 0;
			robot_pos_z_var ++;
			if(robot_pos_z_var > (MAZE_SIZE_Z-1))
			{
				robot_pos_z_var = 0;
			}
		}
		else if(robot_pos_y_var < 0)
		{
			robot_pos_y_var = (MAZE_SIZE_Y-2);
			robot_pos_z_var --;
			if(robot_pos_z_var < 0)
			{
				robot_pos_z_var = (MAZE_SIZE_Z-1);
			}
		}

		robot_pos_x = robot_pos_x_var;
		robot_pos_y = robot_pos_y_var;
		robot_pos_z = robot_pos_z_var;
	}

	return returnvar;
}
/////////////////////////////////////////////

#define SRF10_FAC				2//
#define WALLS_TOP_FAC		5
#define WALLS_FR_FAC		3 //Front (left/right)
#define WALLS_BA_FAC		1 //Back		"
#define WALLS_FRBA_FAC	0 //nach vorne/hinten zeigende Sensoren (wallsensor[FRONT/BACK][LEFT/RIGHT]

void maze_updateWall(int8_t updateFac_wall)
{
	//////////Wand rechts/////////////
	
	if(wallsensor[RIGHT][FRONT] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, WALLS_FR_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, -WALLS_FR_FAC*updateFac_wall);

	if(wallsensor[RIGHT][BACK] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, WALLS_BA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, -WALLS_BA_FAC*updateFac_wall);

	if(wallsensor_top[RIGHT] > TILE1_SIDE_TH_TOP)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, WALLS_TOP_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, -WALLS_TOP_FAC*updateFac_wall);

	if((!(check_srf & (1<<4))) && //SRF NO_ERR
		 (!(check_srf & (1<<5))) &&
		 (!(check_srf & (1<<6))) &&
		 (!(check_srf & (1<<7))))
	{
		if(srf10[RIGHT] < TILE1_SIDE_TH_SRF)
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, SRF10_FAC*updateFac_wall);
		else
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+1, -SRF10_FAC*updateFac_wall);
	}

	//////////Wand links/////////////

	if(wallsensor[LEFT][FRONT] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, WALLS_FR_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, -WALLS_FR_FAC*updateFac_wall);

	if(wallsensor[LEFT][BACK] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, WALLS_BA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, -WALLS_BA_FAC*updateFac_wall);

	if(wallsensor_top[LEFT] > TILE1_SIDE_TH_TOP)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, WALLS_TOP_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, -WALLS_TOP_FAC*updateFac_wall);

	if((!(check_srf & (1<<12))) && //SRF NO_ERR
		 (!(check_srf & (1<<13))) &&
		 (!(check_srf & (1<<14))) &&
		 (!(check_srf & (1<<15))))
	{
		if(srf10[LEFT] < TILE1_SIDE_TH_SRF)
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, SRF10_FAC*updateFac_wall);
		else
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+3, -SRF10_FAC*updateFac_wall);
	}

	//////////Wand vorne/////////////

	if(wallsensor[FRONT][LEFT] > TILE1_FRONT_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, WALLS_FRBA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, -WALLS_FRBA_FAC*updateFac_wall);

	if(wallsensor[FRONT][RIGHT] > TILE1_FRONT_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, WALLS_FRBA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, -WALLS_FRBA_FAC*updateFac_wall);

	if(wallsensor_top[FRONT] > TILE1_FRONT_TH_TOP)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, WALLS_TOP_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, -WALLS_TOP_FAC*updateFac_wall);

	if((!(check_srf & (1<<0))) && //SRF NO_ERR
		 (!(check_srf & (1<<1))) &&
		 (!(check_srf & (1<<2))) &&
		 (!(check_srf & (1<<3))))
	{
		if(srf10[FRONT] < TILE1_FRONT_TH_SRF)
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, SRF10_FAC*updateFac_wall);
		else
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir, -SRF10_FAC*updateFac_wall);
	}

	//////////Wand hinten/////////////


	if(wallsensor[FRONT][LEFT] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, WALLS_FRBA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, -WALLS_FRBA_FAC*updateFac_wall);

	if(wallsensor[FRONT][RIGHT] > TILE1_SIDE_TH)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, WALLS_FRBA_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, -WALLS_FRBA_FAC*updateFac_wall);

	if(wallsensor_top[BACK] > TILE1_BACK_TH_TOP)
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, WALLS_TOP_FAC*updateFac_wall);
	else
		maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, -WALLS_TOP_FAC*updateFac_wall);

	if((!(check_srf & (1<<8))) && //SRF NO_ERR
		 (!(check_srf & (1<<9))) &&
		 (!(check_srf & (1<<10))) &&
		 (!(check_srf & (1<<11))))
	{
		if(srf10[BACK] < TILE1_BACK_TH_SRF)
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, SRF10_FAC*updateFac_wall);
		else
			maze_corrWall(robot_pos_x, robot_pos_y, robot_pos_z, robot_dir+2, -SRF10_FAC*updateFac_wall);
	}
}

////Sackgassen

void maze_updateGround(int8_t updateFac_ground)
{
	if(getLS())
		maze_corrGround(robot_pos_x, robot_pos_y, robot_pos_z, 1*updateFac_ground);
	else
		maze_corrGround(robot_pos_x, robot_pos_y, robot_pos_z, -1*updateFac_ground);
}
/////////////////////////////////////////////

void maze_corrWall(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, uint8_t dir, int8_t value)
{
	if(dir > WEST)
		dir -= WEST;
	
	if(((value > 0) && ((int16_t)(maze_getWall(pos_x, pos_y, pos_z, dir)+value) <= MAZE_WALLVALUE_MAX)) ||
		 ((value < 0) && ((int16_t)(maze_getWall(pos_x, pos_y, pos_z, dir)+value) >= MAZE_WALLVALUE_MIN)))
	{
		switch(dir)
		{
			case 1:	//NORTH
								maze[pos_x][pos_y+1][pos_z] += ((int32_t)value << 8); //Aktuelle Platte Wand Norden = nächste Platte Norden Wand Süden
							break;
			case 2:	//EAST
								maze[pos_x+1][pos_y][pos_z] += ((int32_t)value << 16); //Aktuelle Platte Wand Osten = nächste Platte Wand Westen
							break;
			case 3:	//SOUTH
								maze[pos_x][pos_y][pos_z] += ((int32_t)value << 8); //Aktuelle Platte Wand Süden = Aktuelle Platte Wand Süden
							break;
			case 4:	//WEST
								maze[pos_x][pos_y][pos_z] += ((int32_t)value << 16); //Aktuelle Platte Wand Westen = Aktuelle Platte Wand Westen
							break;
		}
	}
}

int8_t maze_getWall(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, uint8_t dir)
{
	if(dir > WEST)
		dir -= WEST;

	switch(dir)
	{
		case 1:	//NORTH
							return (((int32_t)maze[pos_x][pos_y+1][pos_z] & 0x0000ff00)>>8); //Aktuelle Platte Wand Norden = nächste Platte Norden Wand Süden
						break;
		case 2:	//EAST
							return (((int32_t)maze[pos_x+1][pos_y][pos_z] & 0x00ff0000)>>16); //Aktuelle Platte Wand Osten = nächste Platte Wand Westen
						break;
		case 3:	//SOUTH
							return (((int32_t)maze[pos_x][pos_y][pos_z] & 0x0000ff00)>>8); //Aktuelle Platte Wand Süden = Aktuelle Platte Wand Süden
						break;
		case 4:	//WEST
							return (((int32_t)maze[pos_x][pos_y][pos_z] & 0x00ff0000)>>16); //Aktuelle Platte Wand Westen = Aktuelle Platte Wand Westen
						break;		
		default:
							return 0xff;
						break;
	}
}

void maze_corrGround(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, int8_t value)
{
	if(((value > 0) && ((maze_getGround(pos_x, pos_y, pos_z)+value) <= MAZE_GROUNDVALUE_MAX)) ||
		 ((value < 0) && ((maze_getGround(pos_x, pos_y, pos_z)+value) >= MAZE_GROUNDVALUE_MIN)))
	{
		maze[pos_x][pos_y][pos_z] += ((uint32_t)value << 24);
	}
}

int8_t maze_getGround(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z)
{
	return (((int32_t)maze[pos_x][pos_y][pos_z] & 0xff000000)>>24);
}


/////////Für Tarry Algorithmus//////////
//Setzt auf Eingang der aktuellen Fliese stop

void maze_setStop(int8_t pos_x, int8_t pos_y, int8_t pos_z, int8_t dir)
{
	if(dir > WEST)
		dir -= WEST;

	if(((pos_x >= 0) && (pos_x+1 < MAZE_SIZE_X)) &&
		 ((pos_y >= 0) && (pos_y+1 < MAZE_SIZE_Y)))
	{
		switch(dir)
		{
			case 1:	//Richtung = Norden: Diese Platte Element Süden setzen
								maze[pos_x][pos_y][pos_z] |= (1<<4);
							break;
			case 2:	//EAST
								maze[pos_x][pos_y][pos_z] |= (1<<2);
							break;
			case 3:	//SOUTH
								maze[pos_x][pos_y][pos_z] |= (1<<0);
							break;
			case 4:	//WEST
								maze[pos_x][pos_y][pos_z] |= (1<<6);
							break;
		}
	}
}

//gibt den Status des Eingangs der Fliese in dir (Relation zum Roboter) zurück

int8_t maze_getStop(uint8_t dir)
{
	if(dir > WEST)
		dir -= WEST;

	switch(dir)
	{
		case 1:	//Platte Norden aber Element Süden!
							if(robot_pos_y+2 < MAZE_SIZE_Y)
								return (((int32_t)maze[robot_pos_x][robot_pos_y+1][robot_pos_z] & 0x00000010)>>4);
							else
								return 0; //Da wäre sowieso die Außenand, aber vorsichtshalber.
						break;
		case 2:	//Platte Osten aber Element Westen!
							if(robot_pos_x+2 < MAZE_SIZE_X)
								return (((int32_t)maze[robot_pos_x+1][robot_pos_y][robot_pos_z] & 0x00000004)>>2);
							else
								return 0;
						break;
		case 3:	//Platte Süden aber Element Norden!
							if(robot_pos_y-1 >= 0)
								return ((int32_t)maze[robot_pos_x][robot_pos_y-1][robot_pos_z] & 0x00000001);
							else
								return 0;
						break;
		case 4:	//Platte Westen aber Element Osten!
							if(robot_pos_x-1 >= 0)
								return (((int32_t)maze[robot_pos_x-1][robot_pos_y][robot_pos_z] & 0x00000040)>>6);
							else
								return 0;
						break;
		default:
							return 1;
						break;
	}
}

//Setzt/löscht Ausgang der Fliese hinter Roboter auf Zurück
void maze_setLast(uint8_t set)
{
	switch(robot_dir)
	{
		case 1:	//Platte hinter Roboter
							if((robot_pos_y-1) >= 0)
							{
								if(set)
									maze[robot_pos_x][robot_pos_y-1][robot_pos_z] |= ((int32_t) 1<<1);
								else
									maze[robot_pos_x][robot_pos_y-1][robot_pos_z] &= ~((int32_t) 1<<1);
							}
						break;
		case 2:	//EAST
							if((robot_pos_x-1) >= 0)
							{
								if(set)
									maze[robot_pos_x-1][robot_pos_y][robot_pos_z] |= ((int32_t) 1<<3);
								else
									maze[robot_pos_x-1][robot_pos_y][robot_pos_z] &= ~((int32_t) 1<<3);
							}
						break;
		case 3:	//SOUTH
							if((robot_pos_y+2) < MAZE_SIZE_Y)
							{
								if(set)
									maze[robot_pos_x][robot_pos_y+1][robot_pos_z] |= ((int32_t) 1<<5);
								else
									maze[robot_pos_x][robot_pos_y+1][robot_pos_z] &= ~((int32_t) 1<<5);
							}
						break;
		case 4:	//WEST
							if((robot_pos_x+2) < MAZE_SIZE_X)
							{
								if(set)
									maze[robot_pos_x+1][robot_pos_y][robot_pos_z] |= ((int32_t) 1<<7);
								else
									maze[robot_pos_x+1][robot_pos_y][robot_pos_z] &= ~((int32_t) 1<<7);
							}
						break;
	}
}

//gibt den Status für ,,Last" des Eingangs der Fliese in dir zurück
int8_t maze_getLast(uint8_t dir)
{
	if(dir > WEST)
		dir -= WEST;

	switch(dir)
	{
		case 1:	//Platte Norden aber Element Süden!
							if(robot_pos_y+2 < MAZE_SIZE_Y)
								return ((maze[robot_pos_x][robot_pos_y+1][robot_pos_z] & 0x00000020)>>5);
							else
								return 0; //Da wäre das Ende des Speichers => Bei fehler würde nicht von Karte fahren und nicht rechter Wand folgen.
						break;
		case 2:	//Platte Osten aber Element Westen!
							if(robot_pos_x+2 < MAZE_SIZE_X)
								return ((maze[robot_pos_x+1][robot_pos_y][robot_pos_z] & 0x00000080)>>7);
							else
								return 0;
						break;
		case 3:	//Platte Süden aber Element Norden!
							if((robot_pos_y-1) >= 0)
								return ((maze[robot_pos_x][robot_pos_y-1][robot_pos_z] & 0x00000002)>>1);
							else
								return 0;
						break;
		case 4:	//Platte Westen aber Element Osten!
							if((robot_pos_x-1) >= 0)
								return ((maze[robot_pos_x-1][robot_pos_y][robot_pos_z] & 0x00000008)>>3);
							else
								return 0;
						break;
		default:
							return 1;
						break;
	}
}

////Schwarze Fliesen (Sackgassen)
void maze_posSetBlacktile(void)
{
	maze_setStop(robot_pos_x, robot_pos_y, robot_pos_z, NORTH); //Setze diese Fliese komplett auf Stop
	maze_setStop(robot_pos_x, robot_pos_y, robot_pos_z, EAST);
	maze_setStop(robot_pos_x, robot_pos_y, robot_pos_z, SOUTH);
	maze_setStop(robot_pos_x, robot_pos_y, robot_pos_z, WEST);

	//Setze Eingänge vorne, links und rechts auf Stop, damit Roboter nur in die Richtung rausfährt, in die er reingefahren ist.
	switch(robot_dir)
	{
		case 1:		maze_setStop(robot_pos_x-1, robot_pos_y, robot_pos_z, robot_dir+3);
							maze_setStop(robot_pos_x+1, robot_pos_y, robot_pos_z, robot_dir+1);
							maze_setStop(robot_pos_x, robot_pos_y+1, robot_pos_z, robot_dir);
						break;
		case 2:		maze_setStop(robot_pos_x, robot_pos_y+1, robot_pos_z, robot_dir+3);
							maze_setStop(robot_pos_x, robot_pos_y-1, robot_pos_z, robot_dir+1);
							maze_setStop(robot_pos_x+1, robot_pos_y, robot_pos_z, robot_dir);
						break;
		case 3:		maze_setStop(robot_pos_x+1, robot_pos_y, robot_pos_z, robot_dir+3);
							maze_setStop(robot_pos_x-1, robot_pos_y, robot_pos_z, robot_dir+1);
							maze_setStop(robot_pos_x, robot_pos_y-1, robot_pos_z, robot_dir);
						break;
		case 4:		maze_setStop(robot_pos_x, robot_pos_y-1, robot_pos_z, robot_dir+3);
							maze_setStop(robot_pos_x, robot_pos_y+1, robot_pos_z, robot_dir+1);
							maze_setStop(robot_pos_x-1, robot_pos_y, robot_pos_z, robot_dir);
						break;
	}
}

//Gibt die Anzahl der Wände auf der Fliese zurück
uint8_t maze_posCountWall(void)
{
	uint8_t wallCount = 0;

	if(maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, NORTH) > MAZE_ISWALL)
	{
		wallCount ++;
	}
	if(maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, EAST) > MAZE_ISWALL)
	{
		wallCount ++;
	}
	if(maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, SOUTH) > MAZE_ISWALL)
	{
		wallCount ++;
	}
	if(maze_getWall(robot_pos_x, robot_pos_y, robot_pos_z, WEST) > MAZE_ISWALL)
	{
		wallCount ++;
	}
	
	return wallCount;
}

//Nach Positionsänderung (z.B. LOP -> Eingriff) ist Karte unbrauchbar => zurücksetzen
void maze_clear(void)
{
	for(uint8_t maze_z = 0; maze_z < MAZE_SIZE_Z; maze_z++)
	{
		for(uint8_t maze_y = 0; maze_y < MAZE_SIZE_Y; maze_y++)
		{
			for(uint8_t maze_x = 0; maze_x < MAZE_SIZE_X; maze_x++)
			{
				//x beschreibt den Ursprung einer Grundfliese. Ein Wandspeicherplatz existiert jeweils unten und rechts.
				maze[maze_x][maze_y][maze_z] = 0;
			}
		}
	}
}
//////////////////////////////////////////////////

#define MAPSTART_E1_X 0
#define MAPSTART_E1_Y 63
#define MAPEND_E1_X 60
#define MAPEND_E1_Y 38

#define MAPSTART_E2_X 0
#define MAPSTART_E2_Y 33
#define MAPEND_E2_X 60
#define MAPEND_E2_Y 8

#define WALL_SIZE_E1 (MAPEND_E1_X/MAZE_SIZE_X)
#define WALL_SIZE_E2 (MAPEND_E2_X/MAZE_SIZE_X)

void u8g_DrawMaze(void)
{
	///////Infos//////////////////
	//  Karte   //Pfeil// Frei  //
	//  akt.    //  X  // hier: //
	//  Raum    /////////       //
	//          //Info //       //
	//          //Karte//       //
	//////////////////////////////

	switch(robot_dir)
	{
		case 1: //NORTH
							u8g_drawArrow(25, 73, 21, NORTH);
						break;
		case 2: //EAST
							u8g_drawArrow(25, 73, 21, WEST);
						break;
		case 3: //SOUTH
							u8g_drawArrow(25, 73, 21, SOUTH);
						break;
		case 4: //WEST
							u8g_drawArrow(25, 73, 21, EAST);
						break;
	}

	u8g_DrawHLine(&u8g, 60, 36, 28); //Horizontale Abgrenzung

	///////Infos//////////////////
	//  Karte   //Pfeil// Frei  //
	//          //     // hier: //
	//          /////////       //
	//    X     //Info //       //
	//          //Karte//       //
	//////////////////////////////

	u8g_DrawVLine(&u8g, 59, 7, 57); //Vertikale Abgenzung

  int8_t disp_x = MAPSTART_E1_X;
	int8_t disp_y = MAPSTART_E1_Y;
	for(uint8_t maze_y = 0; maze_y < MAZE_SIZE_Y; maze_y++) //Etage 1, unterer Displaybereich
	{
		for(uint8_t maze_x = 0; maze_x < MAZE_SIZE_X; maze_x++)
		{
			//x beschreibt den Ursprung einer Grundfliese. Ein Wandspeicherplatz existiert jeweils unten und rechts.
			u8g_DrawPixel(&u8g, disp_x, disp_y);

			if((robot_pos_x == maze_x) && (robot_pos_y == maze_y) && (robot_pos_z == 0)) //Wo befindet sich der Roboter?
			{
				u8g_drawArrow(WALL_SIZE_E1/2, disp_x+(WALL_SIZE_E1/2), disp_y-(WALL_SIZE_E1/2), robot_dir);
			}

			if(maze_getWall(maze_x, maze_y, 0, SOUTH) > MAZE_ISWALL)
			{
				u8g_DrawHLine(&u8g, disp_x, disp_y, WALL_SIZE_E1);
			}
			if(maze_getWall(maze_x, maze_y, 0, WEST) > MAZE_ISWALL)
			{
				u8g_DrawVLine(&u8g, disp_x, disp_y-WALL_SIZE_E1, WALL_SIZE_E1);
			}
			if(maze_getGround(maze_x, maze_y, 0) > MAZE_ISBLTILE)
			{
				u8g_DrawBox(&u8g, disp_x+1, disp_y+1-WALL_SIZE_E1, WALL_SIZE_E1-1, WALL_SIZE_E1-1);
			}

			disp_x += WALL_SIZE_E1;
			if(disp_x >= (MAZE_SIZE_X * WALL_SIZE_E1))
			{
				disp_y -= WALL_SIZE_E1;
				disp_x = MAPSTART_E1_X;
			}
		}
	}
	if(MAZE_SIZE_Z == 2) //2. Etage
	{
		disp_x = MAPSTART_E2_X;
		disp_y = MAPSTART_E2_Y;
		for(uint8_t maze_y = 0; maze_y < MAZE_SIZE_Y; maze_y++) //Etage 1, unterer Displaybereich
		{
			for(uint8_t maze_x = 0; maze_x < MAZE_SIZE_X; maze_x++)
			{
				//x beschreibt den Ursprung einer Grundfliese. Ein Wandspeicherplatz existiert jeweils unten und rechts.
				u8g_DrawPixel(&u8g, disp_x, disp_y);

				if((robot_pos_x == maze_x) && (robot_pos_y == maze_y) && (robot_pos_z == 1)) //Wo befindet sich der Roboter?
				{
					u8g_drawArrow(WALL_SIZE_E2/2, disp_x+(WALL_SIZE_E2/2), disp_y-(WALL_SIZE_E2/2), robot_dir);
				}

				if(maze_getWall(maze_x, maze_y, 1, SOUTH) > MAZE_ISWALL)
				{
					u8g_DrawHLine(&u8g, disp_x, disp_y, WALL_SIZE_E2);
				}
				if(maze_getWall(maze_x, maze_y, 1, WEST) > MAZE_ISWALL)
				{
					u8g_DrawVLine(&u8g, disp_x, disp_y-WALL_SIZE_E2, WALL_SIZE_E2);
				}
				if(maze_getGround(maze_x, maze_y, 1) > MAZE_ISBLTILE)
				{
					u8g_DrawBox(&u8g, disp_x+1, disp_y+1-WALL_SIZE_E2, WALL_SIZE_E2-1, WALL_SIZE_E2-1);
				}

				disp_x += WALL_SIZE_E2;
				if(disp_x >= (MAZE_SIZE_X * WALL_SIZE_E2))
				{
					disp_y -= WALL_SIZE_E2;
					disp_x = MAPSTART_E2_X;
				}
			}
		}
	}

	///////Infos//////////////////
	//  Karte   //Pfeil// Frei  //
	//  akt.    //     // hier: //
	//  Raum    /////////       //
	//          //InfoX//       //
	//          //Karte//       //
	//////////////////////////////
	switch(incr_ok_mode)
	{
		case 0:		u8g_DrawStr(&u8g, 62, 43, "chg x");
						break;
		case 1:		u8g_DrawStr(&u8g, 62, 43, "chg y");
						break;
		case 2:		u8g_DrawStr(&u8g, 62, 43, "chg d");
						break;
		case 3:		u8g_DrawStr(&u8g, 62, 43, "ok");
						break;
	}
	u8g_DrawStr(&u8g, 62, 50, "x:"); u8g_DrawLong(70, 50, robot_pos_x);
	u8g_DrawStr(&u8g, 62, 57, "y:"); u8g_DrawLong(70, 57, robot_pos_y);
	u8g_DrawStr(&u8g, 62, 64, "z:"); u8g_DrawLong(70, 64, robot_pos_z);
}

///////////////////////////////////////
//MAPSAMPLE:
/*maze_corrGround(2, 0, 0, 30);

		maze_corrWall(0, 0, 0, WEST, 30);
		maze_corrWall(0, 1, 0, WEST, 30);
		maze_corrWall(0, 2, 0, WEST, 30);
		maze_corrWall(0, 3, 0, WEST, 30);

		maze_corrWall(0, 3, 0, NORTH, 30);
		maze_corrWall(1, 3, 0, NORTH, 30);
		maze_corrWall(2, 3, 0, NORTH, 30);
		maze_corrWall(3, 3, 0, NORTH, 30);

		maze_corrWall(0, 0, 0, SOUTH, 30);
		maze_corrWall(1, 0, 0, SOUTH, 30);
		maze_corrWall(2, 0, 0, SOUTH, 30);
		maze_corrWall(3, 0, 0, SOUTH, 30);
		
		maze_corrWall(3, 1, 0, EAST, 30);
		maze_corrWall(3, 2, 0, EAST, 30);
		maze_corrWall(3, 3, 0, EAST, 30);

		maze_corrWall(1, 0, 0, NORTH, 127);
		maze_corrWall(2, 0, 0, NORTH, 127);
		maze_corrWall(2, 0, 0, WEST, 30);

		maze_corrWall(1, 3, 0, WEST, 30);
		maze_corrWall(1, 3, 0, SOUTH, 30);
		maze_corrWall(3, 3, 0, SOUTH, 30);

	robot_pos_x = 3;*/
