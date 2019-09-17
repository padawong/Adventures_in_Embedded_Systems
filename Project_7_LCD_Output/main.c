#include <util/delay.h>
#include "io.c"

int main(void)
{
	//DDRA = 0xF0; PORTA = 0x0F;
	DDRA = 0x0F; PORTA = 0xF0; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
		
	// Initializes the LCD display
	LCD_init();

	unsigned char inputA = 0x00;
	unsigned char num = 0x00;
	unsigned char state = 0;
	
	unsigned char prevState = 0;
	
	while (1)
	{
		switch(state) {
			case 0:
				prevState = 0;
				num = 0;
				LCD_Cursor(1);
				LCD_WriteData(num +'0');
				state = 1;
				break;
			case 1:
				inputA = ~PINA;
				if(inputA == 0x01) { //pin 0 high
					state = 2;
				}
				else if(inputA == 0x02) { //pin 1 high
					state = 3;
				}
				else if(inputA == 0x03) { //pin 1 and 0 high
					state = 4;
				}
				else {
					state = 5;
				}
				break;
			case 2:
				if (prevState == 2) {
					_delay_ms(4000);
				}
				
				prevState = 2;
				
				if(num != 0x09) {		
					++num;
					LCD_Cursor(1);
					LCD_WriteData(num +'0');
					state = 6;
				}
				else {
					LCD_Cursor(1);
					LCD_WriteData(num +'0');
					state = 6;
				}
				break;
			case 3:
				if (prevState == 3) {
					_delay_ms(4000);
				}
				
				prevState = 3;
				
				if(num != 0x00) {
					--num;
					LCD_Cursor(1);
					LCD_WriteData(num +'0');
					state = 6;
				}
				else {
					LCD_Cursor(1);
					LCD_WriteData(num +'0');
					state = 6;
				}
				break;
			case 4:
				prevState = 4;
				num = 0;
				LCD_Cursor(1);
				LCD_WriteData(num +'0');
			
				state = 6;
				break;
			case 5:
				prevState = 5;
				state = 1;
				break;
			case 6:
				_delay_ms(2000);
				state = 1;
				break;
			default:
			_delay_ms(1000);
				num = 0xFF;
				LCD_Cursor(1);
				LCD_WriteData(num +'0'); //should never be reached
				break;
		}
	}
}
