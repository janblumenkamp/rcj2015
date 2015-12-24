/* ****************************************************************************
 *                           _______________________
 *                           \| RP6  ROBOT SYSTEM |/
 *                            \_-_-_-_-_-_-_-_-_-_/             >>> RP6 CONTROL
 * ----------------------------------------------------------------------------
 * ------------------- [c]2006 / 2007 - AREXX ENGINEERING ---------------------
 * -------------------------- http://www.arexx.com/ ---------------------------
 * ****************************************************************************
 * File: RP6ControlLib.c
 * Version: 1.1
 * Target: RP6 CONTROL - ATMEGA32 @16.00MHz
 * Author(s): Dominik S. Herwald
 * ****************************************************************************
 * Description:
 * The RP6 CONTROL M32 function Library.
 *
 * ****************************************************************************
 * CHANGELOG AND LICENSING INFORMATION CAN BE FOUND AT THE END OF THIS FILE!
 * ****************************************************************************
 */

/*****************************************************************************/
// Includes:

#include "RP6ControlLib.h"


/*****************************************************************************/
// External Interrupts and Event Handlers.
// To allow common usage with the RP6Control library, you need to use
// Event Handlers. 
// Please make sure that you keep the Event Handler Functions as
// short as possible! 
// --> You usually don't need this stuff. 


extern void timerControl(void);

void INT0_event_DUMMY(void){}
static void (*INT0_eventHandler)(void) = INT0_event_DUMMY;
void INT0_setEventHandler(void (*i0eventHandler)(void))
{
	INT0_eventHandler = i0eventHandler;
}

/**
 * External Interrupt 0 ISR
 */
ISR (INT0_vect)
{
	INT0_eventHandler();
}

void INT1_event_DUMMY(void){}
static void (*INT1_eventHandler)(void) = INT1_event_DUMMY;
void INT1_setEventHandler(void (*i1eventHandler)(void))
{
	INT1_eventHandler = i1eventHandler;
}

/**
 * External Interrupt 1 ISR
 */
ISR (INT1_vect)
{
	INT1_eventHandler();
}

void INT2_event_DUMMY(void){}
static void (*INT2_eventHandler)(void) = INT2_event_DUMMY;
void INT2_setEventHandler(void (*i2eventHandler)(void))
{
	INT2_eventHandler = i2eventHandler;
}

/**
 * External Interrupt 2 ISR
 */
ISR (INT2_vect)
{
	INT2_eventHandler();
}

/*****************************************************************************/
// ADC:

/**
 * Read ADC channel (10 bit -> result is an integer from 0 to 1023).
 * The channels (ADC_BAT etc.) are defined in the RP6Control.h file.
 *
 * This is a blocking function, which means it waits until the conversion
 * is complete. 
 *
 */
uint16_t readADC(uint8_t channel)
{
	if((ADCSRA & (1<<ADSC))) return 0; // check if ADC is buisy...
	ADMUX = (1<<REFS0) | (0<<REFS1) | (channel<<MUX0);
	ADCSRA = (0<<ADIE) | (1<<ADSC) | (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADIF);
	while ((ADCSRA & (1<<ADSC))); 
	ADCSRA |= (1<<ADIF);
	return ADC;
}

/*****************************************************************************/
// Hardware SPI Interface:

/**
 * Writes a single Databyte to the SPI Interface.
 */
void writeSPI(uint8_t data) 
{     
	SPDR = data;    
	while(!(SPSR & (1<<SPIF)));
}

/**
 * Reads a single Databyte from the SPI Interface.
 */
uint8_t readSPI(void)
{
	writeSPI(0xFF);
	return SPDR;
}

/**
 * Reads TWO Bytes from the SPI Interface and returns them as
 * a 16 Bit value with first byte read in the upper 8 bits.
 */
uint16_t readWordSPI(void)
{
	uint16_t data = 0;
	data = readSPI() << 8;
	data |= readSPI();
	return data;
}

