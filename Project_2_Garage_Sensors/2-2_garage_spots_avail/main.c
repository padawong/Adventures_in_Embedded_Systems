#include <avr/io.h>

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
	DDRB = 0xFF; PORTC = 0x00; // Configure port C's 8 pins as outputs
	unsigned char spot0 = 0x00; // Temporary variable to hold the value of PA0
	unsigned char spot1 = 0x00; // Temporary variable to hold the value of PA1
	unsigned char spot2 = 0x00; // Temporary variable to hold the value of PA2
	unsigned char spot3 = 0x00; // Temporary variable to hold the value of PA3
	
	unsigned char spaces = 0x00;
	
	while(1)
	{
		// 1) Read input
		spot0 = PINA & 0x01;
		spot1 = PINA & 0x02;
		spot2 = PINA & 0x04;
		spot3 = PINA & 0x08;
		
		int spots = spot0;
		spots += (spot1 / 2);
		spots += (spot2 / 4);
		spots += (spot3 / 8);
		
		// 2) Perform computation
		// All spots empty
		if (spots == 0x00) {
			spaces = 0x04;
		}
		else if (spots == 0x01) {
			spaces = 0x03;
		}
		else if (spots == 0x02) {
			spaces = 0x02;
		}
		else if (spots == 0x03) {
			spaces = 0x01;
		}
		else {
			spaces = 0x00;
		}
		// 3) Write output
		PORTC = spaces;
	}
	return 0;
}
