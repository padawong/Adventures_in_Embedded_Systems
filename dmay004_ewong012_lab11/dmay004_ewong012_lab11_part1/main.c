/*
 *	Partner 1 Name & E-mail: David May dmay004@ucr.edu
 *  Partner 2 Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 
 *	Assignment: Lab # 11 Exercise # 1
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "helper/io.h"
#include "helper/bit.h"

typedef struct task
{
	unsigned char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	void (*TickFct)();
} task;

const unsigned short numTasks = 1;
task tasks[numTasks];
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char note = 0;

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

void TimerISR() {
	TimerFlag = 1;
}

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

int main()
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRA = 0xF0; PORTA = 0x0F;
	
	unsigned char i = 0;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &Keypad;
	
	TimerSet(1);
	TimerOn();
	
	while(1)
	{
		PORTC = 0xFF;
 		for (i = 0; i < numTasks; i++)
 		{
 			if(tasks[i].elapsedTime >= tasks[i].period )
 			{
 				tasks[i].TickFct();
 				tasks[i].elapsedTime = 0;
 			}
 			tasks[i].elapsedTime++;
 		}
 		while(!TimerFlag);
 		TimerFlag = 0;
	}
	return 0;
}
