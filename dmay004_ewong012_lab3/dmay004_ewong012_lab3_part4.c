/*	Partner 1 Name & E-mail: David May; dmay004@ucr.edu
 *	Partner 2 Name & E-mail: Erin Wong; ewong012@ucr.edu
 *	Lab Section: 026
 *	Assignment: Lab 3  Exercise 4 
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

int main() {
    DDRA = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0xF0;
    DDRC = 0xFF; PORTC = 0x0F;
	
	unsigned char tmpOutput = 0x00;
	unsigned char tmp1 = 0x00;
	unsigned char tmp2 = 0x00;
	unsigned char tmp3 = 0x00;
	unsigned char tmp4 = 0x00;
	
	
    while (1) {
		
		tmpOutput = 0x00;
		
		tmp1 = PINA & 0x01;
		tmp2 = PINA & 0x02;
		tmp3 = PINA & 0x04;
		tmp4 = PINA & 0x08;
		
		tmpOutput = SetBit(tmpOutput, 7, tmp4);
		tmpOutput = SetBit(tmpOutput, 6, tmp3);
		tmpOutput = SetBit(tmpOutput, 5, tmp2);
		tmpOutput = SetBit(tmpOutput, 4, tmp1);
		
		PORTC = tmpOutput;
		
		tmpOutput = 0x00;
		
		tmp1 = PINA & 0x10;
		tmp2 = PINA & 0x20;
		tmp3 = PINA & 0x40;
		tmp4 = PINA & 0x80;
		
		tmpOutput = SetBit(tmpOutput, 3, tmp4);
		tmpOutput = SetBit(tmpOutput, 2, tmp3);
		tmpOutput = SetBit(tmpOutput, 1, tmp2);
		tmpOutput = SetBit(tmpOutput, 0, tmp1);
		
		PORTB = tmpOutput;
    }
}

