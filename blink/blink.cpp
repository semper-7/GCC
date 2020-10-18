#include <avr/interrupt.h>
#include "../lib/delay.h"

int main (void); 
typedef uint8_t byte;
typedef uint16_t word;

int main (void) {
DDRB = (1<<5);
DelayInit();
sei();

loop:
PORTB^=(1<<5);
delay(2000);
goto loop;
}
