#include <LPC21xx.h>
#include "SPI_defines.h"
#include "SPI_functions.h"
#include "delay.h"

void InitializeSPI(void){
  PINSEL0 |= (0x55 << 8);        // P0.4 SCK, P0.5 MISO, P0.6 MOSI
  PINSEL0 &= ~(3<<14);           // P0.7 as GPIO
  IODIR0  |= (1<<7);             // P0.7 output
  IOSET0  |= (1<<7);             // SSEL HIGH (inactive)
  S0SPCR   = (1<<5);             // MSTR=1, LSBF=0, CPHA=0, CPOL=0 (Mode 0)
  S0SPCCR  = 14;                 // (14 -> ~1MHz) Must be even and >= 8
}

void Write_enable_SPI(void){
	PULL_CS;                           // SSEL LOW
	Transmit_char_SPI(WRITE_ENABLE);   // Write Enable
	PUSH_CS;                           // SSEL HIGH
	delay_ms(1);
}

void Write_disable_SPI(void){
	PULL_CS;                           // SSEL LOW
	Transmit_char_SPI(WRITE_DISABLE);  // Write Disable
	PUSH_CS;                           // SSEL HIGH
	delay_ms(1);
}

void Transmit_char_SPI(char data){
	char dummy;
	dummy  = S0SPDR;               // Read to clear SPIF
	S0SPDR = data;                 // Load data to transmit
  while(!(S0SPSR & (1<<7)));     // Wait till SPIF = 1
  dummy  = S0SPDR;               // Read to clear SPIF
	dummy++;                       // No logical use for this line (just to remove warning)
}

// Function to Transmit String to EEPROM
void Transmit_string_SPI(char *ptr){
	while(*ptr){
		Transmit_char_SPI(*ptr++);
	}
}

// Function to Erase Entire EEPROM Chip
void ChipErase(void){
	
	Write_enable_SPI();
	
	PULL_CS;                       // SSEL LOW
	Transmit_char_SPI(0xC7);           // CHIP ERASE command
	PUSH_CS;                       // SSEL HIGH
	delay_ms(200);                 // Wait for erase to complete
	
	Write_disable_SPI();
}

// Function to Erase Page from EEPROM
void PageErase(unsigned short int address){
	
	Write_enable_SPI();
	
	PULL_CS;                       // SSEL LOW
	Transmit_char_SPI(0x42);           // PAGE ERASE COMMAND
	Transmit_char_SPI((address >> 8) & 0xFF);  // Erase which Page? That page Starting Location
	Transmit_char_SPI(address & 0xFF);         // That page ending location
	PUSH_CS;                       // SSEL HIGH
	delay_ms(100);                 // Wait for page erase
	
	Write_disable_SPI();
}

// Function to receive string from EEPROM
void Receive_string_SPI(char *ptr, int bytes){
	int i = 0;
	while(i < bytes){
		S0SPDR = 0x00;               // Transmitting dummy byte for clock generation
		while(!(S0SPSR & (1<<7)));   // Waiting for SPIF to 1?
		ptr[i++] = S0SPDR;           // Collecting data from Data register
	}
	ptr[i] = '\0';
}

void Write_string_to_EEPROM(char *str, short int address){
	Write_enable_SPI();
	
	PULL_CS;
	Transmit_char_SPI(WRITE_MODE);
	Transmit_char_SPI((address >> 8) & 0xFF);
	Transmit_char_SPI(address & 0xFF);
	Transmit_string_SPI(str);
	PUSH_CS;
	
	delay_ms(20);
	
	Write_disable_SPI();
}

void Receive_string_from_EEPROM(char *str, int bytes, short int address){
	PULL_CS;                                    // SSEL LOW
	Transmit_char_SPI(READ_MODE);               // Read mode 
	Transmit_char_SPI((address >> 8) & 0xFF);   // Higher Byte Location EEPROM
	Transmit_char_SPI(address & 0xFF);          // Lower byte Location EEPROM
	Receive_string_SPI(str, bytes);             // N bytes will be collected into received_string[]
	PUSH_CS;                                    // SSEL HIGH
	
	delay_ms(100);
}
