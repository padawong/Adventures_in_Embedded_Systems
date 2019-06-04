/*
 *	Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 026
 *	Assignment: CS/EE 120B Final Project
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "helper/io.h"
#include "helper/io.c"
#include "helper/bit.h"
#include "helper/keypad.h"

typedef struct task {
	unsigned char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	void (*TickFct)();
} task;

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

void Keypad() {
	unsigned char value = GetKeypadKey();
	if(value != '\0') {
		//LCD_Cursor(1);
		//LCD_WriteData(value);
	}
}

#define num_tasks 5
task tasks[num_tasks];

// Global variables
unsigned char start;
unsigned char code[5] = "#0000";
unsigned char input[5];
unsigned char input_num;
unsigned char invalid; // Display and Alarm check if invalid > 0
unsigned char alarm;
unsigned char valid;
unsigned char locking;
unsigned char unlocked;
unsigned char reset;
unsigned char TEST[5] = "-----";

// SM: Input; Accepts input from keypad and writes to global variables: input_num, default, alarm, lock, unlock, input, reset, invalid
enum Input_States {Input_Start, Input_Wait, Input_Attempt, Input_Valid, Input_Invalid, Input_Alarm } Input_state ;	
int TickFct_Input (int Input_state) {
	unsigned char value;
	
	// Transitions
	switch (Input_state) {
		case Input_Start: 
			start = 1;
			for (unsigned char i = 0; i < 5; i++) {
				input[i] = '-';
			}
			input_num = 0;
			invalid = 0;
			alarm = 0;
			valid = 0;
			locking = 0;
			unlocked = 0;
			reset = 0; // Input_SM does nothing with this variable
			Input_state = Input_Wait;
			break;
			
		case Input_Wait:
			if (input[0] == '#') {
				Input_state = Input_Attempt;
			}
			break;
			
		case Input_Attempt:
			if (input_num < 5 && (input[input_num] == code[input_num])) {
				input_num++;
			}
			else if (input_num == 5 && (input[input_num] == code[input_num])) {
				Input_state = Input_Valid;
				valid = 1;
				LCD_ClearScreen();
				LCD_DisplayString(1, "Input valid!");
			}
			else if (input_num < 6 && (input[input_num] != code[input_num])) {
				Input_state = Input_Invalid;
			}
			TEST[input_num] = input[input_num];
			break;
		
		case Input_Valid: 
			if (locking == 1) {
				//Input_state = Input_Start;
			}
			break;
			
		case Input_Invalid: 
			if (invalid >= 3) {
				Input_state = Input_Alarm;
			}
			break;
			
		case Input_Alarm:
			// NO WAY OUT >:)
			break;
			
		default: // This should not happen
			//Input_state = Input_Start;
			break;
	}
	// Actions
	switch (Input_state) {
		case Input_Start:
			break;
			
		case Input_Wait:	
			value = GetKeypadKey();
			if(value != '\0') {
				input[input_num] = value;
				input_num++;
			}
			break;
						
		case Input_Attempt:
			value = GetKeypadKey();
			if(value != '\0') {
				input[input_num] = value;
				input_num++;
			}
			break;
		
		case Input_Valid:
			valid = 1;
			// NOTE: NEED TO DO THIS - if 0 (and # ?) have been held down for 3+ seconds, set reset = 1
			break;
		
		case Input_Invalid:
			invalid++;
			for (unsigned char i = 0; i < 5; i++) {
				input[i] = '-';
			}
			break;
			
		case Input_Alarm:
			alarm = 1;
			break;
			
		default: break;
	}
	
	return Input_state;
};

// SM: Lock; Controls servo motor to turn when locking and unlocking
enum Lock_States {Lock_Locked, Lock_Unlocked} Lock_state ;
int TickFct_Lock (int Lock_state) {
	// Transitions
	switch (Lock_state) {
		case Lock_Locked:
			if (unlocked == 1) {
				Lock_state = Lock_Unlocked;
			}
			break;
		case Lock_Unlocked:
			if (locking == 1) {
				Lock_state = Lock_Locked;
				unlocked = 0;
			}			
			break;
		default:
			Lock_state = Lock_Locked;
			break;
	}
	
	// Actions
	switch (Lock_state) {
		case Lock_Locked:
			// SERVO MOTOR CCW 180 DEG
			locking = 0;
			break;
		case Lock_Unlocked:
			// SERVO MOTOR CW 180 DEG
			break;
		default:
			break;	
	}
	return Lock_state;
}

// SM: Alarm; Sounds the piezo when alarm engaged
enum Alarm_States {Alarm_Wait, Alarm_Alarm} Alarm_state ;
int TickFct_Alarm (int Alarm_state) {
	// Transitions
	switch (Alarm_state) {
		case Alarm_Wait:
			if (alarm == 1) {
				Alarm_state = Alarm_Alarm;
			}
			break;
		case Alarm_Alarm:
			// NO WAY OUT WEE WOO WEE WOO
			break;
		default:
			Alarm_state = Alarm_Wait;
			break;
	}
	
	// Actions
	switch(Alarm_state) {
		case Alarm_Wait:
			break; // Do nothing
		case Alarm_Alarm: // WEE WOO WEE WOO
			// SOUND THE PIEZO
			break;
		default: break;
	}
	return Alarm_state;
}
/*
// SM: Reset; Allows user to input custom 4-char unlock code
enum Reset_States {Reset_Start, Reset_Wait, Reset_Reset, Reset_Confirm} Reset_state;
int TickFct_Reset (int Reset_state) {
	unsigned char value;
	
	// Transitions
	switch (Reset_state) {
		case Reset_Start:
			Reset_state = Reset_Wait;
			break;
		case Reset_Wait:
			if (reset == 1) {
				input_num = 0;
				for (unsigned char i = 0; i < 5; i++) {
					input[i] = '-';
				}
				Reset_state = Reset_Reset;
			}
			break;
		case Reset_Reset:
			if (input[0] != '#') {
				reset = 1;
				input_num = 0;
				for (unsigned char i = 0; i < 5; i++) {
					input[i] = '-';
				}
			}
			else if (input_num >= 5) {
				Reset_state = Reset_Confirm;
			}
			break;
		case Reset_Confirm:
			if (input[0] != '#') {
				reset = 1;
				input_num = 0;
				for (unsigned char i = 0; i < 5; i++) {
					input[i] = '-';
				}
			}
			else if (input_num >= 5) {
				reset = 0;
				Reset_state = Reset_Wait;
			}
			break;
		default: break;
	}
	
	// Actions
	switch(Reset_state) {
		case Reset_Wait: break;
		case Reset_Reset:
			if (input_num < 5) {
				unsigned char value = GetKeypadKey();
				if(value != '\0') {
					input[input_num] = value;
					input_num++;
					reset++; // reset starts at 1. With each subsequent key added, Display will behave accordingly
				}
			}
			break;
		case Reset_Confirm: 
			value = GetKeypadKey();
			if(value != '\0') {
				if (value == input[input_num]) {	
					input_num++;
					reset++;
				}	
				// Confirmation code does not match initial code; return to start of reset code input
				else {
					input_num = 0;
					for (unsigned char i = 0; i < 5; i++) {
						input[i] = '-';
					}
					reset = 1;
					Reset_state = Reset_Reset;
				}
				// NOTE: I'm not sure that code is being actually set anywhere
			}
			break;
		default: break;
	}
	
	return Reset_state;
}*/

