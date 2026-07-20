#include <LPC21xx.h>

#define CLEAR_LCD            0x01 
#define RET_CUR_HOME         0x02 
#define SHIFT_CUR_RIGHT      0x06 
#define SHIFT_CUR_LEFT       0x07 
#define DSP_OFF              0x08 
#define DSP_ON_CUR_OFF       0x0c 
#define DSP_ON_CUR_ON        0x0E 
#define DSP_ON_CUR_BLINK     0x0F 
#define SHIFT_DISP_LEFT      0x10 
#define SHIFT_DISP_RIGHT     0x14 
#define MODE_8BIT_1LINE      0x30 
#define MODE_4BIT_1LINE      0x20 
#define MODE_8BIT_2LINE      0x38 
#define MODE_4BIT_2LINE      0x28 
#define GOTO_LINE1_POS0      0x80 
#define GOTO_LINE2_POS0      0xC0 
#define GOTO_LINE3_POS0      0x94 
#define GOTO_LINE4_POS0      0xD4 
#define GOTO_CGRAM_START     0x40 


#define LCD_RS    11
#define LCD_RW    26    // No need
#define LCD_EN    10
#define SHIFT     16    

#define cmd_mode IOCLR0 |= 1 << LCD_RS
#define data_mode IOSET0 |= 1 << LCD_RS
#define write_lcd IOSET1 &= ~(1 << LCD_RW) // No Need
#define read_lcd IOSET1 = 1 << LCD_RW // No Need
