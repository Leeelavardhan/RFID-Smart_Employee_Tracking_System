#include <LPC21xx.h>
#include "rtc_defines.h"
#include "rtc_functions.h"
#include "gen_lcd_defines.h"
#include "gen_lcd_functions.h"
#include "delay.h"

// Weekday names for reference (Sunday to Saturday)
char week[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

// Function to initialize RTC module
void InitializeRTC(void){
	CCR = RTC_RESET;  // Reset the RTC counter and clock
#ifdef _LPC2148
  CCR = RTC_ENABLE | RTC_CLKSRC; // Enable the RTC and select the clock source (specific to LPC2148)
#else
  PREINT = PREINT_VAL; // Set prescaler integer part for RTC clock division
	PREFRAC= PREFRAC_VAL; // Set prescaler fractional part for RTC clock division
	CCR = RTC_ENABLE; // Enable the RTC (start counting)
#endif
}

// Function to read current time from RTC registers
void FetchTime(i16 *hour, i16 *minute, i16 *second){
	*hour = HOUR; // Read current hour from RTC and store at memory location pointed by hour
	*minute = MIN;// Read current minute from RTC and store at memory location pointed by minute
	*second = SEC;// Read current second from RTC and store at memory location pointed by second
}

// Function to display time on LCD (in HH:MM:SS format)
void PrintTime(ui32 hour, ui32 minute, ui32 second){
	delay_ms(1); // Delay for LCD command execution
	DATA_char((hour/10)+48); // Display tens digit of hour
	delay_ms(1);
	DATA_char((hour%10)+48); // Display ones digit of hour
	delay_ms(1);
	DATA_char(':'); // Display colon separator
	delay_ms(1);
	DATA_char((minute/10)+48); // Display tens digit of minute
	delay_ms(1);
	DATA_char((minute%10)+48); // Display ones digit of minute
	delay_ms(1);
	DATA_char(':'); // Display colon separator
	delay_ms(1);						   
	DATA_char((second/10)+48); // Display tens digit of second
	delay_ms(1);
	DATA_char((second%10)+48); // Display ones digit of second
	delay_ms(1);
}

// Function to display ON time on LCD with label
void PrintONTime(ui32 hour, ui32 minute, ui32 second){
	COMMAND(CLEAR_LCD); // Clear LCD display
	COMMAND(GOTO_LINE1_POS0); // Go to first line, position 0
	delay_ms(1);
	DATA_char((hour/10)+48); // Display tens digit of hour
	delay_ms(1);
	DATA_char((hour%10)+48); // Display ones digit of hour
	delay_ms(1);
	DATA_char(':'); // Separator
	delay_ms(1);
	DATA_char((minute/10)+48); // Display tens digit of minute
	delay_ms(1);
	DATA_char((minute%10)+48); // Display ones digit of minute
	delay_ms(1);
	DATA_char(':'); // Separator
	delay_ms(1);						   
	DATA_char((second/10)+48); // Display tens digit of second
	delay_ms(1);
	DATA_char((second%10)+48); // Display ones digit of second
	delay_ms(1);
	DATA(" ON TIME"); // Display text "ON TIME"
}

// Function to display OFF time on LCD with label
void PrintOFFTime(ui32 hour, ui32 minute, ui32 second){
	//COMMAND(CLEAR_LCD); // (Optional) clear LCD if needed
	COMMAND(GOTO_LINE2_POS0); // Move cursor to second line, position 0
	delay_ms(1);
	DATA_char((hour/10)+48); // Display tens digit of hour
	delay_ms(1);
	DATA_char((hour%10)+48); // Display ones digit of hour
	delay_ms(1);
	DATA_char(':'); // Separator
	delay_ms(1);
	DATA_char((minute/10)+48); // Display tens digit of minute
	delay_ms(1);
	DATA_char((minute%10)+48); // Display ones digit of minute
	delay_ms(1);
	DATA_char(':'); // Separator
	delay_ms(1);						   
	DATA_char((second/10)+48); // Display tens digit of second
	delay_ms(1);
	DATA_char((second%10)+48); // Display ones digit of second
	delay_ms(1);
	DATA(" OFFTIME"); // Display text "OFFTIME"
}

// Function to read date from RTC registers
void FetchDate(i16 *date, i16 *month, i16 *year){
	*date = DOM; // Read day of month from RTC
	*month = MONTH; // Read month from RTC
	*year = YEAR; // Read year from RTC
}

// Function to display date on LCD (in DD/MM/YYYY format)
void PrintDate(ui32 date, ui32 month, ui32 year){
		COMMAND(GOTO_LINE2_POS0); // Move cursor to second line, position 0
		DATA_char((date/10)+48); // Display tens digit of date
		DATA_char((date%10)+48); // Display ones digit of date
		DATA_char('/'); // Separator
		DATA_char((month/10)+48); // Display tens digit of month
		DATA_char((month%10)+48); // Display ones digit of month
		DATA_char('/'); // Separator
		U32LCD(year); // Display year using utility function
}

// Function to update RTC registers with new time
void UpdateRTCTime(ui32 hour, ui32 minute, ui32 second){
	HOUR = hour; // Write new hour into RTC register
	MIN = minute; // Write new minute into RTC register
	SEC = second; // Write new second into RTC register
}

// Function to update RTC registers with new date
void UpdateRTCDate(ui32 date, ui32 month, ui32 year){
	DOM = date; // Write day of month value
	MONTH = month; // Write month value
	YEAR = year; // Write year value
}

// Function to fetch day of week from RTC
void FetchDay(i16 *day){
	*day = DOW; // Read day of week (0=Sun, ..., 6=Sat)
}

// Function to display day of week on LCD
void PrintDay(ui32 dow){
	COMMAND(GOTO_LINE2_POS0+11); // Move LCD cursor to line 2, position 11
	DATA(week[dow]);  // Display corresponding day string (from array week)
}

// Function to update RTC with day of week
void UpdateRTCDay(ui32 day){
	DOW = day; // Set day of week in RTC register
}
