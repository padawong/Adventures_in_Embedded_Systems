#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct task {
	int state; // Current state of the task
	unsigned long period; // Rate at which the task should tick
	unsigned long elapsedTime; // Time since task's previous tick
	int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 100;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 1000;

enum BL_States { BL_SMStart, BL_s1 };
int TickFct_BlinkLED(int state);

int TickFct_ThreeLEDs(int state);
enum TL_States { TL_SMStart, TL_s1, TL_s2, TL_s3 };

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
		if (PORTC & 0x01 == 1) {
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