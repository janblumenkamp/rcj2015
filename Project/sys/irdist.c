////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
/////////////////////////////////i2cdev.c///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//    Abfrage/Management der I²C Geräte (Devices)
//    - Temperatursensoren
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "i2cmaster.h"
#include "irdist.h"
#include "main.h"
#include "funktionen.h"

void irDist_init(void)
{
    irDist_setSensorStandby(IRDIST_I2CADR_B_B, 0);
    irDist_setSensorStandby(IRDIST_I2CADR_F_F, 0);
    irDist_setSensorStandby(IRDIST_I2CADR_B_L, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_B_R, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_F_L, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_F_R, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_R_F, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_R_B, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_L_B, 1);
    irDist_setSensorStandby(IRDIST_I2CADR_L_F, 1);
}

void irDist_setSensorStandby(uint8_t adr, uint8_t state)
{
    if(state)
        state = 1;

    i2c_start(adr);
    i2c_write(0xE8);
    i2c_write(state);
    i2c_stop();
}

int16_t irDist_getSensorDist(uint8_t adr, uint8_t offset)
{
    int16_t dist = -1;

    if((i2c_start(adr) == 0) && //Auslesen der Entfernung
       (i2c_write(0x5E) == 0) &&
       (i2c_rep_start(adr + I2C_READ) == 0))
    {
        int16_t lsb = i2c_readAck();
        int16_t msb = i2c_readNak();

        dist = (((lsb << 4) + msb)*10)/64;
    }
    i2c_stop();

    if(dist < -1)
        dist = IRDIST_MAX;
    else
        dist -= offset;

    return dist;
}

//////////SM///////////////////

uint8_t sm = 0;

uint8_t irDist_get(void)
{
    switch (sm) {
        case 0:
            //Mid
            dist[LIN][FRONT][FRONT] = irDist_getSensorDist(IRDIST_I2CADR_F_F, IRDIST_OFFSET);
            dist[LIN][BACK][BACK] = irDist_getSensorDist(IRDIST_I2CADR_B_B, 12);

            irDist_setSensorStandby(IRDIST_I2CADR_F_F, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_B_B, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_B_L, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_F_R, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_R_B, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_L_F, 0);

            sm = 1;

            break;
        case 1:
            //Right
            dist[LIN][BACK][LEFT] = irDist_getSensorDist(IRDIST_I2CADR_B_L, IRDIST_OFFSET);
            dist[LIN][FRONT][RIGHT] = irDist_getSensorDist(IRDIST_I2CADR_F_R, IRDIST_OFFSET);
            dist[LIN][RIGHT][BACK] = irDist_getSensorDist(IRDIST_I2CADR_R_B, IRDIST_OFFSET);
            dist[LIN][LEFT][FRONT] = irDist_getSensorDist(IRDIST_I2CADR_L_F, IRDIST_OFFSET);

            irDist_setSensorStandby(IRDIST_I2CADR_B_L, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_F_R, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_R_B, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_L_F, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_B_R, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_F_L, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_R_F, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_L_B, 0);

            sm = 2;

            break;
        case 2:
            //Right
            dist[LIN][BACK][RIGHT] = irDist_getSensorDist(IRDIST_I2CADR_B_R, IRDIST_OFFSET);
            dist[LIN][FRONT][LEFT] = irDist_getSensorDist(IRDIST_I2CADR_F_L, IRDIST_OFFSET);
            dist[LIN][RIGHT][FRONT] = irDist_getSensorDist(IRDIST_I2CADR_R_F, IRDIST_OFFSET);
            dist[LIN][LEFT][BACK] = irDist_getSensorDist(IRDIST_I2CADR_L_B, IRDIST_OFFSET);

            irDist_setSensorStandby(IRDIST_I2CADR_B_R, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_F_L, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_R_F, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_L_B, 1);
            irDist_setSensorStandby(IRDIST_I2CADR_B_B, 0);
            irDist_setSensorStandby(IRDIST_I2CADR_F_F, 0);

            sm = 0;

            break;
        default:
            break;
    }

    if(um6.isRamp) //If on ramp set all front/ back sensors to max distance to not align
    {
        dist[LIN][BACK][LEFT] = IRDIST_MAX;
        dist[LIN][BACK][BACK] = IRDIST_MAX;
        dist[LIN][BACK][RIGHT] = IRDIST_MAX;
    }

    return 0;
}
