/*
 *	Partner 1 Name & E-mail: David May dmay004@ucr.edu
 *  Partner 2 Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 026
 *	Assignment: Lab 11 Exercise 4
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>#include <stdio.h>#include "helper/io.h"#include "helper/io.c"#include "helper/bit.h"
typedef struct task{	unsigned char state;	unsigned long int period;	unsigned long int elapsedTime;	int (*TickFct)(int);} task;
task tasks[2];
const unsigned short numTasks = 2;volatile unsigned char TimerFlag = 0;unsigned long _avr_timer_M = 1;unsigned long _avr_timer_cntcurr = 0;unsigned char notes = 0;unsigned char temp = 0;unsigned char in = 0;
void TimerOn(){	TCCR1B = 0x0B;	OCR1A = 125;	TIMSK1 = 0x02;	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff(){
	TCCR1B = 0x00;}
void TimerISR(){	TimerFlag = 1;}
ISR(TIMER1_COMPA_vect){	_avr_timer_cntcurr--;	if (_avr_timer_cntcurr == 0){		TimerISR();		_avr_timer_cntcurr = _avr_timer_M;	}}
void TimerSet(unsigned long M){	_avr_timer_M = M;	_avr_timer_cntcurr = _avr_timer_M;}
unsigned char GetKeypadKey(){	PORTB = 0xEF;	asm("nop");	if (GetBit(PINB,0)==0) { return('1'); }	if (GetBit(PINB,1)==0) { return('4'); }	if (GetBit(PINB,2)==0) { return('7'); }	if (GetBit(PINB,3)==0) { return('*'); }
	PORTB = 0xDF;	asm("nop");	if (GetBit(PINB,0)==0) { return('2'); }	if (GetBit(PINB,1)==0) { return('5'); }	if (GetBit(PINB,2)==0) { return('8'); }	if (GetBit(PINB,3)==0) { return('0'); }
	PORTB = 0xBF;	asm("nop");	if (GetBit(PINB,0)==0) { return('3'); }	if (GetBit(PINB,1)==0) { return('6'); }	if (GetBit(PINB,2)==0) { return('9'); }	if (GetBit(PINB,3)==0) { return('#'); }
	PORTB = 0x7F;	asm("nop");	if (GetBit(PINB,0)==0) { return('A'); }	if (GetBit(PINB,1)==0) { return('B'); }	if (GetBit(PINB,2)==0) { return('C'); }	if (GetBit(PINB,3)==0) { return('D'); }
	return('\0');}
enum Keypad_States {OKAY, WAIT, DONE};
unsigned char Keypad(unsigned char state){	in = GetKeypadKey();
	switch(state){		case OKAY:			if(in != '\0'){				state = WAIT;			}			else{				state = OKAY;			}
			break;		
		case WAIT:			if(in != '\0'){
				state = DONE;
			}
			else{
				state = OKAY;
			}
			break;
		case DONE:			if(in != '\0'){				state = DONE;
			}
			else{
				state = OKAY;
			}
			break;		
		default:			state = OKAY;			break;
	}
	switch(state){
		case OKAY:
			temp = 0;
			break;
		case WAIT:
			temp = 1;			break;
		case DONE:			temp = 0;			break;	}
	return state;}
unsigned char Display(unsigned char state){	static unsigned char i = 1;
	if(temp){		LCD_Cursor(i);		LCD_WriteData(in);
		if(i < 16){			i++;		}		else{			i = 1;		}	}
	return state;}
int main(){
	DDRB = 0xF0; PORTB = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned char i = 0;
	tasks[i].state = OKAY;	tasks[i].period = 1;	tasks[i].elapsedTime = tasks[i].period;	tasks[i].TickFct = &Keypad;	i++;	tasks[i].state = 0;	tasks[i].period = 1;	tasks[i].elapsedTime = tasks[i].period;	tasks[i].TickFct = &Display;
	LCD_init();	LCD_DisplayString(1, "Hello World!");
	TimerSet(1);	TimerOn();
	while(1){		for (i = 0; i < numTasks; i++){			if(tasks[i].elapsedTime >= tasks[i].period ){				tasks[i].state = tasks[i].TickFct(tasks[i].state);				tasks[i].elapsedTime = 0;			}			tasks[i].elapsedTime++;		}
		while(!TimerFlag);		TimerFlag = 0;	}

	return 0;
}