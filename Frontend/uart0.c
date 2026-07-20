#include <LPC21xx.h>
#include "defines.h"
#include "uart0_defines.h"
#include "uart0_functions.h"

unsigned char UART0_BYTE;
unsigned char UART0_Fired = 0;

void UART0_ISR(void) __irq{
  // Check if this is RDA interrupt – optional
  if(((U0IIR >> 2) & 1)){   			// Receive Data Available
		if(U0LSR & (1 << 0)){       // RDR set -> data ready
			UART0_BYTE = U0RBR;  			// read exactly one byte
      UART0_Fired = 1;      
    }
  }
  VICVectAddr = 0;              // Acknowledge end of ISR
}

void InitializeUART0(){
	//Configure P0.0 as Tx and P0.1 as Rx
	PINSEL0 |= ((1 << TXD0) | (1 << RXD0));
	//Setting DLAB bit to access Divisor Latch Registers DLL and DLM
	SETBIT(U0LCR,DLAB);
	//Configuring Baudrate 4800, 7200, 9600 
	BAUDRATE0(9600);
	//Clearing DLAB bit to send data
	CLRBIT(U0LCR,DLAB);
	//Select 8N1 mode (8-bit and No parity and 1 stop bit)
	U0LCR = 0x03;
	
	// Enable RX interrupt only (1-byte receive is enough)
  // Bit0 = RBR interrupt enable
  U0IER = 0x01;    // only Receive Data Available interrupt 

  // --- VIC setup for UART0 on slot 2 (not 0 or 1) ---
  // UART0 is VIC channel 6 on LPC21xx. 
  VICIntEnable |= (1 << 6);          // enable UART0 channel
  VICIntSelect &= ~(1 << 6);       // make it IRQ, not FIQ

	VICVectCntl2 = (1 << 5) | 6;     // enable slot2, assign channel 6 
  VICVectAddr2 = (unsigned long)UART0_ISR;  // ISR address for slot2
}

//To receive a character
unsigned char Receive_char_UART0(){
	
	// Disable UART1 Rx interrupt (RDA + CTI)
  U0IER &= ~(1 << 0);          // bit0 = 0 ? no Rx IRQ
	
	while(!(U0LSR & (1 << RDR)));
	
	// Re-enable UART1 Rx interrupt
  U0IER |= (1 << 0);           // bit0 = 1 ? Rx IRQ back on
	
	return U0RBR;
}

//To receive a string
//Upto n length which will be provided as a argument
//with enter the reception will stop or with length exceeds
void Receive_string_UART0(char *string, unsigned int length){
	
	unsigned int i = 0;
	char temp;
	
	// Disable UART1 Rx interrupt (RDA + CTI)
  U0IER &= ~(1 << 0);          // bit0 = 0 ? no Rx IRQ
	
	for(i = 0; i < length - 1; i++){
		while(!(U0LSR & (1 << RDR)));
		temp = U0RBR;
		if((temp != '\r') && (temp != '\n') && (temp != '?')){
			string[i] = temp;
		}
		else{
			string[i] = '\0';
			U0IER |= (1 << 0);
			return;
		}
	}
	string[i] = '\0';
	
	// Re-enable UART1 Rx interrupt
  U0IER |= (1 << 0);           // bit0 = 1 ? Rx IRQ back on
	
}

void Transmit_char_UART0(char character){
	//Checking if the THR is empty or not
	while(!(U0LSR & (1 << TEMT)));
	//Load byte into THR
	U0THR = character;
}

void Transmit_string_UART0(char *string){
	while(*string){
		//Checking if the THR is empty or not
		while(!(U0LSR & (1 << TEMT)));
		//Load byte into THR
		U0THR = *string++;
	}
}
