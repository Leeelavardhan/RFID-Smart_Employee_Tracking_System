#include "defines.h"
#include "reserve_pins_defines.h"
#include "reserve_pins.h"
#include "lcd_defines.h"
#include "lcd_functions.h"
#include "delay.h"
#include "keymatrix.h"
#include "SPI_defines.h"
#include "SPI_functions.h"
#include "uart1_defines.h"
#include "uart1_functions.h"
#include "uart0_defines.h"
#include "uart0_functions.h"
#include "rtc_defines.h"
#include "rtc_functions.h"
#include <string.h>
#include <ctype.h>

#define LCD 16
#define LEFT 0
#define RIGHT 1

char leading_zeros[] = "0000";
extern c8 DATA_FRAME[30];
extern c8 UART1_BUFFER[15];
extern c8 UART0_BUFFER[25];
extern uc8 UART0_BYTE;
extern uc8 UART0_Fired;
extern ui32 UART1_Fired;
char response;
int IN;
extern int one_time;

//Initializing LCD
void InitializeLCD(void){
	ReservePins(1, 8, 16, OUTPUT);
	ReservePins(0, 2, 10, OUTPUT);
	//WRITEBYTE(IODIR1,SHIFT,0x3FF);
	delay_ms(15);                               // Mandatory power-up delay for HD44780
	COMMAND(0x30);                              // Force 8-bit mode (init step 1)
	delay_ms(4);                                // Wait >4.1ms
	delay_us(100);                              // Extra settle time
	COMMAND(0x30);                              // Force 8-bit mode (init step 2)
	delay_us(100);                              // Extra settle time
	COMMAND(0x30);                              // Force 8-bit mode (init step 3)
	COMMAND(MODE_8BIT_2LINE);                   // Function set: 8-bit, 2-line, 5x8 dots
	COMMAND(DSP_ON_CUR_OFF);                    // Display ON, Cursor OFF
	COMMAND(CLEAR_LCD);                         // Clear display & home cursor
	COMMAND(SHIFT_CUR_RIGHT);                   // Entry mode: increment cursor
}

//For giving Commands to the LCD
void COMMAND(uc8 cmd){
	WRITEBYTE(IOCLR1,SHIFT,0xFF);                // Clear data bus bits on port0
	WRITEBYTE(IOSET1,SHIFT,cmd);                 // Put command byte on data bus
	cmd_mode;                                   // RS=0, RW=0 (command write)
	write_lcd;                                  // Assert control lines for write
	enable_lcd();                               // E strobe to latch into LCD
	delay_ms(2);                                // Wait for instruction execution
}

//For printing character on to the screen
void DATA_char(uc8 ch){
	WRITEBYTE(IOCLR1,SHIFT,0xFF);                // Clear data bus bits on port0
	WRITEBYTE(IOSET1,SHIFT,ch);                  // Put data byte on data bus
	data_mode;                                  // RS=1, RW=0 (data write)
	write_lcd;                                  // Assert control lines for write
	enable_lcd();                               // E strobe to latch character
	delay_ms(2);                                // Wait for data write cycle
}

//For printing string on to the screen
void DATA(c8 *ptr){
	int i;                                      // Index through string
	WRITEBYTE(IOCLR1,SHIFT,0xFF);                // Clear data bus before starting
	for(i = 0; ptr[i]; i++){                    // Iterate until null terminator
		WRITEBYTE(IOSET1,SHIFT,ptr[i]);                       // Put next character on bus
		data_mode;                              // RS=1 for data
		write_lcd;                              // Start write
		enable_lcd();                           // Latch with E strobe
		delay_ms(2);                            // Interchar delay for LCD
	  WRITEBYTE(IOCLR1,SHIFT,0xFF);             // Clear bus between chars
	}
}

//Enable pin latching
void enable_lcd(){
	SETBIT(IOSET0,LCD_EN);                      // Set E=1 (start latch window)
	delay_us(2);                                // Short enable pulse width
	IOCLR0 = 1 << LCD_EN;                      // Set E=0 (capture data/command)
	delay_us(2);                                // Enable cycle closing delay
}

