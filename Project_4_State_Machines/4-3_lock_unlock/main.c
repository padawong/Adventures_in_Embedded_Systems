#include <avr/io.h>

enum States{start, LOCKED, UNLOCKED} state;

unsigned char x = 0x00;
unsigned char y = 0x00;
unsigned char y_prev = 0x00; // Can't have been pressed before pound for valid unlock sequence
unsigned char pound = 0x00;
unsigned char pound_prev = 0x00; // "press then release"
unsigned char input1_valid = 0; // 0 if invalid; 1 if valid

void sequence(){
	// Read input
	x = PINA & 0x01;
	y = PINA & 0x02;
	pound = PINA & 0x04;
	unsigned char inner_button = PINA & 0x80;
	
	unsigned char tmpC = 0x00;
	
	switch (state){
		case start:
			state = LOCKED;
			break;
		case LOCKED:
			// ONLY WANT TO CHECK TO UNLOCK IF CURRENTLY LOCKED
			// If value of pound went from 0 to 1, the button was pressed
			// Check to make sure that y is not being pressed; will check for x next
			if (pound == 0x04 && pound_prev == 0x00 && y == 0x00) {
				input1_valid = 1; // first input valid
			}
			pound_prev = pound;
				
			// If any button other than y is pressed, input1 no longer valid
			// Don't need to check inner button
			if (x == 0x01) {
				input1_valid = 0;
			}
				
			unsigned char unlock_valid = 0;
			// If input1 is valid and y went from 0 to 1, unlock
			// Check to make sure that only y is being pressed
			if (y == 0x02 && y_prev == 0x00) {
				if (input1_valid == 1 && pound == 0x00 && x == 0x00 && inner_button == 0x00) {
					unlock_valid = 1;
				}
				else {
					unlock_valid = 0;
				}
			}
			
			y_prev = y;
				
			if(unlock_valid == 1){
				state = UNLOCKED;
				unlock_valid = 0;
			}
			break;
			
		case UNLOCKED:
			// ONLY WANT TO CHECK TO LOCK IF CURRENTLY UNLOCKED
			if (inner_button == 0x80 && x == 0x00 && y == 0x00 && pound == 0x00) {
				state = LOCKED;
			}
			break;
		default:
			state = start;
			break;
	}
	
	if (state == LOCKED) {
		tmpC = 0x00;
	}
	else {
		tmpC = 0x01;
	}
	
	PORTC = tmpC;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	state = start;
	while (1)
	{
		sequence();
	}
}
