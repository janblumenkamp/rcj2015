#include <avr/io.h>
#include <avr/pgmspace.h> 	// Program memory (=Flash ROM) access routines.
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uart.h"

extern volatile int8_t timer_bt_is_busy;


extern void bt_putCh(const uint8_t ch);

extern void bt_putStr(const char *s);

extern void bt_putLong(int32_t num);
