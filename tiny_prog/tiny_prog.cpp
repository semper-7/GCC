/*Arduino 12V Attiny13 reprogrammer
Semper@2017-2018

	12V VPP Sheet
PB4___        VPP(12V)
D12   )|       |
      )|       |
220uH )|       | Zener 10V
      )| 1n4148|  _    LED yellow
     _|---|>|--+--|<|--|>|--
PB3_|_ 2n7000  |            |
D11   |       === 1,0       |
GND___|________|____________|

ATTINY13:
PIN	PINNAME	Arduino 
1	RST/12V	VPP(12V)
2	SCI	PC3(A3)
3	NC
4	GND
5	SDI	PC0(A0)
6	SII	PC1(A1)
7	SDO	PC2(A2)
8       VCC	PB5(D13)
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

#define HFUSE 0x747C
#define LFUSE 0x646C
#define EFUSE 0x666E
// Define ATTiny series signatures
#define ATTINY13 0x9007	// L: 0x6A, H: 0xFF             8 pin

word addr = 0;
byte hl[48];

void HexPrint(byte v) {
char hextab[] = "0123456789ABCDEF";
UsartWrite(hextab[v>>4]);
UsartWrite(hextab[v&0x0f]);
} 

void HexPrintWord(word w) {
HexPrint((uint8_t)(w>>8));
HexPrint((uint8_t)(w&0xff));
} 


byte shiftOut (byte v1, byte v2) {
word r = 0;
word w1 = (word) v1;
word w2 = (word) v2;
while ((PINC&4)==0) {};
 for (int i=0;i<11;i++) {
 byte xc=0;
 w1 <<= 1;
 w2 <<= 1;
 if (w1 & 0x0200) xc|=1;
 if (w2 & 0x0200) xc|=2;
 PORTC = xc;
 r <<= 1;
 r |= PINC & 4;
 PORTC = xc | 8;
 _delay_loop_2(3*DC);
 PORTC = xc;
 }
return r >> 4;
}

void writeFuse (word fuse, byte val) {
shiftOut(0x40, 0x4C);
shiftOut( val, 0x2C);
shiftOut(0x00, (byte) (fuse >> 8));
shiftOut(0x00, (byte) fuse);
}

void PrintFuses () {
byte val;
      shiftOut(0x04, 0x4C);  // LFuse
      shiftOut(0x00, 0x68);
val = shiftOut(0x00, 0x6C);
UsartPrint("LFuse: ");
HexPrint(val);
      shiftOut(0x04, 0x4C);  // HFuse
      shiftOut(0x00, 0x7A);
val = shiftOut(0x00, 0x7E);
UsartPrint("; HFuse: ");
HexPrint(val);
      shiftOut(0x04, 0x4C);  // EFuse
      shiftOut(0x00, 0x6A);
val = shiftOut(0x00, 0x6E);
UsartPrint("; EFuse: ");
HexPrint(val);
UsartPrint("\r\n");
}

void PrintSignature () {
byte sl;
byte sh;
shiftOut(0x08, 0x4C);
shiftOut(0x01, 0x0C);
shiftOut(0x00, 0x68);
sh = shiftOut(0x00, 0x6C);
shiftOut(0x08, 0x4C);
shiftOut(0x02, 0x0C);
shiftOut(0x00, 0x68);
sl = shiftOut(0x00, 0x6C);
UsartPrintPGM(PSTR("Signature is: "));
HexPrint(sh);
HexPrint(sl);
UsartPrint("\r\n");
}

void PrintCalibration () {
shiftOut(0x08, 0x4C);
shiftOut(0x01, 0x0C);
shiftOut(0x00, 0x78);
byte cal = shiftOut(0x00, 0x7C);
UsartPrintPGM(PSTR("Calibration is: "));
HexPrint(cal);
UsartPrint("\r\n");
}

void Read () {
byte vl;
byte vh;
HexPrintWord(addr<<1);
UsartWrite(':');
shiftOut(0x02, 0x4C);
 for(byte i=0;i<8;i++) {
 shiftOut(addr, 0x0C);
 shiftOut(addr>>8, 0x1c);
 shiftOut(0x00, 0x68);
 vl = shiftOut(0x00, 0x6C);
 shiftOut(0x00, 0x78);
 vh = shiftOut(0x00, 0x7C);
 UsartWrite(' ');
 HexPrint(vl);
 UsartWrite(' ');
 HexPrint(vh);
 addr++;
 }
UsartPrint("\r\n");
}

void Erase () {
shiftOut(0x80, 0x4c);
shiftOut(0x00, 0x64);
shiftOut(0x00, 0x6c);
shiftOut(0x00, 0x4c);
UsartPrintPGM(PSTR("Program Memory is erased!!!\r\n"));
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

void Write () {
word w;
byte c;
UsartPrintPGM(PSTR("Start programming...\r\n"));
shiftOut(0x10, 0x4c);
 do {
 ReadLine();
 if(hl[8]=='1') goto pend;
  if(hl[8]=='0') {
  addr = SetWord(&hl[3])>>1;
  UsartPrint(": ");
  c = SetByte(&hl[1])>>1;
   for(byte i=0;i<c;i++) {
   w = SetWord(&hl[i*4+9]);
   shiftOut(addr, 0x0c);
   shiftOut(w>>8, 0x2c);
   shiftOut(w,    0x3c);
   shiftOut(0x00, 0x7d);
   shiftOut(0x00, 0x7c);
   addr++;
   }
  UsartPrint("\r\n");
   if((addr&0x0f) == 0) {
   pend:
   shiftOut(addr>>8, 0x1c);
   shiftOut(0x00, 0x64);
   shiftOut(0x00, 0x6c);
   }
  }
 } while (hl[8]!='1');
shiftOut(0x00, 0x4c);
UsartPrintPGM(PSTR("Done.\r\n"));
}

void Fuse () {
word fs = LFUSE;
byte v1;
byte v2;
UsartPrintPGM(PSTR("Select Fuse: l,h,e:\r\n"));
 switch(UsartRead(0)) {
 case 'l' :
 fs = LFUSE;
 UsartPrintPGM(PSTR("LFUSE="));
 break;
 case 'h' :
 fs = HFUSE;
 UsartPrintPGM(PSTR("HFUSE="));
 break;
 case 'e' :
 fs = EFUSE;
 UsartPrintPGM(PSTR("EFUSE="));
 break;
 }
v1 = UsartRead(0);
UsartWrite(v1);
if (v1 > 0x40) v1 -= 7;
v2 = UsartRead(0);
UsartWrite(v2);
UsartPrint("\r\n");
if (v2 > 0x40) v2 -= 7;
writeFuse(fs,(v1<<4)|(v2&0x0f));
PrintFuses();
}

void Power() {
addr = PORTB = DDRB = DDRC = PORTC = 0;
 for(;;) {
 UsartPrintPGM(PSTR("Send s to start\r\n"));
  if (UsartRead(0)=='s') {
  DDRC = 0x0f;
  PORTB = 0x30;
  DDRB = 0x38;
  delay(10);
  DDRC = 0x0b;
  PrintSignature();
  PrintCalibration();
  PrintFuses();
  UsartPrintPGM(PSTR("Supported commands: f,q,e,r,d\r\n"));
  return;
  }
 }
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
 Read();
 break;

 case 'd' :
 addr = 0;
 for(byte k=0;k<64;k++) Read();
 break;

 case 'p' :
 Write();
 break;

 case 'f' :
 Fuse();
 break;

 case 'q' :
 UsartPrintPGM(PSTR("Power OFF\r\n^"));
 Power();
 break;
 }

goto loop;
}
