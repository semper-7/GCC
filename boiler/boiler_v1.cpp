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

OneWire ds (1<<DSPIN);
typedef const uint8_t DeviceAddress[8];
DeviceAddress rom[9] = { 
{0x28,0x13,0x39,0x0E,0x06,0x00,0x00,0xAF},
{0x28,0x8A,0xB8,0x0C,0x06,0x00,0x00,0x89},
{0x28,0xA3,0x98,0x0D,0x06,0x00,0x00,0xDE},
{0x28,0xA7,0xC6,0x0C,0x06,0x00,0x00,0xBA},
{0x28,0x9F,0xA9,0x0E,0x06,0x00,0x00,0x8F},
{0x28,0xB2,0xEE,0xA9,0x04,0x00,0x00,0xAB},
{0x28,0xF3,0x3D,0xA9,0x04,0x00,0x00,0xF9},
{0x28,0x61,0x64,0x11,0x8D,0x9E,0xD7,0xEA},
{0x28,0x61,0x64,0x11,0x8D,0x8C,0xFF,0x76},
};

byte bufData[9];
int temp[9];
byte tx = 0;
byte flkey = 0;
char ind[] = "^~~0000 0000\r\n";
const char dt[] PROGMEM = "00061219253138445056626975818894";
byte cfg[] = {24,32,42,46,50,92};
const char http_get[] PROGMEM = "GET /log.php?var=";
char var[56];
const char http_ver[] PROGMEM = " HTTP/1.0\r\nHost: ";
const char http_host[] PROGMEM = "yfb7905i.bget.ru";
const char http_add[] PROGMEM = "\r\nUser-Agent: Mozilla/5.0\r\n\r\n";
#define LEN_HTTP (sizeof http_get + sizeof var + sizeof http_ver + sizeof http_host + sizeof http_add - 5)
static void http_callback (char*,word);
http_str http = {http_get,var,http_ver,http_host,http_add, LEN_HTTP, http_callback};

void indnt() {
ind[3] = tx|'0';
ind[4] = ' ';
ind[5] = ' ';
ind[6] = ' ';
}

void indt() {
ind[7] = ' ';
if (var[54] >= '4' )ind[7] = 0x7f;
if (var[54] == '8' )ind[7] = 0x7a;
byte j = tx * 6;
ind[8] = var[j];
ind[9] = var[j+1]&0x1f;
ind[10] = var[j+3];
ind[11] = var[j+4];
}

void keyfunc() {
flkey++;
if (PINC & (1<<KEY)) flkey=0;
 if( (UsartRead(1) && UsartRead(0) == 0x0d) || (flkey>16 && flkey<32 ) ) {
 flkey=32;
 tx++;
 if(tx>8) tx=0;
 indnt();
 indt();
 UsartPrint(ind);
 }
}

static void http_callback (char* off, word len) {
 if (len == 200) {
 off=off+200-22;
 UsartPrint("CFG: ");
 UsartPrint(off,22);
 UsartPrint("\r\n");
 for(byte i=0;i<6;i++) cfg[i] = (*(off+i+i+i+5)&15)*10 + (*(off+i+i+i+6)&15);
 memcpy(ind+3,off,4);
 ind[4]|=0xc0;
 UsartPrint(ind);
 }
}

int main (void) {
timer0init();
UsartInit(115200);
DDRC = 3;
PORTC = 8;
sei();
UsartPrintPGM(PSTR("\r\nBoiler-auto started\r\n"));
if (!ENC28J60_Init()) UsartPrintPGM(PSTR("ENC28J60 not found!\r\n"));
if (!ds.reset()) UsartPrintPGM(PSTR("No 1WIRE sensors!\r\n"));
delay(500);
SetDelayFunc(PacketLoop);
SetDelayFunc(keyfunc,1);
SetDelayFunc(LinkLoop,11);

 for(;;) {
 ds.reset();
 ds.write(0xCC);
 ds.write(0x44);
 delay(1000);
  for (int i=0, j=0; i<9; i++, j+=6) {
  ds.reset();
  ds.select(rom[i]);
  ds.write(0xBE);
  ds.read_bytes(bufData, 9);
  if ( (bufData[0] | bufData[1]) != 0 &&  OneWire::crc8(bufData, 8) == bufData[8] ) temp[i] = int(bufData[0]) | int(bufData[1] << 8);
   if (temp[i]>=0) {
   var[j] = ((temp[i] >> 4)/10) | '0';
   if (var[j]>'9') var[j] -= 10;
   var[j+1] = ((temp[i] >> 4)%10) | '0';
   } else {
   temp[i] = -temp[i];
   var[j] = '-';
   var[j+1] = ((temp[i] >> 4) & 15) | '0';
   }
  var[j+2] = '.';
  *((int*)(&var[j+3])) = pgm_read_word(&dt[(temp[i] & 15)<<1]);
  var[j+5] = '+';
  }
 byte B = byte(temp[0] >> 4);
 byte O = byte(temp[5] >> 4);
 byte H = byte(temp[6] >> 4);
 byte F = byte(temp[8] >> 4);
 bool FI = 0;
 bool PO = 0;
 bool PB = 0;
 bool AL = B >= cfg[5];
  if (AL) UsartPrintPGM(PSTR("OVERHEATING!!!\r\n"));
  else {
  FI = F >= cfg[4] || (F >= cfg[1] && B < cfg[4]);
  PO = H < B && H < cfg[0] && (!FI || O >= cfg[2]);
  PB = FI && O < cfg[4] && (!PO || O < cfg[3]) && (H < cfg[0] || O < cfg[1]);
  }
 PORTC = (PORTC & 0xfc) | ((PO << 1)|PB);
 var[54] = (AL << 3)|(FI << 2)|(PO << 1)|PB|'0';
 var[55] = 0;
 UsartPrint(var);
 UsartPrint("\r\n");
 indt();
 UsartPrint(ind);
 BrowseUrl();
 delay(30000);
 if (var[54]<'4') delay (120000);
 }
}
