/*
 *	Partner 1 Name & E-mail: David May dmay004@ucr.edu
 *  Partner 2 Name & E-mail: Erin Wong ewong012@ucr.edu 
 *	Lab Section: 026
 *	Assignment: Lab 11 Exercise 4
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
typedef struct task{
task tasks[2];
const unsigned short numTasks = 2;
void TimerOn(){
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff(){
	TCCR1B = 0x00;
void TimerISR(){
ISR(TIMER1_COMPA_vect){
void TimerSet(unsigned long M){
unsigned char GetKeypadKey(){
	PORTB = 0xDF;
	PORTB = 0xBF;
	PORTB = 0x7F;
	return('\0');
enum Keypad_States {OKAY, WAIT, DONE};
unsigned char Keypad(unsigned char state){
	switch(state){
			break;
		case WAIT:
				state = DONE;
			}
			else{
				state = OKAY;
			}
			break;
		case DONE:
			}
			else{
				state = OKAY;
			}
			break;
		default:
	}
	switch(state){
		case OKAY:
			temp = 0;
			break;
		case WAIT:
			temp = 1;
		case DONE:
	return state;
unsigned char Display(unsigned char state){
	if(temp){
		if(i < 16){
	return state;
int main(){
	DDRB = 0xF0; PORTB = 0x0F;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	unsigned char i = 0;
	tasks[i].state = OKAY;
	LCD_init();
	TimerSet(1);
	while(1){
		while(!TimerFlag);

	return 0;
}