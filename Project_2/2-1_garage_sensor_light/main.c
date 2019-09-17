#include <avr/io.h>

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
    DDRB = 0xFF; PORTB = 0x00; // Configure port B's 8 pins as outputs
    unsigned char garage = 0x00; // Temporary variable to hold the value of A
	unsigned char light_sensor = 0x00; // Temporary variable to hold the value of B
	unsigned char light = 0x00; // Temporary variable to hold the value of A
    while(1)
    {
	    // 1) Read input
	    garage = PINA & 0x01;
		light_sensor = PINA & 0x02;
		
	    // 2) Perform computation
	    // Garage is open, light not sensed
	    if (garage == 0x01 && light_sensor == 0x00) { 
	        // Sets light to bbbbbb01 (clear rightmost 2 bits, then set to 01)
			light = (light & 0xFC) | 0x01; 
        }
		// Do not turn on light; either light sensed or garage closed
		else { 
			light = 0x00;
		}
	    // 3) Write output
	    PORTB = light;
    }
    return 0;
}

