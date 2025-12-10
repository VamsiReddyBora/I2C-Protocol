#include <LPC21xx.h>                // LPC2129 register definitions
#include "gen_lcd_defines.h"        // LCD Generic functions
#include "gen_lcd_functions.h"      //LCD functions declaration
#include "delay.h"                  // Delay functions declaration
#include "types.h"
#include "defines.h"

/* I2C Speed Calculation:
I2C Speed = (PCLK / (I2SCLH + I2SCLL));
for I2C Speed = 100000 (100KHz) then
      (I2SCLH + I2SCLL) = (15MHz / 100KHz);
			then, I2SCLH = 75 and I2SCLL = 75;
*/

#define I2C_SPEED_DIV   75    // SCL high/low counts for ~100 kHz
#define SLAVE_ADDR      0x50  // 7-bit slave address
#define STA             5     // STA bit in I2CONSET
#define SI              3     // SI bit in I2CONSET
#define I2EN            6     // I2EN bit in I2CONSET
#define AA              2     // AA bit in I2CONSET
#define STO             4     // STO bit in I2CONSET      

void Initialize_Master_I2C(void){
	PINSEL0 &= ~((3 << 4) | (3 << 6));   // clear bits for P0.2 & P0.3
	PINSEL0 |=  ((1 << 4) | (1 << 6));   // set them to I2C0
  I2SCLH   = I2C_SPEED_DIV;            // SCL high time
  I2SCLL   = I2C_SPEED_DIV;            // SCL low time
	I2CONCLR = 0x6C;                     // Clearing all FLags
  I2CONSET = (1 << I2EN);              // I2EN = 1 
}

void Start_I2C(void){
  I2CONSET = (1 << STA);               // STA=1
  while ((I2CONSET & (1 << SI)) == 0); // Wait SI=1 ? START finished
	I2CONCLR = (1 << STA);               // STA = 0
}

void Stop_I2C(void){
  I2CONSET = (1 << STO);              // STO=1, request STOP on bus
  I2CONCLR = (1 << SI);               // Clear SI flag
}

void Write_I2C(unsigned char byte){
	I2DAT    = byte;                       // Load byte to transmit register
  I2CONCLR = (1 << SI);                  // Clear SI and start shifting
  while ((I2CONSET & (1 << SI)) == 0);   // Wait SI=1 ? byte done
}

void EEPROM_Location(char higher_byte, char lower_byte){
	Write_I2C(higher_byte);     //Memory location
	Write_I2C(lower_byte);      //Memory location
}

int main(void){
	
	char data[30] = {0};           // String act as buffer, when reading from EEPROM
	unsigned int i = 0; 
	char dummy_byte;
	
	char str[] = "ELECTRONICS MADE LIFE EASIER";    
	char *pstr = str;                     // Pointer to string str
	
	InitializeLCD();
  Initialize_Master_I2C();  
  
//-------------------------------------- Writing to EEPROM ---------------------------------------------------------

	Start_I2C();                   // Triggering START Condition on to the BUS
	
	//Sending Slave address + data
	Write_I2C(0xA0);               // Sending Slave address on to the BUS
	
	//Checking if the Slave acknowledges to the address or not
	//0x18 is the define state of I2C, means SLA+W (Slave address + Write mode, ACK Received)
	//0x20 in case the Slave didn't acknowledges, means (Slave address + Write mode, No ACK Received)
	if(I2STAT == 0x18){
		EEPROM_Location(0x00,0x00);  // (HigherByte, LowerByte) 16-bit EEPROM address
		pstr = str;
		while(*pstr){
			Write_I2C(*pstr++);        // Sending byte by byte on to the BUS, as a string
		}
	}
	
	Stop_I2C();                    // Triggering STOP Condition
	delay_ms(10);                  // Write cycle delay min. 5ms
	
//------------------------------------- Reading from EEPROM ----------------------------------------------------------------

	Start_I2C();                   // Triggering START Condition on to the BUS
	
	//Sending Slave address + data
	Write_I2C(0xA0);               // Sending Slave address on to the BUS
	
	if(I2STAT == 0x18){
		EEPROM_Location(0x00,0x0A);  // (HigherByte, LowerByte) 16-bit EEPROM address
	}

	// Uncomment this line if RESTART is not appear on the BUS. 
	//I2CONCLR = (1 << SI);
	Start_I2C(); 									 //Repeated start
	
	//Sending Slave address + Read mode, to read a byte from Slave without leaving the BUS
	Write_I2C(0xA1);
		
	if(I2STAT == 0x40){                        // SLA+R, ACK
		I2CONSET = (1 << AA);                    // Assert Acknowledge to slave by the Master
		while(i <= 9){                           // Reading 10 bytes from EEPROM
			I2CONCLR = (1 << SI);                  // clear SI -> start receive
			while (!(I2CONSET & (1 << SI)));       // Wait for byte
			data[i++] = I2DAT;                     // Read byte
		}
		I2CONCLR = (1 << AA);                    // Sending NACK on to the BUS
		
		//Reading I2DAT for Triggering NACK on to the BUS
		I2CONCLR = (1 << SI);                  // clear SI -> start receive
		while (!(I2CONSET & (1 << SI)));       // Wait for byte
		dummy_byte = I2DAT;                     // Just a dummy byte no use
	}
	
	Stop_I2C();                                // Triggering STOP Condition
	delay_ms(10);                              // Write cycle delay min. 5ms

	DATA(data);                             //Sending data on to the LCD
	
//Uncomment this, if you want another EEPROM as slave on the BUS
/*	
//---------------------------------------- Writing to 2nd EEPROM --------------------------------------------------------

	Start_I2C();                            // Triggering START Condition on to the BUS
	
	//Sending Slave address + data
	Write_I2C(0xA2);                        // Sending Slave address on to the BUS

	//Checking if the Slave acknowledges to the address or not
	//0x18 is the define state of I2C, means SLA+W (Slave address + Write mode, ACK Received)
	//0x20 in case the Slave didn't acknowledges, means (Slave address + Write mode, No ACK Received)
	if(I2STAT == 0x18){
		EEPROM_Location(0x00,0x00);           // (HigherByte, LowerByte) 16-bit EEPROM address
		pstr = data;
		while(*pstr){
			Write_I2C(*pstr++);                 // Sending byte by byte on to the BUS, as a string
		}
	}
	
	Stop_I2C();                             // Triggering STOP Condition
	delay_ms(10);                           // Write cycle delay min. 5ms
*/

  while (1);                              // Super loop
}