//For printing unsigned integer on to the screen
void U32LCD(ui32 n){
	i16 i=0;                                    // Digit stack index
	uc8 a[10];                                  // Buffer for up to 10 digits
	if(n==0){                                   // Special case zero
		DATA_char('0');                         // Print single '0'
	}
	else{
		while(n>0){                             // Extract digits in reverse
		  a[i++]=(n%10)+48;                     // Store ASCII of last digit
      n/=10;			                           // Drop last digit
		}
		for(--i;i>=0;i--)                       // Print digits in forward order
		  DATA_char(a[i]);                      // Output one by one
	}
}

//LCD Custom char intialization by storing message at CGRAM Location 0x40
void LCD_Custom_Char(uc8 location, uc8 *msg){
    uc8 i;                                     // Byte index within pattern
    if(location < 8){                           // HD44780 supports 8 custom chars (0..7)
        COMMAND(0x40 + (location * 8));         // Set CGRAM base address for this slot
        for(i = 0; i < 8; i++)                  // Each custom char has 8 rows
            DATA_char(msg[i]);                  // Write row pattern
    }
}

//To move LCD cursor to any one of the position and any one of the line
void LCD_SetCursor(uc8 col, uc8 row){
    uc8 addr;                                   // DDRAM address to compute
    if(row == 0)
        addr = 0x80 + col;                      // Line 1 base 0x80 + column
    else if(row == 1)
        addr = 0xC0 + col;                      // Line 2 base 0xC0 + column
    else
        return;                                 // Invalid row: do nothing
    COMMAND(addr);                              // Issue Set DDRAM Address
}

void admin_settings(void){
	char keypress;
	COMMAND(CLEAR_LCD);
	DATA("1.CHANGE ADMIN");
	COMMAND(GOTO_LINE2_POS0);
	DATA("2.E.RTC   3.EXIT");
	keypress = get_key();
	switch(keypress){
		case '1': change_admin();
							COMMAND(CLEAR_LCD);
							break;
		case '2': edit_time();
							COMMAND(CLEAR_LCD);
							break;
		case '3': COMMAND(CLEAR_LCD);
							return;
		case '#': COMMAND(CLEAR_LCD);
							return;
		default : COMMAND(CLEAR_LCD);
							return;
	}
}

i32 edit_time(){
	START:COMMAND(CLEAR_LCD);                     // Draw submenu
	delay_ms(5);                                  // UI delay
	COMMAND(GOTO_LINE1_POS0);                     // Line1
	DATA("1.TIME    2.DATE");                     // Show options
	COMMAND(GOTO_LINE2_POS0);                     // Line2
	DATA("3.DAY     4.BACK");                     // More options
	
	while(1){                             // Loop until timeout
		uc8 keypress;                             // Key buffer
		keypress = get_key();                     // Scan keypad
		if(keypress != '\0'){                     // If pressed
			switch(keypress){                     // Decode selection
				case '1':if(time_())              // Edit RTC time
										return 1;               // Return to caller with 1
									goto START;                // Otherwise redraw
						
				case '2':if(date_())              // Edit RTC date
										return 1;               // Exit with 1
									goto START;                // Redraw
					
				case '3':if(day_())               // Edit RTC day
										return 1;               // Exit with 1
									goto START;                // Redraw
				
				case '4':COMMAND(CLEAR_LCD);       // Back to previous
								return 0;                      // Indicate normal back
				
				case '*':COMMAND(CLEAR_LCD);       // Emergency exit
								 DATA(" EMERGENCY EXIT");
								 delay_ms(500);
								 COMMAND(CLEAR_LCD);
								 return 1;                     // Signal exit
				
				case '#': COMMAND(CLEAR_LCD);
									return 0;
				
				default:COMMAND(CLEAR_LCD);        // Invalid input message
								DATA("INVALID  CHOICE");
								COMMAND(GOTO_LINE2_POS0);
								DATA("* for EMER.EXIT");
								delay_ms(500);
								goto START;                    // Redraw options
			}
		}
	}
}