/**
 * Writes two Bytes contained in the 16 Bit parameter "data".
 * The first byte to be written needs to be in the upper 8 Bits.
 */
void writeWordSPI(uint16_t data)
{
	writeSPI((uint8_t)(data >> 8));
	writeSPI((uint8_t)data);
}

/** 
 * This function writes up to 255 Bytes to the SPI Interface.
 * The numer of bytes in the Buffer that shall be written is given 
 * by the parameter length.
 */
void writeBufferSPI(uint8_t *buffer, uint8_t length)
{
	uint8_t i = 0;
	for(; i < length; i++) {
		SPDR = buffer[i];    
		while(!(SPSR & (1<<SPIF)));
	}
}

/** 
 * Reads "length" Bytes from SPI Interface into the buffer.
 */
void readBufferSPI(uint8_t *buffer, uint8_t length)
{
	uint8_t i = 0;
	for(; i < length; i++) {
		SPDR = 0xFF;    
		while(!(SPSR & (1<<SPIF)));
		buffer[i] = SPDR;
	}
}

/*****************************************************************************/
// External 32KB SPI EEPROM:

/**
 * Reads a single Byte from the external EEPROM.
 */
uint8_t SPI_EEPROM_readByte(uint16_t memAddr)
{
	uint8_t data;
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_READ);
	writeWordSPI(memAddr);
	data = readSPI();
	PORTB |= MEM_CS;
	return data;
}

/**
 * Reads "length" Bytes into the Buffer "buffer" from startAdr on. 
 * You can read the complete EEPROM into a buffer at once - if it is large enough. 
 * (But you only have 2KB SRAM on a MEGA32 ;) )
 */
void SPI_EEPROM_readBytes(uint16_t startAddr, uint8_t *buffer, uint8_t length)
{
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_READ);
	writeWordSPI(startAddr);
	readBufferSPI(&buffer[0], length);
	PORTB |= MEM_CS;
}

/**
 * Enable Write Mode
 */
void SPI_EEPROM_enableWrite(void)
{
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_WREN);
	PORTB |= MEM_CS;
}

/**
 * Disable Write Mode
 */
void SPI_EEPROM_disableWrite(void)
{
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_WRDI);
	PORTB |= MEM_CS;
}

/**
 * Write a single data byte to the specified EEPROM address.
 */
void SPI_EEPROM_writeByte(uint16_t memAddr, uint8_t data)
{
	while(SPI_EEPROM_getStatus() & SPI_EEPROM_STAT_WIP);
	SPI_EEPROM_enableWrite();
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_WRITE);
	writeWordSPI(memAddr);
	writeSPI(data);
	PORTB |= MEM_CS;
}

/**
 * Write "length" Bytes from the Buffer to the EEPROM. 
 * YOU CAN ONLY WRITE MAXIMAL 64 BYTES AT ONCE!!! This is the Pagesize!
 * You can NOT cross a page boundary! For example when you write 20 Bytes 
 * starting at address 54, you will not write up to Byte address 74, but
 * instead only up to 63 and then it continues at Byte 0 and writes 
 * the rest up to Byte 10!
 *
 */
void SPI_EEPROM_writeBytes(uint16_t startAddr, uint8_t *buffer, uint8_t length)
{
	while(SPI_EEPROM_getStatus() & SPI_EEPROM_STAT_WIP);
	SPI_EEPROM_enableWrite();
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_WRITE);
	writeWordSPI(startAddr);
	writeBufferSPI(&buffer[0], length);
	PORTB |= MEM_CS;
}

/**
 * Returns EEPROM Status register - for checking if EEPROM is buisy. 
 * Writing takes about 5ms. 
 */
uint8_t SPI_EEPROM_getStatus(void)
{
	uint8_t status;
	PORTB &= ~MEM_CS;
	writeSPI(SPI_EEPROM_RDSR);
	status = readSPI();
	PORTB |= MEM_CS;
	return status;
}

/*****************************************************************************/
// External Port - LEDs and LCD:

