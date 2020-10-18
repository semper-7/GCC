/*Arduino AT89C2051 programmer
Semper@2017-2019

	12V VPP Sheet
D12___      RST/VPP
PB4   )|       |
      )|       |
100uH )|       | bzx55c10
      )| 1n4148|  _    LED yellow
     _|---|>|--+--|<|--|>|--
D3__|_ 2n7000  |            |
PD3   |       === 1,0       |
GND___|________|____________|

AT89C2051:
PIN	PINNAME	Arduino AVRPIN
1	RST/VPP
2	NC
3	NC
4	NC
5	CLOCK	D2	PD2
6	PROG    A3	PC3
7	MODE0   A0	PC0
8	MODE1   A1	PC1
9	MODE2   A2	PC2
10	GND
11	MODE2   A2	PC2
12	D0	D4	PD4
13	D1	D5	PD5
14	D2	D6	PD6
15	D3	D7	PD7
16	D4	D8	PB0
17	D5	D9	PB1
18	D6	D10	PB2
19	D7	D11	PB3
20	VDD	D13	PB5
*/
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../lib/delay.h"
#include "../lib/usart.h"

unsigned int addr = 0;
//const char str[] PROGMEM = "\r\n\";

void Power() {
addr = 0;
 for(;;) {
 PORTB = PORTC = TCCR2A = 0;
 PORTD &= 3;
 UsartPrintPGM(PSTR("Send s to start\r\n"));
  if (UsartRead(0)=='s') {
  PORTB = 0x3f;
  PORTC = 8;
  PORTD |= 0xf0;
  delay(500);
  uint8_t c=(PINB<<4)|(PIND>>4);
   if (c==0x1c) {
   UsartPrintPGM(PSTR("89C2051 detected. Supported commands: e,r,d,p,c,R,q\r\n"));
   return;
   } else {
   UsartPrintPGM(PSTR("89C2051 not found!!!\r\n"));
   }
  }
 }
}
  
void Reset() {
PORTB = 0;
PORTC = 0;
PORTD &= 3;
addr = 0;
delay(100);
PORTB = 0x3f;
PORTC = 8;
PORTD |= 0xf0;
} 

void HexPrintByte(uint8_t w) {
char hextab[] = "0123456789ABCDEF";
UsartWrite(hextab[w>>4]);
UsartWrite(hextab[w&0x0f]);
} 
  
void HexPrint(unsigned int w) {
HexPrintByte((uint8_t)(w>>8));
HexPrintByte((uint8_t)(w&0xff));
} 
  
void HexPrintLineByte() {
HexPrint(addr);
UsartWrite(':');
 for(uint8_t j=0;j<16;j++) {
 UsartWrite(' ');
 HexPrintByte((PINB<<4)|(PIND>>4));
 PORTD |= 4;
 PORTD &= 0xfb;
 }
UsartPrint("\r\n");
addr+=16;
} 

int main (void) {
DDRB |= 0x30;
DDRC |= 0x0f;
DDRD |= 0x0c;
DelayInit();
UsartInit(115200);
TIMSK2 = TCCR2B = TCCR2A = 0;
OCR2A = OCR2B = 3; 
TCCR2B = 1;
sei();
Power();

loop:
uint8_t  val;    
uint8_t  val_;    
char command = UsartRead(0);

 switch(command) {
 case 'e' :
 PORTC = 9;
 TCCR2A = 0x12;
 delay(100);
 PORTC = 1;
 delay(10);
 PORTC = 9;
 TCCR2A = 0;
 PORTD &= 0xf7;
 delay(10);
 PORTC = 8;
 delay(100);
 UsartPrintPGM(PSTR("Program Memory is erased!!!\r\n"));
 break;
 
 case 'r' :
 PORTC = 0x0c;
 HexPrintLineByte();
 break;
 
 case 'd' :
 PORTC = 0x0c;
 for(uint8_t k=0;k<64;k++) HexPrintLineByte();
 break;
 
 case 'c' :
 PORTC = 8;
 UsartPrintPGM(PSTR("Signature:\r\n"));
 HexPrintLineByte();
 break;
 
 case 'p' :
 UsartPrintPGM(PSTR("Start programming ..."));
 PORTC = 0x0e;
 TCCR2A = 0x12;
 delay(10);
 DDRB |= 0x3f;
 DDRD |= 0xfc;
  for (;;) {
  UsartPrint("\r\n");
  HexPrint(addr);
  UsartWrite(':');
   for(uint8_t j=0;j<16;j++) {
   UsartWrite(' ');
    do {
    val = UsartRead(0);
     if (val == '$') {
     UsartPrintPGM(PSTR("\r\nDone.\r\n"));
     goto done;
     }
    } while(val < '0');
   UsartWrite(val);
   if (val>0x40) val-=7;
   val &= 0x0f;
   val_ = UsartRead(0);
   UsartWrite(val_);
   if (val_>0x40) val_-=7;
   val_ = val_<<4;
   PORTB = 0x30 | val;
   PORTD = (PORTD & 0x0f) | val_;
   PORTC = 6;
   _delay_loop_2(40);
   PORTC = 0x0e;
   _delay_loop_2(40);
   PORTD |= 4;
   PORTD &= 0xfb;
   addr++;
   }
  }
 done:
 TCCR2A = 0;
 DDRB &= 0xf0;
 DDRD &= 0x0f;
 PORTD &= 0xf7;
 PORTC = 8;
 PORTB = 0x3f;
 PORTD |= 0xf0;
 break;
 
 case 'R' :
 UsartPrintPGM(PSTR("Reset\r\n"));
 Reset();
 break;
 
 case 'q' :
 UsartPrintPGM(PSTR("Power OFF\r\n")); 
 UsartWrite('^');
 Power();
 break;
 }

goto loop;
}

