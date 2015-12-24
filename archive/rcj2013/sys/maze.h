#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "funktionen.h"
#include "display.h"

extern const uint8_t FRONT;
extern const uint8_t BACK;
extern const uint8_t LEFT;
extern const uint8_t RIGHT;
extern uint16_t wallsensor[4][4];
extern uint16_t wallsensor_top[4];

extern const uint8_t NONE; //Richtung (Orientierung), siehe auch DRIVE.C!!!!!
extern const uint8_t NORTH;
extern const uint8_t EAST;
extern const uint8_t SOUTH;
extern const uint8_t WEST;

extern volatile int8_t timer_add_var_drv;
extern volatile int8_t timer_incr_entpr;

extern int16_t speed_l;
extern int16_t speed_r;
extern uint8_t motor_off;
extern uint8_t use_map;
extern uint8_t use_tarry;

extern int8_t incr_delta;

extern uint16_t srf10_l; //Sensorwert
extern uint16_t srf10_vl; //Sensorwert
extern uint16_t srf10_vr; //Sensorwert
extern uint16_t srf10_r; //Sensorwert

extern uint16_t srf10_l_mm; //Sensorwert
extern uint16_t srf10_vl_mm; //Sensorwert
extern uint16_t srf10_vr_mm; //Sensorwert
extern uint16_t srf10_r_mm; //Sensorwert

extern int32_t enc_l; //Encoder
extern int32_t enc_r;

extern int16_t um6_phi;
extern int16_t um6_theta;
extern int16_t um6_psi;
extern int32_t um6_psi_t;

extern int8_t robot_dir;

////////////////////////////////////////

extern uint8_t solve_maze(void);

extern void maze_updateWall(int8_t updateFac_wall);

extern void maze_updateGround(int8_t updateFac_ground);

extern void maze_corrWall(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, uint8_t dir, int8_t value);

extern int8_t maze_getWall(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, uint8_t dir);

extern void maze_corrGround(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z, int8_t value);

extern int8_t maze_getGround(uint8_t pos_x, uint8_t pos_y, uint8_t pos_z);

extern void maze_setStop(int8_t pos_x, int8_t pos_y, int8_t pos_z, int8_t dir);

extern int8_t maze_getStop(uint8_t dir);

extern void maze_setLast(uint8_t set);

extern int8_t maze_getLast(uint8_t dir);

extern void u8g_DrawMaze(void);

extern void maze_posSetBlacktile(void);

extern uint8_t maze_posCountWall(void);

extern void maze_clear(void);