// A shadow register for the external shift register:
externalPort_t externalPort;

/**
 * This routine outputs the value in the shadow register "externalPort" 
 * to the external 8bit shift register on the RP6 Control board with the 
 * hardware SPI module.
 * The lower four bits are connected to the four LEDs, and the upper 
 * to the 4bit LCD databus. (The LCD can be configured to operate
 * in 4 or 8bit mode - we use 4bit mode to save portpins) 
 *
 */
void outputExt(void)
{      
  writeSPI(externalPort.byte);   
  PORTD |= STR;
  nop();
  nop();
  PORTD &= ~STR;        
}

/*****************************************************************************/
// LEDs:

/**
 * Sets the LEDs - this does not affect the other four port lines that
 * are connected to the LCD! 
 *
 */
void setLEDs(uint8_t leds)
{
	externalPort.LEDS = leds;
	outputExt();
}

/*****************************************************************************/
// LCD
// All LCD routines are prepared to control a 2x16 character LCD.
// If you want to connect a bigger LCD you need to change some things in 
// these routines! (especially in the initLCD, setCursorLCD and showScreenLCD
// routines)

char lcd_tmp_buffer[17];

/**
 * Sets the LCD ports without affecting the LEDs and also pulses the
 * enable line of the LCD to 'inform' the LCD about the new data.
 *
 */
void setLCDD(uint8_t lcdd)
{
	externalPort.LCDD = lcdd;
	outputExt();
	PORTB |= LCD_EN;
	delayCycles(50);
	PORTB &= ~LCD_EN;
}

/**
 * Initialize the LCD. Always call this before using the LCD! 
 *
 */
void initLCD(void)
{
	//delayCycles(34000); No need for Power ON delay as usually the
	// Bootloader should have been executed before...
	setLCDD(0b0011);
	delayCycles(18000);
	setLCDD(0b0011);
	delayCycles(5500);
	setLCDD(0b0011);
	delayCycles(5500);
	setLCDD(0b0010);
	delayCycles(5500);
	writeLCDCommand(0b00101000);
	delayCycles(5500);
	writeLCDCommand(0b00001000);
	delayCycles(5500);
	writeLCDCommand(0b00000001);
	delayCycles(5500);
	writeLCDCommand(0b00000010);
	delayCycles(5500);
	writeLCDCommand(0b00001100);
	delayCycles(5500);
}

/**
 * Write a 8bit-byte in two nibbles of 4bit to the LCD.
 */
void write4BitLCDData(uint8_t data)
{
	setLCDD(data >> 4);
	setLCDD(data);
	delayCycles(150);
}

/**
 * Write a command to the LCD.
 */
void writeLCDCommand(uint8_t cmd)
{
	PORTB &= ~LCD_RS;
	write4BitLCDData(cmd);
	delayCycles(150);
}

/**
 * Clears the whole LCD!
 */
void clearLCD(void)
{
	writeLCDCommand(0b00000001);
	delayCycles(5500);
}


/**
 * Write a single character to the LCD.
 *
 * Example:
 *
 *			writeCharLCD('R');
 *			writeCharLCD('P');
 *			writeCharLCD('6');
 *			writeCharLCD(' ');
 *			writeCharLCD('0');
 *			writeCharLCD(48); // 48 is ASCII code for '0'
 *			writeCharLCD(49); // '1'
 *			writeCharLCD(50); // '2'
 *			writeCharLCD(51); // '3'
 *			//...
 *
 *			would output:
 *			RP6 00123
 *			at the current cursor position!
 *			use setCursorPos function to move the cursor to a 
 *			different location!
 */
void writeCharLCD(uint8_t ch)
{
	PORTB |= LCD_RS;
	write4BitLCDData(ch);
	delayCycles(50);
}

