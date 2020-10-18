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

bool inet = 0;
bool nmcu = 0;
char host[] = "yfb7905i.bget.ru";
char var[] = "53.75+52.50+50.88+31.94+30.88+33.56+25.00+06.12+24.31+0";
byte ipaddr[4] = {192,168,1,111};
byte ipgw[4] = {192,168,1,1};
byte ipdns[4] = {212,94,96,123};
byte ipds[4] = {192,168,1,9};

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
UsartPrint(off,len);
nmcu = 1;
}

void DelayFunc(void) {
PacketFunc();
cli();
word msp = msx ^ ms;
msx = ms;
sei();
if(msp&0x0400) LinkFunc();//1024ms
}

int main (void) {
UsartInit(115200);
sei();
UsartPrintPGM(PSTR("\r\nNet started\r\n"));
UsartPrintPGM(PSTR("^~~123456789\r\n"));
if (!ENC28J60_Init()) UsartPrintPGM(PSTR("ENC28J60 not found!\r\n"));
DelayInit(DelayFunc);
delay(3000);

loop:
UsartPrintPGM(PSTR("\r\nLoop:\r\n"));
tcp_connect(ipds,99);
delay(1000);
nmcu = 0;
tcp_connect(host);
delay(5000);
inet = 0;
goto loop;
}
