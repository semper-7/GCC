#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "OneWire.h"
#define DC (F_CPU/4000000UL)
#define CONCAT__(FIRST, SECOND) FIRST ## SECOND
#define CONCAT(FIRST, SECOND) CONCAT__(FIRST, SECOND)
#define OW_PIN CONCAT(PIN, PORT_LETTER)
#define OW_DDR CONCAT(DDR, PORT_LETTER)
#define OW_PORT CONCAT(PORT, PORT_LETTER)

OneWire::OneWire(uint8_t m) {
mask=m;
}

uint8_t OneWire::reset() {
uint8_t m = mask;
OW_DDR &= ~m;
OW_PORT &= ~m;
_delay_loop_2(10*DC);
while (~OW_PIN & m);
OW_DDR |= m;
_delay_loop_2(480*DC);
cli();
OW_DDR &= ~m;
_delay_loop_2(70*DC);
uint8_t r = (~OW_PIN & m);
sei();
_delay_loop_2(400*DC);
return r;
}

void OneWire::write_bit(uint8_t v) {
uint8_t m = mask;
cli();
OW_PORT &= ~m;
OW_DDR |= m;	// drive output low
_delay_loop_2(10*DC);
 if (v) {
 OW_PORT |= m;	// drive output high
 _delay_loop_2(50*DC);
 } else {
 _delay_loop_2(55*DC);
 OW_PORT |= m;	// drive output high
 }
_delay_loop_2(5*DC);
sei();
}

void OneWire::write(uint8_t v) {
for (uint8_t b = 1; b; b <<= 1) write_bit(v & b);
}

void OneWire::write_bytes(const uint8_t *buf, uint16_t count) {
for (uint16_t i = 0; i < count; i++) write(buf[i]);
}

void OneWire::select(const uint8_t *rom) {
write(0x55);
write_bytes(rom,8);
}

uint8_t OneWire::read_bit() {
uint8_t m = mask;
cli();
OW_PORT &= ~m;
OW_DDR |= m;
_delay_loop_2(3*DC);
OW_DDR &= ~m;
_delay_loop_2(10*DC);
uint8_t r = OW_PIN &= m;
sei();
_delay_loop_2(53*DC);
return r;
}

uint8_t OneWire::read() {
uint8_t r = 0;
for (uint8_t Mask = 1; Mask; Mask <<= 1) if (read_bit()) r |= Mask;
return r;
}

void OneWire::read_bytes(uint8_t *buf, uint16_t count) {
for (uint16_t i = 0; i < count; i++) buf[i] = read();
}

static const uint8_t PROGMEM crc_tab[] = {
  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

uint8_t OneWire::crc8(const uint8_t *addr, uint8_t len) {
uint8_t crc = 0;
while (len--) crc = pgm_read_byte(crc_tab + (crc ^ *addr++));
return crc;
}
