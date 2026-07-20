#include "uart1_defines.h"
#include "uart1_functions.h"
#include "defines.h"
#include "types.h"

#define UART_FRAME_LEN 10
extern ui32 UART1_Fired;
extern c8 UART1_BUFFER[15];
extern ui32 UART1_RX_LEN;


void UART1_ISR(void) __irq {
	char ch;
  if ((U1IIR >> 2) & 1) {          // RDA
		while (U1LSR & (1<<0)){
			ch = U1RBR;
      if (UART1_RX_LEN < UART_FRAME_LEN) {
				UART1_BUFFER[UART1_RX_LEN++] = ch;
				if (UART1_RX_LEN == UART_FRAME_LEN) {
					UART1_Fired  = 1;
					UART1_BUFFER[UART1_RX_LEN] = '\0';
					
					//This may create problem?
					//Make sure the FIFO should empty before the card is tapped.
					// startinf with 0x02 and ex=nding with 0x03 while taking into the string is also a good practice
					//Figure out these issues tomorrow.
          UART1_RX_LEN = 0;      // ready for next frame
        }
      }
		}
  }
  VICVectAddr = 0;
}

void InitializeUART1(){
	//Configure P0.8 as Tx and P0.9 as Rx
	PINSEL0 &= ~(0x000F0000);   // Clear P0.8 & P0.9
	PINSEL0 |= 0x00050000;
	//PINSEL0 |= ((1 << TXD1) | (1 << RXD1));
	//Setting DLAB bit to access Divisor Latch Registers DLL and DLM
	SETBIT(U1LCR,DLAB);
	//Configuring Baudrate 4800, 7200, 9600 
	BAUDRATE1(9600);
	//Clearing DLAB bit to send data
	CLRBIT(U1LCR,DLAB);
	//Select 8N1 mode (8-bit and No parity and 1 stop bit)
	U1LCR = 0x03;
	
	//Added code for UART1 Interrupt
	U1FCR = 0x01;           						 // Enable 1-byte FIFO trigger 
	U1IER = (1 << 0);       						 // Enable RDA interrupt only   
	VICIntEnClr = (1 << 7); 						 // Clear UART1 interrupt first 
	VICVectAddr1 = (unsigned int)UART1_ISR;  // Set ISR address 
	VICVectCntl1 = 0x20 | 7;             // Enable channel 7 slot 
	VICIntEnable = (1 << 7);             // Enable UART1 interrupt

}

//To receive a character
unsigned char Receive_char_UART1(){
	while(!(U1LSR & (1 << RDR)));
	return U1RBR;
}

//To receive a string
//Upto n length which will be provided as a argument
//with enter the reception will stop or with length exceeds
void Receive_string_UART1(char *string, unsigned int length){
	unsigned int i = 0;
	char temp;
	
	// Disable UART1 Rx interrupt (RDA + CTI)
  U1IER &= ~(1 << 0);          // bit0 = 0 ? no Rx IRQ
	
	for(i = 0; i < length; i++){
		while(!(U1LSR & (1 << RDR)));
		temp = U1RBR;
		if((temp != '\r') && (temp != '\n')){
			string[i] = temp;
		}
		else{
			string[i] = '\0';
			return;
		}
	}
	string[i] = '\0';
	
	// Re-enable UART1 Rx interrupt
  U1IER |= (1 << 0);           // bit0 = 1 ? Rx IRQ back on
	
}

void Transmit_char_UART1(char character){
	//Checking if the THR is empty or not
	while(!(U1LSR & (1 << TEMT)));
	//Load byte into THR
	U1THR = character;
}

void Transmit_string_UART1(char *string){
	while(*string){
		//Checking if the THR is empty or not
		while(!(U1LSR & (1 << TEMT)));
		//Load byte into THR
		U1THR = *string++;
	}
}

char *ExtractRFID(char *str){
	int i;
	for(i = 0; i < 8; i++){
		str[i] = str[i+1];	
	}
	str[i] = '\0';
	return str;
}
