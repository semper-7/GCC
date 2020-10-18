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

byte p = 0;

void tx220(byte t) {
 for (byte j=0;j<(3-t)*3+27;j++) {
 cli();
  for (byte i=0;i<((t<<1)+7)<<1;i++) {
  DDRD^=8;
  _delay_loop_2(4*20);
  }
 sei();
 _delay_loop_2(4*370);
 }
}

int main (void) {
DelayInit();
UsartInit(115200);
TIMSK2 = TCCR2B = 0;
TCCR2A = 0x12;
OCR2A = OCR2B = 3;
TCCR2B = 1;
sei();
delay(500);
UsartPrintPGM(PSTR("\r\nP220 started\r\n"));

loop:
p++;
if (p > 3) p = 0;
tx220(p);
delay(2000);
goto loop;
}