i32 time_(){
	i32 hour_count = HOUR, min_count = MIN, sec_count = SEC; // Load current RTC values
	i32 index = 1;                             // 1=hour, 2=min, 3=sec selection
	uc8 keypress;                              // Keypad input
	COMMAND(CLEAR_LCD);                        // Clear screen
	
	BEGIN:COMMAND(GOTO_LINE1_POS0);            // Top banner with arrows
	DATA_char('<');                            // Left arrow custom/ASCII
	COMMAND(GOTO_LINE1_POS0 + 15);             // Far right position
	DATA_char('>');                            // Right arrow
	
	while(1){                          // UI loop
		COMMAND(GOTO_LINE2_POS0);              // Move to time row start
		DATA_char(2);                          // Custom icon at start (e.g., clock)
		COMMAND(GOTO_LINE2_POS0 + 4);          // Position for HH tens
		if((hour_count >= 0) && (hour_count <= 9)){
			COMMAND(GOTO_LINE2_POS0 + 4);      // If single digit hour, print leading zero
			DATA_char('0');
		}
		U32LCD(hour_count);                     // Print hours (00-23)
		DATA_char(':');                         // Colon separator
		if((min_count >= 0) && (min_count <= 9)){
			COMMAND(GOTO_LINE2_POS0 + 7);      // Leading zero for minutes
			DATA_char('0');
		}
		U32LCD(min_count);                      // Print minutes (00-59)
		DATA_char(':');                         // Colon separator
		if((sec_count >= 0) && (sec_count <= 9)){
			COMMAND(GOTO_LINE2_POS0 + 10);     // Leading zero for seconds
			DATA_char('0');
		}
		U32LCD(sec_count);                      // Print seconds (00-59)
		COMMAND(GOTO_LINE2_POS0 + 3);           // Cosmetic alignment
		LCD_SetCursor(15,1);                    // Put cursor at end of line
		DATA_char(3);                           // Custom icon at end
	
		
		if(index == 1){                         // Highlight text for selection
			COMMAND(GOTO_LINE1_POS0 + 4);
			DATA("HH      ");                  // Indicate editing hours
		}
		else if(index == 2){
			COMMAND(GOTO_LINE1_POS0 + 4);
			DATA("   MM   ");                  // Indicate editing minutes
		}
		else if(index == 3){
			COMMAND(GOTO_LINE1_POS0 + 4);
		  DATA("      SS");                    // Indicate editing seconds
		}
	
		keypress = get_key();                   // Read keypad
		if(keypress != '\0'){
			switch(keypress){
				case 'A':if(index == 3){        // Next field (cyclic)
										index = 1;
									}	
								else
										index++;
								break;
				case 'B':if(index == 1){        // Previous field (cyclic)
										index = 3;
									}
									else
										index--;
									break;
				case 'C':if(index == 1){        // Increment field
										if(hour_count == 23){
											hour_count = 0;     // Wrap 23->0
										}
										else
											hour_count++;
									}
									else if(index == 2){
											if(min_count == 59){
												min_count = 0;  // Wrap 59->0
											}
											else
												min_count++;
									}
									else if(index == 3){
											if(sec_count == 59){
												sec_count = 0;  // Wrap 59->0
											}
											else
												sec_count++;
									}
									break;
				case 'D':if(index == 1){        // Decrement field
										if(hour_count == 0){
											hour_count = 23;    // Wrap 0->23
										}
										else
											hour_count--;
									}
									else if(index == 2){
											if(min_count == 0){
												min_count = 59; // Wrap 0->59
											}
											else
												min_count--;
									}
									else if(index == 3){
											if(sec_count == 0){
												sec_count = 59; // Wrap 0->59
											}
											else
												sec_count--;
									}
									break;
									
				case '*':COMMAND(CLEAR_LCD);    // Emergency exit
								 DATA(" EMERGENCY EXIT");
								 delay_ms(500);
								 COMMAND(CLEAR_LCD);
								 return 1;              // Signal abort
									
				case '=': HOUR = hour_count;    // Save edited time to global RTC vars
									MIN = min_count;
									SEC = sec_count;
									COMMAND(CLEAR_LCD);
									DATA(" TIME UPDATED!"); // Confirmation
									delay_ms(500);
									COMMAND(CLEAR_LCD);
									return 0;              // Normal exit
									
				case '#': COMMAND(CLEAR_LCD);
									return 0;
									
			default: COMMAND(CLEAR_LCD);        // Help hint for wrong key
								COMMAND(GOTO_LINE1_POS0);
								DATA("B/A TO NAVIGATE");
								COMMAND(GOTO_LINE2_POS0);
								DATA("C/D TO INCREMENT");
								delay_ms(500);
								COMMAND(CLEAR_LCD);
								goto BEGIN;               // Redraw same screen
				}
			}
	}
}

