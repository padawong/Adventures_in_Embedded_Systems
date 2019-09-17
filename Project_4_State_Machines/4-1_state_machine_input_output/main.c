#include <avr/io.h>

enum States{start, ON1, ON2} state;

unsigned char tmpA = 0x00;
unsigned char tmpB = 0x00;
unsigned char tmpA_prev = 0x00;

void sequence(){
	tmpA = PINA & 0x01;

	unsigned int button_pressed = 0;
	// If value of PA0 went from 0 to 1, the button was pressed
	if (tmpA == 0x01 && tmpA_prev == 0x00) {
		button_pressed = 1;
	}
	tmpA_prev = tmpA;

	switch (state){
		case start:
			state = ON1;
			break;
		case ON1:
			if(button_pressed == 1){
				state = ON2;
				button_pressed = 0;
			}
			break;
		case ON2:
			if(button_pressed == 1){
				state = ON1;
				button_pressed = 0;
			}
			break;
		default:
			state = start;
			break;
	}
	
	switch (state){
		case start:
			break;
		case ON1:
			tmpB = 0x01;
			break;
		case ON2:
			tmpB = 0x02;
			break;
		default:
			break;
	}
	
	PORTB = tmpB;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	tmpB = 0x00;
	state = start;
	while (1)
	{
		sequence();
	}
}
