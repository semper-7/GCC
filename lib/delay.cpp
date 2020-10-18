#include <avr/interrupt.h>
#include "delay.h"

volatile word ms = 0;
volatile dword msd =0;
volatile bool fld = 0;
void (*delayfunc)();

void delay(dword m) {
msd = m;
fld = 1;
while(fld) if(delayfunc!=0) (*delayfunc)();
}

void DelayInit(void (*f)()) {
delayfunc = f;
OCR0A = 249;
TCCR0A = 1<<WGM01;
TCCR0B = (1<<CS00) | (1<<CS01);
TIMSK0 = 1<<OCIE0A;
}

ISR(TIMER0_COMPA_vect) {
ms++;
 if(fld) {
 msd--;
 if(msd==0) fld=0;
 }
}
