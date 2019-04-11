/*	Partner 1 Name & E-mail: David May; dmay004@ucr.edu
 *	Partner 2 Name & E-mail: Erin Wong; ewong012@ucr.edu
 *	Lab Section: 026
 *	Assignment: Lab 3  Exercise 1 
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */


#include <avr/io.h>


//unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
//	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
//}

//unsigned char GetBit(unsigned char x, unsigned char k) {
//	return ((x & (0x01 << k)) != 0);
//}

int main() {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	unsigned char tmp1 = 0x00;
	unsigned char tmp2 = 0x00;
	unsigned char tmp3 = 0x00;
	unsigned char tmp4 = 0x00;
	unsigned char tmp5 = 0x00;
	unsigned char tmp6 = 0x00;
	unsigned char tmp7 = 0x00;
	unsigned char tmp8 = 0x00;
	unsigned char numOnes = 0x00;

	
	while (1) {
		numOnes = 0x00;
		
		tmp1 = PINA & 0x01;
		tmp2 = PINA & 0x02;
		tmp3 = PINA & 0x04;
		tmp4 = PINA & 0x08;
		tmp5 = PINA & 0x10;
		tmp6 = PINA & 0x20;
		tmp7 = PINA & 0x40;
		tmp8 = PINA & 0x80;
		
		if (tmp1 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp2 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp3 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp4 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp5 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp6 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp7 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp8 != 0x00) {
			numOnes += 0x01;
		}
		
		
		tmp1 = PINB & 0x01;
		tmp2 = PINB & 0x02;
		tmp3 = PINB & 0x04;
		tmp4 = PINB & 0x08;
		tmp5 = PINB & 0x10;
		tmp6 = PINB & 0x20;
		tmp7 = PINB & 0x40;
		tmp8 = PINB & 0x80;

		if (tmp1 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp2 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp3 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp4 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp5 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp6 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp7 != 0x00) {
			numOnes += 0x01;
		}
		if (tmp8 != 0x00) {
			numOnes += 0x01;
		}
		
		PORTC = numOnes;
	}
	return 0;
}

