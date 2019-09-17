#include <avr/io.h>

unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
	return (b ? x | (0x01 << k) : x & ~(0x01 << k));
}

unsigned char GetBit(unsigned char x, unsigned char k) {
	return ((x & (0x01 << k)) != 0);
}


int main() {
	DDRA = 0x00; PORTA = 0xF0; // input
	DDRC = 0xFF; PORTC = 0x00; // output
	//DDRB = 0xFF; PORTB = 0x00; 
	unsigned char tmpOutput = 0x00;
	unsigned char tmpInput = 0x00;
	//unsigned char tmpInput = 0x00;
	//unsigned char tmp1 = 0x00;
	//unsigned char tmp2 = 0x00;
	//unsigned char tmp3 = 0x00;
	//unsigned char tmp4 = 0x00;	
		
	while (1) {
		tmpOutput = 0x00;
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