i32 date_(){
	int date_count = DOM, month_count = DOW, year_count = 2026; // Defaults for date edit
	int index = 1;                             // 1=DD, 2=MM, 3=YYYY
	unsigned char keypress;                    // Keypress buffer
	COMMAND(CLEAR_LCD);                        // Clear screen
	
	BEGIN:COMMAND(GOTO_LINE1_POS0);            // Draw header with arrows
	DATA_char('<');
	COMMAND(GOTO_LINE1_POS0 + 15);
	DATA_char('>');
	
	while(1){                          // UI loop
		COMMAND(GOTO_LINE2_POS0);              // Start of date display row
		DATA_char(2);                          // Decorative/custom icon
		COMMAND(GOTO_LINE2_POS0 + 3);          // Position before DD
		if((date_count >= 0) && (date_count <= 9)){
			COMMAND(GOTO_LINE2_POS0 + 3);      // Leading zero for day
			DATA_char('0');
		}
		U32LCD(date_count);                     // Print DD
		DATA_char('/');                         // Separator
		if((month_count >= 0) && (month_count <= 9)){
			COMMAND(GOTO_LINE2_POS0 + 6);      // Leading zero for month
			DATA_char('0');
		}
		U32LCD(month_count);                    // Print MM
		DATA_char('/');                         // Separator
		
		//To add leading zeros for Year representation
		add_leading_zeros(count_digits(year_count));
		
		U32LCD(year_count);                     // Print YYYY
		LCD_SetCursor(15,1);                    // End marker
		DATA_char(3);                           // Decorative/custom icon
		
		switch(index){                          // Show which field is selected
			case 1: COMMAND(GOTO_LINE1_POS0 + 3);
							DATA("DD        ");  // Day selected
							break;
			case 2: COMMAND(GOTO_LINE1_POS0 + 3);
							DATA("   MM     ");  // Month selected
							break;
			case 3: COMMAND(GOTO_LINE1_POS0 + 3);
							DATA("      YYYY");  // Year selected
							break;
		}
	
		keypress = get_key();                   // Read keypad
		if(keypress != '\0'){
			switch(keypress){
				case 'A':if(index == 3){        // Next field (cyclic)
										index = 1;
									}
								else
										index++;
								break;
				case 'B':if(index == 1){        // Previous field (cyclic)
										index = 3;
									}
									else
										index--;
									break;
				case 'C':if(index == 1){        // Increment with wrap limits
										if(date_count == 31){
											date_count = 1;
										}
										else
											date_count++;
									}
									else if(index == 2){
											if(month_count == 12){
												month_count = 1;
											}
											else
												month_count++;
									}
									else if(index == 3){
											if(year_count == 4095){
												year_count = 1;
											}
											else
												year_count++;
									}
									break;
				case 'D':if(index == 1){        // Decrement with wrap limits
										if(date_count == 1){
											date_count = 31;
										}
										else
											date_count--;
									}
									else if(index == 2){
											if(month_count == 1){
												month_count = 12;
											}
											else
												month_count--;
									}
									else if(index == 3){
											if(year_count == 1){
												year_count = 4095;
											}
											else
												year_count--;
									}
									break;
									
				case '*':COMMAND(CLEAR_LCD);    // Emergency exit
								 DATA(" EMERGENCY EXIT");
								 delay_ms(500);
								 COMMAND(CLEAR_LCD);
								 return 1;              // Abort
									
				case '=': DOM = date_count;     // Save new date to globals
									MONTH = month_count;
									if(year_count != 2026){ //Giving Warning, but allowing 
										COMMAND(CLEAR_LCD); // user to change Year rather than 2025
										DATA("    WARNING!");
										COMMAND(GOTO_LINE2_POS0);
										DATA("    2026 NOW");
										delay_ms(500);
										COMMAND(CLEAR_LCD);
									}
									YEAR = year_count;
									COMMAND(CLEAR_LCD);
									DATA(" DATE UPDATED!"); // Confirmation
									delay_ms(500);
									COMMAND(CLEAR_LCD);
									return 0;              // Normal return
									
			case '#': COMMAND(CLEAR_LCD);
								return 0;
									
			default: COMMAND(CLEAR_LCD);        // Help for invalid keys
								COMMAND(GOTO_LINE1_POS0);
								DATA("B/A TO NAVIGATE");
								COMMAND(GOTO_LINE2_POS0);
								DATA("C/D TO INCREMENT");
								delay_ms(500);
								COMMAND(CLEAR_LCD);
								goto BEGIN;               // Redraw
			}
		}
	}
}

