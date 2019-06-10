/*
 *	Partner 1 Name & E-mail: David May dmay004@ucr.edu
 *  Partner 2 Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 026
 *	Assignment: Lab 11 Exercise 2
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "helper/io.h"
#include "helper/io.c"
#include "helper/bit.h"

typedef struct task {
	unsigned long int period;
	unsigned long int elapsedTime;
	void (*TickFct)();
} task;

task tasks[1];

const unsigned short numTasks = 1;

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

void display() {
	unsigned char message[44] = {'C','S','1','2','0','B',' ','I','S',' ',
        'L','e','g','e','n','d','.','.','.',' ','w','a','i','t',' ','f','o','r',' ','i','t',' ','D','A','R','Y','!',' ',
        ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static unsigned char clearScreen[16] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static unsigned char i = 0;
	
	for(unsigned char j = 1; j <= 16; j++) {
		LCD_Cursor(j);
		if(j < 16) {
			clearScreen[j - 1] = clearScreen[j];
		}
		else {
			clearScreen[j - 1] = message[i];
		}
		LCD_WriteData(clearScreen[j - 1]);
	}
	
	if (i >= 43) {
		i = 0;
	}
	else {
		i++;
	}
}

int main() {
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFC; PORTD = 0x03;
	unsigned char i = 0;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period; //start high
	tasks[i].TickFct = &display;
	
	TimerSet(200);
	TimerOn();
	LCD_init();
	
	while(1) {
		for (i = 0; i < numTasks; i++) {
			if(tasks[i].elapsedTime >= tasks[i].period) {
				tasks[i].TickFct();
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime = tasks[i].elapsedTime + 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}


