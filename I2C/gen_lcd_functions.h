#include "types.h"

void COMMAND(uc8);
void DATA(c8 *);
void DATA_char(uc8);
void enable_lcd(void);
void U32LCD(ui32 n);
void InitializeLCD(void);
void LCD_Custom_Char(uc8 location, uc8 *msg);
void LCD_SetCursor(uc8, uc8);
