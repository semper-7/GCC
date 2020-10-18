#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "../lib/delay.h"
#include "../lib/usart.h"


int main (void); 
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

byte fl = 0;
byte p = 0x34;
byte t = 0;

ISR(INT0_vect) {
 if (fl) {
  for (byte i=0;i<((((t&1)^1)<<1)+7)<<1;i++) {
  DDRD^=8;
  _delay_loop_2(4*20);
  }
 t>>=1;
 fl--;
 }
}

int main (void) {
DelayInit();
UsartInit(115200);
EICRA = 3;
EIMSK = 1;
TIMSK1 = TIMSK2 = TCCR1A = TCCR2B = 0;
TCCR2A = 0x12;
OCR2A = OCR2B = 3;
TCCR2B = 1;
sei();
delay(500);
UsartPrintPGM(PSTR("\r\nP220 started\r\n"));

loop:
p++;
if (p > 0x37) p = 0x34;
t = p;
if (fl) UsartPrintPGM(PSTR("Error nul detector\r\n"));
fl = 8;
delay(2000);
goto loop;
}
