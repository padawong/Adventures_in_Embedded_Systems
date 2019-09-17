#include <avr/io.h>
#include <avr/interrupt.h>

void set_PWM(double frequency) {
	static double current_frequency; 
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; }
		else { TCCR0B |= 0x03; }
		
		if (frequency < 0.954) { OCR0A = 0xFFFF; }
		
		else if (frequency > 31250) { OCR0A = 0x0000; }
		
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

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


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	DDRC = 0xFF; PORTC = 0x00;
	
	double c4 = 261.63;
	double d4 = 293.66;
	double e4 = 329.63;
	
	PWM_on();
	
    while (1) {
		unsigned char button1 = (~PINA & 0x01);
		unsigned char button2 = (~PINA & 0x02);
		unsigned char button3 = (~PINA & 0x04);
	
	
		while (button1 == 1 && button2 == 0 && button3 == 0) {
			//PWM_on();
			set_PWM(c4);
			
			
			//PORTC = 0x01;
			button1 = (~PINA & 0x01);
			button2 = (~PINA & 0x02);
			button3 = (~PINA & 0x04);

		}
		while (button2 == 0x02 && button1 == 0 && button3 == 0) {
			//PWM_on();
			set_PWM(d4);
			
			//PORTC = 0x02;

			button1 = (~PINA & 0x01);
			button2 = (~PINA & 0x02);
			button3 = (~PINA & 0x04);

		}
		while (button3 == 0x04 && button1 == 0 && button2 == 0) {
			//PWM_on();
			set_PWM(e4);
			
			//PORTC = 0x04;
			button1 = (~PINA & 0x01);
			button2 = (~PINA & 0x02);
			button3 = (~PINA & 0x04);
		}
		
		//PORTC = 0x00;
		//PWM_off();
		set_PWM(0);
    }
}