/**
 * Writes a null terminated string from flash program memory to the LCD.
 * You can use the macro writeStringLCD_P(STRING); instead, this macro
 * ensures that the String is stored in program memory only!
 *
 * Example:
 *
 *			writeNStringLCD_P(PSTR("RP6 Control"));
 *
 *			// There is also a Macro that makes life easier and
 *			// you can simply write:
 *			writeStringLCD_P("RP6 Control");
 *
 */
void writeNStringLCD_P(const char *pstring)
{
    uint8_t c;
    for (;(c = pgm_read_byte_near(pstring++));writeCharLCD(c));
}

/**
 * Writes a String from SRAM to the LCD.
 */
void writeStringLCD(char *string)
{
	while(*string)
		writeCharLCD(*string++);
}

/**
 * Writes a string with specified length and offset from SRAM to the LCD.
 * If it is a null terminated string, output will be stopped at the
 * end. It does not need to be null terminated, but it is recommended
 * to use only null terminated strings/buffers, otherwise the function could
 * output any SRAM memory data stored after the string until it reaches a 0
 * or the specified length!
 *
 * Example:
 *
 *			writeStringLength("RP6 Robot Sytem",16,0);
 *			// would output: "RP6 Robot Sytem\n"
 *			writeStringLength("RP6 Robot Sytem",11,4);
 *			// would output: "Robot System"
 * 			writeStringLength("RP6 Robot Sytem",40,4);
 *			// would output: "Robot System"
 *			// No matter if the specified length is 40 characters!
 *
 */
void writeStringLengthLCD(char *string, uint8_t length, uint8_t offset)
{
	for(string = &string[offset]; *string && length; length--)
		writeCharLCD(*string++);
}

/**
 * Write a number (with specified base) to the LCD.
 *
 * Example:
 *
 *			// Write a hexadecimal number to the LCD:
 *			writeInteger(0xAACC,16);
 *			// Instead of 16 you can also write "HEX" as this is defined in the
 *			// RP6RobotBaseLib.h :
 *			writeInteger(0xAACC, HEX);
 *			// Other Formats:
 *			writeInteger(1024,DEC);  	// Decimal
 *			writeInteger(511,OCT);		// Ocal
 *			writeInteger(0b11010111,BIN); // Binary
 */
void writeIntegerLCD(int16_t number, uint8_t base)
{
	itoa(number, &lcd_tmp_buffer[0], base);
	writeStringLCD(&lcd_tmp_buffer[0]);
}

/**
 * Same as writeInteger, but with defined length.
 * This means this routine will add leading zeros to the number if length is
 * larger than the actual value or cut the upper digits if length is smaller
 * than the actual value.
 *
 * Example:
 *
 *			// Write a hexadecimal number to the LCD:
 *			writeIntegerLength(0xAACC, 16, 8);
 *			// Instead of 16 you can also write "HEX" as this is defined in the
 *			// RP6ControlLib.h :
 *			writeIntegerLength(0xAACC, HEX, 8);
 *			// Other Formats:
 *			writeIntegerLength(1024,DEC,6);  	// Decimal
 *			writeIntegerLength(511,OCT,4);		// Ocal
 *			writeIntegerLength(0b11010111,BIN,8); // Binary
 */
 
void writeIntegerLengthLCD(int16_t number, uint8_t base, uint8_t length)
{
	char buffer[17];
	itoa(number, &buffer[0], base);
	int8_t cnt = length - strlen(buffer);
	if(cnt > 0) {
		for(; cnt > 0; cnt--, writeCharLCD('0'));
		writeStringLCD(&buffer[0]);
	}
	else 
		writeStringLengthLCD(&buffer[0],length,-cnt);
}

/**
 * This function is useful for displaying text screens on the LCD.
 * It clears the whole LCD and writes the two Strings to line 1 and
 * line 2.
 */
void _showScreenLCD_P(const char *line1, const char *line2)
{
	clearLCD();
	writeNStringLCD_P(line1);
	setCursorPosLCD(1, 0);
	writeNStringLCD_P(line2);
}

/**
 * Sets the cursor position on LCD.
 */
