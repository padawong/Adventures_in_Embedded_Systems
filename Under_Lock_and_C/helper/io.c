#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))
#define LCD_Data_Dir DDRC	/* Define LCD data port direction */
#define LCD_Command_Dir DDRD	/* Define LCD command port direction register */
#define LCD_Data_Port PORTC	/* Define LCD data port */
#define LCD_Command_Port PORTD	/* Define LCD data port */
#define RS1 PD6			/* Define Register Select signal pin */
#define RW PD5			/* Define Read/Write signal pin */
#define EN PD7			/* Define Enable signal pin */
          
/*-------------------------------------------------------------------------*/

#define DATA_BUS PORTC		// port connected to pins 7-14 of LCD display
#define CONTROL_BUS PORTD	// port connected to pins 4 and 6 of LCD disp.
#define RS 6			// pin number of uC connected to pin 4 of LCD disp.
#define E 7			// pin number of uC connected to pin 6 of LCD disp.

/*-------------------------------------------------------------------------*/

void LCD_ClearScreen(void) {
   LCD_WriteCommand(0x01);
}

void LCD_init(void) {

    //wait for 100 ms.
	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);						 
}

void LCD_WriteCommand (unsigned char Command) {
   CLR_BIT(CONTROL_BUS,RS);
   DATA_BUS = Command;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(2); // ClearScreen requires 1.52ms to execute
}

void LCD_WriteData(unsigned char Data) {
   SET_BIT(CONTROL_BUS,RS);
   DATA_BUS = Data;
   SET_BIT(CONTROL_BUS,E);
   asm("nop");
   CLR_BIT(CONTROL_BUS,E);
   delay_ms(1);
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
   //LCD_ClearScreen();
   unsigned char c = column;
   while(*string) {
      LCD_Cursor(c++);
      LCD_WriteData(*string++);
   }
}

void LCD_Cursor(unsigned char column) {
   if ( column < 17 ) { // 16x1 LCD: column < 9
						// 16x2 LCD: column < 17
      LCD_WriteCommand(0x80 + column - 1);
   } else {
      LCD_WriteCommand(0xB8 + column - 9);	// 16x1 LCD: column - 1
											// 16x2 LCD: column - 9
   }
}

void delay_ms(int miliSec) //for 8 Mhz crystal
{
    int i,j;
    for(i=0;i<miliSec;i++)
    for(j=0;j<775;j++)
  {
   asm("nop");
  }
}

// Enables custom char write
 void LCD_Char (unsigned char char_data)  /* LCD data write function */
 {
	 LCD_Data_Port= char_data;
	 LCD_Command_Port |= (1<<RS1);	/* RS1=1 Data reg. */
	 LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	 LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	 _delay_us(1);
	 LCD_Command_Port &= ~(1<<EN);
	 _delay_ms(2);			/* Data write delay */
 }

// Function sets ability for custom char to be written
 void LCD_Command(unsigned char cmnd)
 {
	 LCD_Data_Port= cmnd;
	 LCD_Command_Port &= ~(1<<RS1);	/* RS1=0 command reg. */
	 LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	 LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	 _delay_us(1);
	 LCD_Command_Port &= ~(1<<EN);
	 _delay_ms(2);
 }

// Writes custom char to the specified location
 void LCD_Custom_Char (unsigned char loc, unsigned char *msg)
 {
	 unsigned char i;
	 if(loc<8)
	 {
		 LCD_Command (0x40 + (loc*8));	/* Command 0x40 and onwards forces the device to point CGRAM address */
		 for(i=0;i<8;i++)	/* Write 8 byte for generation of 1 character */
		 LCD_Char(msg[i]);
	 }
 }