#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "usart.h"

#define ULB 64 //Usart len buffer 16, 32, 64.

char RXbuf[ULB];
volatile uint8_t RXin = 0;
volatile uint8_t RXout = 0;

void UsartInit(unsigned long b) {
UBRR0L = (F_CPU / 8 / b - 1) / 2;
UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
}

void UsartWrite(char c) {
while(!( UCSR0A & (1 << UDRE0)));
UDR0 = c;
}

void UsartPrint(const char *s) {
char c;
while (c = *s++, c!=0) UsartWrite(c);
}

void UsartPrint(const char *s, uint16_t len) {
while (len--) UsartWrite(*s++);
}

void UsartPrintPGM(const char *s) {
char c;
while (c = pgm_read_byte(s++), c!=0) UsartWrite(c);
}

char UsartRead(bool nowait) {
if (nowait) return (RXin - RXout) & (ULB-1);
while(RXin == RXout);
char c = RXbuf[RXout];
RXout = (RXout + 1) & (ULB-1);
return c;
}

ISR(USART_RX_vect) {
RXbuf[RXin] = UDR0;
RXin = (RXin + 1) & (ULB-1);
}
