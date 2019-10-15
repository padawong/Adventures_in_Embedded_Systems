/*
 *	Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 026
 *	Assignment: CS/EE 120B Final Project
 */

// Servo code with help from  https://www.electronicwings.com/avr-atmega/servo-motor-interfacing-with-atmega16 
//   and https://sites.google.com/site/qeewiki/books/avr-guide/pwm-on-the-atmega328
//   and http://www.avrbeginners.net/architecture/timers/timers.html
//	 and https://maxembedded.files.wordpress.com/2011/07/wave-generation-mode-bit-description-ctc.png?resize=640%2C356

// Custom LCD Chars code with help from https://www.electronicwings.com/avr-atmega/lcd-custom-character-display-using-atmega-16-32-

// PIR sensor code with help from https://exploreembedded.com/wiki/PIR_motion_Sensor_interface_with_Atmega128

// Default CPU Frequency: 1MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include "helper/io.h"
#include "helper/io.c"
#include "helper/bit.h"
#include "helper/keypad.h"
#define PIR_sensor PD0


typedef struct task {
	int state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char note = 0;

void TimerOn() {
	// NOTE: I thought I needed to have TCCR1B as follows for the servo to work, but it was causing super long cycle times
	//TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11); // CTC1 = WGM12 //
	TCCR1B = 0x0B; // Working fine with servo
	OCR1A = 60; // Changed from 125 to accommodate servo
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

#define num_tasks 5
task tasks[num_tasks];

#include <avr/eeprom.h>
#define NUM_EXECUTIONS_ADDRESS 0x00


// Global variables
unsigned char start;
unsigned char code[5] = "#0000";
unsigned char input[5];
unsigned char input_num;
unsigned char invalid; // Display and Alarm check if invalid > 0
unsigned char attempts;
unsigned char alarm;
unsigned char keypad_alarm;
unsigned char valid;
unsigned char locking;
unsigned char unlocking;
unsigned char reset;
unsigned int PIR_Status;
unsigned int motion_delay;
unsigned int motion_trigger;

// SM: Input; Accepts input from keypad and writes to global variables: input_num, default, alarm, lock, unlock, input, reset, invalid
enum Input_States {Input_Start, Input_Wait, Input_Attempt, Input_Valid, Input_Invalid, Input_Alarm } Input_state ;	
int TickFct_Input (int Input_state) {
	unsigned char value;
	unsigned char i = 0;
	
	// Transitions
	switch (Input_state) {
		// Most variables are reset here upon system lock
		case Input_Start: 
			start = 1;
			TCCR1A = (1<<WGM11)|(1<<COM1A1);
			OCR1A = 300; // Engage lock	
			_delay_ms(10400);
			TCCR1A = (1<<WGM11)|(0<<COM1A1);
			OCR1A = 60;
			for (unsigned char i = 0; i < 5; i++) {
				input[i] = '-';
			}
			input_num = 0;
			invalid = 0;
			attempts = 0;
			alarm = 0;
			keypad_alarm = 0;
			valid = 0;
			PIR_Status = 0;
			motion_delay = 0;
			motion_trigger = 0;
			reset = 0; // Input_SM does nothing with this variable
			Input_state = Input_Wait;
			break;
			
		case Input_Wait:
			start = 0;
			if (input[0] == '#') {
				Input_state = Input_Attempt;
			}
			break;
			
		case Input_Attempt:
			if ((input_num == 5) && (input[input_num - 1] == code[input_num - 1])) {
				Input_state = Input_Valid;
			}
			else if (input_num < 6 && (input[input_num - 1] != code[input_num - 1])) {
				invalid = 1;
			}
			if (input_num == 5 && invalid > 0) {
				Input_state = Input_Invalid;
			}
			break;
		
		case Input_Valid: 
			value = GetKeypadKey();
			if(value != '\0') {
				if (value == '#') {
					Input_state = Input_Start;
				}
			}
			break;
			
		case Input_Invalid: 
			if (attempts >= 3) {
				Input_state = Input_Alarm;
			}
			break;
			
		case Input_Alarm:
			// NO WAY OUT >:)
			break;
			
		default: // This should not happen
			Input_state = Input_Start;
			break;
	}
	// Actions
	switch (Input_state) {
		case Input_Start:
			break;
			
		case Input_Wait:	
			value = GetKeypadKey();
			if(value != '\0' && value == '#') {
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
			break;
		
		case Input_Invalid:
			attempts++;
			for (unsigned char i = 0; i < 5; i++) {
				input[i] = '-';
			}
			invalid = 0;
			input_num = 0;
			if (keypad_alarm > 0) {
				Input_state = Input_Alarm;
			}
			else {
				Input_state = Input_Wait;
			}
			break;
			
		case Input_Alarm:
			keypad_alarm = 1;
			break;
			
		default: break;
	}
	
	return Input_state;
};

// SM: Lock; Controls servo motor to turn when locking and unlocking
enum Lock_States {Lock_Locked, Lock_Unlocked} Lock_state ;
int TickFct_Lock (int Lock_state) {
	
	unsigned char i = 0;
	i++;
	
	// Transitions
	switch (Lock_state) {
		case Lock_Locked:
			if (valid == 1) {
				unlocking = 1;
				locking = 0;
				Lock_state = Lock_Unlocked;
			}
			break;
		case Lock_Unlocked:
			if (start == 1) {
				locking = 1;
				unlocking = 0;
				Lock_state = Lock_Locked;
			}			
			break;
		default:
			Lock_state = Lock_Locked;
			break;
	}
	
	// Actions
	switch (Lock_state) {
		case Lock_Locked:
			;locking = 1;
			//OCR1A = 300;	
			break;
		case Lock_Unlocked:
			if (unlocking == 1) {
				TCCR1A = (1<<WGM11)|(1<<COM1A1);
				OCR1A = 60; // unlock
				_delay_ms(10400);
				TCCR1A = (1<<WGM11)|(0<<COM1A1);
				
				unlocking = 0;
			}
			break;
		default:
			break;	
	}
	return Lock_state;
}

// SM: Alarm; Raises the alarm when alarm engaged
enum Alarm_States {Alarm_Wait, Alarm_Motion, Alarm_Alarm} Alarm_state ;
int TickFct_Alarm (int Alarm_state) {
	unsigned char i = 0;
	// Transitions
	switch (Alarm_state) {
		case Alarm_Wait:
			if (keypad_alarm == 1) {
				Alarm_state = Alarm_Alarm;
			}
			if (((PIND) & (1<<PIR_sensor)) == 1) {
				PIR_Status = 1;
				Alarm_state = Alarm_Motion;
			}
			break;
		case Alarm_Motion:
			if (i < 5 && (((PIND) & (1<<PIR_sensor)) == 0)) {
				PIR_Status = 0;
				i = 0;
				motion_delay = 0;
				Alarm_state = Alarm_Wait;
			}
			else if (i >= 5 && PIR_Status > 0) {
				motion_trigger = 1; 
				Alarm_state = Alarm_Alarm;
			}
			else {
				motion_delay = 1;
			}
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
		case Alarm_Motion:
			//motion_delay = 1;
		case Alarm_Alarm: // WEE WOO WEE WOO
			// NOTE: Do the alarm action; LED or speaker in the future
			break;
		default: break;
	}
	return Alarm_state;
}

// SM: Reset; Allows user to input custom 4-char unlock code
/* RAN OUT OF TIME TO TEST AND FULLY IMPLEMENT DUE TO BOARD NOT WORKING THE NIGHT BEFORE SUBMISSION
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
		default:
		
		break;
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
			}
			break;
		default: break;
	}
	
	return Reset_state;
}*/

// SM: Display; 
enum Display_States {Display_Default, Display_Input, Display_Unlock, Display_Lock, Display_Invalid, Display_Alarm, Display_Reset, Display_Motion} Display_state ;
int TickFct_Display (int Display_state) {
	
	static unsigned char i = 0;
	unsigned char heart[8] = { 0x00, 0x0A, 0x15, 0x11, 0x0A, 0x04, 0x00, 0x00 }; // heart
	unsigned char bell[8] = { 0x04, 0x0E, 0x0E, 0x0E, 0x1F, 0x00, 0x04, 0x00 }; // bell
	unsigned char filled_heart[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x0E, 0x04, 0x00, 0x00 };  // filled in heart
	unsigned char lock[8] = { 0x0E, 0x11, 0x11, 0x1F, 0x1B, 0x1B, 0x1F, 0x00 };
	unsigned char unlock[8] = { 0x0E, 0x10, 0x10, 0x1F, 0x1B, 0x1B, 0x1F, 0x00 };
	unsigned char smiley[8] = { 0x00, 0x00, 0x0A, 0x00, 0x04, 0x11, 0x0E, 0x00 }; // smiley face
	unsigned char worried[8] = { 0x0A, 0x11, 0x00, 0x0A, 0x00, 0x0E, 0x11, 0x00 };
	unsigned char angry[8] = { 0x11, 0x0A, 0x00, 0x0A, 0x00, 0x0E, 0x11, 0x11 };
		
	// Transitions
	switch (Display_state) {
		case Display_Default:
			if (input_num > 0) {
				Display_state = Display_Input;
			}
			else if (motion_delay == 1) {
				Display_state = Display_Motion;
			}
			else if (attempts >= 3 || motion_trigger == 1) {
				Display_state = Display_Alarm;
			}
			break;
		case Display_Input:
			if (alarm == 1) {
				Display_state = Display_Alarm;
			}
			else if (input_num == 0) {
				Display_state = Display_Default;
			}
			else if (valid == 1) {
				Display_state = Display_Unlock;
			}
			else if (invalid > 0 && input_num == 5) {
				i = 0;
				Display_state = Display_Invalid;
			}
			break;
		case Display_Unlock:
			if (locking == 1) {
				Display_state = Display_Lock;
			}
			else if (reset > 0) {
				Display_state = Display_Reset;
			}
			break;
		case Display_Lock:
			Display_state = Display_Default;
			break;
		case Display_Invalid: 
			if (attempts >= 3) {
				Display_state = Display_Alarm;
			}
			i++;
			if (i > 5) {
				Display_state = Display_Default;
			}
			break;
		case Display_Alarm:
			// NO WAY OUT WEE WOO WEE WOO
			break;
		case Display_Reset:
			if (reset >= 12) {
				Display_state = Display_Lock;
			}
			break;
		case Display_Motion:
			if (motion_delay == 0) {
				Display_state = Display_Default;
			}
			break;
		default:
			//Display_state = Display_Default;
			break;
	}
	
	// Actions
	switch(Display_state) {
		case Display_Default:
			LCD_ClearScreen();
			LCD_DisplayString(1, "PUSH # THEN CODE"); // 16 chars exactly
			
			if (attempts == 0) {
				LCD_DisplayString(17, "3 ATTEMPTS MAX");
				LCD_Custom_Char(7, lock);
				LCD_Command(0x80);
				LCD_Command(0xc0);
				LCD_Cursor(32);
				LCD_Char(7);
			}
			else if (attempts == 1) {
				LCD_DisplayString(17, "2/3 TRIES LEFT");
				LCD_Custom_Char(7, smiley);
				LCD_Command(0x80);
				LCD_Command(0xc0);
				LCD_Cursor(32);
				LCD_Char(7);
			}
			else if (attempts == 2) {
				LCD_DisplayString(17, "1/3 TRIES LEFT");
				LCD_Custom_Char(7, worried);
				LCD_Command(0x80);
				LCD_Command(0xc0);
				LCD_Cursor(32);
				LCD_Char(7);
			}
			else if (attempts >= 3) {
				LCD_DisplayString(17, "MAX TRIES HIT"); // NOTE: maybe a char here?
				LCD_Custom_Char(7, angry);
				LCD_Command(0x80);
				LCD_Command(0xc0);
				LCD_Cursor(32);
				LCD_Char(7);
			}
			break;
		case Display_Input:
			LCD_ClearScreen();
			LCD_DisplayString(1, "PUSH # THEN CODE"); // 16 chars exactly
			if (input_num == 0) {
				//LCD_Cursor(1);
				//LCD_WriteData('0');
			}
			if (input_num > 0) {
				LCD_Cursor(17);
				LCD_WriteData('#');
			}
			if (input_num == 1) {
				LCD_Cursor(18);
				
				unsigned char x;
				for (x = 1; x < input_num; x++) {
					LCD_WriteData('*');
				}
			}
			else if (input_num == 2) {
				LCD_Cursor(18);
				
				unsigned char x;
				for (x = 1; x < input_num; x++) {
					LCD_WriteData('*');
				}

			}
			if (input_num == 3) {
				LCD_Cursor(18);

				unsigned char x;
				for (x = 1; x < input_num; x++) {
					LCD_WriteData('*');
				}

			}
			if (input_num == 4) {
				LCD_Cursor(18);
				unsigned char x;
				for (x = 1; x < input_num; x++) {
					LCD_WriteData('*');
				}
				
			}
			if (input_num == 5) {
				LCD_Cursor(18);				
				
				unsigned char x;
				for (x = 1; x < input_num; x++) {
					LCD_WriteData('*');
				}
				
			}
			break;
		case Display_Unlock:
			LCD_ClearScreen();
			LCD_DisplayString(1, "HOLD # TO LOCK");
			LCD_DisplayString(19, "WELCOME HOME");
			
			LCD_Custom_Char(0, filled_heart);
			LCD_Command(0x80);
			LCD_Command(0xc0);
			LCD_Cursor(17);
			LCD_Char(0);
			
			LCD_Custom_Char(7, filled_heart);
			LCD_Command(0x80);
			LCD_Command(0xc0);
			LCD_Cursor(32);
			LCD_Char(7);
			break;
		case Display_Lock:
			LCD_ClearScreen();
			LCD_DisplayString(1, "DOOR LOCKED.");
			break;
		case Display_Invalid:
			LCD_ClearScreen();
			LCD_Cursor(1);
			LCD_WriteData(invalid);
			LCD_DisplayString(1, "INVALID CODE.");
			break;
		case Display_Alarm:
			LCD_ClearScreen();
			LCD_DisplayString(1, "INTRUDER ALERT"); // NOTE: special char here
			LCD_DisplayString(17, "COPS INCOMING");
			LCD_Custom_Char(7, angry);
			LCD_Command(0x80);
			LCD_Command(0xc0);
			LCD_Cursor(32);
			LCD_Char(7);
			//}
			break;
		case Display_Reset:
			LCD_ClearScreen();
			if (reset < 7) {
				LCD_DisplayString(1, "# THEN NEW CODE");
				if (reset == 2) {
					LCD_DisplayString(17, "#____");
				}
				if (reset == 3) {
					LCD_DisplayString(17, "#*___");
				}
				if (reset == 4) {
					LCD_DisplayString(17, "#**__");
				}
				if (reset == 5) {
					LCD_DisplayString(17, "#***_");
				}
				if (reset == 6) {
					LCD_DisplayString(17, "#****");
				}
			}
			else {
				LCD_DisplayString(1, "# THEN RE-TYPE");
				if (reset == 7) {
					LCD_DisplayString(17, "#____");
				}
				if (reset == 8) {
					LCD_DisplayString(17, "#*___");
				}
				if (reset == 9) {
					LCD_DisplayString(17, "#**__");
				}
				if (reset == 10) {
					LCD_DisplayString(17, "#***_");
				}
				if (reset == 11) {
					LCD_DisplayString(17, "#****");
				}
			}
			break;
		case Display_Motion:
			LCD_ClearScreen();
			LCD_DisplayString(1, "MOTION DETECTED");
			LCD_DisplayString(17, "CAMERA RECORDING");
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
	
	// Task: Reset
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Reset_Start;
	tasks[i].TickFct = &TickFct_Reset;
	
	// Task: Display
	i++;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].state = Display_Default;
	tasks[i].TickFct = &TickFct_Display;
}

int main(void) {

	DDRA = 0xF0; PORTA = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFC; PORTD = 0x03;
	
	// Initialize servo motor
	ICR1 = 2499; // TOP count for timer1 in set ICR1 register
	// Set Fast PWM, TOP in ICR1, Clear OC1A on compare match, clk/64
	TCCR1A = (1<<WGM11)|(1<<COM1A1);
	//TCCR1B = (1<<WGM12)|(1<<WGM13)|(1<<CS10)|(1<<CS11);
	//TCCR1B = 0x0B;
	

	LCD_init();
	TimerSet(100);
	TimerOn();
	tasks_init();

	unsigned char i;
	
	while (1) {
		for (i = 0; i < num_tasks; i++) {
			//LCD_DisplayString(1, "TEST");
			if (tasks[i].elapsedTime >= tasks[i].period) {
				tasks[i].state = tasks[i].TickFct(tasks[i].state);
				tasks[i].elapsedTime = 0;
			}
			tasks[i].elapsedTime++;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	return 0;
}
