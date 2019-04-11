/*	Partner 1 Name & E-mail: David May; dmay004@ucr.edu
 *	Partner 2 Name & E-mail: 
 *	Lab Section: 026
 *	Assignment: Lab 3  Exercise 3
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}


int main() {
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRC = 0xFF; PORTC = 0x00; // output
	unsigned char tmpOutput = 0x00;
	unsigned char tmpInput = 0x00;
	unsigned char tmpSeatBelt = 0x00;
	unsigned char tmpIgnition = 0x00;
	unsigned char tmpDriver = 0x00;
	
		
	while (1) {
		tmpOutput = 0x00;
		tmpSeatBelt = PINA & 0x40;
		tmpIgnition = PINA & 0x10;
		tmpDriver = PINA & 0x20;

		if (!GetBit(tmpSeatBelt, 0)) {
			if (!GetBit(tmpIgnition, 1) && !GetBit(tmpDriver, 1)) {
				tmpOutput = SetBit(tmpOutput, 7, 1);
			}
		}
		
		tmpInput = 0x0F & PINA;
		
		if (tmpInput >= 13) {
			//light 5-0
			tmpOutput = SetBit(tmpOutput, 5, 1);
			tmpOutput = SetBit(tmpOutput, 4, 1);
			tmpOutput = SetBit(tmpOutput, 3, 1);
			tmpOutput = SetBit(tmpOutput, 2, 1);
			tmpOutput = SetBit(tmpOutput, 1, 1);
			tmpOutput = SetBit(tmpOutput, 0, 1);
		} else if (tmpInput >= 10) {
			//light 5-1
			tmpOutput = SetBit(tmpOutput, 5, 1);
			tmpOutput = SetBit(tmpOutput, 4, 1);
			tmpOutput = SetBit(tmpOutput, 3, 1);
			tmpOutput = SetBit(tmpOutput, 2, 1);
			tmpOutput = SetBit(tmpOutput, 1, 1);
		} else if (tmpInput >= 7) {
			//light 5-2
			tmpOutput = SetBit(tmpOutput, 5, 1);
			tmpOutput = SetBit(tmpOutput, 4, 1);
			tmpOutput = SetBit(tmpOutput, 3, 1);
			tmpOutput = SetBit(tmpOutput, 2, 1);
		} else if (tmpInput >= 5) {
			//light 5-3
			tmpOutput = SetBit(tmpOutput, 5, 1);
			tmpOutput = SetBit(tmpOutput, 4, 1);
			tmpOutput = SetBit(tmpOutput, 3, 1);
		} else if (tmpInput <= 4) {
			//light 5-4 + 6
			tmpOutput = SetBit(tmpOutput, 6, 1);
			if (tmpInput >= 3) {
				tmpOutput = SetBit(tmpOutput, 5, 1);
				tmpOutput = SetBit(tmpOutput, 4, 1);
			} else if (tmpInput >= 1) {
				tmpOutput = SetBit(tmpOutput, 5, 1);
			}
		}
		PORTC = tmpOutput;
	}
	
	return 0;
}