void setCursorPosLCD(uint8_t line, uint8_t pos)
{
	pos |= 128;
	if(line==1) pos += 0x40;
	writeLCDCommand(pos);
}

/**
 * Clears some characters after the given position.
 */
void clearPosLCD(uint8_t line, uint8_t pos, uint8_t length)
{
	setCursorPosLCD(line,pos);
	while(length--)
		writeCharLCD(' ');
}

/*****************************************************************************/
// Keypad:

/**
 * Checks which key is pressed - returns the key number,
 * or 0, if no key is pressed.
 * Maybe you need to adjust these values because of variations
 * in the resitors of the keypad!
 *
 */
uint8_t getPressedKeyNumber(void)
{
	uint16_t keys;
	keys = readADC(ADC_KEYPAD);
	if(keys < 1020) {
		nop();
		nop();
		nop();
		keys += readADC(ADC_KEYPAD);
		keys >>= 1;
	}
	if(keys < 50)
		return 1;
	if(keys < 580)
		return 2;
	if(keys < 700)
		return 3;
	if(keys < 790)
		return 4;
	if(keys < 830)
		return 5;
	return 0;
}

/**
 * This function has to be called frequently out of
 * the main loop and checks if a button is pressed! It only returns 
 * the key number a single time, DIRECTLY when the button is pressed.
 * 
 * This is useful for non-blocking keyboard check in the
 * main loop. You don't need something like 
 * "while(getPressedKeyNumber());" to wait for the button
 * to be released again!
 */
uint8_t checkPressedKeyEvent(void)
{
	static uint8_t pressed_key = 0;
	if(pressed_key) {
		if(!getPressedKeyNumber()) 
			pressed_key = 0;
	}
	else {
		pressed_key = getPressedKeyNumber();
		if(pressed_key)
			return pressed_key;
	}
	return 0;
}

/**
 * This function has to be called frequently out of
 * the main loop and checks if a button is pressed AND
 * released. It only returns the key number a single time, 
 * AFTER the button has been released.
 * 
 * This is useful for non-blocking keyboard check in the
 * main loop. You don't need something like 
 * "while(getPressedKeyNumber());" to wait for the button
 * to be released again!
 */
uint8_t checkReleasedKeyEvent(void)
{
	static uint8_t released_key = 0;
	if(released_key) {
		if(!getPressedKeyNumber()) {
			uint8_t tmp = released_key;
			released_key = 0;
			return tmp;
		}
	}
	else
		released_key = getPressedKeyNumber();
	return 0;
}

/*****************************************************************************/
// Microphone:

/** 
 * This function discharges the Capacitor of the peak detection circuit 
 * used for the Microphone. This is required to remove any previous
 * charge from the capacitor. 
 */
void dischargePeakDetector(void)
{
	DDRA |= MIC;
	PORTA &= ~MIC;
	mSleep(1);
	DDRA &= ~MIC;
}

/**
 * Reads the Microphone peak detector and discharges it afterwards.
 * 
 */
uint16_t getMicrophonePeak(void)
{
	uint16_t tmp;
	tmp = readADC(ADC_MIC);
	if(tmp > 4) 
		dischargePeakDetector();
	return tmp;
}

/*****************************************************************************/
// Delays, Stopwatches and Beeper:


// ---------------------
// Internal status bits
volatile union {
	uint8_t byte;
	struct {
		unsigned beep:1;
		unsigned unused:7;
	};
} controlStatus;

volatile stopwatches_t stopwatches;
volatile uint8_t delay_timer;
volatile uint8_t ms_timer;
volatile uint16_t sound_timer;

volatile uint32_t timer; // You can use this timer for everything you like!


/**
 * Timer 0 Compare ISR - This timer is used for various
 * timing stuff: The delay timer for blocking delays, 
 * "Stopwatches" for non-blocking delays and the timing of
 * the sound generation with timer2...
 *
 * By default, it runs at 10kHz which means this ISR is called
 * every ~100탎! This is nice for timing stuff!
 */
