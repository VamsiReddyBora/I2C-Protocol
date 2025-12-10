#include "gen_lcd_defines.h"     // LCD command/address definitions
#include "gen_lcd_functions.h"   // LCD function prototypes
#include "types.h"           // Project-specific typedefs (ui32, uc8, etc.)
#include "delay.h"           // delay_ms(), delay_us()
#include "defines.h"         // System-wide macros/defines (pins, masks, constants)

//Initializing LCD
void InitializeLCD(void){
	WRITEBYTE(IODIR1,SHIFT,0x3FF);
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
	SETBIT(IOSET1,LCD_EN);                      // Set E=1 (start latch window)
	delay_us(2);                                // Short enable pulse width
	IOCLR1 = 1 << LCD_EN;                      // Set E=0 (capture data/command)
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
