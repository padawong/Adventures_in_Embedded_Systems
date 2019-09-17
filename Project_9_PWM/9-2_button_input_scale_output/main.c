#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { 
			TCCR0B &= 0x08; 
		} else { 
			TCCR0B |= 0x03; 
		} 
		
		if (frequency < 0.954) { 
			OCR0A = 0xFFFF; 
		} else if (frequency > 31250) { 
			OCR0A = 0x0000; 
		} else { 
			OCR0A = (short)(8000000 / (128 * frequency)) - 1; 
		}

		TCNT0 = 0;
		current_frequency = frequency; 
	}
}

void PWM_on() {
	TCCR0A = (1 << COM0A0 | 1 << WGM00);
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

enum States{off, turnOff, on, turnOn, inc, waitInc, dec, waitDec}state;


unsigned char button = 0x00;

const double notes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
unsigned char currNote = 0x00;

void sequence(){
	button = ~PINA & 0x07;
	switch(state){ // Transitions
		case off:
		if(button == 1) {
			state = on;
		} else {
			state = off;
		}
		break;
		
		case turnOff:
		if (!(button == 1)) {
			state = off;
		} else {
			state = turnOff;
		}
		break;
		
		case on:
		if(button == 1) {
			state = on;
		} else {
			state = turnOn;
		}
		break;
		
		case turnOn:
		if(button == 2) {
			if(currNote < 7)
			currNote++;
			state = inc;
		} else if(button == 4){
			if(currNote > 0)
			currNote--;
			state = dec;
		} else if(button == 1) {
			state = turnOff;
		} else {
			state = turnOn;
		}
		break;
		
		case inc:
		state = waitInc;
		break;
		
		case waitInc:
		if (button == 2) {
			state = waitInc;
		} else {
			state = turnOn;
		}
		break;
		
		case dec:
		state = waitDec;
		break;
		
		case waitDec:
		if(button == 4) {
			state = waitDec;
		} else {
			state = turnOn;
		}
		break;
		
	}
	switch (state) {
		case off:
		PWM_off();
		break;
		
		case turnOff:
		break;
		
		case on:
		PWM_on();
		break;
		
		case turnOn:
		break;
		
		case inc:
		set_PWM(notes[currNote]);
		_delay_ms(4000);
		break;
		
		case waitInc:
		break;
		
		case dec:
		set_PWM(notes[currNote]);
		_delay_ms(4000);
		break;
		
		case waitDec:
		break;
	}
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // 3 button input
	DDRB = 0xFF; PORTB = 0x00; // speaker output
	PWM_on();
	state = off;
	while(1){
		sequence();
	}
}