ISR (TIMER0_COMP_vect)
{
	// 16bit timer (100탎 resolution)
	timer++;
	
	// Blocking delay (100탎):
	delay_timer++;
	
	// All 1ms based timing stuff
	if(ms_timer++ >= 10) { // 10 * 100탎 = 1ms
		// 16bit Stopwatches:
		if(stopwatches.watches & STOPWATCH1)
			stopwatches.watch1++;
		if(stopwatches.watches & STOPWATCH2)
			stopwatches.watch2++;
		if(stopwatches.watches & STOPWATCH3)
			stopwatches.watch3++;
		if(stopwatches.watches & STOPWATCH4)
			stopwatches.watch4++;
		if(stopwatches.watches & STOPWATCH5)
			stopwatches.watch5++;
		if(stopwatches.watches & STOPWATCH6)
			stopwatches.watch6++;
		if(stopwatches.watches & STOPWATCH7)
			stopwatches.watch7++;
		if(stopwatches.watches & STOPWATCH8)
			stopwatches.watch8++;

		// Sound generation timing:
		if(controlStatus.beep) {
			if(sound_timer < 1) { // sound_timer * 1ms
				TCCR2 = 0;
				controlStatus.beep = false;
			}
			else
				sound_timer--;
		}
		
		ms_timer = 0;
	}
}

volatile uint16_t timer1; 

ISR (TIMER1_COMPA_vect)
{
  timerControl();
}


/**
 * You can use this function to make the beeper beep ;) 
 * But this function should not be used as it does not
 * generate a delay for the sound and a delay between 
 * two sounds. Better is to use the "sound" macro, which
 * uses this function and adds the required delays.
 *
 * "sound(pitch,time,delay)"
 *
 * 0 = lowest frequency
 * 255 = highest frequency
 *
 * Example:
 * sound(150,50,25);
 * sound(200,50,25);
 *
 * Of course the function "beep" is nice to generate
 * sounds when you need to do other things at the same
 * time... 
 */
void beep(uint8_t pitch, uint16_t time)
{
	controlStatus.beep = true;
	sound_timer = time;
	OCR2 = 255-pitch;
	TCCR2 =  (1 << WGM21) | (1 << COM20) | (1 << CS22) | (1 << CS21);
}

/**
 * This function has no timing stuff, but otherwise
 * it has the same effect as "beep". It only sets the pitch
 * and this can be used to generate tone sequences which
 * would sound bad if the beeper turns of for a very short time
 * in between - such as alarm tones or special melodies etc. 
 */
void setBeeperPitch(uint8_t pitch)
{
	controlStatus.beep = false;
	OCR2 = 255-pitch;
	if(pitch) 
		TCCR2 =  (1 << WGM21) | (1 << COM20) | (1 << CS22) | (1 << CS21);	
	else 
		TCCR2 = 0;
}

/**
 * Delay with the help of the 10kHz timer.
 * sleep(10) delays for *about* 1ms! Not exaclty, as we do not use assembly routines
 * anywhere in this library!
 *
 * This is a blocking routine, which means that the processor
 * will loop in this routine and (except for interrupts) the
 * normal program flow is stopped!
 * Thus you should use the Stopwatch functions wherever you can!
 *
 * Example:
 *		sleep(1); // delay 1 * 100us = 100us = 0.1ms
 *		sleep(10); // delay 10 * 100us = 1000us = 1ms
 *		sleep(100); // delay 100 * 100us = 10000us = 10ms
 *		// The maximum delay is:
 *		sleep(255); // delay 255 * 100us = 25500us = 25.5ms
 */
void sleep(uint8_t time)
{
	delay_timer = 0;
	while (delay_timer <= time+1);
}

/**
 * The same as sleep() but this delays for time*1ms.
 *
 * Example:
 *      mSleep(100); // delay 100 * 1ms = 100ms = 0.1s
 *		mSleep(1000); // delay 1000 * 1ms = 1000ms = 1s
 *
 */
