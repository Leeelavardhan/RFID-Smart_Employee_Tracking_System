#include "headerfiles.h"

i16 hour, min, sec, date, month, year, day;
ui32 IRQ_fired = 0;
ui32 UART1_Fired = 0;
c8 UART1_BUFFER[15];
c8 UART0_BUFFER[25];
ui32 UART1_RX_LEN;
c8 DATA_FRAME[30];
extern uc8 UART0_BYTE;
extern uc8 UART0_Fired;
extern char response;
int one_time = 1;

uc8 up_arrow[8] = {0x04, 0x0E, 0x1F, 0x15, 0x04, 0x04, 0x04, 0x04};
uc8 down_arrow[8] = {0x04, 0x04, 0x04, 0x15, 0x1F, 0x0E, 0x04};

int main(){
	InitializeUART0();
	InitializeUART1();
	InitializeLCD();
	InitializeRTC();
	Eint_Init();
	InitializeSPI();
	LCD_Custom_Char(2, up_arrow); // Store the uparrow_icon at CGRAM position 2
	LCD_Custom_Char(3, down_arrow); // Store the down arrow at CGRAM position 3
	
	//ChipErase();
	
	while(1){
		FetchTime(&hour, &min, &sec);
		COMMAND(GOTO_LINE1_POS0); // Move cursor to first line, position 0
		PrintTime(hour, min, sec);
		FetchDate(&date, &month, &year);
		PrintDate(date, month, year);
		FetchDay(&day);
		PrintDay(day);
		
		if(IRQ_fired){
			IRQ_fired = 0;
			admin_settings();
		}

		if(UART1_Fired){
			UART1_Fired = 0;
			ExtractRFID(UART1_BUFFER);
			switch(check_admin(UART1_BUFFER)){
				case 'A' : admin();
								   COMMAND(CLEAR_LCD);
								   break;
				case 'B' : user();
								   COMMAND(CLEAR_LCD);
								   break;
				default  : COMMAND(CLEAR_LCD);
								   DATA("  INVALID CARD");
								   delay_ms(1000);
								   break;
			}
		}
		
		if(one_time)
			if((HOUR == 0) && (MIN == 0)){
				one_time = 0;
				Transmit_string_UART0("[CHECK ALL IN_OUT FIELDS}");
			}
		
		//UART0 Interrupt testing code
		/*if(UART0_Fired){
			UART0_Fired = 0;
			COMMAND(CLEAR_LCD);
			Receive_string_UART0(UART0_BUFFER, 10);
			DATA(UART0_BUFFER);
			delay_ms(500);
			COMMAND(CLEAR_LCD);
		}*/
		
		//UART1 Interrupt testing code
		/*if(UART1_Fired){
			UART1_Fired = 0;
			COMMAND(CLEAR_LCD);
			ExtractRFID(UART1_BUFFER);
			DATA(UART1_BUFFER);
			delay_ms(500);	      
			COMMAND(CLEAR_LCD);
		}*/
	}
}
