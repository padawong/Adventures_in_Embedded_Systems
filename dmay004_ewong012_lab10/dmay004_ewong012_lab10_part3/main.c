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

typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 1;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 300;
const unsigned long periodBuzz= 1;

enum BL_States { BL_SMStart, BL_s1 };
int TickFct_BlinkLED(int state);

int TickFct_ThreeLEDs(int state);
enum TL_States { TL_SMStart, TL_s1, TL_s2, TL_s3 };

enum Buzz_States {Buzz_SMStart, on, off};
int TickFct_Buzz(int state);

void TimerISR() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}
/*
void TimerISR() {
	TimerFlag = 1;
}*/
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

int main() {
	DDRC = 0xFF;
	PORTC = 0x00;
	DDRA = 0x00;
	PORTA = 0xFF;
	DDRB = 0xFF;
	PORTB = 0x00;
	
	unsigned char i=0;
	tasks[i].state = BL_SMStart;
	tasks[i].period = periodBlinkLED;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_BlinkLED;
	++i;
	tasks[i].state = TL_SMStart;
	tasks[i].period = periodThreeLEDs;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_ThreeLEDs;
	++i;
	tasks[i].state = Buzz_SMStart;
	tasks[i].period = periodBuzz;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Buzz;
		
	TimerSet(tasksPeriodGCD);
	TimerOn();
	
	while(1) {
	}
	return 0;
}

int TickFct_BlinkLED(int state) {
	switch(state) { // Transitions
		case BL_SMStart: // Initial transition
		PORTC = 0x00; // Initialization behavior
		state = BL_s1;
		break;
		case BL_s1:
		state = BL_s1;
		break;
		default:
		state = BL_SMStart;
	} // Transitions

	switch(state) { // State actions
		case BL_s1:
		if ((PORTC & 0x01) == 1) {
			PORTC &= ~1;
		}
		else {
			PORTC |= 1;
		}
		break;
		default:
		break;
	} // State actions
	return state;
}

int TickFct_ThreeLEDs(int state) {
	switch(state) { // Transitions
		case TL_SMStart: // Initial transition
		state = TL_s1;
		break;
		case TL_s1:
		state = TL_s2;
		break;
		case TL_s2:
		state = TL_s3;
		break;
		case TL_s3:
		state = TL_s1;
		break;
		default:
		state = TL_SMStart;
	} // Transitions

	switch(state) { // State actions
		case TL_s1:
		PORTC |= 1 << 1;
		//PORTC & 0x04 = 1;
		PORTC &= ~(1 << 2);
		//PORTC & 0x08 = 0;
		PORTC &= ~(1 << 3);
		//PORTC & 0x10 = 0;
		//B2 = 1; B3 = 0; B4 = 0;
		break;
		case TL_s2:
		PORTC &= ~(1 << 1);
		PORTC |= 1 << 2;
		PORTC &= ~(1 << 3);
		//B2 = 0; B3 = 1; B4 = 0;
		break;
		case TL_s3:
		PORTC &= ~(1 << 1);
		PORTC &= ~(1 << 2);
		PORTC |= 1 << 3;
		//B2 = 0; B3 = 0; B4 = 1;
		break;
		default:
		break;
	} // State actions
	return state;
}

int TickFct_Buzz(int state) {
	double c4 = 261.62;
	
	switch(state) { // Transitions
		case Buzz_SMStart: // Initial transition
			PORTB = 0x00; // Initialization behavior
			PWM_on();
			if (~PINA & 0x01 == 1) {
				state = on;
			}
			else {
				state = off;
			}
			break;
		case on:
			if (~PINA & 0x01 == 1) {
				state = on;
			}
			else {
				state = off;
			}
			break;
		case off:
			if (~PINA & 0x01 == 1) {
				state = on;
			}
			else {
				state = off;
			}
			break;
		default:
			state = Buzz_SMStart;
	} // Transitions

	switch(state) { // State actions
		case on:
			set_PWM(c4);
			//PORTC &= ~(1 << 7);
			//PORTC |= 1 << 6;
			break;
		case off:
			set_PWM(0);
			//PORTC |= 1 << 7;
			//PORTC &= ~(1 << 6);
			break;
		default:
			break;
	} // State actions
	return state;
}