void mSleep(uint16_t time)
{
	while (time--) sleep(10);
}

/**
 * Delay a number of instruction cycles. 
 * No exact delay function! And interrupts can still occur and 
 * add a lot of extra cycles.
 * This function only guarantees to delay for a MINIMAL number
 * of cycles! 
 *
 * Example:
 * 	delayCycles(1000); // Delays for *about* 1000 instruction cycles
 */
void delayCycles(uint16_t dly)
{
	while(dly--) nop();
}


/*****************************************************************************/
// Initialisation:

/**
 * Initialize the Controller - ALWAYS CALL THIS FIRST!
 * The Processor will not work correctly otherwise.
 * (If you don't implement your own init routine...)
 *
 * Example:
 *
 *			int main(void)
 *			{
 *				initRP6Control(); // CALL THIS FIRST!
 *
 *				// ... your application code
 *
 *				while(true);
 *				return 0;
 *			}
 *
 */
void initRP6Control(void)
{
	portInit();		// Setup port directions and initial values.
					// This is the most important step!

	cli();			// Disable global interrupts.

	// UART:
	UBRRH = UBRR_BAUD_LOW >> 8;	// Setup UART: Baud is Low Speed
	UBRRL = (uint8_t) UBRR_BAUD_LOW;
	UCSRA = 0x00;
    UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
    UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);
	
	// Initialize ADC:
	ADMUX = 0; //external reference 
	ADCSRA = (0<<ADIE) | (0<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADIF);
	SFIOR = 0;

	// Initialize External interrupts - all disabled:
	MCUCR = (1 << ISC11) | (1 << ISC10) | (1 << ISC01) | (1 << ISC00);
	GICR = (0 << INT2) | (0 << INT1) | (0 << INT0);
	MCUCSR = (0 << ISC2);
	
	
	// 10kHz Timer 0:
	TCCR0 =   (0 << WGM00) 
			| (1 << WGM01) 
			| (0 << COM00) 
			| (0 << COM01) 
			| (0 << CS02)  
			| (1 << CS01) 
			| (0 << CS00);
	OCR0  = 199;
	
/*
	Timer 1 is free for your application!
*/
	TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS10) | (1 << CS11);
  OCR1A = 6250;
  TIMSK = (1 << OCIE0) | (1<< OCIE1A);


	// Timer 2 - used for beeper:
	TCCR2 =  0; 
	OCR2  = 0xFF; 

	// Enable timer interrupts:
	//TIMSK =   (1 << OCIE0); 

	// SPI Master (SPI Mode 0, SCK Frequency is F_CPU/2, which means it is 8MHz 
	// on the RP6 CONTROL M32...):
	SPCR =    (0<<SPIE) 
			| (1<<SPE) 
			| (1<<MSTR) 
			| (0<<SPR0) 
			| (0<<SPR1) 
			| (0<<CPOL) 
			| (0<<CPHA);  
	SPSR = (1<<SPI2X);
	
	sei(); // Enable Global Interrupts
}

/******************************************************************************
 * Additional info
 * ****************************************************************************
 * Changelog:
 * - v. 1.1 by Dominik S. Herwald
 *		- NEW: universal timer variable with 100탎 resolution added!
 * - v. 1.0 (initial release) 16.05.2007 by Dominik S. Herwald
 *
 * ****************************************************************************
 * Bugs, feedback, questions and modifications can be posted on the AREXX Forum
 * on http://www.arexx.com/forum/ !
 * Of course you can also write us an e-mail to: info@arexx.nl
 * AREXX Engineering may publish updates from time to time on AREXX.com!
 * ****************************************************************************
 * - LICENSE -
 * GNU GPL v2 (http://www.gnu.org/licenses/gpl.txt, a local copy can be found
 * on the RP6 CD in the RP6 sorce code folders!)
 * This program is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 * ****************************************************************************
 */

/*****************************************************************************/
// EOF
