#include <avr/io.h>

enum States{start, increment, decrement, hold, reset} state;

unsigned char incCheck = 0x00;
unsigned char incCheck_prev = 0x00;
unsigned char decCheck = 0x00;
unsigned char decCheck_prev = 0x00;

unsigned char tmpB = 0x00;
//unsigned int checkRst = 0;
//unsigned int checkRst2 = 0;
//unsigned int rstCheck = 0;

void sequence() {
	incCheck = PINA & 0x01;
	decCheck = PINA & 0x02;

	switch (state) {
		case start:
			if (incCheck == 0x01 && decCheck == 0x00) {
				state = increment;
				//checkRst = 1;
				//checkRst2 = 0;
			} else if (decCheck == 0x02 && incCheck == 0x00) {
				state = decrement;
				//checkRst2 = 1;
				//checkRst = 0;
			} else {
				state = start;
			}
			break;
			
		case hold:
			if (decCheck == 0x02 && incCheck == 0x01) {
				state = reset;
			} else if (incCheck == 0x01 && decCheck == 0x00) {
				// Initial button press
				if (incCheck_prev == 0) {
					state = increment;
					//incCheck_prev = 1;
					//decCheck_prev = 0;
				}
				// Button is being held down; has already registered one action
				else {
					state = hold;
				}
			} else if (decCheck == 0x02 && incCheck == 0x00) {
				// Initial button press
				if (decCheck_prev == 0) {
					state = decrement;
					//decCheck_prev = 1;
					//incCheck_prev = 0;
				}
			} else {
				state = hold;
			}
			break;

		case increment:
			/*if (checkRst2) {
				state = reset;
			}
			else {
				state = hold;
			}*/
			break;

		case decrement:
			/*if (checkRst) {
				state = reset;
			}
			else {
				state = hold;
			}*/
 			break;
		
		case reset:
			state = reset;
			break;
		
		default:
			state = start;
			break;
	}
	
	decCheck_prev = decCheck;
	incCheck_prev = incCheck;

	switch (state) {
		case start:
			tmpB = 0x07;
			break;
		
		case increment:
			if (tmpB != 9) {
				tmpB += 0x01;
			}
			incCheck_prev = 1;
			state = hold;
			break;
		
		case decrement:
			if (tmpB != 0) {
				tmpB -= 0x01;
			}
			decCheck_prev = 1;
			state = hold;
			break;
		
		case reset:
			tmpB = 0x00;
			state = hold;
			break;
		
		default:
			break;
	}

	PORTB = tmpB;
}


int main() {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	tmpB = 0x00;
	incCheck = 0x00;
	decCheck = 0x00;
	//rstCheck = 0;
	state = start;
	while (1) {
		sequence();
	}
	return 0;
}
