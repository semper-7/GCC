/*Arduino 12V PIC programmer
Semper@2017-2018

	12V VPP Sheet
PB4___        VPP(12V)
D12   )|       |
      )|       |
100uH )|       | Zener 10V
      )| 1n4148|  _    LED yellow
     _|---|>|--+--|<|--|>|--
PB3_|_ 2n7000  |            |
D11   |       === 1,0       |
GND___|________|____________|

PIC12F635:
PIN	PINNAME	Arduino 
1	VDD	PB5(D13)
2	NC
3	NC
4	VPP	VPP(12V)
5	NC
6	CLOCK	PC1(A1)
7	DATA	PC0(A0)
8       GND	GND
*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "../lib/delay.h"
#include "../lib/usart.h"

int main (void); 
typedef uint8_t byte;
typedef uint16_t word;
#define DC (F_CPU/4000000UL)

word addr;
byte hl[48];
byte strVal[4];

void Reset() {
PORTB = 0x10;
delay(200);
PORTB = 0x30;
addr = 0;
} 

void Power() {
DDRB = DDRC = PORTB = PORTC = 0;
 for(;;) {
 UsartPrintPGM(PSTR("Send s to start\r\n"));
  if (UsartRead(0)=='s') {
  DDRB = 0x38;
  Reset();
  DDRC = 3;
  UsartPrintPGM(PSTR("Power ON\r\n"));
  return;
  }
 }
}

void WriteBit(byte v) {
PORTC = v|2;
_delay_loop_2(3*DC);
PORTC &= 1 ;
_delay_loop_2(3*DC);
PORTC = 0;
}

byte ReadBit() {
PORTC = 2;
_delay_loop_2(3*DC);
byte v = PINC & 1;
PORTC =0;
_delay_loop_2(3*DC);
return v;
}

word ReadDataFromProgramMemory() {
byte w[2];
*((word*)(&w[0]))=0;
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
DDRC = 2;
_delay_loop_2(3*DC);
ReadBit();
 for(byte i=0;i<14;i++) {
 w[1] |= ReadBit() << 6;
 *((word*)(&w[0])) >>=1;
 }
ReadBit();
DDRC =3;
return *((word*)(&w[0]));
}

void LoadDataCommandForProgramMemory(word w) {
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
_delay_loop_2(3*DC);
WriteBit(0);
for(byte i=0;i<14;i++,w>>=1) WriteBit(w&1);
WriteBit(0);
_delay_loop_2(3*DC);
}

void BeginProgrammingCycle() {
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
delay(10);
}

void Erase() {
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(1);
WriteBit(0);
WriteBit(0);
delay(100);
UsartPrintPGM(PSTR("Program Memory is erased!!!\r\n"));
}

void IncrementAddress() {
WriteBit(0);
WriteBit(1);
WriteBit(1);
WriteBit(0);
WriteBit(0);
WriteBit(0);
_delay_loop_2(3*DC);
addr++;
}

void HexPrint(byte v) {
char hextab[] = "0123456789ABCDEF";
UsartWrite(hextab[v>>4]);
UsartWrite(hextab[v&0x0f]);
} 
  
void HexPrintWord(word w) {
HexPrint((uint8_t)(w>>8));
HexPrint((uint8_t)(w&0xff));
} 

void SelectConfiguration() {
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
WriteBit(0);
_delay_loop_2(3*DC);
for(byte i=0;i<16;i++) WriteBit(0);
_delay_loop_2(3*DC);
addr = 0x2000;
}

byte ReadHexLine(byte n) {
byte x = 0;
UsartWrite(':');
HexPrint(n);
byte c = 0;
c-=n;
c-=addr<<1;
c-=addr>>7;
HexPrintWord(addr<<1);
UsartPrint("00");
 for(byte i=0;i<(n>>1);i++) {
 word r = ReadDataFromProgramMemory();
 if(r==0x3fff) x++;
 c-=r;
 c-=r>>8;
 HexPrint(r);
 HexPrint(r>>8);
 IncrementAddress();
 }
HexPrint(c);
UsartPrint("\r\n");
return x;
}

void ReadHex() {
byte n = 0;
 for (byte i=0;i<128;i++) {
 n+=ReadHexLine(0x10);
 if(n>64) break;
 }
SelectConfiguration();
for(byte i=0;i<7;i++) IncrementAddress();
ReadHexLine(2);
UsartPrint(":00000001FF\r\n");
}

word SetWord (byte *z) {
byte v;
word w = 0;
 for(byte i=0;i<4;i++) {
 v = *z;
 UsartWrite(v);
 if (v>0x40) v-=7;
 w=(w<<4) | (word)(v&=0x0f);
 z++;
 }
return w;
}

byte SetByte (byte *z) {
byte v = *z;
if (v>0x40) v-=7;
byte b = v<<4;
z++;
v = *z;
if (v>0x40) v-=7;
return (b | (v&=0x0f));
}

void ReadLine () {
byte v;
 for(byte i=0;i<48;i++) {
 v = UsartRead(0);
 hl[i] = v;
 if (v=='\n') return;
 }
}

void WriteHex () {
word a=0;
word w;
byte c;
UsartPrintPGM(PSTR("Start programming..."));
 for (;;) {
 UsartPrint("\r\n");
 ReadLine();
 if(hl[8]=='1') break;
  if(hl[8]=='0') {
  if(hl[3]=='4') SelectConfiguration();
  a = SetWord(&hl[3])>>1;
  while (addr!=a) IncrementAddress();
  UsartPrint(": ");
  c = SetByte(&hl[1])>>1;
   for(byte i=0;i<c;i++) {
   w = SetWord(&hl[i*4+9]);
   w = (w>>8)|(w<<8);
   LoadDataCommandForProgramMemory(w);
   BeginProgrammingCycle();
    if (w != ReadDataFromProgramMemory()) {
    UsartPrintPGM(PSTR("\r\nERROR PROGRAMMING!!!\r\n"));
    Power();
    return;
    }
   IncrementAddress();
   }
  }
 }
UsartPrintPGM(PSTR("Done.\r\n"));
}

int main (void) {
DelayInit();
UsartInit(115200);
TIMSK2 = TCCR2B = TCCR2A = 0;
OCR2A = 3; 
TCCR2A = 0x42;
TCCR2B = 1;
sei();
Power();

loop:
 switch(UsartRead(0)) {

 case 'e' :
 Erase();
 break;

 case 'r' :
 ReadHex();
 break;

 case 'w' :
 WriteHex();
 break;

 case 'i' :
 ReadHexLine(2);
 break;

 case 'c' :
 SelectConfiguration();
 UsartPrint("Configuration memory selected\r\n");
 break;

 case 'R' :
 UsartPrintPGM(PSTR("Reset\r\n"));
 Reset();
 break;

 case 'q' :
 UsartPrintPGM(PSTR("Power OFF\r\n"));
 Power();
 break;
 }
goto loop;
}
