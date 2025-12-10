# I2C-Protocol
This project demonstrates I2C master communication on the NXP LPC2148 Microcontroller to perform read and write operations on a 24Cxx-series EEPROM. A string is written into EEPROM, then read back using a repeated-start sequence, and finally displayed on a 16x2 LCD.

## ✨Features

1. I2C0 initialization at 100 kHz
2. EEPROM random write + sequential read
3. Correct ACK/NACK handling using AA bit
4. Repeated-START based read operation
5. LCD display output for verification

## ⚒️How It Works

1. Configure P0.2/P0.3 as SDA/SCL
2. Generate START → send SLA+W (0xA0)
3. Send memory address (high + low)
4. Write string bytes sequentially
5. STOP + mandatory EEPROM write delay
6. START → SLA+W → resend address
7. Repeated-START → SLA+R (0xA1)
8. Read bytes with ACK; send NACK for last byte
9. STOP and show received data on LCD

## 🧩Requirements

1. LPC2148 board
2. 24C256 EEPROM
3. 16x2 LCD
