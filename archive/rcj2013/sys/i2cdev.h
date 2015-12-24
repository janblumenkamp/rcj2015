#include "i2cmaster.h"

#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//I²C Adressen
#define I2C_MLX90614_L 0x5C<<1
#define I2C_MLX90614_R 0x5A<<1
//MLX Schwarz Kiste: 0x5B
//MLX Silber 1: 0x5C
//MLX Silber 2: 0x5D
//E0 vorgesehen evtl. für vorhandenen SRF02
#define I2C_SRF10_F 0xE2
#define I2C_SRF10_R 0xE4
#define I2C_SRF10_B 0xE6
#define I2C_SRF10_L 0xE8

//Commands für I²C
#define I2C_REG_MLX90614 0x07
#define I2C_REG_SRF10_MEASERUEMENT_S 0x52 //Laufzeit in µS

//I²C 
extern int32_t displayvar[3];

//IR (MLX90614)
extern int16_t mlx90614_l;
extern int16_t mlx90614_r;

//SRF
extern uint8_t srf10_state[4];
extern uint16_t srf10[4];

extern const uint8_t FRONT;
extern const uint8_t BACK;
extern const uint8_t LEFT;
extern const uint8_t RIGHT;

/////////////////////////////////////////////////////

extern uint8_t getIR(void);

//mode: 0: reihe
//mode: 1-4: jew.sen. (l=>r)
//mode: 5: l/r
//mode: 6: vl/vr
extern uint16_t getSRF(void);
