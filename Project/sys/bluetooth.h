////////////////////////////////////////////////////////////////////////////////
//////////////////////////////teamohnename.de///////////////////////////////////
///////////////////////////RoboCup Junior 2014//////////////////////////////////
///////////////////////////////bluetooth.h//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//    siehe bluetooth.c
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/pgmspace.h>     // Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_STRING_SIZE     256

//////////////////////////////////////////

typedef struct {
    uint8_t active; //Stream active or not?
    uint8_t textcolor; //VT100 terminal command colors
    uint8_t bgcolor;
    int8_t (*put_c)(unsigned char c); //Called if next char of given stream is processed
} stream_t;

extern stream_t str_pcui;
extern stream_t str_debug;
extern stream_t str_debugOS;
extern stream_t str_debugDrive;
extern stream_t str_error;

extern void bt_init(void);

extern void bt_outOnOff(stream_t *stream, uint8_t state);

extern int8_t bt_putCh(unsigned char ch);

extern signed int vsnoutf(char *pStr, size_t length, const char *pFormat, va_list ap);

extern signed int snoutf(char *pString, size_t length, const char *pFormat, ...);

extern signed int vsoutf(char *pString, const char *pFormat, va_list ap);

extern signed int vfoutf(stream_t *pStream, const char *pFormat, va_list ap);

extern signed int foutf(stream_t *pStream, const char *pFormat, ...);

extern signed int soutf(char *pStr, const char *pFormat, ...);

extern void out_puts_l(stream_t *pStream, const char *pStr, uint16_t len);

extern signed int out_n_fputc(signed int c);

extern signed int out_fputs(const char *pStr, stream_t *pStream);
