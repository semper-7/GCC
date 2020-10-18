#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include "../lib/delay.h"
#include "../lib/usart.h"
#include "../lib/OneWire.h"
#include "../lib/enc28j60.h"
#include "../lib/network.h"

#define DSPIN	2//PC2
#define KEY	3//PC3

int main (void); 
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
extern word ms;
word msx;

OneWire ds (1<<DSPIN);
typedef const byte DeviceAddress[8];
DeviceAddress rom[8] = { 
{0x28,0x13,0x39,0x0E,0x06,0x00,0x00,0xAF},
{0x28,0x8A,0xB8,0x0C,0x06,0x00,0x00,0x89},
{0x28,0xA3,0x98,0x0D,0x06,0x00,0x00,0xDE},
{0x28,0xA7,0xC6,0x0C,0x06,0x00,0x00,0xBA},
{0x28,0x9F,0xA9,0x0E,0x06,0x00,0x00,0x8F},
{0x28,0xB2,0xEE,0xA9,0x04,0x00,0x00,0xAB},
{0x28,0xF3,0x3D,0xA9,0x04,0x00,0x00,0xF9},
{0x28,0x61,0x64,0x11,0x8D,0x8C,0xFF,0x76},
};

byte bufData[9];
int temp[9];
byte tx = 0;
bool flkey = 0;
bool inet;
bool nmcu;
char ind[] = "^~~123456789\r\n";
char terr[] = "--.--+";
const char dt[] PROGMEM = "00061219253138445056626975818894";
byte cfg[] = {25,32,42,46,50,96};
char var[56];
char host[] = "yfb7905i.bget.ru";
byte ipaddr[4] = {192,168,1,111};
byte ipgw[4] = {192,168,1,1};
byte ipdns[4] = {212,94,96,123};
byte ipds[4] = {192,168,1,9};

void test() {
DelayInit();
UsartPrintPGM(PSTR("\r\nTest-mode started\r\n"));
UsartPrint(ind);
byte p = 4;
 for(;;) {
 p++;
 if(p>7) p=4;
 PORTC = p;
 delay(2000);
 }
}

void indt() {
ind[3] = tx|'0';
ind[4] = ' ';
ind[5] = ' ';
ind[6] = ' ';
ind[7] = ' ';
if (var[54] >= '4' )ind[7] = 0x7f;
if (var[54] == '8' )ind[7] = 0x7a;
byte j = tx * 6;
if (var[j] == '-') ind[8] = 0x27;
else ind[8] = var[j];
if (var[j+1] == '-') ind[9] = 0x27;
else ind[9] = var[j+1]&0x1f;
if (var[j+3] == '-') ind[10] = 0x27;
else ind[10] = var[j+3];
if (var[j+4] == '-') ind[11] = 0x27;
else ind[11] = var[j+4];
}

word tcp_send(char *off) {
 if (*(off-TCP_OPTIONS+TCP_DST_PORT+1)==80) {
 strcpy_P(off,PSTR("GET /log.php?var="));
 strcat  (off,var);
 strcat_P(off,PSTR(" HTTP/1.0\r\nHost: "));
 strcat (off,host);
 strcat_P(off,PSTR("\r\nUser-Agent: Mozilla/5.0\r\n\r\n"));
 return strlen(off);
 }
return 0;
}

void tcp_receive(char *off, word len) {
 if (len == 200) {
 off=off+200-22;
 UsartPrint("CFG: ");
 UsartPrint(off,22);
 UsartPrint("\r\n");
 for(byte i=0;i<6;i++) cfg[i] = (*(off+i+i+i+5)&15)*10 + (*(off+i+i+i+6)&15);
 memcpy(ind+3,off,4);
 ind[4]|=0xc0;
 UsartPrint(ind);
 inet = 1;
 }
 else {
 UsartPrint(off,len);
 memcpy(&var[48], off+24, 5);
 nmcu = 1;
 }
}

void KeyFunc() {
if(PINC & (1<<KEY)) flkey = 0;
else
 if(!flkey) {
 flkey = 1;
 tx++;
 if(tx>8) tx=0;
 indt();
 UsartPrint(ind);
 }
}

void DelayFunc(void) {
PacketFunc();
cli();
word msp = msx ^ ms;
msx = ms;
sei();
if(msp&0x0008) KeyFunc(); //8ms
if(msp&0x0200) LinkFunc();//1024ms
}

int main (void) {
UsartInit(115200);
DDRC = 3;
PORTC = (1<<KEY);
sei();
UsartPrintPGM(PSTR("\r\nBoiler-auto started\r\n"));
if (!(PINC & (1<<KEY))) test();
if (!ENC28J60_Init()) UsartPrintPGM(PSTR("ENC28J60 not found!\r\n"));
DelayInit(DelayFunc);
delay(1100);

loop:
memcpy(&var[48], terr, 6);
nmcu = 0;
tcp_connect(ipds,99);
if (!ds.reset()) UsartPrintPGM(PSTR("No 1WIRE sensors!\r\n"));
ds.write(0xCC);
ds.write(0x44);
delay(1100);
 for (int i=0, j=0; i<8; i++, j+=6) {
 ds.reset();
 ds.select(rom[i]);
 ds.write(0xBE);
 ds.read_bytes(bufData,9);
 memcpy(&var[j], terr, 6);
  if (OneWire::crc8(bufData,8) == bufData[8]) {
  temp[i] = int(bufData[0]) | int(bufData[1] << 8);
   if (temp[i]>=0) {
   var[j] = ((temp[i] >> 4)/10) | '0';
   if (var[j]>'9') var[j] -= 10;
   var[j+1] = ((temp[i] >> 4)%10) | '0';
   } else {
   temp[i] = -temp[i];
   var[j+1] = ((temp[i] >> 4) & 15) | '0';
   }
  *((int*)(&var[j+3])) = pgm_read_word(&dt[(temp[i] & 15)<<1]);
  }
 }
byte B = byte(temp[0] >> 4);
byte O = byte(temp[5] >> 4);
byte H = byte(temp[6] >> 4);
byte F = byte(temp[7] >> 4);
bool FI = 0;
bool PO = 0;
bool PB = 0;
bool AL = B >= cfg[5];
 if (AL) UsartPrintPGM(PSTR("OVERHEATING!!!\r\n"));
 else {
 FI = F >= cfg[4] || (F >= cfg[1] && B < cfg[4]);
 PO = H < B && H < cfg[0] && (!FI || O >= cfg[2]);
 PB = FI && O < cfg[4] && (!PO || O < cfg[3]) && (H < cfg[0] || O < cfg[0]);
 }
PORTC = (1<<KEY)|((PO << 1)|PB);
var[54] = (AL << 3)|(FI << 2)|(PO << 1)|PB|'0';
var[55] = 0;
indt();
UsartPrint(var);
UsartPrint("\r\n");
inet = 0;
 for(byte i=0;i<12;i++) {
 if (!inet && (nmcu || i>2)) tcp_connect(host);
 delay(2500);
 if (i==0 && !inet) UsartPrint(ind);
 if (!nmcu && i<2) tcp_connect(ipds,99);
 }
if (var[54]<'4') delay (120000);
goto loop;
}
