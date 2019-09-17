/*	
 *	Partner 1 Name & E-mail: David May; dmay004@ucr.edu
 *	Partner 2 Name & E-mail: Erin Wong; ewong012@ucr.edu
 *	Lab Section: 026
 *	Assignment: Lab 8 Exercise 4
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

int main(void)
{
    DDRA = 0x00; 
	PORTA = 0xFF; 
	
	DDRC = 0xFF; // Set C to output
	PORTC = 0x00;
	
	// Brightest light: 35
	// Darkest setting: 992
	unsigned short eighths = 119;// (992 - 35) / 8; // = 119
	
	ADC_init();
	
	unsigned short input;
    while (1) 
    {
		input = ADC;
		//unsigned short input = 512;
		
		// 119*7 + 35; darkest
		if (input >= (eighths * 7) + 35) {
			PORTC = 0x01;
		}
		// 119*6 + 35; 2nd darkest
		else if (input >= (eighths * 6) + 35) {
			PORTC = 0x03;
		}
		// 119*5 + 35; 3rd darkest
		else if (input >= (eighths * 5) + 35) {
			PORTC = 0x07;
		}
		// 119*4 + 35; 4th darkest
		else if (input >= (eighths * 4) + 35) {
			PORTC = 0x0F;
			PORTC = 0x0F;
		}
		// 119*3 + 45; 4th brightest
		else if (input >= (eighths * 3) + 35) {
			PORTC = 0x1F;
		}
		// 119*2 + 35; 3rd brightest
		else if (input >= (eighths * 2) + 35) {
			PORTC = 0x3F;
		}
		// 119 + 35; Second brightest
		else if (input >= eighths + 35) {
			PORTC = 0x7F;
		}
		// 35; Brightest
		else if (input < 119 + 35) {
			PORTC = 0xFF;
		}
		else {
			PORTC = 0x00;
		}
    }
}