i32 day_(){
	i32 day_count = DOW;                        // Load current Day-Of-Week (0..6)
	
	c8 day[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"}; // Day names
	
	uc8 keypress;                               // Key buffer
	COMMAND(CLEAR_LCD);                         // Clear
	
	BEGIN:COMMAND(GOTO_LINE1_POS0);             // Header with arrows
	DATA_char('<');
	COMMAND(GOTO_LINE1_POS0 + 15);
	DATA_char('>');
	
	COMMAND(GOTO_LINE2_POS0);                   // Begin content row
	DATA_char(2);                               // Decorative/custom icon
	COMMAND(GOTO_LINE2_POS0 + 3);               // Spacer
	LCD_SetCursor(15,1);                        // Put end marker
	DATA_char(3);                               // Decorative/custom icon
	
	COMMAND(GOTO_LINE1_POS0 + 4);               // Title text
	DATA("WEEK DAY");                           
	
	while(1){                           // UI loop
	COMMAND(GOTO_LINE2_POS0 + 6);               // Position for day text
	DATA(day[day_count]);                        // Print current day string
	keypress = get_key();                        // Read keypad
	if(keypress != '\0'){
		switch(keypress){
			case 'C': if(day_count == 6){        // Next day (wrap)
									day_count = 0;
								}
								else
									day_count++;
								break;
			case 'D': if(day_count == 0){        // Previous day (wrap)
									day_count = 6;
								}
								else
									day_count--;
								break;
								
			case '*':COMMAND(CLEAR_LCD);         // Emergency exit
							  DATA(" EMERGENCY EXIT");
								delay_ms(500);
								COMMAND(CLEAR_LCD);
								return 1;         // Abort
								
			case '=': DOW = day_count;           // Save new DOW
								COMMAND(CLEAR_LCD);
								DATA("  DAY UPDATED!"); // Confirmation
								delay_ms(500);
								COMMAND(CLEAR_LCD);
								return 0;         // Normal return
								
			case '#': COMMAND(CLEAR_LCD);
								return 0;
								
			default: COMMAND(CLEAR_LCD);         // Help for invalid key
								COMMAND(GOTO_LINE1_POS0);
								DATA("C/D TO INCREMENT");
								delay_ms(500);
								COMMAND(CLEAR_LCD);
								goto BEGIN;       // Redraw
		}
	}
}
}

void add_leading_zeros(int zeros){
	COMMAND(GOTO_LINE2_POS0 + 9);
	DATA(leading_zeros + zeros);
}

i32 count_digits(int year_count){
	i32 count = 0;
	while(year_count){
		count++;
		year_count /= 10;
	}
	return count;
}

void change_admin(void){
	char new_admin_id[12];
	char old_admin_id[10];
	char key;
	COMMAND(CLEAR_LCD);
	DATA(" TAP NEW ADMIN");
	COMMAND(GOTO_LINE2_POS0 + 5);
	DATA("CARD");
	Receive_string_UART1(new_admin_id, 10);
	ExtractRFID(new_admin_id);
	Receive_string_from_EEPROM(old_admin_id, 8, 0x0000);
	
	if(!(strcmp(new_admin_id, old_admin_id))){
		COMMAND(CLEAR_LCD);
		DATA("   ADMIN CARD");
		COMMAND(GOTO_LINE2_POS0 + 4);
		DATA("EXISTED!");
		delay_ms(500);
	}
	else{
		COMMAND(CLEAR_LCD);
		DATA(" PRESS ANY KEY");
		COMMAND(GOTO_LINE2_POS0 + 3);
		DATA("TO CONFIRM");
		key = get_key();
		key = key;            // To eliminate Warning Message
		Write_string_to_EEPROM(new_admin_id, 0x0000);
		COMMAND(CLEAR_LCD);
		DATA("NEW ID UPDATED!");
		delay_ms(500);
	}
}

char check_admin(char *ID){
	char old_admin_id[10];
	Receive_string_from_EEPROM(old_admin_id, 8, 0x0000);
	if(!(strcmp(ID, old_admin_id))){
		return 'A';
	}
	else{
		return 'B';
	}
}

void admin(void){
	char key, response;
	int timeout = 6000000, display = 0;
	COMMAND(CLEAR_LCD);
	DATA("1.ADD      2.DEL");
	COMMAND(GOTO_LINE2_POS0);
	DATA("3.EDIT    4.EXIT");
	key = get_key();
	switch(key){
		case '1':	COMMAND(CLEAR_LCD);
							DATA("  TAP NEW CARD");
							Receive_string_UART1(UART1_BUFFER, 10);
							COMMAND(CLEAR_LCD);
							DATA("  CARD  TAPPED");
							delay_ms(500);      // kept for synchronization purpose
							ExtractRFID(UART1_BUFFER);
							frame(DATA_FRAME, UART1_BUFFER, "ADD");

							Transmit_string_UART0(DATA_FRAME);

							while(1){
								timeout--;
								if(UART0_Fired){
									UART0_Fired = 0;
									response = UART0_BYTE;
								}
								if((response == 'Y') || (response == 'y')){
									COMMAND(CLEAR_LCD);
									DATA("   USER ADDED");
									delay_ms(1000);
									break;
								}
								else if((response == 'N') || (response == 'n')){
									COMMAND(CLEAR_LCD);
									DATA("  USER EXISTED");
									delay_ms(1000);
									break;
								}
								if(!timeout){
									timeout = 6000000;
									if(!display){
										display = 1;
										COMMAND(CLEAR_LCD);
										DATA(" WAITING FOR OS");
										COMMAND(GOTO_LINE2_POS0);
										DATA("   TO RESPOND");
									}
									else{
										display = 0;
										COMMAND(CLEAR_LCD);
										DATA(" PLEASE WAIT...");
									}
								}
							}
							break;
		case '2':	COMMAND(CLEAR_LCD);
							DATA("  TAP USERCARD");
							Receive_string_UART1(UART1_BUFFER, 10);
							COMMAND(CLEAR_LCD);
							DATA("  CARD  TAPPED");
							delay_ms(500);      // kept for synchronization purpose
							ExtractRFID(UART1_BUFFER);
							frame(DATA_FRAME, UART1_BUFFER, "DEL");
							Transmit_string_UART0(DATA_FRAME);
							while(1){
								timeout--;
								if(UART0_Fired){
									UART0_Fired = 0;
									response = UART0_BYTE;
								}
								if((response == 'Y') || (response == 'y')){
									COMMAND(CLEAR_LCD);
									DATA("  USER DELETED");
									delay_ms(1000);
									break;
								}
								else if((response == 'N') || (response == 'n')){
									COMMAND(CLEAR_LCD);
									DATA("USER NOT EXISTED");
									delay_ms(1000);
									break;
								}
								if(!timeout){
									timeout = 6000000;
									if(!display){
										display = 1;
										COMMAND(CLEAR_LCD);
										DATA(" WAITING FOR OS");
										COMMAND(GOTO_LINE2_POS0);
										DATA("   TO RESPOND");
									}
									else{
										display = 0;
										COMMAND(CLEAR_LCD);
										DATA(" PLEASE WAIT...");
									}
								}
							}
							break;
		case '3':	COMMAND(CLEAR_LCD);
							DATA("  TAP USERCARD");
							Receive_string_UART1(UART1_BUFFER, 10);
							COMMAND(CLEAR_LCD);
							DATA("  CARD  TAPPED");
							delay_ms(500);      // kept for synchronization purpose
							ExtractRFID(UART1_BUFFER);
							frame(DATA_FRAME, UART1_BUFFER, "EDT");
							Transmit_string_UART0(DATA_FRAME);
							while(1){
								timeout--;
								if(UART0_Fired){
									UART0_Fired = 0;
									response = UART0_BYTE;
								}
								if((response == 'Y') || (response == 'y')){
									COMMAND(CLEAR_LCD);
									DATA("  USER  EDITED");
									delay_ms(1000);
									break;
								}
								else if((response == 'N') || (response == 'n')){
									COMMAND(CLEAR_LCD);
									DATA("USER NOT EXISTED");
									delay_ms(1000);
									break;
								}
								if(!timeout){
									timeout = 6000000;
									if(!display){
										display = 1;
										COMMAND(CLEAR_LCD);
										DATA(" WAITING FOR OS");
										COMMAND(GOTO_LINE2_POS0);
										DATA("   TO RESPOND");
									}
									else{
										display = 0;
										COMMAND(CLEAR_LCD);
										DATA(" PLEASE WAIT...");
									}
								}
							}
							break;
		case '#': return;
		case '4':	return;
	}
}

void user(void){
	int timeout = 6000000, display = 0, uart_flag = 1, len = 0;
	frame(DATA_FRAME, UART1_BUFFER, "LOG");
	len = len;   // To avoid warning message
	Transmit_string_UART0(DATA_FRAME);
	
	response = '\0';
	while(uart_flag){
		if(UART0_Fired){
			UART0_Fired = 0;
			uart_flag = 0;
			response = UART0_BYTE;
		}
		if((response == 'Y') || (response == 'y')){
			Receive_string_UART0(UART0_BUFFER, 20);
			Capitalize_String(UART0_BUFFER);
			len = string_len(UART0_BUFFER);
			if(!IN){
				IN = 1;
				COMMAND(CLEAR_LCD);
				//COMMAND(GOTO_LINE1_POS0 + ((LCD - len) / 2));
				//DATA(UART0_BUFFER);
				//Comment the below line if animation is not required
				LCD_SCROLL(UART0_BUFFER, 0, 0);
				COMMAND(GOTO_LINE2_POS0);
				DATA("  IN  ");
				PrintTime(HOUR, MIN, SEC);
				delay_ms(1000);
			}
			else{
				IN = 0;
				COMMAND(CLEAR_LCD);
				//COMMAND(GOTO_LINE1_POS0 + ((LCD - len) / 2));
				//DATA(UART0_BUFFER);
				//Comment the below line if animation is not required
				LCD_SCROLL(UART0_BUFFER, 0, 0);
				COMMAND(GOTO_LINE2_POS0);
				DATA("  OUT ");
				PrintTime(HOUR, MIN, SEC);
				delay_ms(1000);
			}
		}
		else if((response == 'N') || (response == 'n')){
			COMMAND(CLEAR_LCD);
			DATA("USER NOT EXISTED");
			delay_ms(1000);
		}
		else if((response == 'S') || (response == 's')){
			COMMAND(CLEAR_LCD);
			DATA("MAXIMUM IN & OUT");
			COMMAND(GOTO_LINE2_POS0);
			DATA("    REACHED!");
			delay_ms(1000);
		}
		else if((response == 'A') || (response == 'a')){
			COMMAND(CLEAR_LCD);
			DATA("  TOO FAST OUT");
			COMMAND(GOTO_LINE2_POS0);
			DATA("WAIT 10 SECONDS!");
			delay_ms(1000);
		}	
		else if((response == 'B') || (response == 'b')){
			Receive_string_UART0(UART0_BUFFER, 20);
			COMMAND(CLEAR_LCD);
			DATA(" TIME VIOLATED!");
			COMMAND(GOTO_LINE2_POS0);
			DATA("IN_OUT MISMATCHS");
			delay_ms(2000);
			while(1){
				timeout--;
				if(UART1_Fired){
					UART1_Fired = 0;
					edit_time();
					COMMAND(CLEAR_LCD);
					break;
				}
				if(!timeout){
					timeout = 6000000;
					if(!display){
						display = 1;
						COMMAND(CLEAR_LCD);
						DATA(" TAP ADMIN CARD");
						COMMAND(GOTO_LINE2_POS0);
						DATA("   TO RESOLVE");
					}
					else{
						display = 0;
						COMMAND(CLEAR_LCD);
						DATA("PREVIOUS  RECORD");
						COMMAND(GOTO_LINE2_POS0 + 4);
						DATA(UART0_BUFFER);
					}
				}	
			}
		}
		else if((response == 'C') || (response == 'c')){
			COMMAND(CLEAR_LCD);
			DATA("   TRY  AFTER");
			COMMAND(GOTO_LINE2_POS0);
			DATA("    09:00:00");
			delay_ms(1000);
		}
	}
}

void frame(char *frame, char *ID, char *purpose){
	int i = 0;
	int j = 0;
	char ones_place;
	char tens_place;
	
	//ID Field
	frame[i] = '[';
	for(i = 1; ID[i - 1]; i++){
		frame[i] = ID[i - 1];
	}
	frame[i++] = ']';
	
	//Purpose Field
	frame[i++] = '(';
	for(j = 0; purpose[j]; j++){
		frame[i++] = purpose[j];
	}
	frame[i++] = ')';
	
	//Time Field
	frame[i++] = '{';
	ones_place = ((HOUR % 10) + 48);
	tens_place = ((HOUR / 10) + 48);
	frame[i++] = tens_place;
	frame[i++] = ones_place;
	frame[i++] = ':';
	ones_place = ((MIN % 10) + 48);
	tens_place = ((MIN / 10) + 48);
	frame[i++] = tens_place;
	frame[i++] = ones_place;
	frame[i++] = ':';
	ones_place = ((SEC % 10) + 48);
	tens_place = ((SEC / 10) + 48);
	frame[i++] = tens_place;
	frame[i++] = ones_place;
	frame[i++] = '}';
	
	//Null Termination
	frame[i] = '\0';
}
int string_len(char *ptr){
	int count = 0;
	while((*ptr != '\0') && (*ptr != '\n')){
		count++;
		ptr++;
	}
	return count;
}

void Capitalize_String(char *ptr){
	while(*ptr){
		if(islower(*ptr))
			*ptr = (*ptr) - 32;
		ptr++;
	}
}

void LCD_SCROLL(char *str, int row, int dir){
  int i, j;
  int len = 0;
  int start_col;
	
  // Calculate string length 
  while (str[len] != '\0')
		len++;

  // Calculate center start column
  start_col = (LCD - len) / 2;
  
	if (dir == LEFT){
    for (i = LCD; i >= start_col; i--){
			LCD_SetCursor(0, row);
			for (j = 0; j < LCD; j++){
				if ((j >= i) && ((j - i) < len))
					DATA_char(str[j - i]);
        else
					DATA_char(' ');
      }
            delay_ms(1);
    }
  }
  else{
		for (i = -len; i <= start_col; i++){
			LCD_SetCursor(0, row);
			for (j = 0; j < LCD; j++){
				if (((j - i) >= 0) && ((j - i) < len))
					DATA_char(str[j - i]);
        else
					DATA_char(' ');
      }
            delay_ms(1);
    }
  }
}