// SM: Display; 
enum Display_States {Display_Start, Display_Input, Display_Unlock, Display_Lock, Display_Invalid, Display_Alarm, Display_Reset} Display_state ;
int TickFct_Display (int Display_state) {
	
	// Transitions
	switch (Display_state) {
		case Display_Start: {
			if (input_num > 0) {
				Display_state = Display_Input;
			}
			break;
		}

		case Display_Input: {
			// WHY WAS THIS NECESSARY TO KEEP MY CODE FROM BEING STUPID??
			if (input_num == 0) {
				Display_state = Display_Start;
			}
			if (valid == 1) { // unlocked == 1
				Display_state = Display_Unlock;
			}
			else if (invalid >= 3) {
				Display_state = Display_Invalid;
			}
			else if (alarm == 1) {
				Display_state = Display_Alarm;
			}
			break;
		}
		case Display_Unlock:
			if (locking == 1) {
				Display_state = Display_Lock;
			}
			else if (reset > 0) {
				Display_state = Display_Reset;
			}
			break;
		case Display_Lock:
			// State will change after lock action in action sequence
			break;
			
		case Display_Invalid: 
			if (invalid >= 3) {
				Display_state = Display_Alarm;
			}
			Display_state = Display_Start;
			break;
		case Display_Alarm:
			// NO WAY OUT WEE WOO WEE WOO
			break;
		case Display_Reset:
			if (reset >= 12) {
				Display_state = Display_Lock;
			}
			break;
		default:
			//Display_state = Display_Start;
			break;
	}
	
	// Actions
	switch(Display_state) {
		case Display_Start: 
			// Starting at position 1 on the LCD screen, writes string
			LCD_DisplayString(1, "PUSH # THEN CODE"); // 16 chars exactly
			
			//LCD_DisplayString(9, "3 ATTEMPTS MAX _"); // NOTE: CUSTOM CHAR HERE
			break;

  		case Display_Input: 
			//LCD_ClearScreen();
			
			//LCD_DisplayString(1, "PUSH # THEN CODE"); // 16 chars exactly
			if (input_num == 0) {
				Display_state = Display_Start;
				//LCD_ClearScreen();
				//LCD_DisplayString(1, "HOW??");
			}
			else if (input_num == 1) {
				LCD_Cursor(1);
				LCD_WriteData('1');
				//LCD_DisplayString(1, "#");
			}
			else if (input_num == 2) {
				LCD_Cursor(1);
				LCD_WriteData('2');
				LCD_Cursor(2);
				LCD_WriteData(input[1]);

				//LCD_DisplayString(1, "##");
			}
			else if (input_num == 3) {
				LCD_Cursor(1);
				LCD_WriteData('3');
				LCD_Cursor(2);
				LCD_WriteData(input[1]);
				LCD_Cursor(3);
				LCD_WriteData(input[2]);

				//LCD_DisplayString(1, "###");
			}
			else if (input_num == 4) {
				LCD_Cursor(1);
				LCD_WriteData('4');
				LCD_Cursor(2);
				LCD_WriteData(input[1]);
				LCD_Cursor(3);
				LCD_WriteData(input[2]);
				LCD_Cursor(4);
				LCD_WriteData(input[3]);
				
				//LCD_DisplayString(1, "####");
			}
			else if (input_num == 5) {
				LCD_Cursor(1);
				LCD_WriteData('5');
				LCD_Cursor(2);
				LCD_WriteData(input[1]);
				LCD_Cursor(3);
				LCD_WriteData(input[2]);
				LCD_Cursor(4);
				LCD_WriteData(input[3]);
				LCD_Cursor(5);
				LCD_WriteData(input[4]);
				
				//LCD_DisplayString(1, "#####");
			}
			break;
		case Display_Unlock:
			LCD_ClearScreen();
			LCD_DisplayString(1, "WELCOME HOME _"); // NOTE Heart custom char
			//LCD_DisplayString(9, "#=LOCK; SET=0 3s");
			break;
		case Display_Lock:
			LCD_ClearScreen();
			LCD_DisplayString(1, "LOCKING DOOR");
			// NOTE: maybe a special char here?
			break;
		case Display_Invalid:
			// NOTE: make this message persist for 3 seconds
			LCD_ClearScreen();
			LCD_Cursor(1);
			LCD_WriteData(invalid);
			LCD_DisplayString(1, "INVALID CODE.");
			if (invalid == 1) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "2/3 TRIES LEFT");
			}
			else if (invalid == 2) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "1/3 TRIES LEFT");
			}
			else if (invalid >= 3) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "MAX TRIES HIT"); // NOTE: maybe a char here?
			}
			break;
			
		case Display_Alarm:
			// NOTE: might want to change alarm to be checked for > 0 elsewhere
			// if (alarm == 1) {
			//	clear
			//	display
			//  alarm++;
			//}
			LCD_ClearScreen();
			LCD_DisplayString(1, "INTRUDER ALERT _"); // NOTE: special char here
			LCD_DisplayString(9, "DOGS RELEASED __"); // NOTE: special char here? Skull?
			break;
		case Display_Reset:
			LCD_ClearScreen();
			if (reset < 7) {
				LCD_DisplayString(1, "# THEN NEW CODE");
				if (reset == 2) {
					LCD_DisplayString(9, "#");
				}
				if (reset == 3) {
					LCD_DisplayString(9, "##");
				}
				if (reset == 4) {
					LCD_DisplayString(9, "###");
				}
				if (reset == 5) {
					LCD_DisplayString(9, "####");
				}
				if (reset == 6) {
					LCD_DisplayString(9, "#####");
				}
			}
			else {
				LCD_DisplayString(1, "# THEN RE-TYPE");
				if (reset == 7) {
					LCD_DisplayString(9, "#");
				}
				if (reset == 8) {
					LCD_DisplayString(9, "##");
				}
				if (reset == 9) {
					LCD_DisplayString(9, "###");
				}
				if (reset == 10) {
					LCD_DisplayString(9, "####");
				}
				if (reset == 11) {
					LCD_DisplayString(9, "#####");
				}
			}
			break;
		default: break;
	}
	
	return Display_state;
}

void tasks_init() {
	// Task: Input
	unsigned char i = 0;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Input_Start;
	tasks[i].TickFct = &TickFct_Input;
	
	// Task: Lock
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Lock_Locked;
	tasks[i].TickFct = &TickFct_Lock;
	
	// Task: Alarm
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Alarm_Wait;
	tasks[i].TickFct = &TickFct_Alarm;
	
/*	// Task: Reset
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Reset_Start;
	tasks[i].TickFct = &TickFct_Reset;
	*/
	
	// Task: Display
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Display_Start;
	tasks[i].TickFct = &TickFct_Display;
}

int main(void) {

	DDRA = 0xF0; PORTA = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFC; PORTD = 0x03;

	LCD_init();
	TimerSet(500);
	TimerOn();
	tasks_init();

	unsigned char i;
	//LCD_DisplayString(1, "Hello, World!");
	//LCD_Cursor(17);
	//LCD_DisplayString(16, "1234567890123456");

	while (1) {
		for (i = 0; i < num_tasks; i++) {
			//LCD_DisplayString(1, "TEST");
			if (tasks[i].elapsedTime >= tasks[i].period) {
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
