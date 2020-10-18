#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#define DC (4000000UL/F_CPU)


int main (void); 

typedef uint8_t byte;
typedef uint16_t word;


word LenImp() {
word i=0;
while ((PINB & 0x10) != 0);
while ((PINB & 0x10) == 0) i++;
return i;
}

void SendPresense() {
PORTB&=0xef;
DDRB|=0x10;
DDRB|=0x10;
DDRB|=0x10;
DDRB|=0x10;
DDRB|=0x10;
DDRB|=0x10;
DDRB|=0x10;
DDRB&=0xef;
} 

unsigned int crc8(unsigned int seed, unsigned int inData)//проверка CRC.
//пример: c=00; crc=0; for(c=0; c<7; c++) crc=crc8(crc,новый байт из массива);//
{ 
  //Процедура не моя. Я даже не знаю чья!!! Спасибо автору!!!
unsigned int bitsLeft=0;
unsigned int tempp=0;

	for (bitsLeft=8;bitsLeft>0;bitsLeft--)
	{
	tempp=((seed^inData)&0x01);
	if(tempp==0) seed >>= 1;
        	else
		{
		 seed ^=0x18;
		 seed >>=1;
		 seed |=0x80;
		}
		inData >>= 1;
	}
	return seed;
}



int main (void) {
ACSR = 1<<ACD;
cli();

 for(;;) {
 if (LenImp() < 7) break;
 SendPresense();
 }
}
