#include <LPC21xx.h>
#include "reserve_pins_defines.h"
#include "reserve_pins.h"
#include "keymatrix.h"
#include "types.h"
#include "delay.h"
#include "defines.h"

c8 get_key(){           // Returns a char for the pressed key, or '\0' or '#'if timeouts
  i32 row,col;          // Iteration indices for scanning rows and columns
	unsigned int timeout = 75000 * SECONDS;  
	
	//For testing in Proteus Simulation
	uc8 key[4][4] = {                // Key mapping for Proteus: rows x cols -> character
    {'7', '8', '9', 'A'},           // Row 0: 7 8 9 A
    {'4', '5', '6', 'B'},           // Row 1: 4 5 6 B
    {'1', '2', '3', 'C'},           // Row 2: 1 2 3 C
    {'*', '0', '=', 'D'}            // Row 3: * 0 = D
	};
	
	//For testing on KIT
	/*uc8 key[4][4] = {              // Alternative mapping for the real kit (commented)
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '=', 'D'}
	};*/
	
	ReservePins(1, 4, 24, OUTPUT);
	ReservePins(1, 4, 28, INPUT);

while(1){
	timeout--;
	if(!timeout)
		return '#';
	for(row = 0; row < ROWS; row++){          // Scan each row one by one
    WRITENIBBLE(IOSET1,R1,0xF);               // Drive all row lines high (set R1..R4 = 1)
    CLEAR_IO(IOCLR1,(R1 + row));                 // Pull the current row low (activate this row)
    for(col = 0; col < COLS; col++){          // Scan all columns for this active row
       if(!(IOPIN1 & (1 << (C1 + col)))){     // Check if column line is pulled low (key press detected)
         delay_us(50);                        // Debounce: short delay to filter bouncing
         if(!(IOPIN1 & (1 << (C1 + col)))){   // Confirm key is still pressed after debounce
            while(!(IOPIN1 & (1 << (C1 + col)))); // Wait here until key is released (blocking)
              WRITENIBBLE(IOSET1,R1,0xF);     // Restore rows to high before exiting
              return key[row][col];           // Return the mapped character for (row, col)
         }
			 }
		 }
	 }
 }
   //return '\0';                                // No key press detected after full scan
}
