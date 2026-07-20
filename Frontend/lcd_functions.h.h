#include "types.h"

void COMMAND(uc8);
void DATA(c8 *);
void DATA_char(uc8);
void enable_lcd(void);
void U32LCD(ui32 n);
void InitializeLCD(void);
void LCD_Custom_Char(uc8 location, uc8 *msg);
void LCD_SetCursor(uc8, uc8);
void admin_settings(void);
i32 edit_time(void);
i32 time_(void);
i32 date_(void);
i32 day_(void);
void add_leading_zeros(int zeros);
i32 count_digits(int year_count);
void change_admin(void);
char check_admin(char *ID);
void admin(void);
void user(void);
void frame(char *frame, char *ID, char *purpose);
int string_len(char *ptr);
void Capitalize_String(char *ptr);
void LCD_SCROLL(char *str, int row, int dir);
