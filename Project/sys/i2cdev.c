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
#include "i2cdev.h"
#include "main.h"


#define I2CBUF_SIZE 3
uint8_t i2cbuf[I2CBUF_SIZE];

const uint8_t mlx90614_i2c_addresses[NUMBER_OF_MLX] PROGMEM = {
    I2C_MLX90614_R,
    I2C_MLX90614_L
};

struct MLX_INFO mlx90614[NUMBER_OF_MLX];

uint8_t getIR(void)
{
    uint16_t tempdata = 0;
    uint8_t returnvar = 0;

    for(uint8_t i = 0; i < NUMBER_OF_MLX; i++)
    {
        if((i2c_start(pgm_read_byte(&mlx90614_i2c_addresses[i])) == 0) &&
           (i2c_write(I2C_REG_MLX90614) == 0) &&
           (i2c_rep_start(pgm_read_byte(&mlx90614_i2c_addresses[i]) + I2C_READ) == 0))
        {
            for(uint8_t i = 0; i<2; i++)
                i2cbuf[i] = i2c_readAck();
            i2cbuf[2] = i2c_readNak();

            if(i2c_stop() != 0)
                returnvar |= (1<<i);
            else
                returnvar &= ~(1<<i);

            // This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
            tempdata = (((i2cbuf[1] & 0x007F) << 8) + i2cbuf[0]);
            tempdata = ((tempdata * 2)-1);
            mlx90614[i].is = (tempdata - 27315);
        }
        else returnvar |= (1<<i);
    }

    return returnvar;
}

/////////////////////////SRF//////////////////////////////////////////////
#define SRF10_US_TO_MM 0.2 //Fakor zur Umrechnung: Laufzeit => mm

uint8_t srf_sm_manage = 0;

struct _SRF srf[NUMBER_OF_SRF];

const uint8_t srf_i2c_addresses[NUMBER_OF_SRF] PROGMEM = {
    I2C_SRF10//,
    //I2C_SRF10_R//,
    //I2C_SRF10_F
};

void init_srf10(void)
{
    srf_writeByte(I2C_SRF10, 1, 50); //Reinforcement (Register 1); 0..255 (max. reinforcement)
    //srf_writeByte(I2C_SRF10_R, 1, 50); //"

    srf_writeByte(I2C_SRF10, 2, 6); //Maximum reach (Register 2); 0..16: 43mm + 6*43mm = 301mm
    //srf_writeByte(I2C_SRF10_R, 2, 6); //"
}

uint8_t srf_writeByte(uint8_t adr, uint8_t reg, uint8_t val)
{
    uint8_t returnvar;

    if((i2c_start(adr+I2C_WRITE) == 0) &&
       (i2c_write(reg) == 0) &&
       (i2c_write(val) == 0) &&
       (i2c_stop() == 0))
    {
        returnvar = 1;
    }
    else
        returnvar = 0;

    return returnvar;
}

//////////SM///////////////////

uint8_t getSRF_returnvar = 0; //Muss global sein, da sich Sensoren sonst gegenseitig überschreiben

uint8_t getSRF(void)
{
    uint8_t lsb_srf = 0;
    uint8_t msb_srf = 0;

    switch(srf[srf_sm_manage].state)
    {
        case 0:
                if((i2c_start(pgm_read_byte(&srf_i2c_addresses[srf_sm_manage])) == 0) &&
                   (i2c_write(0x00) == 0) &&
                   (i2c_write(I2C_REG_SRF10_MEASERUEMENT_S) == 0) &&
                   (i2c_stop() == 0))
                {
                    srf[srf_sm_manage].state = 1; //Messung ok, nächster Schritt
                    getSRF_returnvar &= ~(1<<srf_sm_manage);
                }
                else
                {
                    srf[srf_sm_manage].state = 0; //Beim nächsten Durchlauf nochmal
                    getSRF_returnvar |= (1<<srf_sm_manage); //etwas hat nicht funktioniert

                    srf_sm_manage ++; //nächster Sensor
                    if(srf_sm_manage >= NUMBER_OF_SRF)
                        srf_sm_manage = 0;
                }
            break;

        case 1:
                if((i2c_start(pgm_read_byte(&srf_i2c_addresses[srf_sm_manage])) != 0) || //Abfragen: Ist die Messung beendet?
                   (i2c_write(0x00) != 0) ||
                   (i2c_rep_start(pgm_read_byte(&srf_i2c_addresses[srf_sm_manage])+I2C_READ) != 0) ||
                   (i2c_readNak() == 255) ||
                   (i2c_stop() != 0))
                {
                    srf[srf_sm_manage].state = 0;
                    getSRF_returnvar |= (1<<srf_sm_manage);

                    srf_sm_manage ++; //nächster Sensor
                    if(srf_sm_manage >= NUMBER_OF_SRF)
                        srf_sm_manage = 0;
                }
                else
                {
                    srf[srf_sm_manage].state = 2;
                }
            break;

        case 2:

                if((i2c_start(pgm_read_byte(&srf_i2c_addresses[srf_sm_manage])) == 0) && //Auslesen der ENtfernung
                   (i2c_write(0x02) == 0) &&
                   (i2c_rep_start(pgm_read_byte(&srf_i2c_addresses[srf_sm_manage])+I2C_READ) == 0))
                {
                    lsb_srf = i2c_readAck();
                    msb_srf = i2c_readNak();
                    srf[srf_sm_manage].dist = (((uint16_t)lsb_srf << 8) | msb_srf);// * SRF10_US_TO_MM;
                    if(i2c_stop() != 0)
                        getSRF_returnvar |= (1<<srf_sm_manage);
                }
                else
                {
                    getSRF_returnvar |= (1<<srf_sm_manage);
                }

                srf[srf_sm_manage].state = 0;

                srf_sm_manage ++; //nächster Sensor
                if(srf_sm_manage >= NUMBER_OF_SRF)
                    srf_sm_manage = 0;
            break;
    }

    return getSRF_returnvar;
}

//SRF Adresse ändern:
/*
uint8_t adr_old = 0xE0;
uint8_t adr_new = 0xE2;

i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xA0); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xAA); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(0xA5); //Adresse ändern
i2c_stop();
i2c_start(adr_old); //Aktuelle Adr
i2c_write(0x00); //Adresse ändern
i2c_write(adr_new); //Adresse ändern
i2c_stop();
*/
