/*	
 *	Partner 1 Name & E-mail: David May; dmay004@ucr.edu
 *	Partner 2 Name & E-mail: Erin Wong; ewong012@ucr.edu
 *	Lab Section: 026
 *	Assignment: Lab 8 Exercise 1 & 2
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
	
	DDRD = 0xFF;
	PORTD = 0x00;
	
	ADC_init();
	
	// Brightest light: 35
	// Darkest setting: 992
	unsigned char MAX = (992-35)/2;
    while (1) 
    {
		unsigned short input = ADC;
		if (input <= MAX) { // Brighter than threshold; illuminate LED
			PORTC = 0x01;
		}
		else {
			PORTC = 0x00;
		}
    }
}

